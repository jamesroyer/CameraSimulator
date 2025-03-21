#pragma once

#include <core/INotifier.h>

#include <vector>

namespace ncc
{

class Notifier : public INotifier
{
public:
	~Notifier() override = default;

	bool Add(const std::string& topic, MethodHandle callback) override;
	bool Remove(const std::string& topic) override;
	bool Remove(const std::string& topic, MethodHandle callback) override;

	void Notify(const std::string& topic, const nlohmann::json& json) override;

	std::vector<std::string> GetTopics();

private:
	std::vector<MethodHandle> GetMethodListForTopic_(const std::string& topic);

private:
	std::map<std::string, std::vector<MethodHandle>> m_topicHandlers;

};

} // namespace ncc
