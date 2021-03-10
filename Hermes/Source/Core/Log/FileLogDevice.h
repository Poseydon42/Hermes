#pragma once

#include "Core/Core.h"
#include "Core/Log/ILogDevice.h"

#include "Platform/GenericPlatform/PlatformFile.h"

namespace Hermes
{
	class HERMES_API FileLogDevice : public ILogDevice
	{
	public:
		FileLogDevice(const String& Path, LogLevel InitialLevel);
		
		void Write(LogLevel Level, String Text) override;
		
		void WriteLine(LogLevel Level, String Text) override;
		
		LogLevel GetCurrentLogLevel() override;
		
		void SetCurrentLogLevel(LogLevel NewLevel) override;

	private:
		LogLevel CurrentLevel;
		
		std::shared_ptr<IPlatformFile> Target;
	};
}

