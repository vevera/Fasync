#include <mutex>
#include <optional>
#include <queue>
#include <thread>
#include <tuple>

#include "debug.hpp"

namespace Fasync {


	class Scheduler {

	public: 
		Scheduler() {
			Init(std::thread::hardware_concurrency());
		}

		Scheduler(size_t n) {
			Init(n);
		}

		~Scheduler() {
			DEBUG_LOG("\ndesc called!!\n");

			mStopWork.store(true);

			for (auto& worker : mWorkers) {
				if (worker.joinable())
					worker.join();
			}
		}

		enum class Priority {
			Low,
			Normal,
			High,
		};

		template <typename F, typename... Args>
		inline void Add(F&& task, Args... args) {
			Add<F, Args...>(Priority::Normal, std::forward<F>(task), std::forward<Args>(args)...);
		}


		template <typename F, typename... Args>
		inline void Add(Priority p, F&& task, Args... args) {
			mSynchronizer.lock();
			mTasks.emplace(p, RoutineWrapper<F, Args...>, new Parameters<F, Args...>{ std::forward<F>(task), std::forward<Args>(args)... });
			mSynchronizer.unlock();
		}

	private:

		void Init(size_t n) {
			DEBUG_LOG("Number of workers: " << n);

			for (int i = 0; i < n; ++i) {
				mWorkers.emplace_back(std::thread([&]() {
					while (!mStopWork.load() || !IsTaskVectorEmpty()) {

						auto task = PopTask();

						if (!task.has_value()) {
							continue;
						}

						auto& pParams = task.value().params;
						auto& pTask = task.value().task;

						pTask(pParams);
					}
				}));
			}
		}

		struct Task {
			Task(Priority priority, void(*task)(void*), void* params) : p{ priority }, task{ task }, params{ params } {}

			Priority p;

			void* params;
			void(*task)(void*);

			const bool operator<(const Task& other) const {
				return p < other.p;
			}
		};

		template <typename F, typename... Args>
		struct Parameters {
			Parameters(F&& internal_task, Args... args) : _internal_task{ std::forward<F>(internal_task) }, _internal_args{ std::forward<Args>(args)... } { }

			F _internal_task;
			std::tuple<Args...> _internal_args;
		};


		template<typename F, class... Args>
		static inline void RoutineWrapper(void* parameters) {

			auto* pTypedParameters = reinterpret_cast<Parameters<F, Args...>*>(parameters);

			if (pTypedParameters == nullptr)
			{
				DEBUG_LOG("pTypedParameters is nullptr\n");
				return;
			}

			auto& task = pTypedParameters->_internal_task;
			auto& args = pTypedParameters->_internal_args;

			std::apply(task, args);

			delete pTypedParameters;
		}

		std::optional<Task> PopTask() {
			std::lock_guard mutex(mSynchronizer);

			if (mTasks.empty()) return {};

			Task task = mTasks.top();
			mTasks.pop();

			return task;
		}

		bool IsTaskVectorEmpty() {
			std::lock_guard mutex(mSynchronizer);
			return mTasks.empty();
		}

		std::priority_queue<Task, std::vector<Task>> mTasks;
		std::mutex mSynchronizer;
		std::vector<std::thread> mWorkers;
		std::atomic<bool> mStopWork = false;
	};
};

