//*****************************************************************************
//Copyright 2013 Christopher Mueller
//Permission is hereby granted, free of charge, to any person obtaining a copy
//of this software and associated documentation files (the "Software"), to deal
//in the Software without restriction, including without limitation the rights
//to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//copies of the Software, and to permit persons to whom the Software is
//furnished to do so, subject to the following conditions:
//
//The above copyright notice and this permission notice shall be included in
//all copies or substantial portions of the Software.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.
//*****************************************************************************


#pragma once

#include <string>
#include <map>
#include <set>
#include <vector>
#include <tuple>
#include <stdexcept>

//Note that lua.hpp is not included.

class lua_State;


//This is the max height of the tree formed by nesting tables within tables and obtaining the result in C++.
//If the tree is heigher than this, it throws a table_too_deep.
//This exists to catch circular references which cannot be expressed cleanly by C++ maps.
//The default is too low to handle some legitimate structures and may need to be changed.
//However, if it is too high you may run out of stack space before the exception is thrown.
#define LUA_MAX_TABLE_RECURSION 8
//If this not defined, no exception is thrown; rather, any elements past the limit are set to nil.
#define LUA_THROW_TABLE_TOO_DEEP

namespace lua
{
    class type_mismatch : public std::runtime_error
    {
    public:
        explicit type_mismatch(const std::string& what)
        : runtime_error(what)
        {}
    };

    class script_error : public std::runtime_error
    {
    public:
        explicit script_error(const std::string& what)
        : runtime_error(what)
        {}
    };

    class table_too_deep : public script_error
    {
    public:
        explicit table_too_deep(const std::string& what)
        : script_error(what)
        {}
    };

    class compile_error : public script_error
    {
    public:
        explicit compile_error(const std::string& what)
        : script_error(what)
        {}
    };

    class uninitialized_resource : public std::logic_error
    {
    public:
        explicit uninitialized_resource(const std::string& what)
        : logic_error(what)
        {}
    };


    class Object;

    //equivalent C++ types for each Lua type, provided mainly for convenience
    typedef double LuaNumber;
    typedef int LuaInteger;  //you might want to change this to long long
    typedef std::string LuaString;
    typedef std::map <Object, Object> LuaTable;
    typedef int (*LuaFunction)(lua_State*);
    typedef bool LuaBoolean;
    typedef void* LuaUserdata;
    typedef lua_State* LuaThread;
    typedef std::map <Object, Object>* LuaWeakTable;


    namespace internal
    {
        extern LuaString emptyString;
        extern LuaTable emptyTable;
    }//namespace internal


    //Represents (almost) any Lua type within C++.
    //Use the get* member functions to get the value as a certain type.
    //If the internal type of the Object does not match the requested type, it throws a type_mismatch.
    //If you cannot handle the exception, check the type yourself with the is* member functions.
    //Note that Objects are immutable except when moved.
    class Object
    {
    private:
        void copy(const Object& rhs);
        void moveFrom(Object& rhs);


        //the type the Object holds
        int type;

        //variables for all possible types the Object can hold
        union
        {
            LuaNumber num;
            LuaFunction func;
            LuaBoolean boolean;
            //LuaUserdata userdata;
            //LuaThread thread;
        };
        //these members cannot be in the union, unfortunately
        LuaString str;
        LuaTable table;

    public:
        static const int NIL = 0;
        static const int NUMBER = 1;
        static const int STRING = 2;
        static const int TABLE = 3;
        static const int FUNCTION = 4;
        static const int BOOLEAN = 5;
        static const int USERDATA = 6; //NOT SUPPORTED YET
        static const int THREAD = 7;   //NOT SUPPORTED YET
        static const int WEAK_TABLE = 8;


        //standard constructors and assignment operators
        Object();
        Object(const Object& rhs);
        Object& operator =(const Object& rhs);
        Object(Object&& rhs);
        Object& operator =(Object&& rhs);

        //make* functions are named constructors that return an Object containing the argument as the correct internal type.
        static Object makeNil();
        static Object makeNumber(LuaNumber d = LuaNumber());
        static Object makeInteger(LuaInteger i = LuaInteger());
        static Object makeString(const LuaString& s = internal::emptyString);
        static Object makeTable(const LuaTable& m = internal::emptyTable);
        static Object makeFunction(LuaFunction f);
        static Object makeBoolean(LuaBoolean b = LuaBoolean());
        static Object makeWeakTable(LuaWeakTable t);

