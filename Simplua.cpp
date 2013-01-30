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

#include "Simplua.h"
#include <lua.hpp>

#include <iostream>
#include <fstream>
#include <sstream>

#include <cassert>
#include <cstdlib>

namespace lua
{
    namespace internal
    {
        void growStack(lua_State* state, int extra)
        {
            if(!lua_checkstack(state, extra))
                throw std::overflow_error("lua::internal::growStack");
        }

    } //namespace internal

    void Object::copy(const Object& rhs)
    {
        if(rhs.type == NUMBER)
            num = rhs.num;
        else if(rhs.type == STRING)
            str = rhs.str;
        else if(rhs.type == TABLE)
            table = rhs.table;
        else if(rhs.type == FUNCTION)
            func = rhs.func;
        else if(rhs.type == BOOLEAN)
            boolean = rhs.boolean;
        type = rhs.type;
    }

    void Object::moveFrom(Object& rhs)
    {
        type = rhs.type;
        if(type == NUMBER)
            num = rhs.num;
        else if(type == STRING)
            str = std::move(rhs.str);
        else if(type == TABLE)
            table = std::move(rhs.table);
        else if(type == FUNCTION)
            func = rhs.func;
        else if(type == BOOLEAN)
            boolean = rhs.boolean;
        rhs.type = NIL;
    }

    Object::Object()
    : type(NIL)
    {}

    Object::Object(const Object& rhs)
    : type(NIL)
    {
        copy(rhs);
    }

    Object& Object::operator =(const Object& rhs)
    {
        if(this == &rhs) return *this;

        copy(rhs);

        return *this;
    }

    Object::Object(Object&& rhs)
    : type(rhs.type)
    {
        moveFrom(rhs);
    }

    Object& Object::operator =(Object&& rhs)
    {
        if(this == &rhs) return *this;

        moveFrom(rhs);

        return *this;
    }

    Object Object::makeNil()
    {
        return Object();
    }

    Object Object::makeNumber(LuaNumber d)
    {
        Object o;
        o.type = NUMBER;
        o.num = d;
        return o;
    }

    Object Object::makeInteger(LuaInteger i)
    {
        Object o;
        o.type = NUMBER;
        o.num = static_cast<LuaNumber>(i);
        return o;
    }

    Object Object::makeString(const LuaString& s)
    {
        Object o;
        o.type = STRING;
        o.str = s;
        return o;
    }

    Object Object::makeTable(const LuaTable& m)
    {
        Object o;
        o.type = TABLE;
        o.table = m;
        return o;
    }

    Object Object::makeFunction(LuaFunction f)
    {
        Object o;
        o.type = FUNCTION;
        o.func = f;
        return o;
    }

    Object Object::makeBoolean(LuaBoolean b)
    {
        Object o;
        o.type = BOOLEAN;
        o.boolean = b;
        return o;
    }

    LuaNumber Object::getNumber() const
    {
        if(type != NUMBER)
            throw type_mismatch("Object::getNumber");
        return num;
    }

    LuaInteger Object::getInteger() const
    {
        if(!isInteger())
            throw type_mismatch("Object::getInteger");
        return static_cast<LuaInteger>(num);
    }

    const LuaString& Object::getString() const
    {
        if(type != STRING)
            throw type_mismatch("Object::getString");
        return str;
    }

    const LuaTable& Object::getTable() const
    {
        if(type != TABLE)
            throw type_mismatch("Object::getTable");
        return table;
    }

    LuaFunction Object::getFunction() const
    {
        if(type != FUNCTION)
            throw type_mismatch("Object::getFunction");
        return func;
    }

    LuaBoolean Object::getBoolean() const
    {
        if(type != BOOLEAN)
            throw type_mismatch("Object::getBoolean");
        return boolean;
    }

    bool Object::isNil() const
    {
        return type == NIL;
    }

    bool Object::isNumber() const
    {
        return type == NUMBER;
    }

    bool Object::isInteger() const
    {
        return type == NUMBER && num == static_cast<LuaNumber>(static_cast<LuaInteger>(num));
    }

    bool Object::isString() const
    {
        return type == STRING;
    }

