#include "D3D12Application.h"

void D3D12Application::ParseCommandLineArguments()
{
	int argc;
    //Parse command line arguments for iteration through
	wchar_t** argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

	for (size_t i = 0; i < argc; i++)
	{
        //Specify width of the render window (in pixels)
        if (::wcscmp(argv[i], L"-w") == 0 || ::wcscmp(argv[i], L"--width") == 0)
        {
            m_clientWidth = ::wcstol(argv[++i], nullptr, 10);
        }
        //Specify height of the render window (in pixels)
        if (::wcscmp(argv[i], L"-h") == 0 || ::wcscmp(argv[i], L"--height") == 0)
        {
            m_clientHeight = ::wcstol(argv[++i], nullptr, 10);
        }
        //Use WARP for device creation
        if (::wcscmp(argv[i], L"-warp") == 0 || ::wcscmp(argv[i], L"--warp") == 0)
        {
            m_useWARP = true;
        }
	}

    //free memory allocated by CommandLineToArgvW
    ::LocalFree(argv);
}

void D3D12Application::EnableDebugLayer()
{
#if defined(_DEBUG)
    
    ComPtr<ID3D12Debug> debugInterface;
    //Get an interface pointer supplying IID value automatically
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)), "Couldn't enable d3d12 debug layer!\n");
#endif // DEBUG
}

void D3D12Application::RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName)
{
    // Register a window class for creating our render window with.
    WNDCLASSEXW windowClass = {};

    // Size in bytes of this structure
    windowClass.cbSize = sizeof(WNDCLASSEX);
    // Class styles: HREDRAW/VREDRAW specified the entire window is redrawn after movement/size adjustment of Width/Height
    windowClass.style = CS_HREDRAW | CS_VREDRAW;
    // Pointer to windows procedureto handle window messages
    windowClass.lpfnWndProc = &WndProc;
    // Extra bytes to allocate to window class structure
    windowClass.cbClsExtra = 0;
    // Extra bytes to allocate to window instance
    windowClass.cbWndExtra = 0;
    // Handle to the instance that contains the window  procedure for the class
    windowClass.hInstance = hInst;
    // Handle to class icon, can be loaded using LoadIcon function 
    windowClass.hIcon = ::LoadIcon(hInst, NULL);
    // Handle to class cursor, loading default arrow
    windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    // Handle to class background brush, which takes a colour value converted to a HBRUSH type
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    // pointer to character string that specifies class menu's resource name
    windowClass.lpszMenuName = NULL;
    // const string used to uniquely identify window class, used to create window instance
    windowClass.lpszClassName = windowClassName;
    // handle to small icon associated with window class
    windowClass.hIconSm = ::LoadIcon(hInst, NULL);

    static ATOM atom = ::RegisterClassExW(&windowClass);

    assert(atom > 0);
    //now windows class is registered, OS window instance can be created
}

HWND D3D12Application::CreateWindow(const wchar_t* windowClassName, HINSTANCE hInst, const wchar_t* windowTitle, uint32_t width, uint32_t height)
{
    // retrieve specific system information, i.e. width/heigh of primary display
    int screenWidth = ::GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = ::GetSystemMetrics(SM_CYSCREEN);

    // calculate required size of window rectangle, describing a window that can be maxed and minned
    RECT windowRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };
    ::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

    // dimensions of adjusted window are used to compute width and height of created window
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // center the window within the screen. Clamp to 0, 0 for the top-left corner.
    int windowX = std::max<int>(0, (screenWidth - windowWidth) / 2);
    int windowY = std::max<int>(0, (screenHeight - windowHeight) / 2);

    HWND hWnd = ::CreateWindowExW(  
        NULL,   // extended style of created window https://learn.microsoft.com/en-gb/windows/win32/winmsg/extended-window-styles
        windowClassName,    // class name registered with register class
        windowTitle,    // name to show in windows title bar
        WS_OVERLAPPEDWINDOW,    // style of window being created
        windowX,    // horizontal pos of window
        windowY,    // vertical pos of window
        windowWidth,    // width of window in device units
        windowHeight,   // height of window in device units
        NULL,   // owner/parent window of created window
        NULL,   // handle to a menu/child window
        hInst,  // handle to the instance of the module associated with the window
        nullptr // pointer to a value to be passed to the window through CREATESTRUCT
    );

    assert(hWnd && "Failed to create window");

    return hWnd;
}

ComPtr<IDXGIAdapter4> D3D12Application::GetAdapter(bool useWARP)
{
    // a DXGI factory must be created before querying available adaptors
    ComPtr<IDXGIFactory4> dxgiFactory;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    //enables errors to be caught during device creation adn while querying adapters
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif //DEBUG

    ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));

    ComPtr<IDXGIAdapter1> dxgiAdapter1;
    ComPtr<IDXGIAdapter4> dxgiAdapter4;

    if (useWARP)
    {
        // if a warp device is to be used, EnumWarpAdapter will directly create the warp adapter
        ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
        // cannot static_cast COM, so .As casts com to correct type
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }
    else
    {
        // when not using warp, DXGI factory querys hardware adaptors
        SIZE_T maxDedicatedVideoMemory = 0;
        // enummarate available gpu adapters in the system and iterate through
        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
            dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

            // check to see if the adapter can create a D3D12 device without actually  creating it. The adapter with the largest dedicated video memory is favored
            // DXGI_ADAPTER_FLAG_SOFTWARE avoids software rasterizer as we're not using WARP
            if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0 &&
                SUCCEEDED(D3D12CreateDevice(dxgiAdapter1.Get(), // create a null ddevice if its a compatible DX12 adapter, if it returns S_OK, it is a compatible DX12 header
                    D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)) &&
                dxgiAdapterDesc1.DedicatedVideoMemory > maxDedicatedVideoMemory)
            {
                maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
                ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));  // cast the adapter and return it
            }
        }
    }

    return dxgiAdapter4;
}

ComPtr<ID3D12Device2> D3D12Application::CreateDevice(ComPtr<IDXGIAdapter4> adapter)
{
    ComPtr<ID3D12Device2> d3d12Device2;
    ThrowIfFailed(D3D12CreateDevice(
        adapter.Get(),  //pointer to video adapter to use when creating device
        D3D_FEATURE_LEVEL_11_0, //minimum feature level for successful device creation
        IID_PPV_ARGS(&d3d12Device2)));  //store device in this argument

#if defined(_DEBUG)

    //used to enable break points based on severity of message and filter messages' creation
    ComPtr<ID3D12InfoQueue> pInfoQueue;
    if (SUCCEEDED(d3d12Device2.As(&pInfoQueue)))    //query infoqueue inteface from comptr.as
    {
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);    //sets a message severity level to break on with debugger
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
        
        // suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY Severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // suppress individual messages by their ID
        D3D12_MESSAGE_ID DenyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // warning when render target is cleared using a clear colour
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };

        //info queue filter is defined and filter is pushed onto info queue
        D3D12_INFO_QUEUE_FILTER NewFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        NewFilter.DenyList.NumSeverities = _countof(Severities);
        NewFilter.DenyList.pSeverityList = Severities;
        NewFilter.DenyList.NumIDs = _countof(DenyIds);
        NewFilter.DenyList.pIDList = DenyIds;

        ThrowIfFailed(pInfoQueue->PushStorageFilter(&NewFilter));
    }

#endif // DEBUG

    return d3d12Device2;
}
