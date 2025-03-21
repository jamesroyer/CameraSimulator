#pragma once

#include <core/BaseThread.h>
#include <core/IMqttClient.h>
#include <core/IMqttSubscriber.h>

namespace ncc
{

// There can be zero or more heaters in a camera. Heaters can turn on if there
// is sufficient power. Heaters need to monitor power levels and temperature
// levels to determine if they can/should turn on or off.
//
// If there are multiple heaters, logic would be needed to ensure power is
// shared between the heaters. Heaters have multiple levels of power usage.
//
// NOTE: For now there is no power check and only one heater.
//
// Temperature Range:
// - Heater will turn on if temperature is below 40C.
// - Heater will turn off if temperature is at or above 40C.
//
// NOTE: Only need one heater topic, but two is easier (for now).
//
// Publishes:
//    Topic: /heater/On/#, JSON: {"heater": <N>, "enabled": true}
//    Topic: /heater/Off/#, JSON: {"heater": <N>, "enabled": false}
// Subscribes:
//    Topic: /temperature-monitor/temperature, JSON: {"temperature": -10.0}
//
// Note:
// - Subscription is needed to simulate temperature increasing when heater is
//   turned on.

class HeaterTask : public BaseThread, public IMqttSubscriber
{
public:
	explicit HeaterTask(
		IMqttClient& mqttClient,
		int heaterNum,
		bool autostart = true);
	~HeaterTask() override = default;

	void OnConnect(int rc) override;
	void OnDisconnect(int rc) override;
	void OnMessage(const std::string& topic, const nlohmann::json& json) override;

private:
	void Run_() override;

//	void OnTempUpdate_(const std::string& topic, const nlohmann::json& json);
	void PublishHeater_(bool enabled);

private:
	IMqttClient& m_mqtt;
	int m_heaterNum {0};
	bool m_heaterOn {false};
	bool m_lastPublishedState {false};
	int m_threshold {0};
};

} // namespace ncc
