#pragma once
#include "pch.h"
#include "Texture.h"

/// <summary>Stores a shared pointer to a texture from TextureGroup, and the position and size of the sprite</summary>
struct Sprite
{
    std::shared_ptr<Texture> texture;
    /// <summary>The 2D screen position of the 2D sprite.</summary>
    DirectX::SimpleMath::Vector2 position;
    DirectX::XMUINT2 size;
    Sprite()
    {
        texture = nullptr;
        position = DirectX::SimpleMath::Vector2::Zero;
        size = DirectX::XMUINT2();
    }
    Sprite(std::shared_ptr<Texture> texture)
    {
        this->texture = texture;
        position = DirectX::SimpleMath::Vector2::Zero;
        ResetSize();
    }
    /// <summary>Set the size to that of the texture</summary>
    inline void ResetSize()
    {
        size = DirectX::GetTextureSize(texture->resource.Get());
    }
};

