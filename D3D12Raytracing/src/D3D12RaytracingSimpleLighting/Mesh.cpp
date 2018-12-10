#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Mesh::Mesh(String name) :
	name(name),
	vertices(),
	indices() {
	m_GlobalInverseTransform = glm::mat4(1.f);
}

Mesh::Mesh(Mesh &&other) :
	name(std::move(other.name)),
	vertices(std::move(other.vertices)),
	indices(std::move(other.indices)) {}

std::unique_ptr<Mesh> Mesh::LoadFromAiMesh(aiMesh *mesh) {
	auto m = std::make_unique<Mesh>(mesh->mName.C_Str());
	aiMesh *currMesh = mesh;
	if (!(currMesh->HasPositions()) || !(currMesh->HasFaces())) { return nullptr; }

	int idxOffset = m->vertices.size();


	XMMATRIX rot = XMMatrixRotationX(XMConvertToRadians(-90));
	// Load vertex attributes
	for (int j = 0; j < currMesh->mNumVertices; ++j) {
		aiVector3D pos = currMesh->mVertices[j];
		XMVECTOR posxm{ pos.x, pos.y, pos.z };
		XMFLOAT3 pos3;
		auto roted = XMVector3Transform(posxm, rot);
		XMStoreFloat3(&pos3, roted);
		aiVector3D nor = currMesh->HasNormals() ? currMesh->mNormals[j] : aiVector3D(0, 0, 1);
		//m->vertices.push_back({ { pos.x, pos.y, pos.z },
		//						{ nor.x, nor.y, nor.z } });
		m->vertices.push_back({ pos3,
			{nor.x, nor.y, nor.z} });
	}

	for (int j = 0; j < currMesh->mNumFaces; ++j) {
		aiFace face = currMesh->mFaces[j];
		if (face.mNumIndices != 3) {
			throw std::exception();
		}
		m->indices.push_back(idxOffset + face.mIndices[0]);
		m->indices.push_back(idxOffset + face.mIndices[1]);
		m->indices.push_back(idxOffset + face.mIndices[2]);
	}

	// Load skeleton information
	m->mBones.resize(m->vertices.size());
	for (UINT i = 0; i < currMesh->mNumBones; i++) {
		UINT BoneIndex = 0;
		String BoneName(currMesh->mBones[i]->mName.data);

		if (m->m_BoneMapping.find(BoneName) == m->m_BoneMapping.end()) {
			BoneIndex = m->m_NumBones;
			m->m_NumBones++;
			BoneInfo bi;
			m->m_BoneInfo.push_back(bi);
		}
		else {
			BoneIndex = m->m_BoneMapping[BoneName];
		}

		m->m_BoneMapping[BoneName] = BoneIndex;
		m->m_BoneInfo[BoneIndex].BoneOffset = m->aiToMat4(currMesh->mBones[i]->mOffsetMatrix);
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				if (m->m_BoneInfo[BoneIndex].BoneOffset[i][j] < 1e-6) {
					m->m_BoneInfo[BoneIndex].BoneOffset[i][j] = 0;
				}
			}
		}
		m->m_BoneInfo[BoneIndex].FinalTransformation = glm::mat4(1.f);

		for (UINT j = 0; j < currMesh->mBones[i]->mNumWeights; j++) {
			UINT VertexID = currMesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = currMesh->mBones[i]->mWeights[j].mWeight;
			m->mBones[VertexID].AddBoneData(BoneIndex, Weight);
		}
	}

	//// Bones
	//if (mesh->HasBones())
	//{
	//	m->mBoneWeights.resize(mesh->mNumVertices);

	//	for (UINT i = 0; i < mesh->mNumBones; i++)
	//	{
	//		aiBone* meshBone = mesh->mBones[i];

	//		// Look up the bone in the model's hierarchy, or add it if not found.
	//		UINT boneIndex = 0U;
	//		std::string boneName = meshBone->mName.C_Str();
	//		auto boneMappingIterator = m->mBoneIndexMapping.find(boneName);
	//		if (boneMappingIterator != m->mBoneIndexMapping.end())
	//		{
	//			boneIndex = boneMappingIterator->second;
	//		}
	//		else
	//		{
	//			boneIndex = m->mBones.size();
	//			XMMATRIX offsetMatrix = XMLoadFloat4x4(&(XMFLOAT4X4(reinterpret_cast<const float*>(meshBone->mOffsetMatrix[0]))));
	//			XMFLOAT4X4 offset;
	//			XMStoreFloat4x4(&offset, XMMatrixTranspose(offsetMatrix));

	//			Bone* modelBone = new Bone(boneName, boneIndex, offset);
	//			m->m_bones.push_back(modelBone);
	//			m->mBoneIndexMapping[boneName] = boneIndex;
	//		}

	//		for (UINT i = 0; i < meshBone->mNumWeights; i++)
	//		{
	//			aiVertexWeight vertexWeight = meshBone->mWeights[i];
	//			m->mBoneWeights[vertexWeight.mVertexId].AddWeight(vertexWeight.mWeight, boneIndex);
	//		}
	//	}
	//}

	return m;
}

SceneNode* Mesh::RootNode()
{
	return this->mRootNode;
}

SceneNode* Mesh::BuildSkeleton(aiNode& node, SceneNode* parentSceneNode)
{
	SceneNode* sceneNode = nullptr;

	auto boneMapping = mBoneIndexMapping.find(node.mName.C_Str());
	if (boneMapping == mBoneIndexMapping.end())
	{
		sceneNode = new SceneNode(node.mName.C_Str());
	}
	else
	{
		sceneNode = m_bones[boneMapping->second];
	}

	XMMATRIX transform = XMLoadFloat4x4(&(XMFLOAT4X4(reinterpret_cast<const float*>(node.mTransformation[0]))));
	sceneNode->SetTransform(XMMatrixTranspose(transform));
	sceneNode->SetParent(parentSceneNode);

	for (UINT i = 0; i < node.mNumChildren; i++)
	{
		SceneNode* childSceneNode = BuildSkeleton(*(node.mChildren[i]), sceneNode);
		sceneNode->Children().push_back(childSceneNode);
	}

	return sceneNode;
}