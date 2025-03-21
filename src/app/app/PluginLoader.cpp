#include <app/PluginLoader.h>
#include <core/Logger.h>

#include <algorithm>

#include <dlfcn.h>

// Plugins returned by Create() must be deleted before calling RemovePlugin().

namespace ncc
{

bool PluginFactory::Add(const std::string& name, const std::string& path)
{
	if (m_plugins.find(name) != m_plugins.end())
	{
		ncc::logger()->debug("PluginFactory::AddFactory(): {} already exists", name);
		// XXX: Should this return true (already exists - no reason to fail)
		// or return false (inform user something is wrong with configuration)?
		return false;
	}
	Plugin plugin;
	if (!PluginLoad_(plugin, name, path))
	{
		ncc::logger()->debug("PluginFactory::AddFactory(): {} Plugin::Load() failed", name);
		return false;
	}

	m_plugins.insert({name, plugin});
	return true;
}

bool PluginFactory::Remove(const std::string& name)
{
	return m_plugins.erase(name) == 1;
}

IPlugin* PluginFactory::Create(const std::string& name, Callbacks* cb)
{
	ncc::logger()->trace("PluginFactory::Create()");

	IPlugin* ptr {nullptr};

	try
	{
		auto it = m_plugins.find(name);
		if (it == m_plugins.end())
		{
			return {};
		}

//		const char* dlerr = dlerror();
//		std::string de { dlerr ? dlerr : "<none>" };

//		Plugin& plugin {it->second};
		void* p = it->second.createPlugin(cb);
		ptr = reinterpret_cast<IPlugin*>(p);
		PluginAddObj_(it->second, ptr);
	}
	catch (const std::exception& e)
	{
		ncc::logger()->error("PluginFactory::Create(): Caught={}", e.what());
	}

	return ptr;
}

void PluginFactory::Destroy(const std::string& name, IPlugin* ptr)
{
	auto it = m_plugins.find(name);
	if (it != m_plugins.end())
	{
		Plugin& plugin {it->second};
		plugin.destroyPlugin(ptr);
		PluginRemoveObj_(plugin, ptr);
	}
}

bool PluginFactory::PluginAddObj_(Plugin& plugin, IPlugin* ptr)
{
	if (ptr)
	{
		plugin.objs.push_back(ptr);
		return true;
	}
	return false;
}

bool PluginFactory::PluginRemoveObj_(Plugin& plugin, IPlugin* ptr)
{
	auto it = std::find_if(plugin.objs.begin(), plugin.objs.end(), [ptr](IPlugin* p) { return p == ptr; });
	return plugin.objs.erase(it) != plugin.objs.end(); 
}

bool PluginFactory::PluginLoad_(Plugin& plugin, const std::string& pluginName, const std::string& filePath)
{
	plugin.name = pluginName;

	// Investigate adding flags (see Qt QLoadLibrary implementation).
	// There are compiler flags that expose the executable's objects to the
	// shared libraries that are loaded.
	plugin.handle = dlopen(filePath.c_str(), RTLD_LAZY);
	if (!plugin.handle)
	{
		ncc::logger()->error("Failed to load '{}' library: {}", filePath, dlerror());
		return false;
	}

	dlerror(); // reset errors.

	plugin.createPlugin = (CreatePluginFn*)(dlsym(plugin.handle, "create"));
	if (!plugin.createPlugin)
	{
		ncc::logger()->error("Failed to find 'create()' in '{}' library: {}", filePath, dlerror());
		dlclose(plugin.handle);
		plugin.handle = nullptr;
		return false;
	}

	dlerror(); // reset errors.

	plugin.destroyPlugin = (DestroyPluginFn*)(dlsym(plugin.handle, "destroy"));
	if (!plugin.destroyPlugin)
	{
		ncc::logger()->error("Failed to find 'destroy()' in '{}' library: {}", filePath, dlerror());
		dlclose(plugin.handle);
		plugin.handle = nullptr;
		plugin.createPlugin = nullptr;
		return false;
	}

	ncc::logger()->debug("Successfully opened library");
	return true;
}

void PluginFactory::PluginCleanup_(Plugin& plugin)
{
	while (!plugin.objs.empty())
	{
		auto ptr = plugin.objs.back();
		plugin.objs.pop_back();
		plugin.destroyPlugin(ptr);
	}
	dlclose(plugin.handle);
}

} // namespace ncc
