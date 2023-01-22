#pragma once

#include <concepts>
#include <format>
#include <optional>

#include "Core/Core.h"

namespace Hermes
{
	struct HERMES_API Version
	{
		uint32 Major = 0;
		uint32 Minor = 0;
		uint32 Patch = 0;

		constexpr Version(uint32 InMajor, uint32 InMinor, uint32 InPatch = 0);

		template<std::unsigned_integral InType>
		static constexpr Version FromCompact(InType Value, uint32 Shift);
		template<std::unsigned_integral InType>
		static constexpr Version FromCompact(InType Value, uint32 MajorShift, uint32 MinorShift, uint32 PatchShift);

		static constexpr std::optional<Version> FromString(StringView String);

		template<std::unsigned_integral OutType>
		constexpr OutType ToCompact(uint32 Shift) const;
		template<std::unsigned_integral OutType>
		constexpr OutType ToCompact(uint32 MajorShift, uint32 MinorShift, uint32 PatchShift) const;

		inline String ToString() const;
		inline String ToStringWithoutPatch() const;

		constexpr bool operator==(const Version& Other) const;
		constexpr bool operator!=(const Version& Other) const;

		constexpr bool operator>(const Version& Other) const;
		constexpr bool operator>=(const Version& Other) const;
		constexpr bool operator<(const Version& Other) const;
		constexpr bool operator<=(const Version& Other) const;
	};

	constexpr Version::Version(uint32 InMajor, uint32 InMinor, uint32 InPatch)
		: Major(InMajor)
		, Minor(InMinor)
		, Patch(InPatch)
	{
	}

	template<std::unsigned_integral InType>
	constexpr Version Version::FromCompact(InType Value, uint32 Shift)
	{
		return FromCompact(Value, Shift * 2, Shift, 0);
	}

	template<std::unsigned_integral InType>
	constexpr Version Version::FromCompact(InType Value, uint32 MajorShift, uint32 MinorShift, uint32 PatchShift)
	{
		auto Major = Value >> MajorShift;

		auto MinorAndMajor = (Value >> MinorShift);
		auto Minor = MinorAndMajor - (Major << (MajorShift - MinorShift));

		auto PatchAndMinorAndMajor = (Value >> PatchShift);
		auto Patch = PatchAndMinorAndMajor - (MinorAndMajor << (MinorShift - PatchShift));

		return { static_cast<uint32>(Major), static_cast<uint32>(Minor), static_cast<uint32>(Patch) };
	}

	constexpr std::optional<Version> Version::FromString(StringView String)
	{
		if (String.empty())
			return {};

		for (auto Char : String)
		{
			if (!(Char >= '0' && Char <= '9') && Char != '.')
				return {};
		}

		auto Current = String.begin();

		uint32 Major = 0;
		bool SeenDigitsOfMajor = false;
		for (; Current != String.end() && *Current != '.'; ++Current)
		{
			if (*Current < '0' || *Current > '9')
				return {};

			Major = Major * 10 + (*Current - '0');
			SeenDigitsOfMajor = true;
		}

		if (!SeenDigitsOfMajor)
			return {};

		if (Current == String.end())
			return Version(Major, 0, 0);

		// *Current == '.' at this point
		++Current;

		uint32 Minor = 0;
		bool SeenDigitsOfMinor = false;
		for (; Current != String.end() && *Current != '.'; ++Current)
		{
			if (*Current < '0' || *Current > '9')
				return {};

			SeenDigitsOfMinor = true;
			Minor = Minor * 10 + (*Current - '0');
		}

		if (!SeenDigitsOfMinor)
			return {};

		if (Current == String.end())
			return Version(Major, Minor, 0);

		// *Current == '.' again at this point
		++Current;

		uint32 Patch = 0;
		bool SeenDigitsOfPatch = false;
		for (; Current != String.end() && *Current != '.'; ++Current)
		{
			if (*Current < '0' || *Current > '9')
				return {};

			SeenDigitsOfPatch = true;
			Patch = Patch * 10 + (*Current - '0');
		}

		if (!SeenDigitsOfPatch)
			return {};

		if (Current != String.end())
			return {};

		return Version(Major, Minor, Patch);
	}

	template<std::unsigned_integral OutType>
	constexpr OutType Version::ToCompact(uint32 Shift) const
	{
		return ToCompact<OutType>(Shift * 2, Shift, 0);
	}

	template<std::unsigned_integral OutType>
	constexpr OutType Version::ToCompact(uint32 MajorShift, uint32 MinorShift, uint32 PatchShift) const
	{
		OutType Result = (Major << MajorShift) | (Minor << MinorShift) | (Patch << PatchShift);
		return Result;
	}

	String Version::ToString() const
	{
		return std::format("{}.{}.{}", Major, Minor, Patch);
	}

	String Version::ToStringWithoutPatch() const
	{
		return std::format("{}.{}", Major, Minor);
	}

	constexpr bool Version::operator==(const Version& Other) const
	{
		return Major == Other.Major && Minor == Other.Minor && Patch == Other.Patch;
	}

	constexpr bool Version::operator!=(const Version& Other) const
	{
		return !(*this == Other);
	}

	constexpr bool Version::operator>(const Version& Other) const
	{
		if (Major > Other.Major)
			return true;

		if (Major == Other.Major && Minor > Other.Minor)
			return true;

		if (Major == Other.Major && Minor == Other.Minor && Patch > Other.Patch)
			return true;

		return false;
	}

	constexpr bool Version::operator>=(const Version& Other) const
	{
		return (*this > Other) || (*this == Other);
	}

	constexpr bool Version::operator<(const Version& Other) const
	{
		return !(*this >= Other);
	}

	constexpr bool Version::operator<=(const Version& Other) const
	{
		return !(*this > Other);
	}
}
