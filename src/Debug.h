#pragma once

#include <iostream>

// Global debug flag
extern bool g_debugMode;

// Debug output macros
#define DEBUG_PRINT(msg) \
    do { \
        if (g_debugMode) { \
            std::cout << msg << std::endl; \
        } \
    } while(0)

#define DEBUG_PRINT_STREAM(stream) \
    do { \
        if (g_debugMode) { \
            std::cout << stream << std::endl; \
        } \
    } while(0)

// Essential output (always printed)
#define ESSENTIAL_PRINT(msg) \
    do { \
        std::cout << msg << std::endl; \
    } while(0)

#define ESSENTIAL_PRINT_STREAM(stream) \
    do { \
        std::cout << stream << std::endl; \
    } while(0)
