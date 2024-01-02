#pragma once

#include "Sprite.h"
#include "pch.h"
#include "DeviceResources.h"
#include <queue>
#include <map>
#include "Texture.h"


/// <summary>a texture group loads batches of textures into an dynamic container</summary>
class TextureGroup
{
private:
    /// <summary><para>First: the texture's resource</para>
    /// <para>Second: the texture's descriptor</para></summary>
    using Name = const wchar_t*;

    /// <summary>A queue of paths to be loaded as textures.</summary>
    std::queue<Name> m_szFileNames;
	/// <summary><para>Key: The texture's name</para>
    /// <para>Value: The texture's resource and descriptor, respectively</para></summary>
	std::map<Name, std::shared_ptr<Texture>> m_textures;

    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    std::shared_ptr<DirectX::DescriptorHeap> m_descriptorHeap;

    /// <param name="szFileName">The path to the dds texture to be loaded</param>
    inline void LoadTexture(const wchar_t* szFileName, DirectX::ResourceUploadBatch& resourceUpload);
public:
	/// <summary>a texture group loads batches of textures into an dynamic container</summary>
	TextureGroup(std::shared_ptr<DX::DeviceResources> deviceResources, std::shared_ptr<DirectX::DescriptorHeap> m_descriptorHeap);
    ~TextureGroup();
    
    /// <summary>Loads all of the textures queued by QueueTextures</summary>
    void LoadTextures();

    /// <summary>Queue a texture to be loaded when LoadTextures is called</summary>
    /// <param name="szFileName">The path towards the texture</param>
    inline void QueueTexture(const wchar_t* szFileName)
    {
        m_szFileNames.emplace(szFileName);
    }

    /// <param name="textureName">The name the resource is stored under</param>
    /// <returns>a Texture with a descriptor and resource, or nullptr if the texture could not be found.</returns>
    inline std::shared_ptr<Texture> GetTexture(Name textureName)
    {
        // See if the name has an entry in the map
        auto texture = m_textures.find(textureName);

        //if it does, return it
        if (texture != m_textures.end())
        {
            return texture->second;
        }
        
        // otherwise, return default texture
        return nullptr;
    }
};

