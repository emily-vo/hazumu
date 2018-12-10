#pragma once

#include <DirectXMath.h>

using namespace DirectX;

struct cudaVertex {
	XMFLOAT3 position;
	XMFLOAT3 normal;
};

void cudaFakeSkin(int numVerts, cudaVertex *vertsIn, cudaVertex *vertsOut, const float time);