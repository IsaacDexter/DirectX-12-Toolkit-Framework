//
// Game.cpp
//

#include "pch.h"
#include "Game.h"

extern void ExitGame() noexcept;

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

// MSAA constants
namespace 
{
    // render target and depth bufferwill be 4x larger
    constexpr UINT MSAA_COUNT = 4;
    constexpr UINT MSAA_QUALITY = 0;
    constexpr DXGI_FORMAT MSAA_DEPTH_FORMAT = DXGI_FORMAT_D32_FLOAT;
}

Game::Game() noexcept(false)
{
    //Inform device resources not to create a dpeth buffer, as there is to be an MSAA depth/stencil buffer
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
    
    m_timer.SetFixedTimeStep(true);
    m_timer.SetTargetElapsedSeconds(1.0 / 60);
    
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
    auto quat = Quaternion::CreateFromAxisAngle(Vector3::UnitY, time);

    auto light = XMVector3Rotate(g_XMOne, quat);

    m_effect->SetLightDirection(0, light);

    m_world = Matrix::CreateRotationY(sinf(time));


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
    m_deviceResources->Prepare(
        D3D12_RESOURCE_STATE_PRESENT, 
        D3D12_RESOURCE_STATE_RESOLVE_DEST
    );

    auto commandList = m_deviceResources->GetCommandList();

    //Set up a transition to handle transition of msaaRTV
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_msaaRt.Get(),
        D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    // notify drivers to synchronize multiple access to msaartv
    commandList->ResourceBarrier(1, &barrier);

    Clear();

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    // TODO: Add your rendering code here.

    // Set descriptor heaps in the command list
    ID3D12DescriptorHeap* heaps[] = { m_resourceDescriptors->Heap()
        //};
        , m_states->Heap() };  //Use specific sampler state
    commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);



    

    // Render sprites

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




    // Render primitives

    // Apply wireframe effect
    m_wireframeEffect->SetWorld(m_world);
    m_wireframeEffect->Apply(commandList);

    m_wireframeBatch->Begin(commandList);

    Vector3 xaxis(2.f, 0.f, 0.f);
    Vector3 yaxis(0.f, 0.f, 2.f);
    Vector3 origin = Vector3::Zero;

    constexpr size_t divisions = 20;

    for (size_t i = 0; i <= divisions; ++i)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = xaxis * fPercent + origin;

        WireframeVertexType v1(scale - yaxis, Colors::White);
        WireframeVertexType v2(scale + yaxis, Colors::White);
        m_wireframeBatch->DrawLine(v1, v2);
    }

    for (size_t i = 0; i <= divisions; i++)
    {
        float fPercent = float(i) / float(divisions);
        fPercent = (fPercent * 2.0f) - 1.0f;

        Vector3 scale = yaxis * fPercent + origin;

        WireframeVertexType v1(scale - xaxis, Colors::White);
        WireframeVertexType v2(scale + xaxis, Colors::White);
        m_wireframeBatch->DrawLine(v1, v2);
    }

    m_wireframeBatch->End();



    // apply the basic effect
    // Set matrices
    m_effect->SetWorld(m_world);
    m_effect->Apply(commandList);

    // Start batch of primitive drawing operations
    m_batch->Begin(commandList);

    VertexType v1(Vector3(0.0f, 3.f, 0.f), -Vector3::UnitZ, Vector2(0.5f, 0.f));
    VertexType v2(Vector3(-3.f, -3.f, 0.f), -Vector3::UnitZ, Vector2(0.f, 1.f));
    VertexType v3(Vector3(3.f, -3.f, 0.f), -Vector3::UnitZ, Vector2(1.f, 1.f));

    m_batch->DrawTriangle(v1, v2, v3);

    // Cease this batch of primitive drawing operations
    m_batch->End();

    //Render GUI
    ImGuiIO& io = ImGui::GetIO();

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(nullptr, (void*)commandList);
    }



    //Set up a transition to resolve transition of msaaRTV
    barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_msaaRt.Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_RESOLVE_SOURCE
    );
    // notify drivers to synchronize multiple access to msaartv
    commandList->ResourceBarrier(1, &barrier);

    // resolve the previous transtion of MSAA render target view into deviceresources render target
    commandList->ResolveSubresource(
        m_deviceResources->GetRenderTarget(),
        0,
        m_msaaRt.Get(),
        0,
        m_deviceResources->GetBackBufferFormat()
    );

    PIXEndEvent(commandList);

    // Show the new frame.
    PIXBeginEvent(m_deviceResources->GetCommandQueue(), PIX_COLOR_DEFAULT, L"Present");
    m_deviceResources->Present(D3D12_RESOURCE_STATE_RESOLVE_DEST);
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
    // Use the modified ones set up with MSAA
    //auto const rtvDescriptor = m_deviceResources->GetRenderTargetView();
    //auto const dsvDescriptor = m_deviceResources->GetDepthStencilView();
    auto const rtvDescriptor = m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    auto const dsvDescriptor = m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart();


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
    


    //Create MSAA descriptor heaps
    //Create render target view descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc = {};
    rtvDescriptorHeapDesc.NumDescriptors = 1;
    rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    
    //Create descriptor heap based on the render target view descriptors
    DX::ThrowIfFailed(device->CreateDescriptorHeap(
        &rtvDescriptorHeapDesc,
        IID_PPV_ARGS(m_rtvDescHeap.ReleaseAndGetAddressOf())
    ));

    // Create depth stencil view descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC dsvDescriptorHeapDesc = {};
    dsvDescriptorHeapDesc.NumDescriptors = 1;
    dsvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

    //Create descriptor heap based on the depth stencil view descriptors
    DX::ThrowIfFailed(device->CreateDescriptorHeap(
        &dsvDescriptorHeapDesc,
        IID_PPV_ARGS(m_dsvDescHeap.ReleaseAndGetAddressOf())
    ));


    
    ///<summary>wraps information concerning render target used by DX12 when creating Pipeline State Objects</summary>
    RenderTargetState rtState(
        m_deviceResources->GetBackBufferFormat(),
        MSAA_DEPTH_FORMAT   //Use the MSAA depth buffer format specifically
        //m_deviceResources->GetDepthBufferFormat()
    );
    rtState.sampleDesc.Count = MSAA_COUNT;
    rtState.sampleDesc.Quality = MSAA_QUALITY;
  
    // helper structure for easy initialisation of a D3D12_RASTERIZER_DESC
    // Creates a raster state the same as standard cullnone but with mutisampleenable set to true, to allow for msaa
    CD3DX12_RASTERIZER_DESC rastDesc(
        D3D12_FILL_MODE_SOLID,  //Fill mode
        D3D12_CULL_MODE_NONE,   //Cull mode
        FALSE,  //Front CCW (default false)
        D3D12_DEFAULT_DEPTH_BIAS,   //Depth bias
        D3D12_DEFAULT_DEPTH_BIAS_CLAMP, //Depth bias clamp
        D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS, //Slope scaled depth bias
        TRUE,   //Depth clip enable
        TRUE,   //Multisampled enable (set to true to allow for MSAA)
        FALSE,  //antialiased line enable
        0, //Forced sample count
        D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF   //Conservative raster
    );

    // Create a common states object which provides a descriptor heap with pre-defined sampler descriptors
    m_states = std::make_unique<CommonStates>(device);

    m_resourceDescriptors = std::make_unique<DescriptorHeap>(device, Descriptors::Count);

    //Initialize world matrix
    m_world = Matrix::Identity;




    //Set up sprite batch
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
    SpriteBatchPipelineStateDescription spd(rtState
        //);    // Use default 
        , nullptr, nullptr, nullptr, &sampler); // use specific sampler
    //,&CommonStates::NonPremultiplied);   // Prevent use of premultiplied alpha, for textures without that
    m_spriteBatch = std::make_unique<SpriteBatch>(device, resourceUpload, spd);

    // set position of sprite
    XMUINT2 catSize = GetTextureSize(m_cat.Get());
    m_origin.x = float(catSize.x / 2);
    m_origin.y = float(catSize.y / 2);


    //Create a future allowing the upload process to potentially happen on another thread, and wait for the upload to comlete before continuing
    auto uploadResourcesFinished = resourceUpload.End(
        m_deviceResources->GetCommandQueue()
    );
    uploadResourcesFinished.wait();



    //Set up primitive batch
    m_batch = std::make_unique<PrimitiveBatch<VertexType>>(device);

    // create the pipeline description for the Normal effect objects
    EffectPipelineStateDescription ppd(
        &VertexType::InputLayout,
        CommonStates::Opaque,
        CommonStates::DepthDefault,
        rastDesc, //Define rasterizer setup above
        rtState
    );

    // create the basiceffect to use the pipeline description and colored vertices
    m_effect = std::make_unique<NormalMapEffect>(device, EffectFlags::None, ppd);
    // Set the texture descriptors for this effect
    m_effect->SetTexture(m_resourceDescriptors->GetGpuHandle(Descriptors::Rocks_diff), m_states->LinearClamp());
    m_effect->SetNormalTexture(m_resourceDescriptors->GetGpuHandle(Descriptors::Rocks_norm));

    // Enable built in, dot product lighting
    m_effect->EnableDefaultLighting();
    m_effect->SetLightDiffuseColor(0, Colors::Gray);


    // Set up wireframe batch
    m_wireframeBatch = std::make_unique<PrimitiveBatch<WireframeVertexType>>(device);
    
    // create the pipeline description for the wireframe effect object
    EffectPipelineStateDescription wpd(
        &VertexPositionColor::InputLayout,
        CommonStates::Opaque,
        CommonStates::DepthDefault,
        rastDesc,
        rtState,
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
    );

    // create the wireframe effect from the above description
    m_wireframeEffect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, wpd);



    //Set up GUI
    // Create GUI context
    ImGui::CreateContext();

    // TODO: Set configuration flags, load fonts, setup style
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;   //Enable keyboard controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   //Enable docking, as using docking branch
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   //Enable viewports

    // Initialize platform and rendering backends
    ImGui_ImplWin32_Init(window);

    // TODO: Initialize DX12 rendering backends
    ImGui_ImplDX12_Init(
        device,
        m_deviceResources->GetBackBufferCount(),
        m_deviceResources->GetBackBufferFormat(),
        m_resourceDescriptors->Heap(),
        m_resourceDescriptors->GetCpuHandle(Descriptors::Gui),
        m_resourceDescriptors->GetGpuHandle(Descriptors::Gui)
    );



    device;
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    // Get screen coordinates
    auto viewport = m_deviceResources->GetScreenViewport();
    auto size = m_deviceResources->GetOutputSize();
    auto device = m_deviceResources->GetD3DDevice();

    m_spriteBatch->SetViewport(viewport);
    


    //Initialize MSAA resources
    CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);


    // Create MSAA depth/stencil buffer
    auto dsDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        MSAA_DEPTH_FORMAT,  //dxgi format
        static_cast<UINT>(size.right),  //width
        static_cast<UINT>(size.bottom), //height
        1,  //This depth stencil view has only one texture
        1,  //Use a single mipmap level
        MSAA_COUNT, //sample count
        MSAA_QUALITY // sample quality
    );
    // Set flag allowing depth stencil to be created for the resources
    dsDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    // value used to optimize clear operations for the depth stencil
    D3D12_CLEAR_VALUE dsOptimizedClearValue = {};
    dsOptimizedClearValue.Format = MSAA_DEPTH_FORMAT;
    dsOptimizedClearValue.DepthStencil.Depth = 1.0f;
    dsOptimizedClearValue.DepthStencil.Stencil = 0;

    //Create a resource and an implicit heap big enough to contain it, and map it to the heap
    // This resource houses the depth stencil
    DX::ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &dsDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &dsOptimizedClearValue,
        IID_PPV_ARGS(m_ds.ReleaseAndGetAddressOf())
    ));


    // Create depth stencil view
    // Create descriptor for the depth stencil view
    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = MSAA_DEPTH_FORMAT;
    dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS; //access the resources as a 2d texture with multisampling (MS)

    device->CreateDepthStencilView(
        m_ds.Get(),
        &dsvDesc,
        m_dsvDescHeap->GetCPUDescriptorHandleForHeapStart()
    );
    

    // Create Render Target View
    // Create descriptor for the render target view
    auto msaaRtDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        m_deviceResources->GetBackBufferFormat(),
        static_cast<UINT>(size.right),  //width
        static_cast<UINT>(size.bottom), //height
        1,  //This depth stencil view has only one texture
        1,  //Use a single mipmap level
        MSAA_COUNT, //sample count
        MSAA_QUALITY // sample quality
    );
    msaaRtDesc.Flags |= D3D12_RESOURCE_FLAGS::D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    // value used to optimize clear operations for the msaa render target view
    D3D12_CLEAR_VALUE msaaRtOptimizedClearValue = {};
    msaaRtOptimizedClearValue.Format = m_deviceResources->GetBackBufferFormat();
    memcpy(msaaRtOptimizedClearValue.Color, Colors::CornflowerBlue, sizeof(float) * 4);

    // Create resource to house the render target view
    DX::ThrowIfFailed(device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &msaaRtDesc,
        D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
        &msaaRtOptimizedClearValue,
        IID_PPV_ARGS(m_msaaRt.ReleaseAndGetAddressOf())
    ));

    device->CreateRenderTargetView(
        m_msaaRt.Get(),
        nullptr,
        m_rtvDescHeap->GetCPUDescriptorHandleForHeapStart()
    );



    //Initialize Matrices 
    m_view = Matrix::CreateLookAt(
        Vector3(0.0f, 2.f, 2.f), //Camera position
        Vector3::Zero,  //Camera target
        Vector3::UnitY  //Camera up vector
    );
    m_proj = Matrix::CreatePerspectiveFieldOfView(
        XM_PI / 4.f,
        float(size.right) / float(size.bottom),
        0.1f,
        10.f
    );

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj); 
    
    m_wireframeEffect->SetView(m_view);
    m_wireframeEffect->SetProjection(m_proj);

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
    m_wireframeEffect.reset();
    m_wireframeBatch.reset();

    m_rtvDescHeap.Reset();
    m_dsvDescHeap.Reset();
    m_ds.Reset();
    m_msaaRt.Reset();

    //Clean up GUI
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
