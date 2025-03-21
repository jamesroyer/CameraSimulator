#include <core/Logger.h>

#include <spdlog/details/synchronous_factory.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/udp_sink.h>
#include <spdlog/logger.h>
#include <spdlog/common.h>

namespace ncc
{

std::shared_ptr<spdlog::logger> g_logger;

void InitializeLogger(
	const std::string& name,
	bool truncateLog,
	const std::set<std::string>& sinkTypes,
	spdlog::level::level_enum level)
{
	std::vector<std::shared_ptr<spdlog::sinks::sink>> sinks;

//	if (sinkTypes.contains("stdout")) // C++20
	if (sinkTypes.find("stdout") != sinkTypes.end())
	{
		auto color_sink = std::make_shared<spdlog::sinks::ansicolor_stdout_sink_mt>();
		sinks.push_back(color_sink);
//		color_sink->set_level(level); // Should this be at sink or logger level?
		sinks.push_back(color_sink);
	}

//	if (sinkTypes.contains("udp")) // C++20
	if (sinkTypes.find("udp") != sinkTypes.end())
	{
		spdlog::sinks::udp_sink_config cfg("127.0.0.1", 11091);
		auto udp_sink = std::make_shared<spdlog::sinks::udp_sink_mt>(cfg);
//		udp_sink->set_level(level);
		sinks.push_back(udp_sink);
	}

//	if (sinkTypes.contains("file")) // C++20
	if (sinkTypes.find("file") != sinkTypes.end())
	{
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(name + ".log", truncateLog);
//		file_sink->set_level(level);
		sinks.push_back(file_sink);
	}

	g_logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
	g_logger->set_level(level);
}

spdlog::logger* logger()
{
	if (!g_logger)
	{
		throw std::runtime_error("InitializeLogger() must be called before calling logger()!");
	}
	return g_logger.get();
}

} // namespace ncc
