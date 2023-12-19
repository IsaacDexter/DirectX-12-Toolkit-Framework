# [DX12 Graphics](https://www.3dgep.com/learning-directx-12-1/)
# Pipeline
## Input assembler stage
* read primitive data from **vertex** & **index** buffers
* assemble data into primitives (triangles)
* *fixed function*
## Vertex shader stage
* transforms vertex data from **object-space** into **clip-space**
* perform skeletal animation
* compute per-vertex lighting
* takes single vertex as input and outputs the clip-space position of that vertex
## Hull shader (optional)
* determines how much an input control patch should be tessellated by the tessellation stage
## Tessellator
* subdivides primitive into smaller primitives, according to tessellation factors set in hull shader stage
* *fixed function*
## Domain shader (optional)
* computes final vertex attributes based on output control points from hull shader and interpolation coordinates from tessellator stage
* input is a single output from the tessellator, outputs computed attributes of tessellated primitive
## Geometry shader (optional)
* takes a single geometric primitive as input, can discard, change the type of, or generate additional primitives
## Stream output (optional)
* feed primitive data to GPU memory
* can then be recirculated back to the pipeline to be processed by another set of shaders
* *fixed function*
## Rasterizer
* clips primitives into view frustum
* perform front/back-face culling
* interpolate per-vertex attributes across faces of each primitive
* passes these interpolated values into pixel shader
* *fixed-function*
## Pixel shader
* takes interpolated per-vertex values from rasterizer
* produces per-pixel color values
* can output a depth value of the current pixel to SV_Depth
* invoked once for each pixel covered by a primitive
## Output merger
* combines output data together with the render targets' currently bound contents
* produces final pipeline result

# GPU synchronization
* in previous versions, this was handled by driver
* in DX12, must be explicitly synchronized
* resources cannot be freed if currently being referenced in a command list being executed on a comm and queue
## Fence
* object used to synchronize commands issued to the **command queue**
* stores a single variable that indicates the last value used to **signal** the fence
* create at least one fence for each command queue
	* multiple command queues can wait on a fence to reach a specific value
	* fence should only be signalled from a single command queue
* application tracks a **fence value** used to signal the fence
## Command list
* issues commands:
	* copy
	* compute (dispatch)
	* draw
* commands are not executed immediately like DX11's immediate context
* commands are **deferred**, only ran on GPU after executed on a command queue
## Command queue
* `ID3D12CommandQueue::ExecuteCommandLists`
* `ID3D12CommandQueue::Signal`
```
method IsFenceComplete( _fenceValue )
    return fence->GetCompletedValue() >= _fenceValue
end method

method WaitForFenceValue( _fenceValue )
    if ( !IsFenceComplete( _fenceValue )
        fence->SetEventOnCompletion( _fenceValue, fenceEvent )
        WaitForEvent( fenceEvent )
    end if
end method

method Signal
    _fenceValue <- AtomicIncrement( fenceValue )
    commandQueue->Signal( fence, _fenceValue )
    return _fenceValue
end method

method Render( frameID )
    _commandList <- PopulateCommandList( frameID )
    commandQueue->ExecuteCommandList( _commandList )
    _nextFrameID <- Present()
    fenceValues[frameID] = Signal()
    WaitForFenceValue( fenceValues[_nextFrameID] )
    frameID <- _nextFrameID
end method
```
* **IsFenceComplete**
    * check to see if fence's completed value has been reached
* **WaitForFenceValue**
    * stall CPU thread until fence value has been reached
* **Signal**
    * append a fence value into the command queue
    * when command queue reaches the value, the fence that appended it will have its completed value set
    * call does not block calling thread, returns value to wait for before writable GPU resources can be reused
* **Render**
    * render a frame
        * populate command list with all draw/compute commands needed to render the scene
        * execute this list with ExecuteCommandList, which won't block the calling thread
        * doesn't wait for execution on GPU before returning to caller
    * when that frame's previous fence value is reached, move to next frame
    * `Present` will present the rendered result to the screen
        * returns index of next back-buffer within swap-chain to render to
        * `DXGI_SWAP_EFFECT_FLIP_DISCARD` flip model will prevent present from blocking the main thread
        * this means the back-buffer from the previous frame cannot be reused until the image has been presented
        * to prevent this from being overwritten before presentation, CPU thread must wait for the fence value of the previous frame to be reached
            * `WaitForFenceValue` block's CPU thread until the specified fence value has been reached
### Command queue types 
* **Copy**
    * used to issue commands to copy resources data between/on CPU & GPU
* **Compute**
    * can do everything a copy queue can do and issue compute/dispatch commands
* **Direct**
    * can do everything copy and compute queues can do, and issue draw commands
* allocate one fence object and track one fence value for each allocated command queue
* ensure every command queue tracks its own fence object and fence value, and only signals its own fence object
* a fence value should never be allowed to decrease - the max unsigned int would take 19.5 million years before overflow
# DirectX 12 Device
* used to create resources
* not directly used for issuing draw or dispatch commands
* memory context that tracks allocation in GPU memory

# [DirectXTK 12](https://github.com/microsoft/DirectXTK12/wiki/The-basic-game-loop)

