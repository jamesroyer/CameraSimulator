#include <iostream>
#include <plugin/IPlugin.h>
#include <plugin/Heater/PluginHeater.h>
#include <plugin/Heater/HeaterTask.h>

extern "C" const char* name() { return "PluginHeater"; }
extern "C" const char* version() { return "0.0.1"; }

class PluginHeater : public IPlugin
{
public:
	PluginHeater(Callbacks* cb)
		: IPlugin(cb)
		, m_heater(cb->mqttClient, 1, false)
	{
		m_cb->pLogger->trace("{}::{}()", name(), name());
	}

	~PluginHeater() override
	{
		m_cb->pLogger->trace("{}::~{}()", name(), name());
		m_heater.Stop();
	}

	void Run() override
	{
		m_cb->pLogger->trace("{}::Run()", name());
		m_heater.Start();
	}

private:
	ncc::HeaterTask m_heater;
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
			std::cerr << name() << ": create(): invalid parameter" << std::endl;
			return nullptr;
		}

		cb->pLogger->trace("lib{}.so: create()", name());

		plugin = new PluginHeater(cb);
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
