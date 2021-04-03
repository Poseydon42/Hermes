#pragma once

#include "Core/Core.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	/**
	 * Instance of application that could be run by the engine
	 * Each game module has to implement at least one child class of it and return it when CreateApplicationInstance() is called
	 */
	class HERMES_API IApplication
	{
		MAKE_NON_COPYABLE(IApplication)
		
	public:
		virtual ~IApplication() = default;

		IApplication(IApplication&&) = default;

		IApplication& operator=(IApplication&&) = default;
		
		/**
		 * Called right after CreateApplicationInstance()
		 * Here you can initialize all required subsystems
		 */
		virtual bool Init() = 0;

		/**
		 * Called before each frame
		 * @param Delta Amout of time elapsed since previous call to Run(), in seconds
		 */
		virtual void Run(float Delta) = 0;

		/**
		 * Called when user requested engine to exit
		 * This function is not called when engine exits due to fatal error or crashes
		 */
		virtual void Shutdown() = 0;
	};

	/**
	 * Typedef of function that is called by engine init code to get instance of IApplication that it has to run
	 * Each application should implement it
	 * Note that name of exported function must be strictly CreateApplicationInstance
	 */
	typedef IApplication*(*CreateApplicationInstance)();
}
