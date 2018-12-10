
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cudaSkinning.h"

#include <cuda.h>
#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define PI 3.14159265358979323846

#define cudaCheckError() {                                          \
 cudaError_t e=cudaGetLastError();                                 \
 if(e!=cudaSuccess) {                                              \
   printf("Cuda failure %s:%d: '%s'\n",__FILE__,__LINE__,cudaGetErrorString(e));           \
   exit(0); \
 }                                                                 \
}


__global__ void fakeSkinKernel(int numVerts, cudaVertex *in, cudaVertex *out, const float time);
__global__ void skinKernel(int numVerts, glm::mat4 *transforms, cudaVertexBoneData *bones, cudaVertex *in, cudaVertex *out, const float time);
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

void cudaSkin(int numVerts, int numTransforms, glm::mat4 *transforms, cudaVertexBoneData *bones, cudaVertex *vertsIn, cudaVertex *vertsOut, const float time) {
	int blockSize = 128;
	dim3 blocksPerGrid((numVerts + blockSize - 1) / blockSize);
	dim3 threadsPerBlock(blockSize);

	cudaVertex *dv_idata, *dv_odata;

	cudaMalloc((void **)&dv_idata, numVerts * sizeof(cudaVertex));
	cudaMalloc((void **)&dv_odata, numVerts * sizeof(cudaVertex));
	cudaMemcpy(dv_idata, vertsIn, numVerts * sizeof(cudaVertex), cudaMemcpyHostToDevice);

	glm::mat4 *dv_transforms;
	cudaMalloc((void **)&dv_transforms, numTransforms * sizeof(glm::mat4));
	cudaMemcpy(dv_transforms, transforms, numTransforms * sizeof(glm::mat4), cudaMemcpyHostToDevice);

	cudaVertexBoneData *dv_bones;
	cudaMalloc((void **)&dv_bones, numVerts * sizeof(cudaVertexBoneData));
	cudaMemcpy(dv_bones, bones, numVerts * sizeof(cudaVertexBoneData), cudaMemcpyHostToDevice);

	skinKernel << <blocksPerGrid, threadsPerBlock >> > (numVerts, dv_transforms, dv_bones, dv_idata, dv_odata, time);

	cudaMemcpy(vertsOut, dv_odata, numVerts * sizeof(cudaVertex), cudaMemcpyDeviceToHost);

	cudaFree(dv_idata);
	cudaFree(dv_odata);
	cudaFree(dv_transforms);
	cudaFree(dv_bones);
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
glm::mat4 xm2mat4(XMFLOAT4X4 other) {
	float aaa[16];
	aaa[0] = other._11;
	aaa[1] = other._21;
	aaa[2] = other._31;
	aaa[3] = other._41;

	aaa[4] = other._12;
	aaa[5] = other._22;
	aaa[6] = other._32;
	aaa[7] = other._42;

	aaa[8] = other._13;
	aaa[9] = other._23;
	aaa[10] = other._33;
	aaa[11] = other._43;

	aaa[12] = other._14;
	aaa[13] = other._24;
	aaa[14] = other._34;
	aaa[15] = other._44;
	return glm::make_mat4(aaa);
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

__global__ void skinKernel(int numVerts, glm::mat4 *transforms, cudaVertexBoneData *bones, cudaVertex *in, cudaVertex *out, const float time) {
	int i = threadIdx.x + (blockIdx.x * blockDim.x);
	if (i >= numVerts) { return; }
	auto &v = in[i];
	auto &bone = bones[i];
	glm::vec3 pos = xm2vec3(v.position);
	glm::vec3 nor = xm2vec3(v.normal);
	glm::mat4 transform(0.f);
	float totalWeight = 0.f;
	for (int k = 0; k < 4; k++) {
		int ID = bone.IDs[k];
		float weight = bone.Weights[k];
		totalWeight += weight;
		glm::mat4 mat = (transforms[ID]);
		transform += mat * weight;
	}

	pos = glm::vec3(transform * glm::vec4(pos, 1.0f));
	nor = glm::vec3(transform * glm::vec4(nor, 0.0f));

	out[i] = makeCUDAVertex(pos, nor);
}