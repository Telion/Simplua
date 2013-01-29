Simplua
=======

Simplua is an easy-to-use Lua 5.2 wrapper for C++.  Its goal is to provide a simple alternative to Lua's native C interface.  Registering a native function using Simplua is this easy:

    double nativeFunction(double d, int n, const std::string& printMe)
    {
        std::cout << printMe << std::endl;
        return std::sin(d) + n;
    }

    ...
    lua::State state;
    state.registerFunction("TheNameOfTheFunctionInLua", nativeFunction);

That's it!  Your Lua script can now call TheNameOfTheFunctionInLua with 2 arguments.  If it passes the wrong number of arguments or the wrong types, Simplua automatically throws a type_mismatch exception.

Simplua does not facilitate object oriented programming or any other paradigm beyond what the native Lua API supports.  It simply makes the existing features easier to use.

Requirements
============

Some features of Simplua require C++11 support.  Until MSVC supports variadic templates, Simplua will not work on it.  It should work on any version of GCC after 4.4, although it has only been tested with 4.6 and 4.7.

No languages other than C++ are supported.

You will need a working build of Lua 5.2.  You can download one from the LuaBinaries project or build it yourself.  Lua may be included in the future but is not included at this point.

Simplua is currently licensed under the MIT license.  This is the same as Lua itself except for the new copyright notice.

Classes
=======

lua::State
----------

