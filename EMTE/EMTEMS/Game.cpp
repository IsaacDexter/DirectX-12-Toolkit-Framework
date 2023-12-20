//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    // TODO: Provide parameters for swapchain format, depth/stencil format, and backbuffer count.
    //   Add DX::DeviceResources::c_AllowTearing to opt-in to variable rate displays.
    //   Add DX::DeviceResources::c_EnableHDR for HDR10 display.
    //   Add DX::DeviceResources::c_ReverseDepth to optimize depth buffer clears for 0 instead of 1.
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
    if (m_deviceResources)
    {
        m_deviceResources->WaitForGpu();
    }
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{
    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

    

    // TODO: Change the timer settings if you want something other than the default variable timestep mode.
    // e.g. for 60 FPS fixed timestep update logic, call:
    /*
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    */
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
    // Start ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    m_timer.Tick([&]()
        {
            Update(m_timer);
        });

    Render();
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{
    PIXBeginEvent(PIX_COLOR_DEFAULT, L"Update");

    float elapsedTime = float(timer.GetElapsedSeconds());

    float time = float(m_timer.GetTotalSeconds());
    // TODO: Add your game logic here.


    //Rotate the light based on elapsed time
    float yaw = time * 0.4f;
    float pitch = time * 0.7f;
    float roll = time * 1.1f;
    auto quat = Quaternion::CreateFromYawPitchRoll(pitch, roll, yaw);

    auto light = XMVector3Rotate(g_XMOne, quat);

    m_effect->SetLightDirection(0, light);


    elapsedTime;

    PIXEndEvent();
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    // Prepare the command list to render a new frame.
    m_deviceResources->Prepare();
    Clear();

    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    // TODO: Add your rendering code here.
    
    // Set descriptor heaps in the command list
    ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap()
        //};
        , m_states->Heap()};  //Use specific sampler state
    commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);



    // Render sprites
    {
        // Begin the batch of sprite drawing operations
        m_spriteBatch->Begin(commandList);


        // Draw background texture
        m_spriteBatch->Draw(
            m_resourceDescriptors->GetGpuHandle(Descriptors::Background),
            GetTextureSize(m_background.Get()),
            m_fullscreenRect
        );

        // Submit the work of drawing a texture to the command list
        m_spriteBatch->Draw(
            m_resourceDescriptors->GetGpuHandle(Descriptors::Cat),
            GetTextureSize(m_cat.Get()),
            Vector2(50.f, 50.f), nullptr, Colors::White, 0.f  //Screen position, source rect, tint, rotation, origin
        );



        // End the batch of sprite drawing operations
        m_spriteBatch->End();
    }

    // Render primitives
    {
        // apply the basic effect
        m_effect->Apply(commandList);

        // Start batch of primitive drawing operations
        m_batch->Begin(commandList);

        VertexType v1(Vector3(  400.f,  150.f,  0.f ), -Vector3::UnitZ, Vector2(0.5f,   0.f ));
        VertexType v2(Vector3(  600.f,  450.f,  0.f ), -Vector3::UnitZ, Vector2(1.f,    1.f ));
        VertexType v3(Vector3(  200.f,  450.f,  0.f ), -Vector3::UnitZ, Vector2(0.f,    1.f ));

        m_batch->DrawTriangle(v1, v2, v3);

        // Cease this batch of primitive drawing operations
        m_batch->End();
    }

    // Render GUI
    {
        ImGui::Render();
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_deviceResources->GetCommandList());
    }
    


    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present();
    // Let manager know a frame's worth of video memory has been sent to the GPU
    // This checks to release old frame data.
    m_graphicsMemory->Commit(m_deviceResources->GetCommandQueue());
    PIXEndEvent(m_deviceResources->GetCommandQueue());
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    auto commandList = m_deviceResources->GetCommandList();
    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Clear");

    // Clear the views.
    auto const rtvDescriptor = m_deviceResources->GetRenderTargetView();
    auto const dsvDescriptor = m_deviceResources->GetDepthStencilView();

    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
    commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
    commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    // Set the viewport and scissor rect.
    auto const viewport = m_deviceResources->GetScreenViewport();
    auto const scissorRect = m_deviceResources->GetScissorRect();
    commandList->RSSetViewports(1, &viewport);
    commandList->RSSetScissorRects(1, &scissorRect);

    PIXEndEvent(commandList);
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
    // TODO: Game is becoming active window.
}

