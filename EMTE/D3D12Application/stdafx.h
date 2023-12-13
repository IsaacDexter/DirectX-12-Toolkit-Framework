#pragma once

#pragma region WindowsHeaders

// exclude rarely-used stuff from Windows headers.
#define WIN32_LEAN_AND_MEAN 
#include <Windows.h>

// for CommandLineToArgvW, used to parse command line arguments passed to application
#include <shellapi.h>

// min/max macros conflict with member functions, only use std::min & std::max from <algorithm>
#ifdef min 
#undef min
#endif//min
#ifdef max 
#undef max
#endif //max

// to create a CreateWindow function, windows macro needs undefining. CreateWindowExW is used instead of OS create window
#ifdef CreateWindow
#undef CreateWindow
#endif // CreateWindow

// Windows Runtime Library, needed for ComPtr<> template class
#include <wrl.h>
using namespace Microsoft::WRL;

#pragma endregion

#pragma region DX12Headers

// contains all D3D12 objects
#include <d3d12.h>

// DirectX Graphics Infrastructure used to manage low-level tasks like presenting rendered images
#include <dxgi1_6.h>

// allows HLSL compilation at runtime
#include <d3dcompiler.h>

//SIMD-friendly C++ types and functions
#include <DirectXMath.h>

//Provides useful classes and simplifies functions
#include <d3dx12.h>

//Collection of helper classes from dxtk12
#include <DirectXHelpers.h>

#pragma endregion

#pragma region STLHeaders

//Math related functions
#include <algorithm>

//Contains assert macro
#include <cassert>

//Used performing timing between update calls
#include <chrono>

#pragma endregion

#if defined(_DEBUG)

#include "helpers.h"

#endif // !DEBUG

