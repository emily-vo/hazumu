
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "cudaSkinning.h"

#include <cuda.h>
#define GLM_FORCE_PURE
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/compatibility.hpp>
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
__global__ void triangleTransformKernel(int numTriangles, glm::mat4 *transformsIn, glm::mat4 *transformsOut, const float time);
__global__ void triangleSimKernel(int numVerts, glm::mat4 *transforms, cudaVertex *in, cudaVertex *out, const float time);
__global__ void skinKernel(int numVerts, glm::mat4 *transforms, cudaVertexBoneData *bones, cudaVertex *in, cudaVertex *out, const float time);
__global__ void morphTargetKernel(int numVerts, cudaVertex *target1, cudaVertex *target2, cudaVertex *out, const float alpha);
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

void cudaMorph(int numVerts, cudaVertex *target1, cudaVertex *target2, cudaVertex *vertsOut, const float alpha) {
	int blockSize = 128;
	dim3 blocksPerGrid((numVerts + blockSize - 1) / blockSize);
	dim3 threadsPerBlock(blockSize);

	cudaVertex *dv_itarget1, *dv_itarget2, *dv_odata;

	cudaMalloc((void **) &dv_itarget1, numVerts * sizeof(cudaVertex));
	cudaMalloc((void **) &dv_itarget2, numVerts * sizeof(cudaVertex));
	cudaMalloc((void **) &dv_odata, numVerts * sizeof(cudaVertex));
	cudaMemcpy(dv_itarget1, target1, numVerts * sizeof(cudaVertex), cudaMemcpyHostToDevice);
	cudaMemcpy(dv_itarget2, target2, numVerts * sizeof(cudaVertex), cudaMemcpyHostToDevice);

	morphTargetKernel << <blocksPerGrid, threadsPerBlock >> > (numVerts, dv_itarget1, dv_itarget2, dv_odata, alpha);

	cudaMemcpy(vertsOut, dv_odata, numVerts * sizeof(cudaVertex), cudaMemcpyDeviceToHost);
	cudaFree(dv_itarget1);
	cudaFree(dv_itarget2);
	cudaFree(dv_odata);
}

