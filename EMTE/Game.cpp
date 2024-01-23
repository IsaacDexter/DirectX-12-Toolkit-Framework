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
    //Create device resource instance
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


    // handle mouse input
    auto mouse = m_mouse->GetState();

    if (mouse.positionMode == Mouse::MODE_RELATIVE)
    {
        Vector3 delta = Vector3(float(mouse.x), float(mouse.y), 0.f)
            * m_rotationGain * elapsedTime;

        m_pitch -= delta.y;
        m_yaw -= delta.x;
    }

    m_mouse->SetMode(mouse.leftButton
        ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);



    auto kb = m_keyboard->GetState();
    // handle user input
    if (kb.Escape)
    {
        ExitGame();
    }
    if (kb.Home)
    {
        // reset camera position and rotation
        m_cameraPos = m_startPos;
        m_pitch = m_yaw = 0;
    }

    // move the camera in response to keyboard input
    Vector3 move = Vector3::Zero;

    if (kb.Up || kb.W)
        move.z += 1.f;

    if (kb.Down || kb.S)
        move.z -= 1.f;

    if (kb.Left || kb.A)
        move.x += 1.f;

    if (kb.Right || kb.D)
        move.x -= 1.f;

    if (kb.PageUp || kb.Space)
        move.y += 1.f;

    if (kb.PageDown || kb.X)
        move.y -= 1.f;

    Quaternion q = Quaternion::CreateFromYawPitchRoll(m_yaw, 0.f, 0.f);

    move = Vector3::Transform(move, q);
    move *= m_movementGain * elapsedTime;
    m_cameraPos += move;




    // update the camera positon

        // limit pitch to straight up or straight down
    constexpr float limit = XM_PIDIV2 - 0.01f;
    m_pitch = std::max(-limit, m_pitch);
    m_pitch = std::min(+limit, m_pitch);

    // keep longitude in sane range by wrapping
    if (m_yaw > XM_PI)
    {
        m_yaw -= XM_2PI;
    }
    else if (m_yaw < -XM_PI)
    {
        m_yaw += XM_2PI;
    }

    float y = sinf(m_pitch);
    float r = cosf(m_pitch);
    float z = r * cosf(m_yaw);
    float x = r * sinf(m_yaw);

    XMVECTOR lookAt = m_cameraPos + Vector3(x, y, z);
    m_view = XMMatrixLookAtRH(m_cameraPos, lookAt, Vector3::Up);

    /*
    char buffer[500];
    sprintf_s(buffer, "\nm_cameraPos = (%f, %f, %f)\nm_cameraLookAt = (%f, %f, %f)", m_cameraPos.x, m_cameraPos.y, m_cameraPos.z, x, y, z);
    OutputDebugStringA(buffer);*/


    //Rotate the light based on elapsed time
    auto quat = Quaternion::CreateFromAxisAngle(Vector3::UnitY, time);

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
    auto commandList = m_deviceResources->GetCommandList();

    m_renderTexture->BeginScene(commandList);

    Clear();

    PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

    // TODO: Add your rendering code here.

    // Set descriptor heaps in the command list
    ID3D12DescriptorHeap* heaps[] = { m_srvPile->Heap()
        //};
        , m_states->Heap() };  //Use specific sampler state
    commandList->SetDescriptorHeaps(static_cast<UINT>(std::size(heaps)), heaps);





    // Render sprites

        // Begin the batch of sprite drawing operations
    m_spriteBatch->Begin(commandList);

    {
        Microsoft::WRL::ComPtr<ID3D12Resource> texture;
        auto texHand = m_texHands->at(L"textures/sunset.jpg");
        m_textureResources->GetResource(texHand.slot, texture.ReleaseAndGetAddressOf());

        // Draw background texture
        m_spriteBatch->Draw(
            m_textureResources->GetGpuDescriptorHandle(texHand.desc),
            GetTextureSize(texture.Get()),
            m_fullscreenRect
        );
    }

    {
        Microsoft::WRL::ComPtr<ID3D12Resource> texture;
        auto texHand = m_texHands->at(L"textures/cat.dds");
        m_textureResources->GetResource(texHand.slot, texture.ReleaseAndGetAddressOf());

        // Submit the work of drawing a texture to the command list
        m_spriteBatch->Draw(
            m_srvPile->GetGpuHandle(texHand.desc),
            GetTextureSize(texture.Get()),
            Vector2(50.f, 50.f), nullptr, Colors::White, 0.f  //Screen position, source rect, tint, rotation, origin
        );
    }



    // End the batch of sprite drawing operations
    m_spriteBatch->End();


    // Render primitives
    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);

    m_wireframeEffect->SetView(m_view);
    m_wireframeEffect->SetProjection(m_proj);


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

    m_shape->Draw(commandList);

    // Start batch of primitive drawing operations
    m_batch->Begin(commandList);

    VertexType v1(Vector3(0.0f, 1.f, 0.f), -Vector3::UnitZ, Vector2(0.5f, 0.f));
    VertexType v3(Vector3(-1.f, -1.f, 0.f), -Vector3::UnitZ, Vector2(0.f, 1.f));
    VertexType v2(Vector3(1.f, -1.f, 0.f), -Vector3::UnitZ, Vector2(1.f, 1.f));

    m_batch->DrawTriangle(v1, v2, v3);

    // Cease this batch of primitive drawing operations
    m_batch->End();

    m_renderTexture->EndScene(commandList);

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




    // render scene to offscreen texture
    auto rtvDescriptor = m_deviceResources->GetRenderTargetView();
    commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, nullptr);

    // Begin the batch of sprite drawing operations
    m_spriteBatch->Begin(commandList);

    m_spriteBatch->Draw(
        m_srvPile->GetGpuHandle(Descriptors::RenderTexture),
        GetTextureSize(m_renderTexture->GetResource()),
        m_deviceResources->GetOutputSize()
    );

    // Begin the batch of sprite drawing operations
    m_spriteBatch->End();

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
    {
        // render to the render target and depth/stencil buffer
        // clear the offscreen render target
        auto const rtvDescriptor = m_renderDescriptors->GetCpuHandle(RTDescriptors::OffscreenRT);
        //auto const rtvDescriptor = m_deviceResources->GetRenderTargetView();
        auto const dsvDescriptor = m_deviceResources->GetDepthStencilView();

        commandList->OMSetRenderTargets(1, &rtvDescriptor, FALSE, &dsvDescriptor);
        //commandList->ClearRenderTargetView(rtvDescriptor, Colors::CornflowerBlue, 0, nullptr);
        // Clear the offscreen RT
        m_renderTexture->Clear(commandList);
        commandList->ClearDepthStencilView(dsvDescriptor, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    }
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

    // set up input devices and bind them to the window
    m_keyboard = std::make_unique<Keyboard>();
    m_mouse = std::make_unique<Mouse>();

    m_mouse->SetWindow(window);
    
    //set up camera variables
    m_pitch = 0.0f;
    m_yaw = 0.0f;

    m_cameraPos = m_startPos;


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

    m_textureLoadList = { L"textures/cat.dds", L"textures/sunset.jpg", L"textures/rocks_diff.dds", L"textures/rocks_norm.dds" };

    // TODO: Initialize device dependent objects here (independent of window size).
    m_graphicsMemory = std::make_unique<GraphicsMemory>(device);


    ///<summary>wraps information concerning render target used by DX12 when creating Pipeline State Objects</summary>
    RenderTargetState rtState(
        m_deviceResources->GetBackBufferFormat(),
        m_deviceResources->GetDepthBufferFormat()
    );

    // Create a common states object which provides a descriptor heap with pre-defined sampler descriptors
    m_states = std::make_unique<CommonStates>(device);

    m_srvPile = std::make_unique<DescriptorPile>(device, Descriptors::Count, Descriptors::Reserve);
    
    // Create render descriptor heap to store render target views
    m_renderDescriptors = std::make_unique<DescriptorHeap>(device, D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV, D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE, RTDescriptors::RTCount);



    //Initialize world matrix
    m_world = Matrix::Identity;

    LoadTextures();

    // create render texture, used for the portal
    // Create a render texture of the same size and foramt as the swapchain
    m_renderTexture = std::make_unique<DX::RenderTexture>(m_deviceResources->GetBackBufferFormat());
    // Set optimized clear colour
    m_renderTexture->SetClearColor(Colors::CornflowerBlue);
    m_renderTexture->SetDevice(
        device,
        m_srvPile->GetCpuHandle(Descriptors::RenderTexture),
        m_renderDescriptors->GetCpuHandle(RTDescriptors::OffscreenRT)
    );

    // Instanciate sprites
    {
        // set position of sprite
        Microsoft::WRL::ComPtr<ID3D12Resource> cat;
        auto texHand = m_texHands->find(L"textures/cat.dds")->second;
        //TODO sanitize if the map does not contain this value

        m_textureResources->GetResource(texHand.slot, cat.ReleaseAndGetAddressOf());
        XMUINT2 catSize = GetTextureSize(cat.Get());
        m_origin.x = float(catSize.x / 2);
        m_origin.y = float(catSize.y / 2);
    }

    // Initialize the primitive batch used for rendering lit objects 
    {
        //Set up primitive batch
        m_batch = std::make_unique<PrimitiveBatch<VertexType>>(device);

        // create the pipeline description for the Normal effect objects
        EffectPipelineStateDescription ppd(
            &GeometricPrimitive::VertexType::InputLayout,
            CommonStates::Opaque,
            CommonStates::DepthDefault,
            CommonStates::CullCounterClockwise, //Define CCW winding order
            rtState
        );

        // create the basiceffect to use the pipeline description and colored vertices
        // utilize built in normal effect, per pixel lighting and use of textures
        m_effect = std::make_unique<NormalMapEffect>(device, EffectFlags::PerPixelLighting | EffectFlags::Texture, ppd);
        // Set the texture descriptors for this effect
        m_effect->SetTexture(m_textureResources->GetGpuDescriptorHandle(m_texHands->at(L"textures/rocks_diff.dds").desc), m_states->LinearClamp());
        m_effect->SetNormalTexture(m_textureResources->GetGpuDescriptorHandle(m_texHands->at(L"textures/rocks_norm.dds").desc));

        // enable the first light in the scene
        m_effect->SetLightEnabled(0, true);
        m_effect->SetLightDiffuseColor(0, Colors::White);
        m_effect->SetLightDirection(0, -Vector3::UnitZ);
    
        // instanciate geometric primitive
        m_shape = GeometricPrimitive::CreateSphere();

        ResourceUploadBatch resourceUpload(device);

        resourceUpload.Begin();

        // Load geometric primitive data into the dedicated video memory for faster performance
        m_shape->LoadStaticBuffers(device, resourceUpload);

        //Create a future allowing the upload process to potentially happen on another thread, and wait for the upload to comlete before continuing
        auto uploadResourcesFinished = resourceUpload.End(
            m_deviceResources->GetCommandQueue()
        );
        uploadResourcesFinished.wait();
    }

    // Create the primitive batch used to render wireframe vertices, which are unlit, and use a different vertex type and effect so need their own batch
    // TODO: make this code more applicable to switching effects by giving it more in common with the other primitive batch
    {
        // Set up wireframe batch
        m_wireframeBatch = std::make_unique<PrimitiveBatch<WireframeVertexType>>(device);

        // create the pipeline description for the wireframe effect object
        EffectPipelineStateDescription wpd(
            &VertexPositionColor::InputLayout,
            CommonStates::Opaque,
            CommonStates::DepthDefault,
            CommonStates::CullNone, //Define CCW winding order
            rtState,
            D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE
        );

        // create the wireframe effect from the above description
        m_wireframeEffect = std::make_unique<BasicEffect>(device, EffectFlags::VertexColor, wpd);
    }

    // Initialize the sprite batch
    {
        // Create a resource batch to upload the sprite batch. Done seperately to the texture loading as they might happen at seperate times.
        ResourceUploadBatch resourceUpload(device);

        resourceUpload.Begin();

        auto sampler = m_states->LinearWrap();

        ///<summary>state description used when creating PSO used in the sprite batch</summary>
        SpriteBatchPipelineStateDescription spd(rtState
            //);    // Use default 
            , nullptr, nullptr, nullptr, &sampler); // use specific sampler
        //,&CommonStates::NonPremultiplied);   // Prevent use of premultiplied alpha, for textures without that
        m_spriteBatch = std::make_unique<SpriteBatch>(device, resourceUpload, spd);

        //Create a future allowing the upload process to potentially happen on another thread, and wait for the upload to comlete before continuing
        auto uploadResourcesFinished = resourceUpload.End(
            m_deviceResources->GetCommandQueue()
        );
        uploadResourcesFinished.wait();
    }

    //Set up GUI
    {
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
            m_srvPile->Heap(),
            m_srvPile->GetCpuHandle(Descriptors::Gui),
            m_srvPile->GetGpuHandle(Descriptors::Gui)
        );

    }
}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    // TODO: Initialize windows-size dependent objects here.
    // Get screen coordinates
    auto viewport = m_deviceResources->GetScreenViewport();
    m_spriteBatch->SetViewport(viewport);

    auto size = m_deviceResources->GetOutputSize();
    // size the render texture to be the same size as the swapchain
    m_renderTexture->SetWindow(size);


    //Initialize Matrices 
    m_view = Matrix::CreateLookAt(
        Vector3(m_cameraPos), //Camera position
        Vector3::Zero,  //Camera target
        Vector3::Up  //Camera up vector
    );
    m_proj = Matrix::CreatePerspectiveFieldOfView(
        XM_PI / 4.f,
        float(size.right) / float(size.bottom),
        0.1f,
        100.f
    );

    m_effect->SetView(m_view);
    m_effect->SetProjection(m_proj);

    m_wireframeEffect->SetView(m_view);
    m_wireframeEffect->SetProjection(m_proj);

    m_fullscreenRect = m_deviceResources->GetOutputSize();
}

