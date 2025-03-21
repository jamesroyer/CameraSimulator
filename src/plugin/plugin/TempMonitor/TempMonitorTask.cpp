#include <core/IMqttClient.h>
#include <core/Logger.h>
#include <plugin/TempMonitor/TempMonitorTask.h>

#include <nlohmann/json.hpp>

using namespace std::chrono_literals;

namespace ncc
{

TempMonitorTask::TempMonitorTask(
		IMqttClient& mqttClient,
		bool autostart)
	: BaseThread("TempMonitorTask", autostart)
	, m_mqtt(mqttClient)
{
//	Trace trace("TempMonitorTask::TempMonitorTask()");

	m_mqtt.RegisterSub("/heater/#", this);
#if 0
		[this](const std::string& topic, const nlohmann::json& json) {
			OnHeater_(topic, json);
		});
#endif
}


void TempMonitorTask::OnConnect(int rc)
{
	// Start publishing temperature.
}

void TempMonitorTask::OnDisconnect(int rc)
{
}

void TempMonitorTask::OnMessage(const std::string& topic, const nlohmann::json& json)
{
	// TODO: Verify topic.
	bool enabled = json["enabled"];

	// Topic: /heater/On/#, JSON: {"heater": <N>, "enabled": true}
	logger()->debug("TempMonitorTask::OnMessage(): enabled={}", (enabled ? "ON" : "OFF"));
	if (enabled)
	{
		++m_numHeaters;
	}
	else if (m_numHeaters > 0)
	{
		--m_numHeaters;
	}
}

// In a real system the temperature monitor would be reading the temperature
// from somewhere in the system (i.e. use IOC commands to poll an MCU).
//
// For this simulation, the temperature will start at -20C and max out at 40C.
// If heaters are off, the temperature will slow decrease to a nominal
// temperature of 12C.
void TempMonitorTask::Run_()
{
	m_running = true;
	m_currentTemp = -20.0;
	constexpr float maximumTemp = 40.0;
	constexpr float nominalTemp = 12.0; // 
	auto updateDelay = 1s;

	for (;;)
	{
		std::unique_lock lock(m_mutex);
		if (!m_running)
		{
			break;
		}

#if 0
		static int throttle = 1;
		--throttle;
		if (throttle == 0)
		{
			logger()->debug("TempMonitorTask::Run_(): current temperature={}", m_currentTemp);
			throttle = 5;
		}
#endif
		// Adjust the temperature (simulator relies on heater subscription).
		if (m_numHeaters && m_currentTemp < maximumTemp)
		{
			// Heater is on, increase temperature until maximum reached.
			// Each heater raises temperature by 1C every cycle.
			m_currentTemp += m_numHeaters;
			if (m_currentTemp > maximumTemp)
			{
				m_currentTemp = maximumTemp;
			}
		}
		if (m_numHeaters == 0 && m_currentTemp > nominalTemp)
		{
			// Over heated, returning to nominal temperature.
			--m_currentTemp;
		}

		// Publish temperature every cycle.
		PublishTemperature_();

		// NOTE: Accurate timer is not needed since this is only a simulation.
		// May need to implement something better...
		m_cv.wait_for(lock, updateDelay);
	}
}

void TempMonitorTask::PublishTemperature_()
{
	// If the temperature changes, publish the current temperature.
	// If the temperature hasn't changed, publish it every ten cycles. This
	// will generate less noise but still inform late subscribers.
	constexpr int refreshCount {10};
	if (m_threshold == 0 || !CompareAlmostEqual(m_lastPublishedTemp, m_currentTemp))
	{
		m_lastPublishedTemp = m_currentTemp;

		std::string topic = "/temperature-monitor/temperature";
		std::string tempJson = "{\"temperature\":" + std::to_string(m_currentTemp) + "}";
		auto json = nlohmann::json::parse(tempJson);
		m_mqtt.Publish(topic, json);

		m_threshold = 0;
	}
	m_threshold = ++m_threshold % refreshCount;
}

} // namespace ncc