void triangleSim(int numVerts, glm::mat4 *transformsIn, glm::mat4 *transformsOut, cudaVertex *vertsIn, cudaVertex *vertsOut, const float time) {

	int blockSize = 128;
	dim3 blocksPerGrid((numVerts + blockSize - 1) / blockSize);
	dim3 threadsPerBlock(blockSize);

	cudaVertex *dv_idata, *dv_odata;

	cudaMalloc((void **)&dv_idata, numVerts * sizeof(cudaVertex));
	cudaMalloc((void **)&dv_odata, numVerts * sizeof(cudaVertex));
	cudaMemcpy(dv_idata, vertsIn, numVerts * sizeof(cudaVertex), cudaMemcpyHostToDevice);


	glm::mat4 *dv_itransforms, *dv_otransforms;
	cudaMalloc((void **)&dv_itransforms, (numVerts / 3) * sizeof(glm::mat4));
	cudaMalloc((void **)&dv_otransforms, (numVerts / 3) * sizeof(glm::mat4));
	cudaMemcpy(dv_itransforms, transformsIn, (numVerts / 3) * sizeof(glm::mat4), cudaMemcpyHostToDevice);
	triangleTransformKernel << <blocksPerGrid, threadsPerBlock >> > (numVerts / 3, dv_itransforms, dv_otransforms, time);
	triangleSimKernel << <blocksPerGrid, threadsPerBlock >> > (numVerts, dv_itransforms, dv_idata, dv_odata, time);

	cudaMemcpy(vertsOut, dv_odata, numVerts * sizeof(cudaVertex), cudaMemcpyDeviceToHost);
	cudaMemcpy(transformsOut, dv_otransforms, numVerts / 3 * sizeof(glm::mat4), cudaMemcpyDeviceToHost);
	cudaFree(dv_idata);
	cudaFree(dv_odata);
	cudaFree(dv_itransforms);
	cudaFree(dv_otransforms);
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
	//glm::vec3 pos = xm2vec3(v.position);
	//glm::vec3 nor = xm2vec3(v.normal);
	//auto rot = glm::rotate((time / 24.f) * 360.f, glm::vec3(0, 1, 0));
	//rot = glm::mat4(0.0f);
	//pos = glm::vec3(rot * glm::vec4(pos, 1.f));
	out[i] = in[i];
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

	__global__ void morphTargetKernel(int numVerts, cudaVertex *target1, cudaVertex *target2, cudaVertex *out, float alpha) {
	int i = threadIdx.x + (blockIdx.x * blockDim.x);
	if (i >= numVerts) { return; }
	auto &v1 = target1[i];
	auto &v2 = target2[i];
	glm::vec3 pos1 = xm2vec3(v1.position);
	glm::vec3 pos2 = xm2vec3(v2.position);
	glm::vec3 nor1 = xm2vec3(v1.normal);
	glm::vec3 nor2 = xm2vec3(v2.normal);
	glm::vec3 pos = glm::lerp(pos1, pos2, alpha);
	glm::vec3 nor = glm::lerp(nor1, nor2, alpha);
	out[i] = makeCUDAVertex(pos, nor);
}
__global__ void triangleTransformKernel(int numTriangles, glm::mat4 *transformsIn, glm::mat4 *transformsOut, const float time) {
	int i = threadIdx.x + (blockIdx.x * blockDim.x);
	if (i >= numTriangles) { return; }
	//auto rot = glm::rotate(5, glm::vec3(0, 1, 0));
	glm::vec3 t = glm::vec3(0.1, 0, 0);
	transformsOut[i] = transformsIn[i];
	//transformsOut[i] = glm::translate(transformsIn[i], t);
	//transformsOut[i] = glm::rotate(transformsIn[i], time, glm::vec3(glm::normalize(transformsIn[i][3])));
}

//Twist function from  http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm


__forceinline__ __device__  glm::vec3 invertSpace(glm::vec3 p, float s)
{
	return s * p / glm::dot(p, p);
}
__forceinline__ __device__ glm::vec3 twist(glm::vec3 p, float time) {
	float t = glm::sin(time) * p.y;
	float ct = glm::cos(t) * 1.0;
	float st = glm::sin(t) * 1.0;

	glm::vec3 pos = p;

	pos.x = p.x * ct - p.z * st;
	pos.z = p.x * st + p.z * ct;
	return pos;
}

__forceinline__ __device__ float fract(float a) {
	float x = floor(a);
	return a - x;
}


__forceinline__ __device__ float lerp(float a, float b, float t) {
	return a * (1.0f - t) + b * t;
}

__forceinline__ __device__  glm::vec4 lerp(glm::vec4 a, glm::vec4 b, float t) {
	return a * (1.0f - t) + b * t;
}

__forceinline__ __device__  float cerp(float a, float b, float t) {
	float cos_t = (1.0f - cos(t*3.14159f)) * 0.5f;
	return lerp(a, b, cos_t);
}

__forceinline__ __device__ glm::vec3 palette(float t, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d)
{
	return a + b * cos(6.28318f *(c*t + d));
}

__forceinline__ __device__ float random(float a, float b, float c) {
	return fract(glm::sin(glm::dot(glm::vec3(a, b, c), glm::vec3(12.9898, 78.233, 578.233)))*43758.5453);
}


__forceinline__ __device__ float interpolateNoise(float x, float y, float z) {
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

__forceinline__ __device__ float generateNoise(float x, float y, float z) {
	float total = 0.0;
	float persistence = 1.0 / 2.0;
	int its = 0;
	float scale = 2.0;
	float freq = 1.0;
	float ampl = 1.0;
	for (int i = 0; i < 32; i++) {
		freq *= scale;
		ampl *= persistence;
		total += interpolateNoise(freq*x, freq*y, freq*z)*ampl;
	}
	return total;
}


__global__ void triangleSimKernel(int numVerts, glm::mat4 *transforms, cudaVertex *in, cudaVertex *out, const float time) {
	int i = threadIdx.x + (blockIdx.x * blockDim.x);
	if (i >= numVerts) { return; }
	int triangleIdx = i / 3;
	int firstPos = triangleIdx * 3;
	glm::vec3 triangleCenter = glm::vec3(0.f);
	for (int j = 0; j < 3; j++) {
		auto &v = in[firstPos + j];
		glm::vec3 p = xm2vec3(v.position);
		triangleCenter += p;
	}
	triangleCenter /= 3.0f;
	//for (int j = 0; j < 3; ++j) {
	auto &v = in[i];
	glm::vec3 pos = xm2vec3(v.position);
	glm::vec3 nor = xm2vec3(v.normal);

	if (nor[1] == 1.0) {
		glm::vec3 p = twist(triangleCenter, time / 10.f);
		glm::mat4 transform = glm::translate(p);
		pos = glm::vec3(transform * glm::vec4(pos, 1.0f));
		out[i] = makeCUDAVertex(pos, nor);
	}
	else {
		glm::vec3 p = twist(pos, time / 10.f);
		glm::mat4 transform = glm::mat4(1.f);
		pos = glm::vec3(transform * glm::vec4(p, 1.0f));

		out[i] = makeCUDAVertex(pos, nor);
	}
	
	//}

}