void Game::OnDeactivated()
{
    // TODO: Game is becoming background window.
}

void Game::OnSuspending()
{
    // TODO: Game is being power-suspended (or minimized).
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

    // TODO: Game is being power-resumed (or returning from minimize).
}

void Game::OnWindowMoved()
{
    auto const r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnDisplayChange()
{
    m_deviceResources->UpdateColorSpace();
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();

    // TODO: Game window is being resized.
}

// Properties
void Game::GetDefaultSize(int& width, int& height) const noexcept
{
    // TODO: Change to desired default window size (note minimum size is 320x200).
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();
    auto window = m_deviceResources->GetWindow();
    
    // Check Shader Model 6 support
    D3D12_FEATURE_DATA_SHADER_MODEL shaderModel = { D3D_SHADER_MODEL_6_0 };
    if (FAILED(device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &shaderModel, sizeof(shaderModel)))
        || (shaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_0))
    {
#ifdef _DEBUG
        OutputDebugStringA("ERROR: Shader Model 6.0 is not supported!\n");
#endif
        throw std::runtime_error("Shader Model 6.0 is not supported!");
    }

    // TODO: Initialize device dependent objects here (independent of window size).
    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);

    ///<summary>wraps information concerning render target used by DX12 when creating Pipeline State Objects</summary>
    RenderTargetState rtState(
        m_deviceResources->GetBackBufferFormat(),
        m_deviceResources->GetDepthBufferFormat()
    );

    // Create a common states object which provides a descriptor heap with pre-defined sampler descriptors
    m_states = std::make_unique<CommonStates>(device);

    m_resourceDescriptors = std::make_unique<DescriptorHeap>(device, Descriptors::Count);
    


    //Set up sprite batch
    {
        // Load Texture
        // Make resource descriptor heap

        // Initialize helper class for uploading textures to GPU
        ResourceUploadBatch resourceUpload(device);
        resourceUpload.Begin();

        // Load 2D dds texture, use CreateWICTextureFromFile for pngs
        DX::ThrowIfFailed(CreateDDSTextureFromFile(
            device,
            resourceUpload, // ResourceUploadBatch to upload texture to GPU
            L"textures/cat.dds", // Path of the texture to load
            m_cat.ReleaseAndGetAddressOf()  // Release the interface associated with comptr and retreieve a pointer to the released interface to store the texture into
        ));
        // Create a shader resource view, which describes the properties of a texture
        CreateShaderResourceView(
            device,
            m_cat.Get(),    // Pointer to resource object that represents the Shader Resource
            m_resourceDescriptors->GetCpuHandle(Descriptors::Cat)   // Descriptor handle that represents the SRV 
        );

        // Load background texture
        DX::ThrowIfFailed(CreateWICTextureFromFile(
            device,
            resourceUpload, // ResourceUploadBatch to upload texture to GPU
            L"textures/sunset.jpg", // Path of the texture to load
            m_background.ReleaseAndGetAddressOf()  // Release the interface associated with comptr and retreieve a pointer to the released interface to store the texture into
        ));
        CreateShaderResourceView(
            device,
            m_background.Get(),    // Pointer to resource object that represents the Shader Resource
            m_resourceDescriptors->GetCpuHandle(Descriptors::Background)   // Descriptor handle that represents the SRV 
        ); 
        
        // Load rocks texture
        DX::ThrowIfFailed(CreateDDSTextureFromFile(
            device,
            resourceUpload, // ResourceUploadBatch to upload texture to GPU
            L"textures/rocks_diff.dds", // Path of the texture to load
            m_rocks_diff.ReleaseAndGetAddressOf()  // Release the interface associated with comptr and retreieve a pointer to the released interface to store the texture into
        ));
        CreateShaderResourceView(
            device,
            m_rocks_diff.Get(),    // Pointer to resource object that represents the Shader Resource
            m_resourceDescriptors->GetCpuHandle(Descriptors::Rocks_diff)   // Descriptor handle that represents the SRV 
        );
        DX::ThrowIfFailed(CreateDDSTextureFromFile(
            device,
            resourceUpload, // ResourceUploadBatch to upload texture to GPU
            L"textures/rocks_norm.dds", // Path of the texture to load
            m_rocks_norm.ReleaseAndGetAddressOf()  // Release the interface associated with comptr and retreieve a pointer to the released interface to store the texture into
        ));
        CreateShaderResourceView(
            device,
            m_rocks_norm.Get(),    // Pointer to resource object that represents the Shader Resource
            m_resourceDescriptors->GetCpuHandle(Descriptors::Rocks_norm)   // Descriptor handle that represents the SRV 
        );

        auto sampler = m_states->LinearWrap();

        ///<summary>state description used when creating PSO used in the sprite batch</summary>
        SpriteBatchPipelineStateDescription pd(rtState
            //);    // Use default 
            , nullptr, nullptr, nullptr, &sampler); // use specific sampler
        //,&CommonStates::NonPremultiplied);   // Prevent use of premultiplied alpha, for textures without that
        m_spriteBatch = std::make_unique<SpriteBatch>(device, resourceUpload, pd);

        // set position of sprite
        XMUINT2 catSize = GetTextureSize(m_cat.Get());
        m_origin.x = float(catSize.x / 2);
        m_origin.y = float(catSize.y / 2);


        //Create a future allowing the upload process to potentially happen on another thread, and wait for the upload to comlete before continuing
        auto uploadResourcesFinished = resourceUpload.End(
            m_deviceResources->GetCommandQueue()
        );
        uploadResourcesFinished.wait();
    }



    //Set up primitive batch
    {
        m_batch = std::make_unique<PrimitiveBatch<VertexType>>(device);

        // create the pipeline description for the BasicEffect object
        EffectPipelineStateDescription pd(
            &VertexType::InputLayout,
            CommonStates::Opaque,
            CommonStates::DepthDefault,
            CommonStates::CullCounterClockwise, //Define CCW winding order
            rtState
        );

        // create the basiceffect to use the pipeline description and colored vertices
        m_effect = std::make_unique<NormalMapEffect>(device, EffectFlags::None, pd);
        // Set the texture descriptors for this effect
        m_effect->SetTexture(m_resourceDescriptors->GetGpuHandle(Descriptors::Rocks_diff), m_states->LinearClamp());
        m_effect->SetNormalTexture(m_resourceDescriptors->GetGpuHandle(Descriptors::Rocks_norm));

        // Enable built in, dot product lighting
        m_effect->EnableDefaultLighting();
        m_effect->SetLightDiffuseColor(0, Colors::Gray);
    }



    //Set up GUI
    {
        // Create GUI context
        ImGui::CreateContext();

        // TODO: Set configuration flags, load fonts, setup style
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   //Enable keyboard controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   //Enable docking, as using docking branch

        // Initialize platform and rendering backends
        ImGui_ImplWin32_Init(window);

        // TODO: Initialize DX12 rendering backends
        ImGui_ImplDX12_Init(
            device,
            m_deviceResources->GetBackBufferCount(),
            m_deviceResources->GetBackBufferFormat(),
            m_resourceDescriptors->Heap(),
            m_resourceDescriptors->GetFirstCpuHandle(),
            m_resourceDescriptors->GetFirstGpuHandle()
        );
    }



    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    // Get screen coordinates
    auto viewport = m_deviceResources->GetScreenViewport();
    m_spriteBatch->SetViewport(viewport);

    auto size = m_deviceResources->GetOutputSize();

    // Create matrix to allow for primitive batch to be rendered with a pixel coordinate system
    // - Shifts the 0,0 origin to the upper right corner
    // - shifts the y axis so is at the top of the screen
    // - scales the size in pixels to take up the entire -1 to 1 range
    Matrix proj = 
        Matrix::CreateScale(
            2.f / float(size.right),
            -2.f / float(size.bottom), 
            1.f
        ) * Matrix::CreateTranslation(-1.f, 1.f, 0.f);

    m_effect->SetProjection(proj);

    m_fullscreenRect = m_deviceResources->GetOutputSize();
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    m_graphicsMemory.reset();
    m_cat.Reset();
    m_background.Reset();
    m_rocks_diff.Reset();
    m_rocks_norm.Reset();
    m_resourceDescriptors.reset();
    m_spriteBatch.reset();
    m_states.reset();
    m_effect.reset();
    m_batch.reset();

    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
