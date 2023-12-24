#pragma once
#include "pch.h"
#include "Texture.h"

class Sprite
{
private:
    std::shared_ptr<Texture> m_texture;
    /// <summary>The 2D screen position of the 2D sprite.</summary>
    DirectX::SimpleMath::Vector2 m_position;
    DirectX::XMUINT2 m_size;

private:
    ///// <summary><para>Using an existing resourceUpload batch, load a texture from a file name and set it as this device's texture</para>
    ///// <para>Not recommended, load textures seperately in their own batch</para></summary>
    ///// <param name="device">pointer to the ID3D12Device used to load the texture.</param>
    ///// <param name="resourceUpload">ResourceUpload that has already begun, and is to be ended seperately after a bulk of sprites are loaded.</param>
    ///// <param name="szFileName">The path to the dds texture to be loaded</param>
    //void LoadTexture(ID3D12Device* device, DirectX::ResourceUploadBatch& resourceUpload, const wchar_t* szFileName);
    
    inline void SetSize();
public:
    inline std::shared_ptr<Texture> GetTexture()
    {
        return m_texture;
    }
    inline void SetSize(UINT x, UINT y)
    {
        m_size = DirectX::XMUINT2(x, y);
    }
    inline const DirectX::XMUINT2& GetSize()
    {
        return m_size;
    }
    /// <summary>A sprite is a 2D image of a given size and position with on the screen that displays a texture.</summary>
    /// <param name="device">pointer to the ID3D12Device used to load the texture.</param>
    /// <param name="descriptorHeap">Descriptor heap used to store the descriptors to the SRV in the CPU</param>
    Sprite(std::shared_ptr<Texture> texture);
    ///// <summary><para>Creates a sprite by loading its texture from a given path.</para>
    ///// <para>Not recommended, load textures seperately in their own batch</para></summary>
    ///// <param name="device">pointer to the ID3D12Device used to load the texture.</param>
    ///// <param name="resourceUpload">ResourceUpload that has already begun, and is to be ended seperately after a bulk of sprites are loaded.</param>
    ///// <param name="szFileName">The path to the dds texture to be loaded</param>
    //Sprite(ID3D12Device* device, DirectX::ResourceUploadBatch& resourceUpload, const wchar_t* szFileName, std::unique_ptr<DirectX::DescriptorHeap> descriptorHeap);
    
    ~Sprite();

};

