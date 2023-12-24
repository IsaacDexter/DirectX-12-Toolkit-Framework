#pragma once

#include "pch.h"

struct Texture
{
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    size_t descriptor;
};