        //Makes an object out of any valid type.
        //This has trouble with type conversions, so the specific named constructors should be preferred.
        template <typename T>
        static Object makeAuto(T t); //defined later


        //get* functions return the value of the Object, but only if the internal type matches the requested type.
        //If the types mismatch, these functions will throw type_mismatch.
        LuaNumber getNumber() const;
        LuaInteger getInteger() const;
        const LuaString& getString() const;
        const LuaTable& getTable() const;
        LuaFunction getFunction() const;
        LuaBoolean getBoolean() const;
        LuaWeakTable getWeakTable() const;

        //is* functions return true only if the Object's dynamic type is the one being checked.
        //Use these if you need to guarantee that no type_mismatch exceptions will be thrown.
        bool isNil() const;
        bool isNumber() const;
        bool isInteger() const; //note that integer is not an actual type
        bool isString() const;
        bool isTable() const;
        bool isFunction() const;
        bool isBoolean() const;
        bool isWeakTable() const;

        //returns the internal type number of the Object
        int getType() const;

        bool operator <(const Object& rhs) const;
        bool operator >(const Object& rhs) const;
        bool operator <=(const Object& rhs) const;
        bool operator >=(const Object& rhs) const;
        bool operator ==(const Object& rhs) const;
        bool operator !=(const Object& rhs) const;
    };

    std::ostream& operator <<(std::ostream& out, const Object& obj);

    namespace internal
    {
        //templates used to make an Object out of a generic type
        template <typename T>
        struct MakeObject
        {

        };

        template <>
        struct MakeObject<LuaNumber>
        {
            Object operator()(LuaNumber d) const
            {
                return Object::makeNumber(d);
            }
        };

        template <>
        struct MakeObject<LuaInteger>
        {
            Object operator()(LuaInteger i) const
            {
                return Object::makeInteger(i);
            }
        };

        template <>
        struct MakeObject<LuaString>
        {
            Object operator()(const LuaString& s) const
            {
                return Object::makeString(s);
            }
        };

        template <>
        struct MakeObject<LuaTable>
        {
            Object operator()(const LuaTable& t) const
            {
                return Object::makeTable(t);
            }
        };

        template <>
        struct MakeObject<LuaFunction>
        {
            Object operator()(LuaFunction f) const
            {
                return Object::makeFunction(f);
            }
        };

        template <>
        struct MakeObject<LuaBoolean>
        {
            Object operator()(LuaBoolean b) const
            {
                return Object::makeBoolean(b);
            }
        };

        template <>
        struct MakeObject<LuaWeakTable>
        {
            Object operator()(LuaWeakTable t) const
            {
                return Object::makeWeakTable(t);
            }
        };
    } //namespace internal

    template <typename T>
    Object Object::makeAuto(T t)
    {
        return internal::MakeObject<T>()(t);
    }


    namespace internal
    {
        extern std::set <Object> emptySet;
        extern std::vector <std::string> emptyVector;

        //this function is defined later
        template <typename T>
        void pushVar(lua_State* state, T t);

        template <typename T>
        struct PushVar
        {

        };

        template <>
        struct PushVar<Object>
        {
            void operator()(lua_State* state, Object object) const;
        };

        template <>
        struct PushVar<LuaNumber>
        {
            void operator()(lua_State* state, LuaNumber d) const;
        };

        template <>
        struct PushVar<LuaInteger>
        {
            void operator()(lua_State* state, LuaInteger i) const;
        };

        template <>
        struct PushVar<LuaString>
        {
            void operator()(lua_State* state, const LuaString& s) const;
        };

        template <>
        struct PushVar<LuaTable>
        {
            void operator()(lua_State* state, const LuaTable& t) const;
        };

        template <>
        struct PushVar<LuaFunction>
        {
            void operator()(lua_State* state, LuaFunction f) const;
        };

        template <>
        struct PushVar<LuaBoolean>
        {
            void operator()(lua_State* state, LuaBoolean b) const;
        };

        template <typename T>
        void pushVar(lua_State* state, T t)
        {
            PushVar<T>()(state, t);
        }


        template <typename T>
        struct GetStackVar
        {

        };

        template <>
        struct GetStackVar<Object>
        {
             Object operator()(lua_State* state, int index, const std::set<Object>& ignoreList = emptySet, int level = LUA_MAX_TABLE_RECURSION) const;
        };

        template <>
        struct GetStackVar<LuaNumber>
        {
            LuaNumber operator()(lua_State* state, int index) const;
        };

        template <>
        struct GetStackVar<LuaInteger>
        {
            LuaInteger operator()(lua_State* state, int index) const;
        };