void Game::LoadTextures()
{
    auto device = m_deviceResources->GetD3DDevice();

    ResourceUploadBatch resourceUpload(device);

    resourceUpload.Begin();

    m_texHands = std::make_unique<std::map<const wchar_t*, TexHand>>();
    // Create the EffectTextureFactory, a helper object to provide sharing of texture resources from the existing reosurce heap
    m_textureResources = std::make_unique<DirectX::EffectTextureFactory>(
        device,
        resourceUpload,
        m_srvPile->Heap()
    );

    size_t descriptor = Descriptors::Reserve;
    for (auto path : m_textureLoadList)
    {
        // Create a texture from the path and tie it to a descriptor
        size_t slot = m_textureResources->CreateTexture(path, descriptor);
        TexHand th(descriptor, slot);
        m_texHands->emplace(path, th);
        descriptor++;
    }

    //Create a future allowing the upload process to potentially happen on another thread, and wait for the upload to comlete before continuing
    auto uploadResourcesFinished = resourceUpload.End(
        m_deviceResources->GetCommandQueue()
    );
    uploadResourcesFinished.wait();
}

void Game::OnDeviceLost()
{
    // TODO: Add Direct3D resource cleanup here.
    m_graphicsMemory.reset();
    m_srvPile.reset();
    m_spriteBatch.reset();
    m_states.reset();
    m_effect.reset();
    m_batch.reset();
    m_wireframeEffect.reset();
    m_wireframeBatch.reset();

    //Clean up textures
    m_textureLoadList.clear();
    m_texHands->clear();
    m_textureResources->ReleaseCache();
    m_textureResources.reset();
    m_renderTexture->ReleaseDevice();
    m_renderDescriptors.reset();

    //Clean up GUI
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    m_shape.reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion
