/*
* MIT License
*
* Copyright (c) 2024 Edgar Bernal (edgar.bernal@gmail.com)
*/

#ifndef BT_PCH_HEADER
#define BT_PCH_HEADER

#ifdef BT_PLATFORM_WINDOWS

//#define NOMINMAX
#include <Windows.h>
#endif

#include <iostream>
#include <string>
#include <vector>

#include "Berta/Core/Log.h"
#include "Berta/Core/Assert.h"
#include "Berta/Core/Backend.h"

#endif