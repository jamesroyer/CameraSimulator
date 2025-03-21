#pragma once

#include <string>
#include <nlohmann/json_fwd.hpp>

namespace ncc
{

class IMqttSubscriber
{
public:
	virtual ~IMqttSubscriber() = default;
	virtual void OnConnect(int rc) = 0;
	virtual void OnDisconnect(int rc) = 0;
	virtual void OnMessage(const std::string& topic, const nlohmann::json& json) = 0;
};

} // namespace ncc
