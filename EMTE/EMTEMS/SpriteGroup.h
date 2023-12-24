#pragma once

#include "Sprite.h"
#include "pch.h"
#include "DeviceResources.h"
#include <queue>
#include <unordered_map>
#include "Texture.h"


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
	std::unordered_map<Name, std::shared_ptr<Texture>> m_textures;

    std::shared_ptr<DX::DeviceResources>    m_deviceResources;
    std::shared_ptr<DirectX::DescriptorHeap> m_descriptorHeap;

    /// <param name="szFileName">The path to the dds texture to be loaded</param>
    inline void LoadTexture(const wchar_t* szFileName, DirectX::ResourceUploadBatch& resourceUpload);
public:
	TextureGroup(std::shared_ptr<DX::DeviceResources> deviceResources, std::shared_ptr<DirectX::DescriptorHeap> m_descriptorHeap);
    ~TextureGroup();
    void LoadTextures();
    inline void QueueTexture(const wchar_t* szFileName)
    {
        m_szFileNames.emplace(szFileName);
    }

    inline std::shared_ptr<Texture> GetTexture(Name textureName)
    {
        try
        {
            return m_textures.at(textureName);
        }
        catch (const std::out_of_range&)
        {
            //return default texture
            return m_textures.at(L"default");
        }
    }
};

