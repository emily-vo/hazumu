#pragma once

#include "stdafx.h"
#include "RaytracingHlslCompat.h"
#include <assimp/scene.h>
#include "Bone.h"

using namespace DirectX;
using String = std::string;

class Mesh {
public:
	Mesh(String name);
	Mesh(Mesh &&other);

	static std::unique_ptr<Mesh> LoadFromAiMesh(aiMesh *mesh);

	DirectX::XMMATRIX aiMatToXMMatrix(const aiMatrix4x4 &a) const {
		float arr[16] = {
			a.a1, a.a2, a.a3, a.a4,
			a.b1, a.b2, a.b3, a.b4,
			a.c1, a.c2, a.c3, a.c4,
			a.d1, a.d2, a.d3, a.d4 };
		DirectX::XMMATRIX mat = DirectX::XMMATRIX(arr);
		return mat;
	}

	SceneNode* RootNode();
	SceneNode* BuildSkeleton(aiNode& node, SceneNode* parentSceneNode);

	String name;
	std::vector<VertexBoneData> mBones;
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
	std::unordered_map<String, int> m_BoneMapping;

	std::vector<BoneInfo> m_BoneInfo;
	XMMATRIX m_GlobalInverseTransform;
	int m_NumBones;


	std::vector<Bone*> m_bones;
	std::unordered_map<String, UINT> mBoneIndexMapping;
	SceneNode *mRootNode;
	std::vector<BoneVertexWeights> mBoneWeights;
};