    bool Object::isTable() const
    {
        return type == TABLE;
    }

    bool Object::isFunction() const
    {
        return type == FUNCTION;
    }

    bool Object::isBoolean() const
    {
        return type == BOOLEAN;
    }

    int Object::getType() const
    {
        return type;
    }

    bool Object::operator <(const Object& rhs) const
    {
        if(type == rhs.type)
        {
            if(type == NIL)
                return false;
            if(type == NUMBER)
                return num < rhs.num;
            if(type == STRING)
                return str < rhs.str;
            if(type == TABLE)
                return table < rhs.table;
            if(type == FUNCTION)
                return func < rhs.func;
            if(type == BOOLEAN)
                return !boolean && rhs.boolean;
            //if(type == USERDATA)
            //    return (intptr_t)userdata < (intptr_t)rhs.userdata;
            //if(type == THREAD)
            //    return (intptr_t)thread < (intptr_t)rhs.thread;
        }

        return type < rhs.type;
    }

    bool Object::operator >(const Object& rhs) const
    {
        return rhs < *this && !(*this == rhs);
    }

    bool Object::operator <=(const Object& rhs) const
    {
        return !(rhs < *this);
    }

    bool Object::operator >=(const Object& rhs) const
    {
        return !(*this < rhs);
    }

    bool Object::operator ==(const Object& rhs) const
    {
        if(type == rhs.type)
        {
            if(type == NIL)
                return true;
            if(type == NUMBER)
                return num == rhs.num;
            if(type == STRING)
                return str == rhs.str;
            if(type == TABLE)
                return table == rhs.table;
            if(type == FUNCTION)
                return func == rhs.func;
            if(type == BOOLEAN)
                return boolean == rhs.boolean;
            //if(type == USERDATA)
            //    return (intptr_t)userdata == (intptr_t)rhs.userdata;
            //if(type == THREAD)
            //    return (intptr_t)thread == (intptr_t)rhs.thread;
        }

        return false;
    }

    bool Object::operator !=(const Object& rhs) const
    {
        return !(*this == rhs);
    }

    namespace internal
    {
        void printObject(std::ostream& out, const Object& obj, int indents = 2)
        {
            if(obj.isNil())
                out << "nil";
            if(obj.isNumber())
                out << obj.getNumber();
            if(obj.isString())
                out << obj.getString();
            if(obj.isBoolean())
                out << (obj.getBoolean() == false ? "false" : "true");
            if(obj.isTable())
            {
                out << "Table:\n";
                std::string indent(indents, ' ');
                if(indents == 0)
                    indent = "";
                for(auto& p : obj.getTable())
                {
                    out << indent;
                    printObject(out, p.first, indents + 2);
                    out << ": ";
                    printObject(out, p.second, indents + 2);
                    if(!p.second.isTable())
                        out << '\n';
                }
            }
            if(obj.isFunction())
                out << "Function";
        }
    }//namespace internal

    std::ostream& operator <<(std::ostream& out, const Object& obj)
    {
        internal::printObject(out, obj);

        return out;
    }




    void State::cleanup()
    {
        if(state)
        {
            lua_close(state);
            state = nullptr;
        }
    }

    State::State()
    : state(nullptr)
    {
        create();
    }

    State::State(lua_State* s)
    : state(s)
    {}

    State::State(State&& rhs)
    : state(rhs.state)
    {
        rhs.state = nullptr;
    }

    State& State::operator =(State&& rhs)
    {
        if(this == &rhs)
            return *this;

        cleanup();
        state = rhs.state;
        rhs.state = nullptr;

        return *this;
    }

    State::~State()
    {
        cleanup();
    }

    lua_State* State::get() const
    {
        return state;
    }

    void State::create()
    {
        cleanup();
        state = luaL_newstate();
    }

    void State::destroy()
    {
        cleanup();
    }


    //Helper functions for State::loadFile
    static std::string readFile(const std::string& filename)
    {
        std::ifstream t(filename, std::ios::binary);
        if(!t.is_open())
            throw compile_error("lua::readFile");
        t.seekg(0, std::ios::end);
        size_t size = t.tellg();
        std::string buffer(size, ' ');
        t.seekg(0);
        t.read(&buffer[0], size);

        return buffer;
    }

