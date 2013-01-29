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

API Reference
=======

The Simplua library is provided through the header Simplua.h.  Note that Simplua.h does not include any native Lua headers; you need to include them yourself if you need their functionality.  The entire API is in the namespace lua.

lua::State
----------

State is the most important class in Simplua.  It serves the same purpose as a lua_State pointer and contains the pointer internally.

Each State object represents a separate script and set of variables.  Depending on your application, you may or may not need to instantiate more than one State object.

State objects clean up after themselves when their lifetime ends.  While it is possible to destroy the object earlier, this is generally not needed.

###State()
Creates an empty but valid State object.

###State(lua_State*)
Creates a State object initialized to an existing native lua_State*.  The State object takes ownership of the lua_State* and will automatically destroy it.

###State(State&& rhs)
Move-constructs a State object.

###State& operator =(State&& rhs)
Move-assigns a State object.

###~State()
Destroys the internal lua_State object.

###lua_State* get() const
Returns the internal lua_State pointer.  Use this only if you need to use the Lua C interface directly.

###void create()
Destroys the existing state (if any), and then constructs a new state as if the default constructor had been called.

###void destroy()
Destroys the existing state (if any), leaving the object in an invalid state.

###void loadFile(const std::string& filename, const std::string& mode = "bt")
Loads a script from the filesystem.  Mode specifies whether the file can be binary, text, or either.

Throws std::invalid_argument if mode is not "bt", "tb", "b", or "t".  Throws compile_error if the file could not be opened or if compilation fails.

###void loadString(const std::string& script, const std::std::string& mode = "t")
Loads a script stored in a string.  Otherwise behaves identically to loadFile.

###void setVariable(const std::string& name, const Object& object)
Sets a variable within the Lua script as if it had executed name=object internally.  The name can be a standard name, indicating a global variable, or it can contain periods to set a variable within a table.  Both of the following are valid:
    state.setVariable("someVar", lua::Object::makeString("Hello world!!!!!"));
    state.setVariable("someTable.someKey", lua::Object::makeNumber(7));
Referencing within a table is undefined behavior if the table does not exist (or is not actually a table).

###Object getVariable(const std::string& name, const std::set <Object> ignoreList = /*set of no elements*/)
Gets the variable with the specified name, returning it as an Object.  The variable can be within a table just like in setVariable; behavior is still undefined if the table is invalid.

###std::vector <Object> run()
Runs the script and stores any return values in a vector of Objects.

It is typically not meaningful to call run multiple times, as the State object remembers the "program counter".  However, you generally must call this before calling call (see below), as any variables/functions defined in the script will not actually exist until the script runs.

###std::vector <Object> call(const std::string& function, /*variadic arguments*/ args)
Calls the Lua function called function with the arguments args.  The function's return values are returned in a vector of Objects.

Unlike run, call can usually be used multiple times.  If you want to run a script repeatedly, wrap the repeated code in a function and call it repeatedly after running the script once.

###void registerFunction(const std::string& name, /*function pointer*/ func)
Registers the native function func so that it can be called from the Lua script.  Its name in Lua is set by the parameter name, and this can be within a table (see setVariable()).RegisterServiceCtrlHandler

Func can accept any (reasonable) number of arguments of any accepted types and can return one value.  Simplua will automatically handle parameter passing to this function.  If the script attempts to pass the wrong type or the wrong number of arguments, a type_mismatch is thrown (which manifests itself as a script_error).

Valid types are double, int, std::string, std::table<Object, Object>, int(*)(lua_State*), and bool.  Object can also be used and will accept any type passed from the script.  Any of these parameters can be taken by value or by reference to const.

###void loadLib(Lib lib)
###void loadLib(Lib lib, const std::string& name)
Loads the Lua standard library specified by lib.  In the second form, the library in Lua is given the desired name; in the first form, it receives the "typical" name (such as "base", "bit32", etc.).

Values for lua::Lib are:
    base
    coroutine
    table
    io
    os
    string
    bit32
    math
    debug
    package

The special value lua::Lib::all loads all the standard libraries.
