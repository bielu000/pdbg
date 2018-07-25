#pragma once
#include <functional>
#include <vector>

namespace signals {

	template<typename T>
	class signal {
	public:
		std::vector<std::function<T>> _listeners;
		void operator()() const
		{
			for (auto &x : _listeners)
			{
				x();
			}
		}

		template<typename... TArg>
		void operator ()(TArg... args)
		{
			for (auto &x : _listeners)
			{
				x(args...);
			}
		}
		signal() = default;
		~signal() = default;

		void connect(std::function<T> callback)
		{
			_listeners.push_back(callback);
		}
	};
}