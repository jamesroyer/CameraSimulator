#include <core/Logger.h>
#include <core/MqttClient.h>

#include <mosquitto.h>

namespace ncc
{

MqttClient::MqttClient(
		const std::string& name,
		const std::string& host,
		int port)
	: m_name(name)
	, m_host(host)
	, m_port(port)
{
	Setup_();

	constexpr bool cleanSession {true};
	m_mosq = mosquitto_new(name.c_str(), cleanSession, this);

	if (m_mosq)
	{
		mosquitto_connect_callback_set(m_mosq, MqttClient::OnConnect);
		mosquitto_disconnect_callback_set(m_mosq, MqttClient::OnDisconnect);

		mosquitto_subscribe_callback_set(m_mosq, &MqttClient::OnSubscribe);
		mosquitto_unsubscribe_callback_set(m_mosq, &MqttClient::OnUnsubscribe);
//		mosquitto_publish_callback_set(m_mosq, &MqttClient::OnPublish);

		mosquitto_message_callback_set(m_mosq, MqttClient::OnMessage);

		mosquitto_log_callback_set(m_mosq, &MqttClient::OnLog);

		// Creates one thread here and share it with multiple pubs and subs.
		int rc = mosquitto_loop_start(m_mosq);
		if (rc != MOSQ_ERR_SUCCESS)
		{
			logger()->error("mosquitto_loop_start() failed: rc={}", rc);
			mosquitto_disconnect(m_mosq);
			mosquitto_destroy(m_mosq);
			m_mosq = nullptr;
//			Cleanup_(); // XXX: Need to test throwing an exception in ctor to see if this is needed.
			throw std::runtime_error("Failed to start MQTT thread.");
		}

//		rc = mosquitto_connect(m_mosq, m_host.c_str(), m_port, 60);
	}
	Connect_();
}

MqttClient::~MqttClient()
{
	mosquitto_disconnect(m_mosq);
	mosquitto_loop_stop(m_mosq, false);
	mosquitto_destroy(m_mosq);

	Cleanup_();
}

void MqttClient::RegisterSub(const std::string& topic, IMqttSubscriber* sub)
{
	logger()->trace("MqttClient::RegisterSub()");

	m_subs.insert(sub);

	auto topicIt = m_topicSubs.find(topic);
	if (topicIt == m_topicSubs.end())
	{
		auto it = m_topicSubs.insert({topic, {}});
		topicIt = it.first;
	}

	auto& subList = topicIt->second;
	auto subIt = std::find(subList.begin(), subList.end(), sub);
	if (subIt != subList.end())
	{
		return; // Already in list
	}
	subList.push_back(sub);

	// If we are already attached to the broker, send subscription now.
	if (m_connected)
	{
		Subscribe_(topic);
	}
}

void MqttClient::UnregisterSub(IMqttSubscriber* sub)
{
	logger()->trace("MqttClient::UnregisterSub()");

	m_subs.erase(sub);

	// A subscriber may have registered with multiple topics.
	// While loop used to allow iterator to be deleted.
	auto topicIt = m_topicSubs.begin();
	while (topicIt != m_topicSubs.end())
	{
		auto& subList = topicIt->second;
		subList.erase(
			std::remove_if(subList.begin(), subList.end(),
				[sub](IMqttSubscriber* s) { return s == sub; }),
			subList.end());

		if (subList.empty())
		{
			Unsubscribe_(topicIt->first);
			topicIt = m_topicSubs.erase(topicIt);
		}
		else
		{
			++topicIt;
		}
	}
}

void MqttClient::OnConnect(mosquitto*, void* obj, int rc)
{
	if (rc == MOSQ_ERR_SUCCESS)
	{
		logger()->debug("MqttClient::OnConnect(): connection established");
	}
	else
	{
		logger()->error("MqttClient::OnConnect() failed: rc={}", rc);
	}

	auto self = reinterpret_cast<MqttClient*>(obj);
	self->OnConnect_(rc);
}

void MqttClient::OnDisconnect(mosquitto*, void* obj, int rc)
{
	if (rc == MOSQ_ERR_SUCCESS)
	{
		logger()->debug("MqttClient::OnDisconnect(): connection closed");
	}
	else
	{
		logger()->error("MqttClient::OnDisconnect() failed: rc={}", rc);
	}

	auto self = reinterpret_cast<MqttClient*>(obj);
	self->OnDisconnect_(rc);
}

void MqttClient::OnSubscribe(mosquitto*, void* obj, int mid, int qosCount, const int* grantedQos)
{
	logger()->trace("MqttClient::OnSubscribe()");

	auto self = reinterpret_cast<MqttClient*>(obj);
	self->OnSubscribe_(mid, qosCount, grantedQos);
}

void MqttClient::OnUnsubscribe(mosquitto*, void* obj, int mid)
{
	logger()->trace("MqttClient::OnUnsubscribe()");

	auto self = reinterpret_cast<MqttClient*>(obj);
	self->OnUnsubscribe_(mid);
}

void MqttClient::OnPublish(mosquitto*, void* obj, int mid)
{
	logger()->trace("MqttClient::OnPublish(mid={})", mid);

	auto self = reinterpret_cast<MqttClient*>(obj);
	self->OnPublish_(mid);
}

void MqttClient::OnMessage(mosquitto*, void* obj, const mosquitto_message* msg)
{
//	logger()->trace("MqttClient::OnMessage()");

	auto self = reinterpret_cast<MqttClient*>(obj);
	self->OnMessage_(msg);
}

void MqttClient::OnLog(mosquitto*, void* obj, int level, const char* str)
{
#if 0
	logger()->trace("MqttClient::OnLog()");

	auto self = reinterpret_cast<MqttClient*>(obj);
	self->OnLog_(level, str);
#endif
}

bool MqttClient::IsConnected() const
{
	return m_connected;
}

bool MqttClient::Publish(
	const std::string& topic,
	const nlohmann::json& json,
	int qos, // QoS: 0 => at most once, 1 => at least once, 2 => exactly once
	bool retain,
	const std::chrono::duration<long, std::ratio<1, 1>>& delay)
{
//	logger()->trace("MqttClient::Publish(topic=\"{}\")", topic);

	if (!m_connected)
	{
		std::unique_lock lock(m_mutex);

		if (!m_connected)
		{
			logger()->debug("MqttClient::Publish(): waiting to connect...");
			if (!m_cv.wait_for(lock, delay, [this]() {return m_connected; }))
			{
				logger()->debug("MqttClient::Publish() timed out.");
				return false;
			}
		}
	}

//	logger()->debug("MqttClient::Publish(): Sending message...");
	std::string jsonstr = json.dump();
	int rc = mosquitto_publish(m_mosq, nullptr, topic.c_str(), jsonstr.size(), jsonstr.c_str(), qos, retain);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		logger()->error("MqttClient::Publish() failed: rc={}", rc);
	}
	return (rc == MOSQ_ERR_SUCCESS);
}

