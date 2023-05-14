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
		IApplication() = default;
		
		virtual ~IApplication() = default;

		IApplication(IApplication&&) = default;

		IApplication& operator=(IApplication&&) = default;

		/**
		 * Called very early in the engine initialization process. At this point,
		 * most systems are not ready to be used. You should use this function to
		 * set application-specific logging, mount necessary filesystems etc.
		 */
		virtual bool EarlyInit() = 0;
		
		/**
		 * Called when the engine and all its systems are initialized, you should
		 * do the main initialization work in this function.
		 */
		virtual bool Init() = 0;

		/**
		 * Called before each frame
		 * @param Delta Time elapsed since previous call to Run(), in seconds
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