    struct TrivialLuaReaderData
    {
        bool first;
        std::string text;
    };

    static const char* trivialLuaReader(lua_State*, void* d, std::size_t* size)
    {
        TrivialLuaReaderData* data = reinterpret_cast<TrivialLuaReaderData*>(d);

        if(!data->first)
            return nullptr;

        data->first = false;
        *size = data->text.size();
        return data->text.c_str();
    }


    void State::loadFile(const std::string& filename, const std::string& mode)
    {
        if(!state)
            throw uninitialized_resource("lua::State::loadFile");
        if(mode != "b" && mode != "t" && mode != "bt" && mode != "tb")
            throw std::invalid_argument("lua::State::loadFile");

        TrivialLuaReaderData trivialData;
        trivialData.first = true;
        trivialData.text = readFile(filename);
        int ret = lua_load(state, trivialLuaReader, (void*)&trivialData, filename.c_str(), mode.c_str());

        if(ret != LUA_OK)
        {
            Object err = internal::GetStackVar<Object>()(state, -1);
            lua_pop(state, 1);
            std::stringstream ss;
            ss << "lua::State::loadFile - " << err;
            throw compile_error(ss.str());
        }
    }

    void State::loadString(const std::string& script, const std::string& mode)
    {
        if(!state)
            throw uninitialized_resource("lua::State::loadString");
        if(mode != "b" && mode != "t" && mode != "bt" && mode != "tb")
            throw std::invalid_argument("lua::State::loadFile");

        TrivialLuaReaderData trivialData;
        trivialData.first = true;
        trivialData.text = script;
        int ret = lua_load(state, trivialLuaReader, (void*)&trivialData, "string_script", mode.c_str());

        if(ret != LUA_OK)
        {
            Object err = internal::GetStackVar<Object>()(state, -1);
            lua_pop(state, 1);
            std::stringstream ss;
            ss << "lua::State::loadFile - " << err;
            throw compile_error(ss.str());
        }
    }

    void State::setVariable(const std::string& name, const Object& object)//, const std::vector<std::string>& path)
    {
        if(!state)
            throw uninitialized_resource("lua::State::makeGlobal");

        int index = lua_gettop(state);


        std::size_t period = name.find('.');
        if(period != std::string::npos)
        {
            lua_getglobal(state, name.substr(0, period).c_str());

            std::size_t last = period + 1;
            period = name.find('.', period + 1);
            while(period != std::string::npos)
            {
                lua_getfield(state, -1, name.substr(last, period - last).c_str());
                last = period + 1;
                period = name.find('.', period + 1);
            }

            internal::growStack(state, 2);
            lua_pushstring(state, name.substr(last).c_str());
            internal::pushVar(state, object);
            lua_settable(state, -3);
        }
        else
        {
            internal::pushVar(state, object);
            lua_setglobal(state, name.c_str());
        }

        lua_settop(state, index);
    }

    Object State::getVariable(const std::string& name, const std::set<Object>& ignoreList) const
    {
        if(!state)
            throw uninitialized_resource("lua::State::makeGlobal");

        std::size_t period = name.find('.');
        if(period != std::string::npos)
        {
            lua_getglobal(state, name.substr(0, period).c_str());

            std::size_t last = period + 1;
            period = name.find('.', period + 1);
            while(period != std::string::npos)
            {
                lua_getfield(state, -1, name.substr(last, period - last).c_str());
                last = period + 1;
                period = name.find('.', period + 1);
            }

            lua_getfield(state, -1, name.substr(last).c_str());
        }
        else
            lua_getglobal(state, name.c_str());

        Object o = internal::GetStackVar<Object>()(state, -1, ignoreList);
        lua_pop(state, 1);
        return o;
    }



