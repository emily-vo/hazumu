//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once

#include "DXSample.h"
#include "StepTimer.h"
#include "RaytracingHlslCompat.h"
#include "Mesh.h"
#include "Model.h"
#include "Animation.h"
#include "cudaSkinning.h"
#define _WIN32_WINNT 0x600
#include <stdio.h>

#include <d3d11.h>
#include <d3dcompiler.h>

#include <glm/glm.hpp>

#include "AnimationPlayer.h"
#include "MorphTarget.h"

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

namespace GlobalRootSignatureParams {
    enum Value {
        OutputViewSlot = 0,
        AccelerationStructureSlot,
        SceneConstantSlot,
        VertexBuffersSlot,
        Count 
    };
}

namespace LocalRootSignatureParams {
    enum Value {
        CubeConstantSlot = 0,
        Count 
    };
}

enum ComputeRootParameters : UINT32
{
	ComputeRootCBV = 0,
	ComputeRootSRVTable,
	ComputeRootUAVTable,
	ComputeRootParametersCount
};

// The sample supports both Raytracing Fallback Layer and DirectX Raytracing APIs. 
// This is purely for demonstration purposes to show where the API differences are. 
// Real-world applications will implement only one or the other. 
// Fallback Layer uses DirectX Raytracing if a driver and OS supports it. 
// Otherwise, it falls back to compute pipeline to emulate raytracing.
// Developers aiming for a wider HW support should target Fallback Layer.
class D3D12RaytracingSimpleLighting : public DXSample
{
    enum class RaytracingAPI {
        FallbackLayer,
        DirectXRaytracing,
    };

public:
    D3D12RaytracingSimpleLighting(UINT width, UINT height, std::wstring name);

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    virtual void OnInit();
    virtual void OnKeyDown(UINT8 key);
	virtual void OnKeyUp(UINT8 key);
	virtual void OnLeftButtonDown(UINT x, UINT y);
	virtual void OnLeftButtonUp(UINT x, UINT y);
	virtual void OnMouseMove(UINT x, UINT y);
	virtual void OnMouseWheel(int delta);
	virtual void OnMiddleButtonDown(UINT x, UINT y);
	virtual void OnMiddleButtonUp(UINT x, UINT y);

    virtual void OnUpdate();
    virtual void OnRender();
    virtual void OnSizeChanged(UINT width, UINT height, bool minimized);
    virtual void OnDestroy();
    virtual IDXGISwapChain* GetSwapchain() { return m_deviceResources->GetSwapChain(); }

private:
    static const UINT FrameCount = 3;

    // We'll allocate space for several of these and they will need to be padded for alignment.
    static_assert(sizeof(SceneConstantBuffer) < D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT, "Checking the size here.");

    union AlignedSceneConstantBuffer
    {
        SceneConstantBuffer constants;
        uint8_t alignmentPadding[D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT];
    };
    AlignedSceneConstantBuffer*  m_mappedConstantData;
    ComPtr<ID3D12Resource>       m_perFrameConstants;
        
    // Raytracing Fallback Layer (FL) attributes
    ComPtr<ID3D12RaytracingFallbackDevice> m_fallbackDevice;
    ComPtr<ID3D12RaytracingFallbackCommandList> m_fallbackCommandList;
    ComPtr<ID3D12RaytracingFallbackStateObject> m_fallbackStateObject;
    WRAPPED_GPU_POINTER m_fallbackTopLevelAccelerationStructurePointer;

    // DirectX Raytracing (DXR) attributes
    ComPtr<ID3D12Device5> m_dxrDevice;
    ComPtr<ID3D12GraphicsCommandList5> m_dxrCommandList;
    ComPtr<ID3D12StateObject> m_dxrStateObject;
    bool m_isDxrSupported;

    // Root signatures
    ComPtr<ID3D12RootSignature> m_raytracingGlobalRootSignature;
    ComPtr<ID3D12RootSignature> m_raytracingLocalRootSignature;

    // Descriptors
    ComPtr<ID3D12DescriptorHeap> m_descriptorHeap;
    UINT m_descriptorsAllocated;
    UINT m_descriptorSize;
    
    // Raytracing scene
    SceneConstantBuffer m_sceneCB[FrameCount];
    CubeConstantBuffer m_cubeCB;

    // Geometry
    struct D3DBuffer
    {
        ComPtr<ID3D12Resource> resource;
        D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptorHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE gpuDescriptorHandle;
    };
    D3DBuffer m_indexBuffer;
    D3DBuffer m_vertexBuffer;

	// Buffers for swapping
	D3DBuffer m_indexBufferSwap;
	D3DBuffer m_vertexBufferSwap;
	UINT64 numIdxBytes;
	UINT64 numVtxBytes;

