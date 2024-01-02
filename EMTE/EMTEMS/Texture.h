#pragma once

#include "pch.h"

/// <summary>Stores a ID3D12Resource of a texture and its descriptor</summary>
struct Texture
{
    Microsoft::WRL::ComPtr<ID3D12Resource> resource;
    size_t descriptor;
};

