#ifndef FASYNC_CALLBACK
#define FASYNC_CALLBACK

#include <functional>
#include <thread>
#include <memory>

namespace Fasync {

	template <class... Args>
	struct Signal {
		Signal() : mCallbacks{ std::make_unique<CallbackContainer>() } {};

		Signal(const Signal& other) = delete;
		Signal(const Signal&& other) = delete;

		void Notify(Args... args) {
			for (const auto& f : *mCallbacks) {
				f(std::forward<Args>(args)...);
			}
		}

		void AsyncNotify(std::vector<std::thread>& asyncOperations, Args... args) {
			for (const auto& f : *mCallbacks) {
				asyncOperations.emplace_back(f, args...);
			}
		}

		template <typename F>
		void Subscribe(F&& callable) {
			mCallbacks->push_back(std::forward<F>(callable));
		}

	private:
		using Signature = void(Args...);
		using CallbackContainer = std::vector<std::function<Signature>>;

		std::unique_ptr<CallbackContainer> mCallbacks;
	};

	template <class... Args, typename F>
	static inline void Connect(Signal<Args...>* signal, F&& function) {
		signal->Subscribe(std::forward<F>(function));
	}

	template <class... Args, typename T>
	static inline void Connect(Signal<Args...>* signal, T* target, void (T::* function)(Args...)) {
		signal->Subscribe([target, function](Args... args) -> void {
			(target->*function)(std::forward<Args>(args)...);
			});
	}

	template <class... Args, typename T>
	static inline void Connect(Signal<Args...>* signal, T* target, void (T::* function)(Args...) const) {
		signal->Subscribe([target, function](Args... args) -> void {
			(target->*function)(std::forward<Args>(args)...);
			});
	}

	/*
		Synchronous emit, blocking.
	*/

	template <class... Args>
	static inline void Emit(Signal<Args...>* signal, Args... args) {
		signal->Notify(std::forward<Args>(args)...);
	}

	/*
		Managing the lifetime of objects while certain tasks are running is tricky,
		so the responsibility is passed to the users of this function.

		When EmitAsync is called, it returns the thread on which the execution is running.
		The user must ensure that the work is complete before deleting any data used
		in any registered callback of the signal.

		Note that it is necessary to either join or detach the returned thread,
		otherwise the program will crash.
	*/

	template <class... Args>
	static inline std::thread EmitAsync(Signal<Args...>* signal, Args... args) {
		return std::thread(&Signal<Args...>::Notify, signal, std::forward<Args>(args)...);
	}

	template <class... Args>
	static inline std::thread& EmitAsync(std::vector<std::thread>& asyncOperations, Signal<Args...>* signal, Args... args) {
		return asyncOperations.emplace_back(&Signal<Args...>::Notify, signal, std::forward<Args>(args)...);
	}

	/*
		Like the previous functions, this Emit is also asynchronous, but it is slightly different.
		It calls all the callbacks in different threads, so they will all execute concurrently.
	*/

	template <class... Args>
	static inline void EmitAsyncMulti(std::vector<std::thread>& asyncOperations, Signal<Args...>* signal, Args... args) {
		signal->AsyncNotify(asyncOperations, std::forward<Args>(args)...);
	}
}


#endif // !FASYNC_CALLBACK