    std::vector <Object> State::run()
    {
        if(!state)
            throw uninitialized_resource("lua::State::run");

        if(lua_pcall(state, 0, LUA_MULTRET, 0) != LUA_OK)
        {
            Object err = internal::GetStackVar<Object>()(state, -1);
            lua_pop(state, 1);
            std::stringstream ss;
            ss << "lua::State::run - " << err;
            throw script_error(ss.str());
        }

        unsigned retsLeft = lua_gettop(state);
        std::vector <Object> ret(retsLeft);
        while(retsLeft > 0)
        {
            Object obj = internal::GetStackVar<Object>()(state, -1);
            lua_pop(state, 1);
            ret[--retsLeft] = std::move(obj);
        }

        return ret;
    }


    static LuaFunction getLibraryFunction(Lib lib)
    {
        switch(lib)
        {
            case Lib::base:
                return luaopen_base;
            case Lib::coroutine:
                return luaopen_coroutine;
            case Lib::table:
                return luaopen_table;
            case Lib::io:
                return luaopen_io;
            case Lib::os:
                return luaopen_os;
            case Lib::string:
                return luaopen_string;
            case Lib::bit32:
                return luaopen_bit32;
            case Lib::math:
                return luaopen_math;
            case Lib::debug:
                return luaopen_debug;
            case Lib::package:
                return luaopen_package;
            default:
                return luaopen_base; //this should not happen
        }
    }

    static std::string getLibraryName(Lib lib)
    {
        switch(lib)
        {
            case Lib::base:
                return "base";
            case Lib::coroutine:
                return "coroutine";
            case Lib::table:
                return "table";
            case Lib::io:
                return "io";
            case Lib::os:
                return "os";
            case Lib::string:
                return "string";
            case Lib::bit32:
                return "bit32";
            case Lib::math:
                return "math";
            case Lib::debug:
                return "debug";
            case Lib::package:
                return "package";
            default:
                return "Error:getLibraryName"; //this should not happen
        }
    }

    void State::loadLib(Lib lib)
    {
        if(lib == Lib::all)
        {
            loadLib(Lib::base);
            loadLib(Lib::coroutine);
            loadLib(Lib::table);
            loadLib(Lib::io);
            loadLib(Lib::os);
            loadLib(Lib::string);
            loadLib(Lib::bit32);
            loadLib(Lib::math);
            loadLib(Lib::debug);
            loadLib(Lib::package);
        }
        else
        {
            LuaFunction f = getLibraryFunction(lib);
            std::string name = getLibraryName(lib);
            luaL_requiref(state, name.c_str(), f, 1);
            lua_pop(state, 1);
        }
    }

    void State::loadLib(Lib lib, const std::string& name)
    {
        if(lib == Lib::all)
        {
            loadLib(Lib::base);
            loadLib(Lib::coroutine);
            loadLib(Lib::table);
            loadLib(Lib::io);
            loadLib(Lib::os);
            loadLib(Lib::string);
            loadLib(Lib::bit32);
            loadLib(Lib::math);
            loadLib(Lib::debug);
            loadLib(Lib::package);
        }
        else
        {
            LuaFunction f = getLibraryFunction(lib);
            luaL_requiref(state, name.c_str(), f, 1);
            lua_pop(state, 1);
        }
    }



    namespace internal
    {
        LuaString emptyString;
        LuaTable emptyTable;

        std::set <Object> emptySet;
        std::vector <std::string> emptyVector;

        void* toUserData(lua_State* state, int upvalueindex)
        {
            return lua_touserdata(state, lua_upvalueindex(upvalueindex));
        }

        void throwLuaError(lua_State* state, const char* str)
        {
            if(lua_checkstack(state, 1))
                lua_pushstring(state, str);
            lua_error(state);
        }

        int getStackTop(lua_State* state)
        {
            return lua_gettop(state);
        }

        void getGlobal(lua_State* state, const char* function)
        {
            lua_getglobal(state, function);
        }

        std::vector <Object> callLuaFunction(lua_State* state, int nargs)
        {
            if(lua_pcall(state, nargs, LUA_MULTRET, 0) != 0)
            {
                Object err = internal::GetStackVar<Object>()(state, -1);
                lua_pop(state, 1);
                std::stringstream ss;
                ss << "lua::State::call - " << err;
                throw script_error(ss.str());
            }

            unsigned retsLeft = lua_gettop(state);
            std::vector <Object> ret(retsLeft);
            while(retsLeft > 0)
            {
                Object obj = internal::GetStackVar<Object>()(state, -1);
                lua_pop(state, 1);
                ret[--retsLeft] = std::move(obj);
            }

            return ret;
        }