	// Compute
	static const UINT ThreadCount = 1;
	ComPtr<ID3D12PipelineState> m_computeState;
	ComPtr<ID3D12RootSignature> m_computeRootSignature;
	ComPtr<ID3D12CommandAllocator> m_computeAllocator[ThreadCount];
	ComPtr<ID3D12CommandQueue> m_computeCommandQueue[ThreadCount];
	ComPtr<ID3D12GraphicsCommandList> m_computeCommandList[ThreadCount];

    // Acceleration structure
    ComPtr<ID3D12Resource> m_bottomLevelAccelerationStructure;
    ComPtr<ID3D12Resource> m_topLevelAccelerationStructure;
	ComPtr<ID3D12Resource> m_scratchUpdateASResource;

    // Raytracing output
    ComPtr<ID3D12Resource> m_raytracingOutput;
    D3D12_GPU_DESCRIPTOR_HANDLE m_raytracingOutputResourceUAVGpuDescriptor;
    UINT m_raytracingOutputResourceUAVDescriptorHeapIndex;

    // Shader tables
    static const wchar_t* c_hitGroupName;
    static const wchar_t* c_raygenShaderName;
    static const wchar_t* c_closestHitShaderName;
    static const wchar_t* c_missShaderName;
    ComPtr<ID3D12Resource> m_missShaderTable;
    ComPtr<ID3D12Resource> m_hitGroupShaderTable;
    ComPtr<ID3D12Resource> m_rayGenShaderTable;
    
    // Application state
    RaytracingAPI m_raytracingAPI;
    bool m_forceComputeFallback;
    StepTimer m_timer;
    float m_curRotationAngleRad;
    XMVECTOR m_eye;
    XMVECTOR m_at;
    XMVECTOR m_up;
	XMVECTOR m_ref;

	// Animation data
	float dt = 0.2;
	float currentTime = 0;
	float prevTime = 0;
	float elapsedTime = 0;
	std::vector<std::unique_ptr<Animation>> anims;
	cudaVertex *newVertices;

	// Model data
	std::unique_ptr<Model> m;
	bool m_altDown;
	bool m_lMouseDown;
	bool m_mMouseDown;
	int m_xClick;
	int m_yClick;

	float m_theta;
	float m_phi;
	float m_zoom;
	bool m_sphereAnglesDirty;

    void EnableDirectXRaytracing(IDXGIAdapter1* adapter);
    void ParseCommandLineArgs(WCHAR* argv[], int argc);
    void UpdateCameraMatrices();
	void UpdateCameraSphere();
    void InitializeScene();
	void ResetCamera();
    void RecreateD3D();
    void DoRaytracing();
    void CreateConstantBuffers();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
    void ReleaseDeviceDependentResources();
    void ReleaseWindowSizeDependentResources();
    void CreateRaytracingInterfaces();
    void SerializeAndCreateRaytracingRootSignature(D3D12_ROOT_SIGNATURE_DESC& desc, ComPtr<ID3D12RootSignature>* rootSig);
    void CreateRootSignatures();
    void CreateLocalRootSignatureSubobjects(CD3D12_STATE_OBJECT_DESC* raytracingPipeline);
    void CreateRaytracingPipelineStateObject();
    void CreateDescriptorHeap();
    void CreateRaytracingOutputResource();
    void BuildGeometry();
	void BuildModelGeometry(Model &m);
	std::unique_ptr<MorphTarget> mt;
	void BuildMorphTarget(MorphTarget &m);
	void UpdateMorphTarget(float elapsed);
	void SwapGeometryBuffers();
    void BuildAccelerationStructures();
	void UpdateTopLevelAS();
	void UpdateBottomLevelAS();
	void FakeSkinTest();
    void BuildShaderTables();
    void SelectRaytracingAPI(RaytracingAPI type);
    void UpdateForSizeChange(UINT clientWidth, UINT clientHeight);
    void CopyRaytracingOutputToBackbuffer();
    void CalculateFrameStats();
    UINT AllocateDescriptor(D3D12_CPU_DESCRIPTOR_HANDLE* cpuDescriptor, UINT descriptorIndexToUse = UINT_MAX);
    UINT CreateBufferSRV(D3DBuffer* buffer, UINT numElements, UINT elementSize);
    WRAPPED_GPU_POINTER CreateFallbackWrappedPointer(ID3D12Resource* resource, UINT bufferNumElements);
	HRESULT CompileComputeShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint,
		_In_ ID3D11Device* device, _Outptr_ ID3DBlob** blob);
	int CompileComputeShaders();
	void Skin();
	void GPUFakeSkin(float);
	void GPUSkin(float);
	void GPUTriangleSim(float);
	std::unique_ptr<AnimationPlayer> animPlayer;
	std::unique_ptr<AnimationClip> animClip;
	void BuildTriangles();
	std::vector<glm::mat4> triangleTransforms;
	std::vector<Vertex> triangleVerts;
	std::vector<Index> triangleIdx;
	glm::mat4 *transformsOut;
	void BuildAllGeometry();
	void UpdateAllGeometry();
};

inline std::wstring s2ws(const std::string &s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}