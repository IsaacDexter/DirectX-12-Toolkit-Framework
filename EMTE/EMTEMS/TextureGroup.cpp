#include "TextureGroup.h"

inline void TextureGroup::LoadTexture(const wchar_t* szFileName, DirectX::ResourceUploadBatch& resourceUpload)
{
    auto device = m_deviceResources->GetD3DDevice();
    std::shared_ptr<Texture> texture = std::make_shared<Texture>();

    // TODO: make thread safe here to avoid duplicate descriptors
    texture->descriptor = m_textures.size();
    auto result = m_textures.try_emplace(szFileName, texture);
    // if a texture with this name already exists, 
    if (!result.second)
    {
        return;
    }

    DX::ThrowIfFailed(DirectX::CreateDDSTextureFromFile(
        device,
        resourceUpload, // ResourceUploadBatch to upload texture to GPU
        szFileName, // Path of the texture to load
        texture->resource.ReleaseAndGetAddressOf()  // Release the interface associated with comptr and retreieve a pointer to the released interface to store the texture into
    ));

    // Create a shader resource view, which describes the properties of a texture
    DirectX::CreateShaderResourceView(
        device,
        texture->resource.Get(),    // Pointer to resource object that represents the Shader Resource
        m_descriptorHeap->GetCpuHandle(texture->descriptor)   // Descriptor handle that represents the SRV, make the descriptor the index of the new texture
    );
}

TextureGroup::TextureGroup(std::shared_ptr<DX::DeviceResources> deviceResources, std::shared_ptr<DirectX::DescriptorHeap> descriptorHeap)
{
	m_textures = std::map<const wchar_t*, std::shared_ptr<Texture>>();
    m_deviceResources = deviceResources;
    m_descriptorHeap = descriptorHeap;
}

TextureGroup::~TextureGroup()
{
    m_textures.clear();
}

void TextureGroup::LoadTextures()
{
    auto device = m_deviceResources->GetD3DDevice();

    // Begin batch of resource uploads to GPU
    DirectX::ResourceUploadBatch resourceUpload(device);
    resourceUpload.Begin();

    // For each file to be loaded...
    while (!m_szFileNames.empty())
    {
        // Load it with it's descriptor to the size of the array. then push 
        LoadTexture(m_szFileNames.front(), resourceUpload);
        m_szFileNames.pop();
    }

    // End batch of resource uploads to GPU, providing future to wait for said upload to complete
    auto uploadResourcesFinished = resourceUpload.End(
        m_deviceResources->GetCommandQueue()
    );
    uploadResourcesFinished.wait();
}
