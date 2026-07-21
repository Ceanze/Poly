#include "ThreadPool.h"

#include <latch>

namespace Poly
{
	void ThreadPool::Init(uint32 workerCount)
	{
		if (workerCount == 0)
		{
			uint32 hardwareCount = std::thread::hardware_concurrency();
			workerCount          = hardwareCount > 1 ? hardwareCount - 1 : 1;
		}

		m_Workers.reserve(workerCount);
		for (uint32 i = 0; i < workerCount; i++)
			m_Workers.emplace_back(&ThreadPool::WorkerLoop);
	}

	void ThreadPool::Release()
	{
		for (auto& worker : m_Workers)
			worker.request_stop();

		m_Workers.clear(); // jthread destructor joins automatically
	}

	void ThreadPool::Submit(std::function<void()> task)
	{
		{
			std::lock_guard<std::mutex> lock(m_QueueMutex);
			m_TaskQueue.push_back(std::move(task));
		}
		m_QueueCV.notify_one();
	}

	void ThreadPool::SubmitAndWait(std::vector<std::function<void()>> tasks)
	{
		if (tasks.empty())
			return;

		std::latch remaining(tasks.size());

		{
			std::lock_guard<std::mutex> lock(m_QueueMutex);
			for (auto& task : tasks)
			{
				m_TaskQueue.push_back([task = std::move(task), &remaining]() {
					task();
					remaining.count_down();
				});
			}
		}
		m_QueueCV.notify_all();

		remaining.wait();
	}

	void ThreadPool::WorkerLoop(std::stop_token stopToken)
	{
		while (true)
		{
			std::function<void()> task;
			{
				std::unique_lock<std::mutex> lock(m_QueueMutex);

				const bool hasWork = m_QueueCV.wait(lock, stopToken, [] { return !m_TaskQueue.empty(); });
				if (!hasWork) // Stop requested and the queue has been fully drained
					return;

				task = std::move(m_TaskQueue.front());
				m_TaskQueue.pop_front();
			}
			task();
		}
	}
} // namespace Poly
