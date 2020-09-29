/***************************************************************************
    General C++ Helper Functions

    Copyright Chris White.
    See license.txt for more details.
***************************************************************************/

#pragma once

#include <string>
#include "stdint.hpp"

class Utils
{
public:
    static std::string to_string(int i);
    static std::string to_string(char c);
    static std::string to_hex_string(int i);
    static uint32_t from_hex_string(std::string s);
    static long map(long x, long in_min, long in_max, long out_min, long out_max);

private:
};