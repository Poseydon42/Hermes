#pragma once

#define ADD_DEFAULT_DESTRUCTOR(Name) \
public: \
	~Name() = default;

#define ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Name) \
public: \
	virtual ~Name() = default;

#define ADD_DEFAULT_CONSTRUCTOR(Name) \
public: \
	Name() = default;

#define ADD_DEFAULT_COPY_CONSTRUCTOR(Name) \
public:\
	Name(const Name&) = default; \
	Name& operator=(const Name&) = default;

#define ADD_DEFAULT_MOVE_CONSTRUCTOR(Name) \
public:\
	Name(Name&&) = default; \
	Name& operator=(Name&&) = default;

#define ADD_INTERFACE_CONSTRUCTORS_AND_DESTRUCTOR(Name) \
public: \
	ADD_DEFAULT_CONSTRUCTOR(Name) \
	ADD_DEFAULT_VIRTUAL_DESTRUCTOR(Name) \
	ADD_DEFAULT_COPY_CONSTRUCTOR(Name) \
	ADD_DEFAULT_MOVE_CONSTRUCTOR(Name)
