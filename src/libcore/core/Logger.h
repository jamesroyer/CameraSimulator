#pragma once

#include <set>
#include <string>

#include <spdlog/spdlog.h>

namespace ncc
{

/**
 * Creates an "spdlog" logger that outputs to zero or more sinks.
 *
 * @param name is display at the start of all log messages
 * @param truncateLog is used to append logs to an existing file or start anew
 * @param sinkTypes set of desired sinks options: { stdout, udp, file }
 * @param level maximum spdlog level to show
 */
void InitializeLogger(
	const std::string& name,
	bool truncateLog = false,
	const std::set<std::string>& sinkTypes = { "file", "udp" },
	spdlog::level::level_enum level = spdlog::level::warn);

/**
 * Returns a pointer to the sdplog logger.
 */
spdlog::logger* logger();

} // namespace ncc
