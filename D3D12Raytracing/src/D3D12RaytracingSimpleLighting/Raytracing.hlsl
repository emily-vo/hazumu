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

#ifndef RAYTRACING_HLSL
#define RAYTRACING_HLSL

#define HLSL
#include "RaytracingHlslCompat.h"

RaytracingAccelerationStructure Scene : register(t0, space0);
RWTexture2D<float4> RenderTarget : register(u0);
ByteAddressBuffer Indices : register(t1, space0);
StructuredBuffer<Vertex> Vertices : register(t2, space0);

ConstantBuffer<SceneConstantBuffer> g_sceneCB : register(b0);
ConstantBuffer<CubeConstantBuffer> g_cubeCB : register(b1);

// Load three 16 bit indices from a byte addressed buffer.
uint3 Load3x16BitIndices(uint offsetBytes)
{
    uint3 indices;

    // ByteAdressBuffer loads must be aligned at a 4 byte boundary.
    // Since we need to read three 16 bit indices: { 0, 1, 2 } 
    // aligned at a 4 byte boundary as: { 0 1 } { 2 0 } { 1 2 } { 0 1 } ...
    // we will load 8 bytes (~ 4 indices { a b | c d }) to handle two possible index triplet layouts,
    // based on first index's offsetBytes being aligned at the 4 byte boundary or not:
    //  Aligned:     { 0 1 | 2 - }
    //  Not aligned: { - 0 | 1 2 }
    const uint dwordAlignedOffset = offsetBytes & ~3;    
    const uint2 four16BitIndices = Indices.Load2(dwordAlignedOffset);
 
    // Aligned: { 0 1 | 2 - } => retrieve first three 16bit indices
    if (dwordAlignedOffset == offsetBytes)
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = (four16BitIndices.x >> 16) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else // Not aligned: { - 0 | 1 2 } => retrieve last three 16bit indices
    {
        indices.x = (four16BitIndices.x >> 16) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = (four16BitIndices.y >> 16) & 0xffff;
    }

    return indices;
}

typedef BuiltInTriangleIntersectionAttributes MyAttributes;
struct RayPayload
{
    float4 color;
};

// Retrieve hit world position.
float3 HitWorldPosition()
{
    return WorldRayOrigin() + RayTCurrent() * WorldRayDirection();
}

// Retrieve attribute at a hit position interpolated from vertex attributes using the hit's barycentrics.
float3 HitAttribute(float3 vertexAttribute[3], BuiltInTriangleIntersectionAttributes attr)
{
    return vertexAttribute[0] +
        attr.barycentrics.x * (vertexAttribute[1] - vertexAttribute[0]) +
        attr.barycentrics.y * (vertexAttribute[2] - vertexAttribute[0]);
}

// Generate a ray in world space for a camera pixel corresponding to an index from the dispatched 2D grid.
inline void GenerateCameraRay(uint2 index, out float3 origin, out float3 direction)
{
    float2 xy = index + 0.5f; // center in the middle of the pixel.
    float2 screenPos = xy / DispatchRaysDimensions().xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates.
    screenPos.y = -screenPos.y;

    // Unproject the pixel coordinate into a ray.
    float4 world = mul(float4(screenPos, 0, 1), g_sceneCB.projectionToWorld);

    world.xyz /= world.w;
    origin = g_sceneCB.cameraPosition.xyz;
    direction = normalize(world.xyz - origin);
}
float fract(float a) {
	float x = floor(a);
	return a - x;
}
float lerp(float a, float b, float t) {
	return a * (1.0 - t) + b * t;
}

float4 lerp(float4 a, float4 b, float t) {
	return a * (1.0 - t) + b * t;
}

float cerp(float a, float b, float t) {
	float cos_t = (1.0 - cos(t*3.14159)) * 0.5;
	return lerp(a, b, cos_t);
}

float3 palette(float t, float3 a, float3 b, float3 c, float3 d)
{
	return a + b * cos(6.28318*(c*t + d));
}

float random(float a, float b, float c) {
	return fract(sin(dot(float3(a, b, c), float3(12.9898, 78.233, 578.233)))*43758.5453);
}