        void PushVar<Object>::operator()(lua_State* state, Object object) const
        {
            internal::growStack(state, 1);
            switch(object.getType())
            {
                case Object::NIL:
                    lua_pushnil(state);
                    break;
                case Object::NUMBER:
                    lua_pushnumber(state, object.getNumber());
                    break;
                case Object::STRING:
                    lua_pushstring(state, object.getString().c_str());
                    break;
                case Object::TABLE:
                    lua_newtable(state);
                    for(auto& p : object.getTable())
                    {
                        pushVar(state, p.first);
                        pushVar(state, p.second);

                        lua_settable(state, -3);
                    }
                    break;
                case Object::FUNCTION:
                    lua_pushcfunction(state, object.getFunction());
                    break;
                case Object::BOOLEAN:
                    lua_pushboolean(state, object.getBoolean());
                    break;
                default:
                    throw type_mismatch("lua::PushVar<Object>");
                    break;
            }
        }

        void PushVar<LuaNumber>::operator()(lua_State* state, LuaNumber d) const
        {
            internal::growStack(state, 1);
            lua_pushnumber(state, d);
        }

        void PushVar<LuaInteger>::operator()(lua_State* state, LuaInteger i) const
        {
            internal::growStack(state, 1);
            lua_pushnumber(state, static_cast<LuaNumber>(i));
        }

        void PushVar<LuaString>::operator()(lua_State* state, const LuaString& s) const
        {
            internal::growStack(state, 1);
            lua_pushstring(state, s.c_str());
        }

        void PushVar<LuaTable>::operator()(lua_State* state, const LuaTable& t) const
        {
            internal::growStack(state, 1);
            lua_newtable(state);
            for(auto& p : t)
            {
                pushVar(state, p.first);
                pushVar(state, p.second);
                lua_settable(state, -3);
            }
        }

        void PushVar<LuaFunction>::operator()(lua_State* state, LuaFunction f) const
        {
            internal::growStack(state, 1);
            lua_pushcfunction(state, f);
        }

        void PushVar<LuaBoolean>::operator()(lua_State* state, LuaBoolean b) const
        {
            internal::growStack(state, 1);
            lua_pushboolean(state, b);
        }


        Object GetStackVar<Object>::operator()(lua_State* state, int index, const std::set<Object>& ignoreList, int level) const
        {
            if(level <= 0)
            {
                #ifdef LUA_THROW_TABLE_TOO_DEEP
                throw table_too_deep("lua::GetStackVar<Object>::operator()");
                #else
                return Object::makeNil();
                #endif
            }

            index = lua_absindex(state, index);

            Object obj;
            int type = lua_type(state, index);
            switch(type)
            {
                case LUA_TNIL:
                    break;
                case LUA_TNUMBER:
                    obj = Object::makeNumber(lua_tonumber(state, index));
                    break;
                case LUA_TSTRING:
                    obj = Object::makeString(lua_tostring(state, index));
                    break;
                case LUA_TTABLE:
                {
                    std::map <Object, Object> table;

                    internal::growStack(state, 2);
                    lua_pushnil(state);
                    while (lua_next(state, index) != 0)
                    {
                        Object key = GetStackVar<Object>()(state, -2, emptySet, level - 1);
                        if(ignoreList.find(key) != ignoreList.end())
                        {
                            lua_pop(state, 1);
                            continue;
                        }
                        Object value = GetStackVar<Object>()(state, -1, emptySet, level - 1);
                        if(ignoreList.find(value) != ignoreList.end())
                        {
                            lua_pop(state, 1);
                            continue;
                        }
                        table[std::move(key)] = std::move(value);
                        lua_pop(state, 1);
                    }

                    obj = Object::makeTable(table);

                    break;
                }
                case LUA_TBOOLEAN:
                    obj = Object::makeBoolean(lua_toboolean(state, index));
                    break;
                case LUA_TFUNCTION:
                    obj = Object::makeFunction(lua_tocfunction(state, index));
                    break;
                case LUA_TTHREAD:
                case LUA_TLIGHTUSERDATA:
                case LUA_TUSERDATA:
                default:
                    std::cout << "Tried to get an Object of unsupported type: " << type << std::endl;
                    //throw type_mismatch("lua::GetStackVar<Object>");
            }
            return obj;
        }

