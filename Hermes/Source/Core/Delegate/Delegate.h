#pragma once

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

		RetType Invoke(ArgsType&&... Args)
		{
			return FuncWrapper(Instance, std::forward<ArgsType>(Args)...);
		}

		RetType operator()(ArgsType&&... Args)
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

		InstancePtr Instance;
		InternalFunc FuncWrapper;
	};
}