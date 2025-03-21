#pragma once

#include <core/IMqttSubscriber.h>

#include <chrono>
#include <ratio>
#include <string>

#include <nlohmann/json_fwd.hpp>

using namespace std::chrono_literals;

namespace ncc
{

class IMqttClient
{
public:
	virtual ~IMqttClient() = default;

	// TODO: Add support for std::function
	virtual void RegisterSub(const std::string& topic, IMqttSubscriber* sub) = 0;
	virtual void UnregisterSub(IMqttSubscriber* sub) = 0;
	virtual bool IsConnected() const = 0;

	virtual bool Publish(
		const std::string& topic,
		const nlohmann::json& json,
		int qos = 0, // QoS: 0 => at most once, 1 => at least once, 2 => exactly once
		bool retain = false,
		const std::chrono::duration<long, std::ratio<1, 1>>& delay = 5s) = 0;

	virtual bool IsTopicMatch(const std::string& sub, const std::string& topic) = 0;
};

} // namespace ncc
