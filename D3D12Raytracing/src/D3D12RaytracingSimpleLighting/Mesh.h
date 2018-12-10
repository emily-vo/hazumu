#pragma once

#include "stdafx.h"
#include "RaytracingHlslCompat.h"
#include <assimp/scene.h>
#include "Bone.h"
#include "Animation.h"
#include <glm/glm.hpp>

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
		return tmp;
	}

	SceneNode* RootNode();
	SceneNode* BuildSkeleton(aiNode& node, SceneNode* parentSceneNode);

	String name;
	std::vector<VertexBoneData> mBones;
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
	std::unordered_map<String, int> m_BoneMapping;

	std::vector<BoneInfo> m_BoneInfo;
	glm::mat4 m_GlobalInverseTransform;
	int m_NumBones;


	std::vector<Bone*> m_bones;
	std::unordered_map<String, UINT> mBoneIndexMapping;
	SceneNode *mRootNode;
	std::vector<BoneVertexWeights> mBoneWeights;
};