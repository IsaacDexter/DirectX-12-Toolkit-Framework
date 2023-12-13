#pragma once

#include "stdafx.h"

//https://github.com/microsoft/DirectXTK12/wiki/Using-DeviceResources

class Engine
{
public:
	Engine();
	~Engine();
	void Initialize(HWND window, int width, int height);
};

