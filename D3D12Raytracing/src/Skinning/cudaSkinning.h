#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct cudaVertex {
	XMFLOAT3 position;
	XMFLOAT3 normal;
};

struct cudaVertexBoneData
{
	int IDs[4];
	float Weights[4];
};

void cudaFakeSkin(int numVerts, cudaVertex *vertsIn, cudaVertex *vertsOut, const float time);
void cudaSkin(int numVerts, int numTransforms, XMFLOAT4X4 *transforms, cudaVertexBoneData *bones, cudaVertex *vertsIn, cudaVertex *vertsOut, const float time);