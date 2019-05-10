/*
 * =====================================================================================
 *
 *       Filename:  Utils.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/23/18 22:33:01
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Utils.hpp"


std::string random_string(size_t len)
{
     std::string str("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

     std::random_device rd;
     std::mt19937 generator(rd());

     std::shuffle(str.begin(), str.end(), generator);

     return str.substr(0, len);
}

bool does_file_exist(std::string fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

void print_vector(std::vector<uint8_t>& vec)
{

    std::stringstream s;
    for(auto i : vec){
        s << "0x" << std::hex << static_cast<unsigned int>(i) << " ";
    }
    s << "\n\n";
    std::cout << s.str();
}

std::shared_ptr<spdlog::logger> get_logger(std::string name)
{
    std::shared_ptr<spdlog::logger> logger;
    try
    {
        logger = spdlog::stdout_color_mt(name);
    }catch (spdlog::spdlog_ex)
    {
        logger = spdlog::get(name);
    }

    return logger;
}
