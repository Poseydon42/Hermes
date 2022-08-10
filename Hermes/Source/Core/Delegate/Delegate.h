#pragma once

#include <vector>

#include "Core/Core.h"

namespace Hermes
{

	// https://blog.molecular-matters.com/2011/09/19/generic-type-safe-delegates-and-events-in-c/
	/**
	 * Simple unicast delegate
	 * Currently only free and member functions are supported
	 * Stores strong pointer to underlying instance in case of member function delegation
	 * Usage:
	 * Declare like Hermes::TDelegate<ReturnType, ArgumentType1, ArgumentType2...>
	 * Example: Hermes::TDelegate<float, float, Hermes::String&> could bind functions that return float and take float and Hermes::String& as its arguments
	 * To bind free function: Delegate.Bind<Function>();
	 * To bind member function: Delegate.Bind<Class, Function>(&ClassInstance);
	 * To invoke call either Ivoke(Args) or simply use operator () as in regular function call
	 */
	template<typename RetType, typename ...ArgsType>
	struct TDelegate
	{
	private:
		using InstancePtr = void*;
		using InternalFunc = RetType(*)(InstancePtr, ArgsType&&...);
	public:
		/**
		 * Binds new function by replacing previous
		 */
		template<RetType(*Function)(ArgsType...)>
		void Bind()
		{
			Instance = 0;
			FuncWrapper = &FreeFunctionWrapper<Function>;
		}

		template<class C, RetType(C::*Function)(ArgsType...)>
		void Bind(C* NewInstance)
		{
			Instance = NewInstance;
			FuncWrapper = &MemberFunctionWrapper<C, Function>;
		}

		RetType Invoke(ArgsType&&... Args) const
		{
			return FuncWrapper(Instance, std::forward<ArgsType>(Args)...);
		}

		RetType operator()(ArgsType&&... Args) const
		{
			return Invoke(std::forward<ArgsType>(Args)...);
		}
	private:
		template<RetType(*Function)(ArgsType...)>
		static RetType FreeFunctionWrapper(InstancePtr, ArgsType&&... Args)
		{
			return Function(std::forward<ArgsType>(Args)...);
		}

		template<class C, RetType(C::*Function)(ArgsType...)>
		static RetType MemberFunctionWrapper(InstancePtr InstanceToCall, ArgsType&&... Args)
		{
			return (((C*)InstanceToCall->*Function)(std::forward<ArgsType>(Args)...));
		}

		InstancePtr Instance = 0;
		InternalFunc FuncWrapper = 0;
	};

	/**
	 * Delegate that supports binding multiple functions
	 * Each function has to have same signature and may not have return type different from void as it makes no sense
	 * Usage is similar to TDelegate
	 */
	template<typename ...ArgsType>
	struct TMulticastDelegate
	{
	private:
		using InstancePtr = void*;
		using InternalFunc = void(*)(InstancePtr, ArgsType&&...);
		using CallbackContainer = std::vector<std::pair<InstancePtr, InternalFunc>>;
	public:
		TMulticastDelegate(size_t InitialContainerSize = 4)
		{
			Callbacks.reserve(InitialContainerSize);
		}

		/**
		 * Binds new function without touching previous
		 */
		template<void(*Function)(ArgsType...)>
		void Bind()
		{
			Callbacks.push_back(std::make_pair<InstancePtr, InternalFunc>(0, &FreeFunctionWrapper<Function>));
		}

		template<class C, void(C::* Function)(ArgsType...)>
		void Bind(C* NewInstance)
		{
			Callbacks.push_back(std::make_pair<InstancePtr, InternalFunc>((InstancePtr)NewInstance, &MemberFunctionWrapper<C, Function>));
		}

		/**
		 * Clears all bindings
		 */
		void Clear()
		{
			Callbacks.clear();
		}

		void Invoke(ArgsType&&... Args)
		{
			for (auto Callback : Callbacks)
				Callback.second(Callback.first, std::forward<ArgsType>(Args)...);
		}

		void operator()(ArgsType&&... Args)
		{
			Invoke(std::forward<ArgsType>(Args)...);
		}
	private:
		template<void(*Function)(ArgsType...)>
		static void FreeFunctionWrapper(InstancePtr, ArgsType&&... Args)
		{
			return Function(std::forward<ArgsType>(Args)...);
		}

		template<class C, void(C::* Function)(ArgsType...)>
		static void MemberFunctionWrapper(InstancePtr InstanceToCall, ArgsType&&... Args)
		{
			return (((C*)InstanceToCall->*Function)(std::forward<ArgsType>(Args)...));
		}

		CallbackContainer Callbacks;
	};
}
