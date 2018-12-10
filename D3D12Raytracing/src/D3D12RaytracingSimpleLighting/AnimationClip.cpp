#include "AnimationClip.h"
#include "MatrixHelper.h"
#include "BoneAnimation.h"
#include <assimp/scene.h>

AnimationClip::AnimationClip(Mesh& model, aiAnimation& animation)
	: mName(animation.mName.C_Str()), mDuration(static_cast<float>(animation.mDuration)), mTicksPerSecond(static_cast<float>(animation.mTicksPerSecond)),
	mBoneAnimations(), mBoneAnimationsByBone(), mKeyframeCount(0)
{
	assert(animation.mNumChannels > 0);

	if (mTicksPerSecond <= 0.0f)
	{
		mTicksPerSecond = 1.0f;
	}

	for (UINT i = 0; i < animation.mNumChannels; i++)
	{
		try {
			BoneAnimation* boneAnimation = new BoneAnimation(model, *(animation.mChannels[i]));
			mBoneAnimations.push_back(boneAnimation);

			assert(mBoneAnimationsByBone.find(&(boneAnimation->GetBone())) == mBoneAnimationsByBone.end());
			mBoneAnimationsByBone[&(boneAnimation->GetBone())] = boneAnimation;
		}
		catch (std::exception &) {

		}
	}

	for (BoneAnimation* boneAnimation : mBoneAnimations)
	{
		if (boneAnimation->Keyframes().size() > mKeyframeCount)
		{
			mKeyframeCount = boneAnimation->Keyframes().size();
		}
	}
}

AnimationClip::~AnimationClip()
{
	for (BoneAnimation* boneAnimation : mBoneAnimations)
	{
		delete boneAnimation;
	}
}

const std::string& AnimationClip::Name() const
{
	return mName;
}

float AnimationClip::Duration() const
{
	return mDuration;
}

float AnimationClip::TicksPerSecond() const
{
	return mTicksPerSecond;
}

const std::vector<BoneAnimation*>& AnimationClip::BoneAnimations() const
{
	return mBoneAnimations;
}

const std::map<Bone*, BoneAnimation*>& AnimationClip::BoneAnimationsByBone() const
{
	return mBoneAnimationsByBone;
}

const UINT AnimationClip::KeyframeCount() const
{
	return mKeyframeCount;
}

UINT AnimationClip::GetTransform(float time, Bone& bone, XMFLOAT4X4& transform) const
{
	auto foundBoneAnimation = mBoneAnimationsByBone.find(&bone);
	if (foundBoneAnimation != mBoneAnimationsByBone.end())
	{
		return foundBoneAnimation->second->GetTransform(time, transform);
	}
	else
	{
		transform = MatrixHelper::Identity;
		return UINT_MAX;
	}
}

void AnimationClip::GetTransforms(float time, std::vector<XMFLOAT4X4>& boneTransforms) const
{
	for (BoneAnimation* boneAnimation : mBoneAnimations)
	{
		boneAnimation->GetTransform(time, boneTransforms[boneAnimation->GetBone().Index()]);
	}
}

void AnimationClip::GetTransformAtKeyframe(UINT keyframe, Bone& bone, XMFLOAT4X4& transform) const
{
	auto foundBoneAnimation = mBoneAnimationsByBone.find(&bone);
	if (foundBoneAnimation != mBoneAnimationsByBone.end())
	{
		foundBoneAnimation->second->GetTransformAtKeyframe(keyframe, transform);
	}
	else
	{
		transform = MatrixHelper::Identity;
	}
}

void AnimationClip::GetTransformsAtKeyframe(UINT keyframe, std::vector<XMFLOAT4X4>& boneTransforms) const
{
	for (BoneAnimation* boneAnimation : mBoneAnimations)
	{
		boneAnimation->GetTransformAtKeyframe(keyframe, boneTransforms[boneAnimation->GetBone().Index()]);
	}
}

void AnimationClip::GetInteropolatedTransform(float time, Bone& bone, XMFLOAT4X4& transform) const
{
	auto foundBoneAnimation = mBoneAnimationsByBone.find(&bone);
	if (foundBoneAnimation != mBoneAnimationsByBone.end())
	{
		foundBoneAnimation->second->GetInteropolatedTransform(time, transform);
	}
	else
	{
		transform = MatrixHelper::Identity;
	}
}

void AnimationClip::GetInteropolatedTransforms(float time, std::vector<XMFLOAT4X4>& boneTransforms) const
{
	for (BoneAnimation* boneAnimation : mBoneAnimations)
	{
		boneAnimation->GetInteropolatedTransform(time, boneTransforms[boneAnimation->GetBone().Index()]);
	}
}