//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "TextureGroup.h"
#include "Sprite.h"

// Forward declarations


// A basic game implementation that creates a D3D12 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    Game(Game&&) = default;
    Game& operator= (Game&&) = default;

    Game(Game const&) = delete;
    Game& operator= (Game const&) = delete;

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnDisplayChange();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize(int& width, int& height) const noexcept;

private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::shared_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    /// <summary>
    /// <para>Manages video memory  allocations</para>
    /// <para>Call commit after presenting buffers to track and free memory</para>
    /// <para>Ensure initialization when creating resources</para>
    /// </summary>
    std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;

    /// <summary>Stores and allocates objects needed by shaders</summary>
    std::shared_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
    RECT m_fullscreenRect;

    std::unique_ptr<TextureGroup> m_textures;

    // Reserve a section of the descriptors to use as static, and reserve the other section to use as dynamic
    enum Descriptors
    {
        Gui,
        Reserve,
        Count = 128
    };

    /// <summary>Helper that handles additional D3D resources required for drawing</summary>
    std::unique_ptr<DirectX::SpriteBatch> m_spriteBatch;

    DirectX::SimpleMath::Vector2 m_origin;

    std::unique_ptr<DirectX::CommonStates> m_states;

    // select the vertex input layout
    using VertexType = DirectX::VertexPositionNormalTexture;
    //provides root signature and PSO
    std::unique_ptr<DirectX::NormalMapEffect> m_effect;
    // provides vertex buffer and primitive topology
    std::unique_ptr<DirectX::PrimitiveBatch<VertexType>> m_batch;

    //Do the same for the wireframe effect and batch
    using WireframeVertexType = DirectX::VertexPositionColor;
    std::unique_ptr<DirectX::BasicEffect> m_wireframeEffect;
    std::unique_ptr<DirectX::PrimitiveBatch<WireframeVertexType>> m_wireframeBatch;
    
    // DXTK Helper class to simplify MSAA, available at https://github.com/microsoft/DirectXTK12/wiki/MSAAHelper
    std::unique_ptr<DX::MSAAHelper> m_msaaHelper;

    std::unique_ptr<Sprite> m_catSprite;
    
    DirectX::SimpleMath::Matrix m_world;
    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_proj;
};
