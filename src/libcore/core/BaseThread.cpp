#include <core/BaseThread.h>

namespace ncc
{

BaseThread::BaseThread(const std::string& name, bool autostart)
	: m_name(name)
{
	if (autostart)
	{
		Start();
	}
}

BaseThread::~BaseThread()
{
	Stop();
}

void BaseThread::Start()
{
	if (!m_thread.joinable())
	{
		std::unique_lock lock(m_mutex);
//		logger()->debug("{}: Start(): Starting thread...", m_name);
		m_thread = std::thread(&BaseThread::Run, this);
	}
}

void BaseThread::Stop()
{
	if (m_thread.joinable())
	{
		std::unique_lock lock(m_mutex);
//		logger()->debug("{}: Stop(): Stopping thread...", m_name);
		m_running.store(false);
		lock.unlock();
		m_cv.notify_one();
//		logger()->debug("{}: Stop(): Waiting for thread to terminate...", m_name);
		m_thread.join();
//		logger()->debug("{}: Stop(): Thread terminated", m_name);
	}
}

void BaseThread::Run()
{
	Run_();
}

} // namespace ncc
