#pragma once

#include <core/Counter.h>
#include <core/IMqttSubscriber.h>
#include <core/IMqttClient.h>

#include <condition_variable>
#include <set>
#include <string>

#include <mosquitto.h>
#include <nlohmann/json.hpp>

using namespace std::chrono_literals;

namespace ncc
{

class MqttClient
	: private Counter<MqttClient>	// Initialize mosquitto library only once
	, public IMqttClient
{
public:
	MqttClient(
		const std::string& name,
		const std::string& host = "localhost",
		int port = 1883);

	~MqttClient();

	void RegisterSub(const std::string& topic, IMqttSubscriber* sub) override;
	void UnregisterSub(IMqttSubscriber* sub) override;
	bool IsConnected() const override;

	bool IsTopicMatch(const std::string& sub, const std::string& topic) override;

	bool Publish(
		const std::string& topic,
		const nlohmann::json& json,
		int qos = 0, // QoS: 0 => at most once, 1 => at least once, 2 => exactly once
		bool retain = false,
		const std::chrono::duration<long, std::ratio<1, 1>>& delay = 5s) override;

public:
	static void OnConnect(mosquitto*, void* obj, int rc);
	static void OnDisconnect(mosquitto*, void* obj, int rc);
	static void OnSubscribe(mosquitto*, void* obj, int mid, int qosCount, const int* grantedQos);
	static void OnUnsubscribe(mosquitto*, void* obj, int mid);
	static void OnPublish(mosquitto*, void* obj, int mid);
	static void OnMessage(mosquitto*, void* obj, const mosquitto_message* msg);
	static void OnLog(mosquitto*, void* obj, int level, const char* str);

private:
	void Connect_();

	// TODO: Support connection disconnections and reconnections
	void OnConnect_(int rc);
	void OnDisconnect_(int rc);

	void OnSubscribe_(int mid, int qosCount, const int* grantedQos);
	void OnUnsubscribe_(int mid);
	void OnPublish_(int mid);
	void Subscribe_();
	void Subscribe_(const std::string& topic, int qos = 0);
	void Unsubscribe_(const std::string& topic);
	void OnMessage_(const mosquitto_message* msg);
	void OnLog_(int level, const char* str);

private:
	// May want to use our own thread to service mosquitto messages.
//	void Run_();

	// Uses Counter class to call moquitto library setup and cleanup functions
	// only once.
	void Setup_();
	void Cleanup_();

private:
	mosquitto* m_mosq {nullptr};

	const std::string m_name;
	const std::string m_host;
	int m_port {0};

	std::mutex m_mutex;
	std::condition_variable m_cv;
	bool m_connected {false}; // TODO: add support for connection reconnections

	std::set<IMqttSubscriber*> m_subs;
	std::map<std::string, std::vector<IMqttSubscriber*>> m_topicSubs;
};

} // namespace ncc
