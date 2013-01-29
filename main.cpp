#include <iostream>
#include <vector>
#include <map>
#include <cassert>

#include "Simplua.h"

int testFunc(double d, std::string s)
{
    std::cout << "The native function received this text: " << s << std::endl;
    return static_cast<int>(2 * d);
}

void testLua()
{
    lua::State state;
    state.loadFile("script.lua");
    state.loadLib(lua::Lib::all);
    state.setVariable("myLib", lua::Object::makeTable());
    state.registerFunction("myLib.testFunc", testFunc);
    state.run();

    std::set <lua::Object> ignore{lua::Object::makeString("_G"), lua::Object::makeString("base"), lua::Object::makeString("package")};
    std::cout << state.getVariable("_G", ignore) << std::endl;
}


int main(int, char**)
{
    try
    {
        testLua();
        return 0;
    }
    catch(const lua::type_mismatch& e)
    {
        std::cerr << "Caught a type mismatch: " << e.what() << std::endl;
    }
    catch(const lua::compile_error& e)
    {
        std::cerr << "Caught a compile error:\n " << e.what() << std::endl;
    }
    catch(const lua::table_too_deep& e)
    {
        std::cerr << "Caught a recursive table: " << e.what() << std::endl;
    }
    catch(const lua::script_error& e)
    {
        std::cerr << "Caught a script error: " << e.what() << std::endl;
    }
    catch(const lua::uninitialized_resource& e)
    {
        std::cerr << "Caught an uninitialized resource: " << e.what() << std::endl;
    }
    catch(const std::invalid_argument& e)
    {
        std::cerr << "Caught an std::invalid_argument: " << e.what() << std::endl;
    }
    catch(const std::bad_alloc& e)
    {
        std::cerr << "Caught a std::bad_alloc (out of memory?): " << e.what() << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << "Caught a different exception: " << e.what() << std::endl;
    }
    catch(...)
    {
        std::cerr << "Caught something else" << std::endl;
    }
    return 1;
}