void MqttClient::Connect_()
{
	constexpr const int keepalive = 60;

	int rc = mosquitto_connect(m_mosq, m_host.c_str(), m_port, keepalive);
	if (rc == MOSQ_ERR_SUCCESS)
	{
		logger()->debug("MqttClient::mosquitto_connect(): establishing connection...");
	}
	else
	{
		logger()->error("MqttClient::mosquitto_connect() failed: rc={}", rc);
	}
}

// TODO: Support connection disconnections and reconnections
void MqttClient::OnConnect_(int rc)
{
	logger()->trace("MqttClient::OnConnect_()");
	if (rc == MOSQ_ERR_SUCCESS)
	{
		m_connected = true;

		Subscribe_();

		for (auto sub : m_subs)
		{
			sub->OnConnect(rc);
		}

		// Notify publishers that they can send messages.
		m_cv.notify_all();
	}
}

void MqttClient::OnDisconnect_(int rc)
{
	logger()->trace("MqttClient::OnDisconnect_()");
	if (rc == MOSQ_ERR_SUCCESS)
	{
		m_connected = false;

//		Unsubscribe_(); // ???

		for (auto sub : m_subs)
		{
			sub->OnDisconnect(rc);
		}

		// Notify publishers that they can send messages.
		m_cv.notify_all();
	}
}

