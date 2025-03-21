#include <iostream>
#include <plugin/IPlugin.h>
#include <plugin/TempMonitor/PluginTempMonitor.h>
#include <plugin/TempMonitor/TempMonitorTask.h>


extern "C" const char* name() { return "PluginTempMonitor"; }
extern "C" const char* version() { return "0.0.1"; }

class PluginTempMonitor : public IPlugin
{
public:
	PluginTempMonitor(Callbacks* cb)
		: IPlugin(cb)
		, m_tempMonitor(cb->mqttClient, false)
	{
		m_cb->pLogger->trace("{}::{}()", name(), name());
	}

	~PluginTempMonitor() override
	{
		m_cb->pLogger->trace("{}::~{}()", name(), name());
		m_tempMonitor.Stop();
	}

	void Run() override
	{
		m_cb->pLogger->trace("{}::Run()", name());
		m_tempMonitor.Start();
	}

private:
	ncc::TempMonitorTask m_tempMonitor;
};

extern "C"
{

void* create(void* ptr)
{
	IPlugin* plugin {nullptr};

	try
	{
		auto cb = reinterpret_cast<Callbacks*>(ptr);
		if (!cb || !cb->pLogger)
		{
			std::cerr << name() << ": create: invalid parameter" << std::endl;
			return nullptr;
		}

		cb->pLogger->trace("lib{}.so: create()", name());

		plugin = new PluginTempMonitor(cb);
		if (plugin)
			cb->pLogger->info("Successfully instantiated {}.", name());
		else
			cb->pLogger->error("Failed to instantiate {}.", name());
	}
	catch (const std::exception& e)
	{
		std::cerr << "lib" << name() << ": caught: " << e.what() << std::endl;
	}

	return plugin;
}

void destroy(void* ptr)
{
	IPlugin* plugin = reinterpret_cast<IPlugin*>(ptr);
	delete plugin;
}

} // extern "C"
