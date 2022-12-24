# brigadier-plusplus

brigadier-plusplus is a C++ port of Mojang's [brigadier](https://github.com/mojang/brigadier) command line parser & dispatcher, designed and developed for Minecraft: Java Edition and now freely available for use elsewhere under the MIT license.

This port is not supported by Mojang nor Microsoft.

IMPORTANT: brigadier-plusplus is not a one-to-one recreation of the original version. Some modifications were applied due to C++ specification and to make it easier to write command structure. I also tried to optimize memory and cpu usage as far as i could.

## Installation
Brigadier is written as a header-only library, so you can just copy the source code and include it in your project. There is also single-header version. There is no need for additional compilation. 
To include: `#include "brigadier.hpp"`
It is recommended to include it inside precompiled header (pch) to reduce compilation times.

### Additional info:
- C++17 language
- No external dependencies, standard library only
- No RTTI
- Compatible with gcc, clang, msvc compilers
- Compatible for windows and linux platform

### Optional features
- If `"magic_enum.hpp"` is available, you can use enum type argument
- If `"nameof.hpp"` is available, type names can be resolved automatically as you write new argument types


Tests are written and developed under Visual Studio 2022 using CppUnitTest framework.

## Contributing
Contributions are welcome! :D

## Usage
At the heart of Brigadier, you need a `CommandDispatcher<S>`, where `<S>` is any custom object you choose to identify a "command source". (Note that the source might be copied a couple of times, so it is recommended to keep the source lightweight or even use a pointer).

A command dispatcher holds a "command tree", which is a series of `CommandNode`s that represent the various possible syntax options that form a valid command.

For more examples see [tests](brigadier-plusplus-test/brigadier).

### Registering a new command
Before we can start parsing and dispatching commands, we need to build up our command tree. Every registration is an append operation,
so you can freely extend existing commands in a project without needing access to the source code that created them.

Command registration also encourages the use of a builder pattern to keep code cruft to a minimum.

A "command" is a fairly loose term, but typically it means an exit point of the command tree.
Every node can have an `executes` function attached to it, which signifies that if the input stops here then this function will be called with the context so far.

Consider the following example:
```cpp
using namespace brigadier;
using S = CommandSourceStack;

CommandDispatcher<S> dispatcher;

auto foo = dispatcher.Register<Literal>("foo");
foo.Then<Argument, Integer>("bar").Executes([](CommandContext<S>& ctx) {
    printf("Bar is %d\n", ctx.GetArgument<Integer>("bar"));
    return 1;
});
foo.Executes([](CommandContext<S>& ctx) {
    puts("Called foo without arguments");
    return 1;
});
```

This snippet registers two "commands": `foo` and `foo <bar>`. It is also common to refer to the `<bar>` as a "subcommand" of `foo`, as it's a child node.

At the start of the tree is a "root node", and it should have `LiteralCommandNode`s as children (But arguments are allowed too). Here, we register one command under the root: `<Literal>("foo")`, which means "the user must type the literal string 'foo'".

Under that is two extra definitions: a child node for possible further evaluation, or an `Executes` block if the user input stops here.

The child node works exactly the same way, but is no longer limited to literals. The other type of node that is now allowed is an `ArgumentCommandNode`, which takes in a name and an argument type.

Arguments can be anything, and you are encouraged to build your own for seamless integration into your own product. There are some standard arguments included in brigadier, such as `IntegerArgumentType`.

Argument types will be asked to parse input as much as they can, and then store the "result" of that argument however they see fit or throw a relevant error if they can't parse.

For example, an integer argument would parse "123" and store it as `123` (`int`), but throw an error if the input were `onetwothree`.

When a command is actually run, it can access these arguments in the context provided to the registered function.

### Parsing user input
So, we've registered some commands and now we're ready to take in user input. If you're in a rush, you can just call `dispatcher.Execute("foo 123", source)` and call it a day.

The result of `Execute` is an integer that was returned from an evaluated command. The meaning of this integer depends on the command, and will typically not be useful to programmers.

The `source` is an object of `<S>`, your own custom class to track users/players/etc. It will be provided to the command so that it has some context on what's happening.

If the command failed or could not parse, some form of `CommandSyntaxException` will be thrown. It is also possible for a `std::runtime_error` to be bubbled up, if not properly handled in a command.

If you wish to have more control over the parsing & executing of commands, or wish to cache the parse results so you can execute it multiple times, you can split it up into two steps:

```cpp
auto parse = dispatcher.Parse("foo 123", source);
int result = dispatcher.Execute(parse);
```

This is highly recommended as the parse step is the most expensive, and may be easily cached depending on your application.

You can also use this to do further introspection on a command, before (or without) actually running it.

### Inspecting a command
If you `parse` some input, you can find out what it will perform (if anything) and provide hints to the user safely and immediately.

The parse will never fail, and the `ParseResults<S>` it returns will contain a *possible* context that a command may be called with
(and from that, you can inspect which nodes the user entered, complete with start/end positions in the input string).
It also contains a map of parse exceptions for each command node it encountered. If it couldn't build a valid context, then
the reason why is inside this exception map.

### Displaying usage info
There are two forms of "usage strings" provided by this library, both require a target node.

`GetAllUsage(node, source, restricted)`  will return a list of all possible commands (executable end-points) under the target node and their human readable path. If `restricted`, it will ignore commands that `source` does not have access to. This will look like [`foo`, `foo <bar>`].

`GetSmartUsage(node, source)` will return a map of the child nodes to their "smart usage" human readable path. This tries to squash future-nodes together and show optional & typed information, and can look like `foo (<bar>)`.