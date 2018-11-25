#pragma once

#include "stdafx.h"
#include "RaytracingHlslCompat.h"
#include <assimp/scene.h>
#include "Mesh.h"

using namespace DirectX;
using String = std::string;

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
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const XMMATRIX& ParentTransform);
	void BoneTransform(float TimeInSeconds, std::vector<XMMATRIX>& Transforms);

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
		float arr[16] = {
			a.x, 0, 0, 0,
			0, a.y, 0, 0,
			0, 0, a.z, 0,
			0, 0, 0, 1 };
		DirectX::XMMATRIX mat = DirectX::XMMATRIX(arr);
		return mat;
	}
	String name;
	aiScene *m_pScene;
	Mesh *m;
};