float interpolateNoise(float x, float y, float z) {
	float x0, y0, z0, x1, y1, z1;

	// Find the grid voxel that this point falls in
	x0 = floor(x);
	y0 = floor(y);
	z0 = floor(z);

	x1 = x0 + 1.0;
	y1 = y0 + 1.0;
	z1 = z0 + 1.0;

	// Generate noise at each of the 8 points
	float FUL, FUR, FLL, FLR, BUL, BUR, BLL, BLR;

	// front upper left
	FUL = random(x0, y1, z1);

	// front upper right
	FUR = random(x1, y1, z1);

	// front lower left
	FLL = random(x0, y0, z1);

	// front lower right
	FLR = random(x1, y0, z1);

	// back upper left
	BUL = random(x0, y1, z0);

	// back upper right
	BUR = random(x1, y1, z0);

	// back lower left
	BLL = random(x0, y0, z0);

	// back lower right
	BLR = random(x1, y0, z0);

	// Find the interpolate t values
	float n0, n1, m0, m1, v;
	float tx = fract(x - x0);
	float ty = fract(y - y0);
	float tz = fract(z - z0);
	tx = (x - x0);
	ty = (y - y0);
	tz = (z - z0);

	// interpolate along x and y for back
	n0 = cerp(BLL, BLR, tx);
	n1 = cerp(BUL, BUR, tx);
	m0 = cerp(n0, n1, ty);

	// interpolate along x and y for front
	n0 = cerp(FLL, FLR, tx);
	n1 = cerp(FUL, FUR, tx);
	m1 = cerp(n0, n1, ty);

	// interpolate along z
	v = cerp(m0, m1, tz);

	return v;
}

float generateNoise(float x, float y, float z) {
	float total = 0.0;
	float persistence = 1.0 / 2.0;
	int its = 0;
	for (int i = 0; i < 10; i++) {
		float freq = pow(2.0, float(i));
		float ampl = pow(persistence, float(i));
		total += interpolateNoise(freq*x, freq*y, freq*z)*ampl;
	}
	return total;
}

// Diffuse lighting calculation.
float4 CalculateDiffuseLighting(float3 hitPosition, float3 normal)
{
    float3 pixelToLight = normalize(g_sceneCB.lightPosition.xyz - hitPosition);

    // Diffuse contribution.
    float fNDotL = abs(dot(pixelToLight, normal));

	float3 a = float3(0.5, 0.5, 0.5);
	float3 b = float3(0.5, 0.5, 0.5);
	float3 c = float3(1.0, 1.0, 1.0);
	float3 d = float3(0.00, 0.33, 0.67);
	float s = 2.0;
	float3 color = palette(generateNoise(hitPosition.x / s, hitPosition.y / s, hitPosition.z / s), a, b, c, d);
	float4 color4 = float4(color[0], color[1], color[2], 1.0);
	return color4;
}


[shader("raygeneration")]
void MyRaygenShader()
{
    float3 rayDir;
    float3 origin;
    
    // Generate a ray for a camera pixel corresponding to an index from the dispatched 2D grid.
    GenerateCameraRay(DispatchRaysIndex().xy, origin, rayDir);

    // Trace the ray.
    // Set the ray's extents.
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = rayDir;
    // Set TMin to a non-zero small value to avoid aliasing issues due to floating - point errors.
    // TMin should be kept small to prevent missing geometry at close contact areas.
    ray.TMin = 0.001;
    ray.TMax = 10000.0;
    RayPayload payload = { float4(0, 0, 0, 0) };
    TraceRay(Scene, RAY_FLAG_NONE, ~0, 0, 1, 0, ray, payload);

    // Write the raytraced color to the output texture.
    RenderTarget[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void MyClosestHitShader(inout RayPayload payload, in MyAttributes attr)
{
    float3 hitPosition = HitWorldPosition();

    // Get the base index of the triangle's first 16 bit index.
    uint indexSizeInBytes = 2;
    uint indicesPerTriangle = 3;
    uint triangleIndexStride = indicesPerTriangle * indexSizeInBytes;
    uint baseIndex = PrimitiveIndex() * triangleIndexStride;

    // Load up 3 16 bit indices for the triangle.
    const uint3 indices = Load3x16BitIndices(baseIndex);

    // Retrieve corresponding vertex normals for the triangle vertices.
    float3 vertexNormals[3] = { 
        Vertices[indices[0]].normal, 
        Vertices[indices[1]].normal, 
        Vertices[indices[2]].normal 
    };

	float3 vertexPos[3] = {
		Vertices[indices[0]].position,
		Vertices[indices[1]].position,
		Vertices[indices[2]].position
	};

    // Compute the triangle's normal.
    // This is redundant and done for illustration purposes 
    // as all the per-vertex normals are the same and match triangle's normal in this sample. 
    float3 triangleNormal = HitAttribute(vertexNormals, attr);
	triangleNormal = cross(vertexPos[2] - vertexPos[1], vertexPos[1] - vertexPos[0]);

    float4 diffuseColor = CalculateDiffuseLighting(hitPosition, triangleNormal);
    float4 color = diffuseColor;

    payload.color = color;
}

[shader("miss")]
void MyMissShader(inout RayPayload payload)
{
    float4 background = float4(0.0f, 0.0f, 0.0f, 1.0f);
    payload.color = background;
}

#endif // RAYTRACING_HLSL