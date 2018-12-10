#include "BoneAnimation.h"

#include "Bone.h"
#include "Keyframe.h"
#include "Model.h"
#include <assimp/scene.h>

BoneAnimation::BoneAnimation(Mesh& model, aiNodeAnim& nodeAnim)
	: mModel(&model), mBone(nullptr), mKeyframes()
{
	UINT boneIndex = model.mBoneIndexMapping.at(nodeAnim.mNodeName.C_Str());
	mBone = model.m_bones.at(boneIndex);

	assert(nodeAnim.mNumPositionKeys == nodeAnim.mNumRotationKeys);
	assert(nodeAnim.mNumPositionKeys == nodeAnim.mNumScalingKeys);

	for (UINT i = 0; i < nodeAnim.mNumPositionKeys; i++)
	{
		aiVectorKey positionKey = nodeAnim.mPositionKeys[i];
		aiQuatKey rotationKey = nodeAnim.mRotationKeys[i];
		aiVectorKey scaleKey = nodeAnim.mScalingKeys[i];

		assert(positionKey.mTime == rotationKey.mTime);
		assert(positionKey.mTime == scaleKey.mTime);

		Keyframe* keyframe = new Keyframe(static_cast<float>(positionKey.mTime), XMFLOAT3(positionKey.mValue.x, positionKey.mValue.y, positionKey.mValue.z),
			XMFLOAT4(rotationKey.mValue.x, rotationKey.mValue.y, rotationKey.mValue.z, rotationKey.mValue.w), XMFLOAT3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z));
		mKeyframes.push_back(keyframe);
	}
}

BoneAnimation::~BoneAnimation()
{
	for (Keyframe* keyframe : mKeyframes)
	{
		delete keyframe;
	}
}

Bone& BoneAnimation::GetBone()
{
	return *mBone;
}

const std::vector<Keyframe*> BoneAnimation::Keyframes() const
{
	return mKeyframes;
}

UINT BoneAnimation::GetTransform(float time, XMFLOAT4X4& transform) const
{
	UINT keyframeIndex = FindKeyframeIndex(time);
	Keyframe* keyframe = mKeyframes[keyframeIndex];

	XMStoreFloat4x4(&transform, keyframe->Transform());

	return keyframeIndex;
}

void BoneAnimation::GetTransformAtKeyframe(UINT keyframeIndex, XMFLOAT4X4& transform) const
{
	// Clamp the keyframe
	if (keyframeIndex >= mKeyframes.size())
	{
		keyframeIndex = mKeyframes.size() - 1;
	}

	Keyframe* keyframe = mKeyframes[keyframeIndex];

	XMStoreFloat4x4(&transform, keyframe->Transform());
}

void BoneAnimation::GetInteropolatedTransform(float time, XMFLOAT4X4& transform) const
{
	Keyframe* firstKeyframe = mKeyframes.front();
	Keyframe* lastKeyframe = mKeyframes.back();

	if (time <= firstKeyframe->Time())
	{
		// Specified time is before the start time of the animation, so return the first keyframe
		XMStoreFloat4x4(&transform, firstKeyframe->Transform());
	}
	else if (time >= lastKeyframe->Time())
	{
		// Specified time is after the end time of the animation, so return the last keyframe
		XMStoreFloat4x4(&transform, lastKeyframe->Transform());
	}
	else
	{
		// Interpolate the transform between keyframes
		UINT keyframeIndex = FindKeyframeIndex(time);
		Keyframe* keyframeOne = mKeyframes[keyframeIndex];
		Keyframe* keyframeTwo = mKeyframes[keyframeIndex + 1];

		XMVECTOR translationOne = keyframeOne->TranslationVector();
		XMVECTOR rotationQuaternionOne = keyframeOne->RotationQuaternionVector();
		XMVECTOR scaleOne = keyframeOne->ScaleVector();

		XMVECTOR translationTwo = keyframeTwo->TranslationVector();
		XMVECTOR rotationQuaternionTwo = keyframeTwo->RotationQuaternionVector();
		XMVECTOR scaleTwo = keyframeTwo->ScaleVector();

		float lerpValue = ((time - keyframeOne->Time()) / (keyframeTwo->Time() - keyframeOne->Time()));
		XMVECTOR translation = XMVectorLerp(translationOne, translationTwo, lerpValue);
		XMVECTOR rotationQuaternion = XMQuaternionSlerp(rotationQuaternionOne, rotationQuaternionTwo, lerpValue);
		XMVECTOR scale = XMVectorLerp(scaleOne, scaleTwo, lerpValue);

		static XMVECTOR rotationOrigin = XMVectorZero();
		XMStoreFloat4x4(&transform, XMMatrixAffineTransformation(scale, rotationOrigin, rotationQuaternion, translation));
	}
}

UINT BoneAnimation::FindKeyframeIndex(float time) const
{
	Keyframe* firstKeyframe = mKeyframes.front();
	if (time <= firstKeyframe->Time())
	{
		return 0;
	}

	Keyframe* lastKeyframe = mKeyframes.back();
	if (time >= lastKeyframe->Time())
	{
		return mKeyframes.size() - 1;
	}

	UINT keyframeIndex = 1;

	for (; keyframeIndex < mKeyframes.size() - 1 && time >= mKeyframes[keyframeIndex]->Time(); keyframeIndex++);

	return keyframeIndex - 1;
}
