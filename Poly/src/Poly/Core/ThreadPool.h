#pragma once

#include "Core.h"

#include <condition_variable>
#include <deque>
#include <functional>
#include <stop_token>
#include <thread>
#include <vector>

namespace Poly
{
	class ThreadPool
	{
	public:
		CLASS_STATIC(ThreadPool);

		/**
		 * Spawns the worker threads
		 * @param workerCount - Number of worker threads to spawn, 0 = auto (hardware_concurrency() - 1, clamped to >= 1)
		 */
		static void Init(uint32 workerCount = 0);

		/**
		 * Requests all workers to stop, drains any queued work, and joins all worker threads
		 */
		static void Release();

		/**
		 * Queues a task to run on a worker thread - fire and forget
		 * @param task - Task to run, must be self-contained (capture by value / own its data)
		 */
		static void Submit(std::function<void()> task);

		/**
		 * Queues all tasks and blocks the calling thread until every one of them has completed
		 * @param tasks - Tasks to run in parallel
		 */
		static void SubmitAndWait(std::vector<std::function<void()>> tasks);

		/**
		 * @return Number of worker threads
		 */
		static uint32 GetWorkerCount() { return static_cast<uint32>(m_Workers.size()); }

	private:
		static void WorkerLoop(std::stop_token stopToken);

		inline static std::vector<std::jthread>         m_Workers;
		inline static std::deque<std::function<void()>> m_TaskQueue;
		inline static std::mutex                        m_QueueMutex;
		inline static std::condition_variable_any       m_QueueCV;
	};
} // namespace Poly
