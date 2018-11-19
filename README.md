# Hazumu
Alexander Chan and Emily Vo

## Goal
Real time skeletal animation with DXR.

## Summary
In a traditional rasterization pipeline, skeletal animation is done entirely on the GPU in the vertex shader. However, with raytracing, this technique is unavailable, as the entire scene along with the acceleration structure must be sent to the GPU before rendering happens. Transforming the vertices individually on the CPU will affect performance greatly. We attempt to remedy this by first transforming the vertices per frame via a compute shader, and then refitting the acceleration structure (rather than building it from scratch).

## Requirements
* Windows 10
* [Visual Studio 2017](https://www.visualstudio.com/) with the [Windows 10 October 2018 Update SDK (17763)](https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk)

## Acknowledgements
This code is based on the [DirectX raytracing samples](https://github.com/Microsoft/DirectX-Graphics-Samples/tree/master/Samples/Desktop/D3D12Raytracing/src).