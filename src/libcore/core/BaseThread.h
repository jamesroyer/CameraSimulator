#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>

namespace ncc
{

class BaseThread
{
public:
	explicit BaseThread(const std::string& name, bool autostart = true);
	virtual ~BaseThread();

	void Start();
	void Stop();

	void Run();

protected:
	virtual void Run_() = 0;

protected:
	std::thread m_thread;
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::atomic_bool m_running;
	const std::string m_name;
};

} // namespace ncc
