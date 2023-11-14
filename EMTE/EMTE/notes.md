# [DX12 Graphics Pipeline](https://www.3dgep.com/learning-directx-12-1/)
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
* determines how much an input control patch should be tesselated by the tesselation stage
## Tessellator
* subdivides primitive into smaller primitives, according to tessellation factiors set in hull shader stage
* *fixed function*
## Domain shader (optional)
* computes final vertex attributes based on output control points from hull shader and interpolation coordinates from tessellator stage
* input is a single output from the tesselator, outputs computed attributes of tessellated primitive
## Geometry shader (optional)
* takes a single geometric primitive as input, can discard, change the type of, or generate additional primitives
## Stream output (optional)
* feed primitive data to GPU memory
* can then be recirculated back to the pipeline to be processed by another set of shaders
* *fixed function*
## Rasterizer
* clips primitives into view frustum
* perform front/back-face culling
* interpolate per-vertex attributes accross faces of each primitive
* passes these interpolated balues into pixel shader
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
	* fence should only be signaled from a single command queue
* application tracks a **fence value** used to signal the fence
## Command list
* issues commands:
	* copy
	* compute (dispatch)
	* draw
* commands are not executed immediately like DX11's immediate context
* commands are **deferred**, only ran on GPU after executed on a command queue
## Command queue
* ```ID3D12CommandQueue::ExecuteCommandLists```
* ```ID3D12CommandQueue::Signal```
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
    * 


