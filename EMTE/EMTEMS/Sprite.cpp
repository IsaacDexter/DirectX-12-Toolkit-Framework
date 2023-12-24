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

inline void Sprite::SetSize()
{
    m_size = DirectX::GetTextureSize(m_texture->resource.Get());
}

Sprite::Sprite(std::shared_ptr<Texture> texture)
{
    m_texture = texture;
    SetSize();
}

Sprite::~Sprite()
{
}
