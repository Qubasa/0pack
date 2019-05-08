/*
 * =====================================================================================
 *
 *       Filename:  RegisterManager.cpp
 *
 *    Description:  Keeps track of used registers in asm functions
 *
 *        Version:  1.0
 *        Created:  20.12.2018 11:44:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Luis Hebendanz (),
 *   Organization:
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include "RegisterManager.hpp"
#include <iostream>

namespace Obfuscator {
namespace ELF {

RegisterManager::RegisterManager()
{
    _logger = get_logger("RegisterManager");
}

Register& RegisterManager::get_by_str(std::string name)
{
    for(auto i = all_registers.begin(); i != all_registers.end(); i++)
    {
        Register& current = i->second;
        if(current.name.compare(name))
        {
            return current;
        }
    }

    throw std::logic_error("Register '" + name + "' not found!");
}

Register& RegisterManager::get_by_enum(RegName name)
{

    auto current = all_registers.find(name);

    if(current == all_registers.end())
    {
        throw std::logic_error("Register enum '" + std::to_string(static_cast<int>(name)) + "' not found!");
    }

    return current->second;
}

void RegisterManager::setRegisterHolding(RegName name, bool holding)
{
    Register& reg = get_by_enum(name);
    reg.holdsValue = holding;
}

bool RegisterManager::getRegisterHolding(RegName name)
{
    Register& reg = get_by_enum(name);
    return reg.holdsValue;
}


std::string RegisterManager::get_name(RegName name)
{
    return get_by_enum(name).name;
}

std::vector<std::reference_wrapper<Register>> RegisterManager::disjoint(std::initializer_list<RegName> black_list)
{
    std::vector<std::reference_wrapper<Register>> disjoint = {};

    for(auto d = all_registers.begin(); d != all_registers.end(); d++)
    {
        Register& current = d->second;

        bool matches = false;
        for(auto i = black_list.begin(); i != black_list.end(); i++)
        {
            Register& current_blacklist = get_by_enum(*i);

            if(current_blacklist.name == current.name)
            {
                matches = true;
                break;
            }
        }

        if(!matches)
        {
            disjoint.push_back(current);
        }
    }

    return disjoint;
}

}
}