        LuaNumber GetStackVar<LuaNumber>::operator()(lua_State* state, int index) const
        {
            if(!lua_isnumber(state, index))
                throw type_mismatch("lua::GetStackVar<LuaNumber>");
            return lua_tonumber(state, index);
        }

        LuaInteger GetStackVar<LuaInteger>::operator()(lua_State* state, int index) const
        {
            if(!lua_isnumber(state, index))
                throw type_mismatch("lua::GetStackVar<LuaInteger>");

            LuaNumber n = lua_tonumber(state, index);
            LuaInteger i = static_cast<LuaInteger>(n);

            if(static_cast<LuaNumber>(i) != n)
                throw type_mismatch("lua::GetStackVar<LuaInteger>");

            return i;
        }

        LuaString GetStackVar<LuaString>::operator()(lua_State* state, int index) const
        {
            if(!lua_isstring(state, index))
                throw type_mismatch("lua::GetStackVar<LuaString>");
            return lua_tostring(state, index);
        }

        LuaTable GetStackVar<LuaTable>::operator()(lua_State* state, int index, int level) const
        {
        	//don't try to duplicate this functionality
        	return GetStackVar<Object>()(state, index, internal::emptySet, level).getTable();
        	/*
            if(level <= 0)
                throw table_too_deep("lua::GetStackVar<Object>::operator()");

            if(!lua_istable(state, index))
                throw type_mismatch("lua::GetStackVar<LuaFunction>");
            std::map <Object, Object> table;

            internal::growStack(state, 2);
            lua_pushnil(state);
            while (lua_next(state, index) != 0)
            {
                Object key = GetStackVar<Object>()(state, -2, emptySet, level - 1);
                if(key == Object::makeString("_G") || key == Object::makeString("base"))
                {
                    lua_pop(state, 1);
                    continue;
                }
                Object value = GetStackVar<Object>()(state, -1, emptySet, level - 1);
                table[std::move(key)] = std::move(value);
                lua_pop(state, 1);
            }

            return table;
            */
        }

        LuaFunction GetStackVar<LuaFunction>::operator()(lua_State* state, int index) const
        {
            if(!lua_iscfunction(state, index))
                throw type_mismatch("lua::GetStackVar<LuaFunction>");
            return lua_tocfunction(state, index);
        }

        LuaBoolean GetStackVar<LuaBoolean>::operator()(lua_State* state, int index) const
        {
            if(!lua_isboolean(state, index))
                throw type_mismatch("lua::GetStackVar<LuaBoolean>");
            return lua_toboolean(state, index);
        }
        
        
        void registerFunction(lua_State* state, const std::string& name, void* func, int(*registered)(lua_State*))
    	{
        	if(!state)
            	throw uninitialized_resource("lua::State::registerFunction");

        	int index = lua_gettop(state);

        	std::size_t period = name.find('.');
        	if(period != std::string::npos)
        	{
            	lua_getglobal(state, name.substr(0, period).c_str());

            	std::size_t last = period + 1;
            	period = name.find('.', period + 1);
            	while(period != std::string::npos)
            	{
                	lua_getfield(state, -1, name.substr(last, period - last).c_str());
                	last = period + 1;
                	period = name.find('.', period + 1);
            	}

            	internal::growStack(state, 2);
            	lua_pushstring(state, name.substr(last).c_str());
            	lua_pushlightuserdata(state, func);
            	lua_pushcclosure(state, registered, 1);
            	lua_settable(state, -3);
        	}
        	else
        	{
            	internal::growStack(state, 2);
            	lua_pushlightuserdata(state, func);
            	lua_pushcclosure(state, registered, 1);
            	lua_setglobal(state, name.c_str());
        	}

        	lua_settop(state, index);
    	}
    }//namespace internal
}//namespace lua
