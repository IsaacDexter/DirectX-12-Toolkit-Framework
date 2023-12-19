//
// Game.h
//

#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"


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

    // Dear ImGui
    void StartGuiFrame();
    void RenderGui();
    void InitGui();
    void ShutdownGui();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;
    /// <summary>
    /// <para>Manages video memory  allocations</para>
    /// <para>Call commit after presenting buffers to track and free memory</para>
    /// <para>Ensure initialization when creating resources</para>
    /// </summary>
    std::unique_ptr<DirectX::GraphicsMemory> m_graphicsMemory;

    /// <summary>Stores and allocates objects needed by shaders</summary>
    std::unique_ptr<DirectX::DescriptorHeap> m_resourceDescriptors;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_cat;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_background;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_rocks_diff;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_rocks_norm;
    RECT m_fullscreenRect;

    enum Descriptors
    {
        Cat,
        Background,
        Rocks_diff,
        Rocks_norm,
        Count
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
};
