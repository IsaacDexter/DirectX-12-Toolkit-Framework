#pragma once
#include "pch.h"

class Sprite
{
private:
    /// <summary>The texture to render as a sprite.</summary>
    Microsoft::WRL::ComPtr<ID3D12Resource> m_texture;
    /// <summary>The descriptor CPU handle to be obtained from the resource vector.</summary>
    size_t m_descriptor;
    /// <summary>The 2D screen position of the 2D sprite.</summary>
    DirectX::SimpleMath::Vector2 m_position;
    DirectX::XMUINT2 m_size;

private:
    void SetDescriptor()
    {
        m_descriptor = 1;
    }
    ///// <summary><para>Using an existing resourceUpload batch, load a texture from a file name and set it as this device's texture</para>
    ///// <para>Not recommended, load textures seperately in their own batch</para></summary>
    ///// <param name="device">pointer to the ID3D12Device used to load the texture.</param>
    ///// <param name="resourceUpload">ResourceUpload that has already begun, and is to be ended seperately after a bulk of sprites are loaded.</param>
    ///// <param name="szFileName">The path to the dds texture to be loaded</param>
    //void LoadTexture(ID3D12Device* device, DirectX::ResourceUploadBatch& resourceUpload, const wchar_t* szFileName);
    
    /// <summary>Create Shader resource view with given texture and descriptor heap</summary>
    /// <param name="device">D3D device</param>
    /// <param name="descriptorHeap">descriptor heap to provide a CPU handle to the descriptor</param>
    void CreateSRV(ID3D12Device* device, DirectX::DescriptorHeap* descriptorHeap);
    inline void SetSize()
    {
        m_size = DirectX::GetTextureSize(m_texture.Get());
    }
public:
    inline const size_t& GetDescriptor()
    {
        return m_descriptor;
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
    Sprite(ID3D12Device* device, Microsoft::WRL::ComPtr<ID3D12Resource> texture, DirectX::DescriptorHeap* descriptorHeap);
    ///// <summary><para>Creates a sprite by loading its texture from a given path.</para>
    ///// <para>Not recommended, load textures seperately in their own batch</para></summary>
    ///// <param name="device">pointer to the ID3D12Device used to load the texture.</param>
    ///// <param name="resourceUpload">ResourceUpload that has already begun, and is to be ended seperately after a bulk of sprites are loaded.</param>
    ///// <param name="szFileName">The path to the dds texture to be loaded</param>
    //Sprite(ID3D12Device* device, DirectX::ResourceUploadBatch& resourceUpload, const wchar_t* szFileName, std::unique_ptr<DirectX::DescriptorHeap> descriptorHeap);
    
    ~Sprite();

};

