/*
 * =====================================================================================
 *
 *       Filename:  Utils.hpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/23/18 22:43:00
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef _Utils_
#define _Utils_

#include <stdlib.h>
#include <string>
#include "spdlog/sinks/stdout_color_sinks.h" //support for stdout logging
#include <random>


std::string random_string( size_t length );
bool does_file_exist(std::string fileName);
void print_vector(std::vector<uint8_t>& vec);
std::shared_ptr<spdlog::logger> get_logger(std::string name);

#endif