void MqttClient::OnSubscribe_(int mid, int qosCount, const int* grantedQos)
{
	logger()->trace("MqttClient::OnSubscribe_(mid={}, qosCount={}, grantedQos={})", mid, qosCount, *grantedQos);
}

void MqttClient::OnUnsubscribe_(int mid)
{
	logger()->trace("MqttClient::OnUnsubscribe_(mid={})", mid);
}

void MqttClient::OnPublish_(int mid)
{
	logger()->trace("MqttClient::OnPublish_(mid={})", mid);
}

void MqttClient::Subscribe_()
{
	logger()->trace("MqttClient::Subscribe_()");

	for (auto [topic, subs] : m_topicSubs)
	{
		Subscribe_(topic);
	}
}

void MqttClient::Subscribe_(const std::string& topic, int qos)
{
	logger()->trace("MqttClient::Subscribe_(\"{}\")", topic);

	int rc = mosquitto_subscribe(m_mosq, nullptr, topic.c_str(), qos);
	if (rc != MOSQ_ERR_SUCCESS)
	{
		logger()->error("MqttClient::Subscribe_(): mosquitto_subscribe() failed: rc={}", rc);
	}
}

void MqttClient::Unsubscribe_(const std::string& topic)
{
	logger()->trace("MqttClient::Unsubscribe_(\"{}\")", topic);
	constexpr int* mid = nullptr;
	int rc = mosquitto_unsubscribe(m_mosq, mid, topic.c_str());
	if (rc != MOSQ_ERR_SUCCESS)
	{
		logger()->error("MqttClient::Unsubscribe_(): mosquitto_unsubscribe() failed: rc={}", rc);
	}
}

void MqttClient::OnMessage_(const mosquitto_message* msg)
{
//	logger()->trace("MqttClient::OnMessage_()");
	if (msg)
	{
		std::string jsonstr(reinterpret_cast<char*>(msg->payload), msg->payloadlen);
		auto json = nlohmann::json::parse(jsonstr);

//		logger()->debug("MqttClient::OnMessage_(topic=\"{}\", json=\"{}\")", msg->topic, jsonstr);

//		int msgSentCount {0};
		for (auto [topic, subs] : m_topicSubs)
		{
			bool match {false};
			int rc = mosquitto_topic_matches_sub(topic.c_str(), msg->topic, &match);
			if (rc == MOSQ_ERR_SUCCESS && match)
			{
				for (auto subIt = subs.begin(); subIt != subs.end(); ++subIt)
				{
					(*subIt)->OnMessage(msg->topic, json);
//					++msgSentCount;
				}
			}
		}
//		logger()->debug("MqttClient()::OnMessage_(): Sub::OnMessage() called {} time(s)", msgSentCount);
	}
}

void MqttClient::OnLog_(int level, const char* str)
{
	switch (level)
	{
	case MOSQ_LOG_INFO:
	case MOSQ_LOG_NOTICE:
		logger()->info("{}", str);
		break;
	case MOSQ_LOG_WARNING:
		logger()->warn("{}", str);
		break;
	case MOSQ_LOG_ERR:
		logger()->error("{}", str);
		break;
	case MOSQ_LOG_DEBUG:
		logger()->debug("{}", str);
		break;
	}
}

#if 0
void MqttClient::Run_()
{
	int m_running {true};

	while (m_running)
	{
		int rc = mosquitto_loop(m_mosq, -1, 1);
	}
}
#endif

bool MqttClient::IsTopicMatch(const std::string& sub, const std::string& topic)
{
	bool match {false};
	int rc = mosquitto_topic_matches_sub(sub.c_str(), topic.c_str(), &match);
	return (rc == MOSQ_ERR_SUCCESS && match);
}

void MqttClient::Setup_()
{
	logger()->debug("MqttClient::Setup_(): count={}", Counter<MqttClient>::HowMany());
	if (Counter<MqttClient>::HowMany() == 1)
	{
		mosquitto_lib_init();
	}
}

void MqttClient::Cleanup_()
{
	logger()->debug("MqttClient::Cleanup_(): count={}", Counter<MqttClient>::HowMany());
	if (Counter<MqttClient>::HowMany() == 1)
	{
		mosquitto_lib_cleanup();
	}
}

} // namespace ncc
