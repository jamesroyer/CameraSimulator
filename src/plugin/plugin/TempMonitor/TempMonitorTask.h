#pragma once

#include <core/BaseThread.h>
#include <core/IMqttClient.h>
#include <core/IMqttSubscriber.h>

namespace ncc
{

static float CompareAlmostEqual(float x, float y)
{
	// https://stackoverflow.com/a/2411661
	// May need to increase epsilon if truncation errors can occur.
	static float epsilon = std::max(std::abs(x), std::abs(y)) * 1e-15;
	return std::abs(x - y) <= epsilon;
}

// Temperature Monitor continuously polls the system to retrieve the
// temperature and then publishes the temperature.
//
// Temperature Range:
// - Start at -20C
// - Turning on one heater will increase temperature by 1C at a time.
// - Turning on two heaters will increase temperature by 2C at a time.
// - If heaters do not turn off, temperature will increase to 80C.
// - If heaters are off, temperature will decrease from 80C to 40C.
//
// NOTE: Only need one heater topic, but two is easier (for now).
//
// Publishes:
//    Topic: /temperature-monitor/temperature, JSON: {"temperature": -10.0}
// Subscribes:
//    Topic: /heater/On/#, JSON: {"heater": <N>, "enabled": true}
//    Topic: /heater/Off/#, JSON: {"heater": <N>, "enabled": false}
//
// Note:
// - Subscription is needed to simulate temperature increasing when heater is
//   turned on.
class TempMonitorTask : public BaseThread, public IMqttSubscriber
{
public:
	explicit TempMonitorTask(
		IMqttClient& mqttClient,
		bool autostart = true);
	~TempMonitorTask() override = default;

	void OnConnect(int rc) override;
	void OnDisconnect(int rc) override;
	void OnMessage(const std::string& topic, const nlohmann::json& json) override;

private:
	void Run_() override;

	void PublishTemperature_();

private:
	IMqttClient& m_mqtt;
	int m_numHeaters {0};
	float m_currentTemp {0.0};
	float m_lastPublishedTemp {0.0};
	int m_threshold {0};
};

} // namespace ncc
