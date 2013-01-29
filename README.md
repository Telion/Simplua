Simplua
=======

Simplua is an easy-to-use Lua wrapper for C++.  Its goal is to provide a simple alternative to Lua's native C interface.  Registering a native function using Simplua is this easy:

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
