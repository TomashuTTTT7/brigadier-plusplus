#pragma once

// Following code makes that you don't have to specify command source type inside arguments.
// Command source type is automatically distributed from dispatcher.

// Default registration for arguments with or without template, without specialization
#define REGISTER_ARGTYPE(type, name)                                      \
using name = type

// Registration for arguments with template parameters
#define REGISTER_ARGTYPE_TEMPL(type, name)                                \
template<typename... Args>                                                \
using name = type<Args...>

// Registration for arguments with specialized templates
#define REGISTER_ARGTYPE_SPEC(type, name, ...)                            \
using name = type<__VA_ARGS__>

// Registration for arguments with specialized templates and template parameters
#define REGISTER_ARGTYPE_SPEC_TEMPL(type, name, ...)                      \
template<typename... Args>                                                \
using name = type<__VA_ARGS__, Args...>
