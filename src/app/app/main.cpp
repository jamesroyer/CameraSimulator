#include <app/Application.h>
#include <app/PluginLoader.h>
#include <core/Logger.h>
#include <core/MqttClient.h>
//#include <plugin/Heater/HeaterTask.h>
//#include <plugin/TempMonitor/TempMonitorTask.h>

#include <filesystem>

#include <iostream>
#include <signal.h>

int g_exit = 0;

void SigIntHandler(int signum)
{
	++g_exit;
	if (g_exit < 3)
	{
		// Continue to catch SIGINT to give app time to shutdown.
		signal(SIGINT, SigIntHandler);
	}
}

IPlugin* LoadPlugin(
	ncc::PluginFactory& pluginFactory,
	Callbacks* cb,
	const std::string& pluginName)
{
	namespace fs = std::filesystem;

	IPlugin* plugin {nullptr};
	int pos {0};
	try
	{
		auto exePath = fs::canonical("/proc/self/exe").parent_path();
		pos = 1;
		auto filePath = exePath.string() + "/lib/lib" + pluginName + ".so";

		pos = 2;
		if (!pluginFactory.Add(pluginName, filePath))
		{
			ncc::logger()->error("Failed to load {} from {}", pluginName, filePath);
		}
		else
		{
			pos = 3;
			plugin = pluginFactory.Create(pluginName, cb);
			if (!plugin)
			{
				ncc::logger()->error("Failed to create {}", pluginName);
			}
		}
		pos = 4;
	}
	catch (const std::exception& e)
	{
		ncc::logger()->error("Loadplugin(pos={}): Caught: {}", pos, e.what());
	}
	return plugin;
}

int main()
{
	signal(SIGINT, SigIntHandler);

	int ret {0};

	try
	{
		ncc::InitializeLogger("camera", false, {"udp", "file"}, spdlog::level::trace);
		ncc::logger()->trace("main()");
		// TODO: Use a better way to manage version number rather than hard-coding it.
		ncc::logger()->info("Camera Simulator v0.0.2");

		const std::string host{"localhost"};
		constexpr int port {1883};
		ncc::MqttClient mqttClient("client", host, port);
		constexpr int cbversion {1};
		Callbacks cb {
			ncc::logger(),
			mqttClient,
			cbversion
		};

		ncc::PluginFactory pluginFactory;

		// TODO: Load and configure plugins from a configuration file.
		// For now, create some here to verify that loading works and MQTT works.

		std::string pluginName {"PluginHeater"};
		auto heaterPlugin = LoadPlugin(pluginFactory, &cb, pluginName);
		if (!heaterPlugin)
		{
			std::cerr << "Failed to load and create " << pluginName << std::endl;
			return 1;
		}

		pluginName = "PluginTempMonitor";
		auto tempMonitorPlugin = LoadPlugin(pluginFactory, &cb, pluginName);
		if (!tempMonitorPlugin)
		{
			std::cerr << "Failed to load and create " << pluginName << std::endl;
			return 1;
		}

		// After all plugins have been loaded, call their Run() method to
		// kick them off.
		//
		// The Plugin's Run() method must not block. They can spawn their own
		// thread (std::thread or std::async), register with a thread pool,
		// passively respond to MQTT requests (using callers thread).
		heaterPlugin->Run();
		tempMonitorPlugin->Run();

		// Now start ncurses interface to visualize what is happening.
		ncc::Application app(mqttClient);
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "main(): caught: " << e.what() << std::endl;
		ret = 1;
	}

	return ret;
}