        template <>
        struct GetStackVar<LuaString>
        {
            LuaString operator()(lua_State* state, int index) const;
        };

        template <>
        struct GetStackVar<LuaTable>
        {
            LuaTable operator()(lua_State* state, int index, int level = LUA_MAX_TABLE_RECURSION) const;
        };

        template <>
        struct GetStackVar<LuaFunction>
        {
            LuaFunction operator()(lua_State* state, int index) const;
        };

        template <>
        struct GetStackVar<LuaBoolean>
        {
            LuaBoolean operator()(lua_State* state, int index) const;
        };



        template<int... Args>
        struct Sequence
        {

        };

        template<int N, int... Args>
        struct SequenceGenerator : SequenceGenerator<N-1, N-1, Args...>
        {

        };

        template<int... Args>
        struct SequenceGenerator<0, Args...>
        {
            typedef Sequence<Args...> type;
        };

        template <typename R, typename ...Args>
        struct Unpacker
        {
            std::tuple<typename std::decay<Args>::type...> params;
            R (*func)(Args...);

            R call()
            {
                return callFunc(typename SequenceGenerator<sizeof...(Args)>::type());
            }

            template<int... MyArgs>
            R callFunc(Sequence<MyArgs...>)
            {
                return func(std::get<MyArgs>(params)...);
            }
        };

        template <typename R, typename ...Args>
        Unpacker <R, Args...> makeUnpacker(R (*func)(Args...), std::tuple<typename std::decay<Args>::type...> args)
        {
            Unpacker <R, Args...> u = {std::move(args), func};
            return u;
        }



        template <typename T>
        std::tuple <typename std::decay<T>::type> prepareArgs(lua_State* state, int index)
        {
            auto t = GetStackVar<typename std::decay<T>::type>()(state, index);
            return std::make_tuple(t);
        }

        template <typename T, typename U, typename... Args>
        std::tuple <typename std::decay<T>::type, typename std::decay<U>::type, typename std::decay<Args>::type...> prepareArgs(lua_State* state, int index)
        {
            auto t = GetStackVar<typename std::decay<T>::type>()(state, index);
            auto others = prepareArgs<typename std::decay<U>::type, typename std::decay<Args>::type...>(state, index + 1);
            return std::tuple_cat(std::tuple<typename std::decay<T>::type>(t), std::move(others));
        }

        template <int N, typename... Args>
        struct CallPrepareArgs
        {
            std::tuple <typename std::decay<Args>::type...> operator()(lua_State* state, int index) const
            {
                return prepareArgs<Args...>(state, index);
            }
        };

        template <typename... Args>
        struct CallPrepareArgs<0, Args...>
        {
            std::tuple <typename std::decay<Args>::type...> operator()(lua_State* state, int index) const
            {
                return std::make_tuple();
            }
        };



        template <typename T>
        struct PushReturnValues
        {
            int operator()(lua_State* state, T& t)
            {
                pushVar(state, std::move(t));
                return 1;
            }
        };

        template <>
        struct PushReturnValues <std::vector<Object>>
        {
            int operator()(lua_State* state, std::vector <Object>& v)
            {
                for(unsigned i = 0; i < v.size(); ++i)
                    pushVar(state, std::move(v[i]));
                return v.size();
            }
        };

        template <typename R, typename F, typename Args>
        struct PushReturnValuesIfNotVoid
        {
            int operator()(lua_State* state, F func, Args&& args)
            {
                auto u = makeUnpacker(func, std::move(args));
                R r = u.call();
                return PushReturnValues<R>()(state, r);
            }
        };

        template <typename F, typename Args>
        struct PushReturnValuesIfNotVoid <void, F, Args>
        {
            int operator()(lua_State* state, F func, Args&& args)
            {
                auto u = makeUnpacker(func, std::move(args));
                u.call();
                return 0;
            }
        };

        //these helper functions allow the header to avoid including <lua.hpp>
        void* toUserData(lua_State* state, int upvalueindex);
        void throwLuaError(lua_State* state, const char* str);
        int getStackTop(lua_State* state);
        void getGlobal(lua_State* state, const char*);
        std::vector <Object> callLuaFunction(lua_State* state, int nargs);

