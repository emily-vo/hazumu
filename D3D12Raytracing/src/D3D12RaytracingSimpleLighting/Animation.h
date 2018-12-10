#pragma once

#include "stdafx.h"
#include "RaytracingHlslCompat.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <glm/glm.hpp>

using namespace DirectX;
using String = std::string;

class Mesh;

struct BoneInfo {
	glm::mat4 BoneOffset;
	glm::mat4 FinalTransformation;
};

class Animation {
public:
	Animation(String name, Mesh *m);
	static std::unique_ptr<Animation> LoadFromFile(String filename, Mesh *m);
	~Animation();
	UINT FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
	UINT FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	UINT FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const String NodeName);
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);
	void BoneTransform(float TimeInSeconds, std::vector<glm::mat4>& Transforms);

	glm::mat4 aiToMat4(const aiMatrix4x4& in_mat) {
		glm::mat4 tmp;
		tmp[0][0] = in_mat.a1;
		tmp[1][0] = in_mat.b1;
		tmp[2][0] = in_mat.c1;
		tmp[3][0] = in_mat.d1;

		tmp[0][1] = in_mat.a2;
		tmp[1][1] = in_mat.b2;
		tmp[2][1] = in_mat.c2;
		tmp[3][1] = in_mat.d2;

		tmp[0][2] = in_mat.a3;
		tmp[1][2] = in_mat.b3;
		tmp[2][2] = in_mat.c3;
		tmp[3][2] = in_mat.d3;

		tmp[0][3] = in_mat.a4;
		tmp[1][3] = in_mat.b4;
		tmp[2][3] = in_mat.c4;
		tmp[3][3] = in_mat.d4;
		return glm::transpose(tmp);
	}

	DirectX::XMMATRIX aiMatToXMMatrix(const aiMatrix4x4 &a) const {
		float arr[16] = {
			a.a1, a.a2, a.a3, a.a4,
			a.b1, a.b2, a.b3, a.b4,
			a.c1, a.c2, a.c3, a.c4,
			a.d1, a.d2, a.d3, a.d4 };
		DirectX::XMMATRIX mat = DirectX::XMMATRIX(arr);
		return mat;
	}

	DirectX::XMMATRIX aiMatToXMMatrix(const aiMatrix3x3 &a) const {
		float arr[16] = {
			a.a1, a.a2, a.a3, 0,
			a.b1, a.b2, a.b3, 0,
			a.c1, a.c2, a.c3, 0,
			0, 0, 0, 1 };
		DirectX::XMMATRIX mat = DirectX::XMMATRIX(arr);
		return mat;
	}

	DirectX::XMMATRIX InitScaleTransform(const aiVector3D &a) const {
		return XMMatrixScaling(a.x, a.y, a.z);
	}
	String name;
	std::unique_ptr<Assimp::Importer> sceneImporter;
	const aiScene *m_pScene;
	Mesh *m;
};