#pragma once

#include <plugin/IPlugin.h>

#include <map>
#include <string>

namespace ncc
{

/**
 * This class creates a mapping from a name to the shared library filename.
 *
 * @note
 * This class currently doesn't keep track of the plugins created by the
 * CreatePlugin() method. If the PluginFactory instance goes out of scope it
 * will unload the shared library without first calling DestroyPlugin() for all
 * plugin instances.
 */
class PluginFactory
{
public:
	bool Add(const std::string& name, const std::string& path);

	bool Remove(const std::string& name);

	/**
	 * Looks up the plugin "name" and if found, calls the create() function
	 * inside the plugin to create a new plugin instance.
	 *
	 * @param name Specify the factory name used in the call to AddFactory().
	 * @param cb Callbacks passed to plugin allowing access to executable variables.
	 */
	IPlugin* Create(const std::string& name, Callbacks* cb);

	/**
	 * Looks up the plugin "name" and if found, calls the destroy() function
	 * inside the plugin to shutdown the plugin and release any allocated resources.
	 *
	 * @param name Specify the factory name used in the call to AddFactory().
	 * @param cb Callbacks passed to plugin allowing access to executable variables.
	 */
	void Destroy(const std::string& name, IPlugin* plugin);

private:
	struct Plugin
	{
		std::string name;
		void* handle {nullptr};
#if 1
		// IPlugin* create(Callbacks*)
		void* (*createPlugin)(void*);
		// void destroy(IPlugin*)
		void (*destroyPlugin)(void*);
#else
		CreatePluginFn* createPlugin {nullptr};
		DestroyPluginFn* destroyPlugin {nullptr};
#endif
		std::vector<IPlugin*> objs;
	};

	bool PluginAddObj_(Plugin& plugin, IPlugin* ptr);
	bool PluginRemoveObj_(Plugin& plugin, IPlugin* ptr);

	bool PluginLoad_(Plugin& plugin, const std::string& pluginName, const std::string& filePath);
	void PluginCleanup_(Plugin& plugin);

private:
	std::map<std::string, Plugin> m_plugins;
};

} // namespace ncc