# Rendering setup

## COM interface
* Component Object Model interface
* COM is a specification for creating reusable software components
    * defines a set of methods an object can support
    * doesn't dictate implementation
    * abstract
* C++ lacks, uses pure virtual class
* Interface names start with I by convention
* graphics library provides objects that implement instances
    * inherit from common base abstract parent
* binary standard, language neutral
* never declare a variable of derived type, use the interface pointer
    * maintains strict separation between interface and implementation
    * can change derived classes without changing calling code
### COM library
* initialize COM library with `HRESULT CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);`
    * 1st param must be `NULL`
    * 2nd param specifies threading model used
___
#### Apartment threading
* each COM object accessed from single thread
* COM interface pointers *not* shared between multiple threads
    * thread has a **message loop**
        * hidden 
        * OS calls windows procedure for each window
        * queue hidden, but can be accessed with `GetMessage(&msg, NULL, 0, 0);`
            * blocking  
#### Multithreaded
* not apartment

## Direct3D device
* primary graphics **COM interface** for creating addition Direct3D resource objects
* each instance associate with specific GPU
* thread-safe, same instance usable across multiple threads
* **DXGI factory** must be used when creating device

## DXGI factory
* implements methods for generating DirectX Graphics Infrastructure
    * DXGI manages low level tasks independent of the DirectX graphics runtime
* Create by calling `CreateDXGIFactory`

## Graphics command list
* primary COM interface for drawing
* maps/un-maps resources into memory for CPU access
* *not* thread-safe
    * ensure only one thread uses it at a time
* associated with a **command allocator**
    * manages command list memory allocations and lifetime

### DIRECT
* equivalent to DX11's immediate device context

### BUNDLE
* equivalent to DX11's deferred device context

### COMPUTE
* cannot do drawing commands
* can only do dispatch and copy operations

### COPY
* cannot do drawing commands
* can only do copy operations

## Command queue
* First-In, First-Out (FIFO)
* queue of command-lists submitted to GPU for processing
* records all calls to it
* none are executed until **command list** is *closed* and submitted to **command queue**

### DIRECT
* usually, one
* supports all GPU operations

### COMPUTE
* optionally multiple
* for asynchronous compute work

### COPY
* optionally multiple
* for background **direct memory access** work between GPU/CPU

## Fence
* needed for GPU to signal to the application that it has completed processing a submitted **command-list**
* used to signal Win32 event to indicate a frame has completed GPU rendering, and the CPU is free to release/reuse the resources

## Swap chain
* interface that manages multiple **back-buffers**
    * render target resources
    * one, the **front-buffer**, is displayed on out monitor 
    * the others are available for rendering the next frame
* `Present` flips the buffers (**buffer rotation**) 
    * the next back-buffer is the new front-buffer
    * the old front-buffer is available for reuse
* application must explicitly deal with **buffer rotation**
    * keep submitted **command-lists** alive until they're fully consumed by GPU
    * requires maintaining three arrays, one for each back-buffer
        * array of **command allocators**
        * array of last submitted **fence** values
        * array of **render target** resources

## Render target
* graphics resource
* holds a screen's worth of drawn pixels

## Depth Buffer
* graphics resource
* contains **Z-buffer**
    * used for **Hidden Surface Removal** (HSV)
* can have space set aside for a **stencil buffer**
    * VERY relevant

## Descriptor heap
* contains descriptors

### Render target view descriptor
* provides Direct3D the properties of the **render target**
    * the surface graphics output is written on

### Depth/stencil view descriptor
* provides Direct3D the properties of the depth/stencil resource

___

## [GraphicsMemory](https://github.com/microsoft/DirectXTK12/wiki/GraphicsMemory)
* manages video memory allocations for
    * constants
    * dynamic vertex buffers
    * dynamic index buffers
    * uploading data to the GPU
* singleton
* requires explicit initialization as it requires the device
* call `Commit` after swapChains's `Present` to inform the manager that a frame's worth of video memory has been sent to the GPU
* allows manager to check if a previous frames data can be released
* DirectX12 applications must manage the lifetime of video memory resources
* this manages allocations and lifetimes for an upload heap, used for
    * constant buffers
    * dynamic vertex & index buffers
    * as a source for copying data to 'dedicated video memory' on the GPU
* lifetime is managed by 
    * fences which are injected once per frame
    * reference counts
* commit must be called once-per-frame to ensure proper tracking and cleanup

___

## Descriptor heaps
* shaders reference object types that aren't part of the **Pipeline State Object** (PSO)
    * **Shader Resource Views** (SRV)
    * **Unordered Access Views** (UAV)
    * **Constant Buffer Views** (CBV)
    * **Samplers**
* descriptor heaps store these object types and encompass memory allocation for them
* they do this for as large a window as possible, ideally an entire frame or more

if an application rapidly switches textures from the api that the pipeline sees,
the descriptor needsspace to define descriptor tables on the fly for every set of state needed

* required by DX12 to mirror GPU hardware
* there is no option to put descriptors in memory, so they must be stored in descriptor heaps
* they are accessible, and can be edited by the CPU, but cannot be edited by the GPU