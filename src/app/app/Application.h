#pragma once

#include <app/Base.h>
#include <core/IMqttClient.h>
#include <core/IMqttSubscriber.h>

#include <map>
#include <vector>

#define NCURSES_NOMACROS
#include <panel.h>

namespace ncc
{

class Application : public IMqttSubscriber
{
public:
	Application(IMqttClient& mqttClient);
	~Application();
	void Run();

	void OnConnect(int rc) override;
	void OnDisconnect(int rc) override;
	void OnMessage(const std::string& topic, const nlohmann::json& json) override;

private:
	void InitWindows_();

	std::vector<std::string> ProcessMessage_(const std::string& msg);
	void SetActiveWindow_(const std::string& name);
	void Resize_();
	void ClearStatusLine_();
	void ShowStatusLine_();

	void AddSubscriptions_();
	void OnMessage_(const std::string& topic, const nlohmann::json&);
	void OnCompMessage_(const std::vector<std::string>& msg);
	void SendCompMessage_(const std::vector<std::string>& msg);

private:
	IMqttClient& m_mqtt;

	std::map<std::string, Base*> m_wins;
	Base* m_activeWin {nullptr};
	int m_statusX {-1};
};

} // namespace ncc
