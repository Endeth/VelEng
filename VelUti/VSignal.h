#pragma once

#pragma once
#include <functional>
#include <list>

namespace Vel
{
	template<typename T>
	struct function_traits {};

	template<typename R, typename ...Args>
	struct function_traits<R(Args...)>
	{
		using return_type = R;
	};


	template<typename T>
	struct BasicResultType
	{
		T _lastCall;
		void Collect(T t)
		{
			_lastCall = t;
		}
		T GetResult()
		{
			return _lastCall;
		}
	};

	template<>
	struct BasicResultType<void>
	{
		void Collect()
		{

		}
		void GetResult()
		{
		}
	};


	template<typename F, typename Result = BasicResultType< typename function_traits<F>::return_type >>
	class Signal
	{
	};

	template<typename R, typename... Args, typename Result>
	class Signal<R(Args...), Result>
	{
	private:
		std::list<std::function<R(Args...)>> _functionsList;
		Result _res;

	public:
		void Connect(std::function<R(Args...)> NewFunction)
		{
			_functionsList.push_back(NewFunction);
		}
		R operator()(Args... a)
		{
			for (auto &Fun : _functionsList)
			{
				_res.Collect(Fun(a...));
			}
			return _res.GetResult();

		}

	};

	template<typename... Args>
	class Signal<void(Args...)>
	{
	private:
		std::list<std::function<void(Args...)>> _functionsList;

	public:
		void Connect(std::function<void(Args...)> NewFunction)
		{
			_functionsList.push_back(NewFunction);
		}
		void operator()(Args... a)
		{
			for (auto &Fun : _functionsList)
			{
				Fun(a...);
			}
		}

	};
}
