#include "Bone.h"
#include "Model.h"
#include "MatrixHelper.h"
#include <assimp/scene.h>

RTTI_DEFINITIONS(Bone)

const std::vector<BoneVertexWeights::VertexWeight>& BoneVertexWeights::Weights()
{
	return mWeights;
}

void BoneVertexWeights::AddWeight(float weight, UINT boneIndex)
{
	if (mWeights.size() == MaxBoneWeightsPerVertex)
	{
		throw std::runtime_error("Maximum number of bone weights per vertex exceeded.");
	}

	mWeights.push_back(VertexWeight(weight, boneIndex));
}

Bone::Bone(const std::string& name, UINT index, const XMFLOAT4X4& offsetTransform)
: SceneNode(name), mIndex(index), mOffsetTransform(offsetTransform)
{
}

UINT Bone::Index() const
{
	return mIndex;
}

void Bone::SetIndex(UINT index)
{
	mIndex = index;
}

const XMFLOAT4X4& Bone::OffsetTransform() const
{
	return mOffsetTransform;
}

XMMATRIX Bone::OffsetTransformMatrix() const
{
	return XMLoadFloat4x4(&mOffsetTransform);
}
