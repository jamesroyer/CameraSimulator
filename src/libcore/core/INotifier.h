#pragma once

#include <functional>
#include <string>

#include <nlohmann/json_fwd.hpp>

namespace ncc
{

// INotifier handles incoming messages from the MQTT broker and dispatches the
// messages to registered callback functions.
//
// The 'topic' is needed to subscribe for messages from the MQTT broker as well
// as to identify which callback to call when messages are received.
class INotifier
{
public:
	using MethodHandle = std::function<void(const std::string& topic, const nlohmann::json&)>;

	virtual ~INotifier() = default;

	virtual bool Add(const std::string& topic, MethodHandle callback) = 0;
	virtual bool Remove(const std::string& topic) = 0;
	virtual bool Remove(const std::string& topic, MethodHandle callback) = 0;

	virtual void Notify(const std::string& topic, const nlohmann::json& json) = 0;
};

#if 0
// Maybe something like this would simplify topic list management and
// subscription lifetime. The mosquitto library already requires that the
// client subscribes to it with the topic.
// This helper class subscribes to the MQTT broker and when a response is
// received, it forwards it to the owning object via its Notify() method.
template <typename ContainerType>
class SubscriberObj
{
public:
	SubscriberObj(mosquitto* mosq, ContainerType* obj, const std::string& topic)
		: m_mosq(mosq)
		, m_topic(topic)
		, m_obj(obj)
	{
		mosquitto_message_callback_set(mosq, [this](mosquitto*, void* ptr, const mosquitto_message* msg) {
				auto self = reinterpret_cast<Subscriber*>(ptr);
				std::string jsonstr(reinterpret_cast<char*>(msg->payload), msg->payloadlen);
				auto json = nlohmann::json::parse(jsonstr);

				m_obj->Notify(msg->topic, json);
			});
		int qos = 2;
		int rc = mosquitto_subscribe(m_mosq, nullptr, m_topic.c_str(), qos);
		if (rc != MOSQ_ERR_SUCCESS)
		{
			std::cerr << "mosquitto_subscribe() failed: rc=" << rc << std::endl;
		}
	}

	~SubscriberObj()
	{
		int rc = mosquitto_unsubscribe(m_mosq, nullptr, m_topic.c_str());
		if (rc != MOSQ_ERR_SUCCESS)
		{
			std::cerr << "mosquitto_unsubscribe() failed: rc=" << rc << std::endl;
		}
	}

private:
	mosquitto* m_mosq {nullptr};
	const std::string m_topic;
	ContainerType* m_obj {nullptr};
};

#endif

} // namespace ncc
