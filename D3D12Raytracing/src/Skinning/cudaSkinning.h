#pragma once

#include <DirectXMath.h>
#include <cuda.h>
#define GLM_FORCE_PURE
#include <glm/glm.hpp>

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
void cudaSkin(int numVerts, int numTransforms, glm::mat4 *transforms, cudaVertexBoneData *bones, cudaVertex *vertsIn, cudaVertex *vertsOut, const float time);
void cudaMorph(int numVerts, cudaVertex *target1, cudaVertex *target2, cudaVertex *vertsOut, const float alpha);
void triangleSim(int numVerts, glm::mat4 *transformsIn, glm::mat4 *transformsOut, cudaVertex *vertsIn, cudaVertex *vertsOut, const float time);
