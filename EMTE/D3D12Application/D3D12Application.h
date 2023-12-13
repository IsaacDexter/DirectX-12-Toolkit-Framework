#pragma once
#include "stdafx.h"

//Windows message callback procedure forward declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

class D3D12Application
{
protected:
#pragma region TweakAndInitVariables

	/// <summary>Number of "in-flight" frames, orswap chain back buffers. Cannot be less than 2 when using flip presentation model</summary>
	const uint8_t m_frameCount = 3;

	/// <summary>Whether to use a software rasterizer (Windows Advanced Rasterization Platform) or not</summary>
	bool m_useWARP = false;

	// Size of client area on window creation
	uint32_t m_clientWidth = 1280;
	uint32_t m_clientHeight = 720;

	// Set to true after DX12 objects are created
	bool m_isInitialized = false;

#pragma endregion

#pragma region WindowsAndDX12Variables

	/// <summary>Handle to OS window used to display rendered image</summary>
	HWND m_hWnd;

	/// <summary>Stores previous size of window for switching back to windowed mode</summary>
	RECT m_windowRect;

	/// <summary>DX12 Device object</summary>
	ComPtr<ID3D12Device2> m_device;

	/// <summary>Command queue, used to issue commands to GPU</summary>
	ComPtr<ID3D12CommandQueue> m_commandQueue;

	/// <summary>Command list, where GPU commands are initially recorded</summary>
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	/// <summary>Array of command allocator, backinh memory for recording GPU commands into command list. Cannot be reused until commands have finished executing, so must be one per back buffer.</summary>
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[m_frameCount];

	/// <summary>Swap chain, responsible for presenting rendered image to the window</summary>
	ComPtr<IDXGISwapChain4> m_swapChain;

	/// <summary>Array of back buffer textures for the swap chain</summary>
	ComPtr<ID3D12Resource> m_backBuffers[m_frameCount];

	/// <summary>Back buffers described through render target view, used to clear back buffers of render target</summary>
	ComPtr<ID3D12DescriptorHeap> m_RTVDescriptorHeap;

#pragma endregion

#pragma region SynchronizationVariables

	/// <summary>Used to synchronize commands issued to command queue</summary>
	ComPtr<ID3D12Fence> m_fence;

	/// <summary>Next fence value to signal the command queue next</summary>
	uint64_t m_fenceValue = 0;

	/// <summary>array of tracked fence value used to signal command queue for each frame</summary>
	uint64_t m_frameFenceValues[m_frameCount] = {};

	/// <summary>Handle to OS event used to recieve notification that fences have reached a certain value</summary>
	HANDLE m_fenceEvent;

#pragma endregion

#pragma region SwapChainVariables

	/// <summary>Whether swap chain's present should wait for vertical refresh. False for unthrottled framerate</summary>
	bool m_vSync = false;
	bool m_tearingSupported = false;

	bool m_fullscreen = false;

#pragma endregion

public:
	/// <summary>Allows overwriting globally defined functions by supplying command line args on execution</summary>
	void ParseCommandLineArguments();
	/// <summary>Create the debug later, which must be done before the device.</summary>
	void EnableDebugLayer();
	/// <summary>Before creating OS window instance, register window class. Automatically unregistered on application termination</summary>
	/// <param name="hInst">handle to the instance</param>
	/// <param name="windowClassName">const string used to uniquely identify window class, used to create window instance</param>
	void RegisterWindowClass(HINSTANCE hInst, const wchar_t* windowClassName);
	/// <summary>Create instance of OS window</summary>
	/// <param name="windowClassName">const string used to uniquely identify window class, used to create window instance</param>
	/// <param name="hInst">handle to the instance</param>
	/// <param name="windowTitle">const string as the window title</param>
	/// <param name="width">width of window in pixels</param>
	/// <param name="height">height of window in pixels</param>
	/// <returns>handle to the created window instance</returns>
	HWND CreateWindow(const wchar_t* windowClassName, HINSTANCE hInst, const wchar_t* windowTitle, uint32_t width, uint32_t height);
	/// <summary>Compatible adapter must be present before creating Dx12 device</summary>
	/// <param name="useWARP">Whether or not to use software rasterizer</param>
	/// <returns>pointer to the adapter</returns>
	ComPtr<IDXGIAdapter4> GetAdapter(bool useWARP);
	/// <summary>Create Dx12 device, used to create resources </summary>
	/// <param name="adapter">The adapter from GetAdapter</param>
	/// <returns>comptr to the dx12 device</returns>
	ComPtr<ID3D12Device2> CreateDevice(ComPtr<IDXGIAdapter4> adapter);


};

