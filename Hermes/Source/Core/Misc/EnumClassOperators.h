#pragma once

#include "Core/Core.h"

#define ENUM_CLASS_OPERATORS(Name) \
	inline           Name& operator|=(Name& Lhs, Name Rhs) { return Lhs = (Name)((std::underlying_type_t<Name>)Lhs | (std::underlying_type_t<Name>)Rhs); } \
	inline           Name& operator&=(Name& Lhs, Name Rhs) { return Lhs = (Name)((std::underlying_type_t<Name>)Lhs & (std::underlying_type_t<Name>)Rhs); } \
	inline           Name& operator^=(Name& Lhs, Name Rhs) { return Lhs = (Name)((std::underlying_type_t<Name>)Lhs ^ (std::underlying_type_t<Name>)Rhs); } \
	inline constexpr Name  operator| (Name  Lhs, Name Rhs) { return (Name)((std::underlying_type_t<Name>)Lhs | (std::underlying_type_t<Name>)Rhs); } \
	inline constexpr Name  operator& (Name  Lhs, Name Rhs) { return (Name)((std::underlying_type_t<Name>)Lhs & (std::underlying_type_t<Name>)Rhs); } \
	inline constexpr Name  operator^ (Name  Lhs, Name Rhs) { return (Name)((std::underlying_type_t<Name>)Lhs ^ (std::underlying_type_t<Name>)Rhs); } \
	inline constexpr bool  operator! (Name  E)             { return !(std::underlying_type_t<Name>)E; } \
	inline constexpr Name  operator~ (Name  E)             { return (Name)~(std::underlying_type_t<Name>)E; }