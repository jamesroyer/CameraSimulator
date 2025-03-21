#pragma once

#include <core/IMqttClient.h>

#include <spdlog/spdlog.h>

struct Callbacks
{
	spdlog::logger* pLogger {nullptr};
	ncc::IMqttClient& mqttClient;
	int version {0};
};

// XXX: What methods does a plugin need to have?
class IPlugin
{
public:
	explicit IPlugin(Callbacks* cb) : m_cb(cb) {}
	virtual ~IPlugin() = default;
	virtual void Run() = 0;

protected:
	Callbacks* m_cb {nullptr};
};

extern "C"
{

//using CreatePluginFn = IPlugin*(Callbacks*);
//using CreatePluginFn = void*(Callbacks*);
using CreatePluginFn = void*(void*);
using DestroyPluginFn = void(void*);

// NOTE: Plugins must implement two functions:
// void* create(Callbacks*) - Need to cast void* to IPlugin*
// void destroy(void* pPlugin)

} // extern "C"
