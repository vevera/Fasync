#ifndef FASYNC_CALLBACK
#define FASYNC_CALLBACK

#include <functional>

namespace Fasync {

	template <class... Args>
	struct Signal {
		Signal() = default;

		Signal(const Signal& other) = delete;
		Signal(const Signal&& other) = delete;

		void Notify(Args... args) {
			for (const auto& f : mObservers) {
				f(std::forward<Args>(args)...);
			}
		}

		template <typename F>
		void Subscribe(F&& callable) {
			mObservers.push_back(std::forward<F>(callable));
		}

		// DOES NOT WORK RIGHT NOW, NEED TO THINK IN A GOOD WAY TO DO THAT
		//template <typename F>
		//void RemoveObserver(F&& callable) {
		//	mObservers.erase(std::remove(std::begin(mObservers), std::end(mObservers), std::forward<F>(callable)));
		//}

	private:
		using Signature = void(Args...);

		std::vector<std::function<Signature>> mObservers;
	};

	template <class... Args>
	static inline void Emit(Signal<Args...>* signal, Args... args) {
		signal->Notify(std::forward<Args>(args)...);
	}

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
}


#endif // !FASYNC_CALLBACK
