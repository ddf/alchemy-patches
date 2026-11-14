/**
 * Macro definitions from Jimmy Quadra's pull-request.
 * 
 * https://github.com/hermetic-modular/alchemy-template/pull/5
 */
#pragma once

/* Function-like macros to create a string literal from an object-like macro. */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* Fallback build version identifier if one is not supplied to the compiler by Make. */
#ifndef TARGET_GIT_VERSION
#define TARGET_GIT_VERSION unknown
#endif