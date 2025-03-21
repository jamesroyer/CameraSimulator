#include <plugin/Heater/HeaterTask.h>
#include <core/IMqttClient.h>

#include <nlohmann/json.hpp>

namespace ncc
{

HeaterTask::HeaterTask(
		IMqttClient& mqttClient,
		int heaterNum,
		bool autostart)
	: BaseThread("HeaterTask", autostart)
	, m_mqtt(mqttClient)
	, m_heaterNum(heaterNum)
{
	m_mqtt.RegisterSub("/temperature-monitor/temperature", this);
#if 0
	mqtt..Add(
		"/temperature-monitor/temperature",
		[this](const std::string& topic, const nlohmann::json& json) {
			OnTempUpdate_(topic, json);
		});
#endif
}

void HeaterTask::Run_()
{
	m_running = true;
	m_heaterOn = false;
	for (;;)
	{
		std::unique_lock lock(m_mutex);
		if (!m_running)
		{
			break;
		}
		m_cv.wait(lock);
	}
}

void HeaterTask::OnConnect(int rc)
{
	// noop - Wait for message.
}

void HeaterTask::OnDisconnect(int rc)
{
	// noop - Wait for message.
}

void HeaterTask::OnMessage(const std::string& topic, const nlohmann::json& json)
{
	constexpr float nominalTemp = 10.0;
	float currentTemp = json["temperature"];

	// If the heater state changes, publish the current state.
	// If the heater state hasn't changed, publish it every X cycles. This
	// will generate less noise but still inform late subscribers.
	// TODO: Fix refreshCount as it depends on receiving messages. It should
	// rely on a timer instead.
	constexpr int refreshCount {3};
	if (m_threshold == 0 || m_lastPublishedState != m_heaterOn)
	{
		m_lastPublishedState = m_heaterOn;

		PublishHeater_(m_heaterOn);

		m_threshold = 0;
	}
	m_threshold = ++m_threshold % refreshCount;

//	logger()->debug("currentTemp={}, heater={}", currentTemp, (m_heaterOn ? "ON" : "OFF"));
	if (currentTemp < nominalTemp)
	{
		if (!m_heaterOn)
		{
			m_heaterOn = true;
			PublishHeater_(m_heaterOn);
		}
	}
	else if (currentTemp >= nominalTemp)
	{
		if (m_heaterOn)
		{
			m_heaterOn = false;
			PublishHeater_(m_heaterOn);
		}
	}
}

void HeaterTask::PublishHeater_(bool enabled)
{
	std::string topic = std::string("/heater/") + std::to_string(m_heaterNum);

	std::string heaterJson =
		"{\"heater\":" + std::to_string(m_heaterNum) + "," +
		"\"enabled\":" + (enabled ? "true" : "false") + "}";
	auto json = nlohmann::json::parse(heaterJson);

	m_mqtt.Publish(topic, json);
}

} // namespace ncc
