
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cudaSkinning.h"

#include <cuda.h>
#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include <stdio.h>

#define PI 3.14159265358979323846


__global__ void fakeSkinKernel(int numVerts, cudaVertex *in, cudaVertex *out, const float time);

void cudaFakeSkin(int numVerts, cudaVertex *vertsIn, cudaVertex *vertsOut, const float time) {
	int blockSize = 128;
	dim3 blocksPerGrid((numVerts + blockSize - 1) / blockSize);
	dim3 threadsPerBlock(blockSize);

	cudaVertex *dv_idata, *dv_odata;

	cudaMalloc((void **) &dv_idata, numVerts * sizeof(cudaVertex));
	cudaMalloc((void **) &dv_odata, numVerts * sizeof(cudaVertex));
	cudaMemcpy(dv_idata, vertsIn, numVerts * sizeof(cudaVertex), cudaMemcpyHostToDevice);

	fakeSkinKernel << <blocksPerGrid, threadsPerBlock >> > (numVerts, dv_idata, dv_odata, time);

	cudaMemcpy(vertsOut, dv_odata, numVerts * sizeof(cudaVertex), cudaMemcpyDeviceToHost);
	cudaFree(dv_idata);
	cudaFree(dv_odata);
}

__forceinline__
__device__ __host__
double convertToRadians(double deg) {
	return deg * (PI / 180.);
}

__forceinline__
__device__ __host__
glm::vec3 xm2vec3(XMFLOAT3 other) {
	return glm::vec3(other.x, other.y, other.z);
}

__forceinline__
__device__ __host__
cudaVertex makeCUDAVertex(glm::vec3 position, glm::vec3 normal) {
	cudaVertex ret;
	ret.position.x = position.x;
	ret.position.y = position.y;
	ret.position.z = position.z;
	ret.normal.x = normal.x;
	ret.normal.y = normal.y;
	ret.normal.z = normal.z;
	return ret;
}

__global__ void fakeSkinKernel(int numVerts, cudaVertex *in, cudaVertex *out, const float time) {
	int i = threadIdx.x + (blockIdx.x * blockDim.x);
	if (i >= numVerts) { return; }
	auto &v = in[i];
	glm::vec3 pos = xm2vec3(v.position);
	glm::vec3 nor = xm2vec3(v.normal);
	auto rot = glm::rotate((time / 24.f) * 360.f, glm::vec3(0, 1, 0));
	pos = glm::vec3(rot * glm::vec4(pos, 1.f));
	out[i] = makeCUDAVertex(pos, nor);
}
