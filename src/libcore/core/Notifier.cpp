#include <core/Logger.h>
#include <core/Notifier.h>

#include <mosquitto.h>

namespace ncc
{

bool Notifier::Add(const std::string& topic, MethodHandle callback)
{
	// Find "topic" and create it if it doesn't yet exist.
	auto itTopic = m_topicHandlers.find(topic);
	if (itTopic == m_topicHandlers.end())
	{
		auto it = m_topicHandlers.insert({topic, {}});
		itTopic = it.first;
	}

	// Check to see if callback already in list.
	auto& methods = itTopic->second;

	auto it = find_if(methods.begin(), methods.end(), [&](const MethodHandle& f) {
		return f.target<decltype(callback)>() == &callback;
	});
	if (it != methods.end())
	{
		return false;
	}

	methods.push_back(std::move(callback));
	return true;
}

bool Notifier::Remove(const std::string& topic)
{
	return m_topicHandlers.erase(topic) == 1;
}

bool Notifier::Remove(const std::string& topic, MethodHandle callback)
{
	auto itTopic = m_topicHandlers.find(topic);
	if (itTopic != m_topicHandlers.end())
	{
		auto& methods = itTopic->second;

		auto it = find_if(methods.begin(), methods.end(), [&](const MethodHandle& f) {
			return f.target<decltype(callback)>() == &callback;
		});
		if (it != methods.end())
		{
			methods.erase(it);
			// Remove topic if all methods erased.
			if (methods.empty())
			{
				m_topicHandlers.erase(itTopic);
			}
			return true;
		}
	}
	return false;
}

void Notifier::Notify(const std::string& topic, const nlohmann::json& json)
{
	auto list = GetMethodListForTopic_(topic);

	for (auto cb : list)
	{
		try
		{
			cb(topic, json);
		}
		catch (const std::exception& e)
		{
			logger()->error("Exception caught: {}", e.what());
		}
	}
}

std::vector<Notifier::MethodHandle>
Notifier::GetMethodListForTopic_(const std::string& topic)
{
	for (auto& [key, value] : m_topicHandlers)
	{
		bool result;
		int rc = mosquitto_topic_matches_sub(key.c_str(), topic.c_str(), &result);
		if (rc == MOSQ_ERR_SUCCESS && result)
		{
			return value;
		}
	}
	return {};
}

std::vector<std::string> Notifier::GetTopics()
{
	std::vector<std::string> topics;

	for (auto& [key, value] : m_topicHandlers)
	{
		topics.push_back(key);
	}
	return topics;
}

} // namespace ncc
