#pragma once

#include "Core/Core.h"
#include "Core/Misc/DefaultConstructors.h"
#include "Core/Misc/NonCopyableMovable.h"

namespace Hermes
{
	enum class AssetType : uint8
	{
		Image = 1,
		Mesh = 2
	};

	/*
	 * A generic asset representation
	 * Hermes assumes every file that is loaded by engine to be asset(with exceptions being dynamically linked libraries and configuration files)
	 * This class should be inherited for each known asset type to implement behaviour specific to it
	 */
	class HERMES_API Asset
	{
		MAKE_NON_COPYABLE(Asset)
		ADD_DEFAULT_MOVE_CONSTRUCTOR(Asset)
		ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Asset)
	public:
		Asset(const String& InName, AssetType InType);

		String GetName() const;

		AssetType GetType() const;

		virtual bool IsValid() const;

		virtual size_t GetMemorySize() const;

	private:
		String Name;
		AssetType Type;
	};
}
