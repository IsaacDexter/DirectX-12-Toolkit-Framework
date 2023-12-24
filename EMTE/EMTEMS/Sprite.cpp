#include "Sprite.h"

//void Sprite::LoadTexture(ID3D12Device* device, DirectX::ResourceUploadBatch& resourceUpload, const wchar_t* szFileName)
//{
//    DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(
//        device,
//        resourceUpload, // ResourceUploadBatch to upload texture to GPU
//        szFileName, // Path of the texture to load
//        m_texture.ReleaseAndGetAddressOf()  // Release the interface associated with comptr and retreieve a pointer to the released interface to store the texture into
//    ));
//}

void Sprite::CreateSRV(ID3D12Device* device, DirectX::DescriptorHeap* descriptorHeap)
{
    // Create a shader resource view, which describes the properties of a texture
    DirectX::CreateShaderResourceView(
        device,
        m_texture.Get(),    // Pointer to resource object that represents the Shader Resource
        descriptorHeap->GetCpuHandle(m_descriptor)   // Descriptor handle that represents the SRV 
    );
}

Sprite::Sprite(ID3D12Device* device, Microsoft::WRL::ComPtr<ID3D12Resource> texture, DirectX::DescriptorHeap* descriptorHeap)
{
    m_texture = texture;
    SetSize();
    SetDescriptor();
    CreateSRV(device, descriptorHeap);
}

Sprite::~Sprite()
{
    m_texture.Reset();
}
