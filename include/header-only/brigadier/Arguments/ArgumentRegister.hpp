#pragma once

#include "../Common.hpp"

// Following code makes that you don't have to specify command source type inside arguments.
// BasicCommand source type is automatically distributed from dispatcher.

// Default registration for arguments with or without template, without specialization
#define BRIGADIER_REGISTER_ARGTYPE(type, name)                            \
template<typename CharT>                                                  \
using name = type<CharT>

// Registration for arguments with template parameters
#define BRIGADIER_REGISTER_ARGTYPE_TEMPL(type, name)                      \
template<typename CharT, typename... Args>                                \
using name = type<CharT, Args...>

// Registration for arguments with specialized templates
#define BRIGADIER_REGISTER_ARGTYPE_SPEC(type, name, ...)                  \
template<typename CharT>                                                  \
using name = type<CharT, __VA_ARGS__>

// Registration for arguments with specialized templates and template parameters
#define BRIGADIER_REGISTER_ARGTYPE_SPEC_TEMPL(type, name, ...)            \
template<typename CharT, typename... Args>                                \
using name = type<CharT, __VA_ARGS__, Args...>