        //A version of this function is used when functions are registered.
        template <typename R, typename... Args>
        int registeredCFunction(lua_State* state)
        {
            try
            {
                typedef R (*TypedFunction)(Args...);
                //get the actual function pointer from Lua's storage
                TypedFunction func = (TypedFunction)toUserData(state, 1);//lua_touserdata(state, lua_upvalueindex(1));
                //get the arguments from Lua's stack
                auto args = CallPrepareArgs<sizeof...(Args), Args...>()(state, 1);
                if(std::tuple_size<decltype(args)>::value != (unsigned)getStackTop(state))
                    throw type_mismatch("registeredCFunction");
                //call the function and push the return values onto the stack
                return PushReturnValuesIfNotVoid<R, TypedFunction, decltype(args)>()(state, func, std::move(args));
            }
            catch(const type_mismatch& e)
            {
                throwLuaError(state, "Native function: type mismatch");
            }
            catch(...)
            {
                throwLuaError(state, "Native function: unknown exception");
            }

            //this can't actually happen
            return 0;
        }


        template <typename T>
        void pushArgs(lua_State* state, T t)
        {
            PushVar<T>()(state, t);
        }

        template <typename S, typename T, typename... Args>
        void pushArgs(lua_State* state, S s, T t, Args... args)
        {
            PushVar<S>()(state, s);
            pushArgs <T, Args...>(state, t, args...);
        }
    }//namespace internal


    enum class Lib
    {
        base = 1,
        coroutine,
        table,
        io,
        os,
        string,
        bit32,
        math,
        debug,
        package,
        all
    };

    class State
    {
        void cleanup();

        void internal_registerFunction(const std::string& name, void* func, int(*registered)(lua_State*));

        lua_State* state;

    public:
        State();
        State(lua_State* s);

        State(const State& rhs) = delete;
        State& operator =(const State& rhs) = delete;

        State(State&& rhs);
        State& operator =(State&& rhs);

        ~State();

        //returns the native lua_State pointer
        lua_State* get() const;
        //Creates a new state, deleting any existing state first.
        //This is called automatically.
        void create();
        //Clears the script and destroys all of its state.
        //This is called automatically and usually is not needed.
        void destroy();


        //Throws std::invalid_argument if mode is invalid.
        void loadFile(const std::string& filename, const std::string& mode = "bt");
        void loadString(const std::string& script, const std::string& mode = "t");

        //Creates a new global object for the script to use.
        void setVariable(const std::string& name, const Object& object);//, const std::vector<std::string>& path = internal::emptyVector);

        //Returns the Lua global object with the specified name.
        //Returns a nil object if no such variable exists.
        //Any table entries (keys or values) contained in ignoreList are ignored, but only for the top-level table.
        //This is useful for getting a table of global values while removing recursive references (_G, base, and package)
        //Object getVariable(const std::string& name, const std::vector<std::string>& path = internal::emptyVector, const std::set<Object>& ignoreList = internal::emptySet) const;
        Object getVariable(const std::string& name, const std::set<Object>& ignoreList = internal::emptySet) const;

        //Runs the script, returning all the script's return values in a vector.
        std::vector <Object> run();

        //Call a global Lua function from C++ with the specified arguments.
        //This can generally be done only after calling run() to initialize the function variables.
        //Keep in mind that LuaNumber arguments must be doubles, not integers (unless reconfigured).
        //Throws uninitialized_resource if the State object is invalid.
        //Otherwise throws script_error if anything else goes wrong.
        template <typename... Args>
        std::vector <Object> call(const std::string& function, Args... args)
        {
            if(!state)
                throw uninitialized_resource("lua::State::registerFunction");

            internal::getGlobal(state, function.c_str());//lua_getglobal(state, function.c_str());
            internal::pushArgs(state, args...);

            return internal::callLuaFunction(state, sizeof...(Args));
        }

        //Registers a native function for the script to call.
        //The function can take any number of parameters and can return one value.
        //The parameters and return value can be any type accepted by Lua: doubles,
        //strings, map<Object,Object>, booleans, and LuaFunctions, or just void.
        //This will automatically convert the types to and from the Lua script, and
        //will throw an exception on failure.
        //Alternatively, the function may use generic Objects for some or all of its
        //parameters, allowing it to handle multiple types passed from the script.
        template <typename R, typename... Args>
        void registerFunction(const std::string& name, R(*func)(Args... args))
        {
            internal_registerFunction(name, (void*)func, internal::registeredCFunction<R, Args...>);
        }


        //Loads the specified library, assigning a name equal to the library's variable name.
        void loadLib(Lib lib);
        //Loads the specified library with the specified name.  name is ignored if the library "all" is specified.
        void loadLib(Lib lib, const std::string& name);
    };



}//namespace lua
