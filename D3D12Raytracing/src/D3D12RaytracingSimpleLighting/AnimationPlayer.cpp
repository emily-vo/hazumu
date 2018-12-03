#include "AnimationPlayer.h"
#include "Mesh.h"
#include "Bone.h"
#include "AnimationClip.h"
#include "BoneAnimation.h"
#include "Keyframe.h"
#include "MatrixHelper.h"

AnimationPlayer::AnimationPlayer(Mesh& model, bool interpolationEnabled)
:
mModel(&model), mCurrentClip(nullptr), mCurrentTime(0.0f), mCurrentKeyframe(0U), mToRootTransforms(), mFinalTransforms(),
mInverseRootTransform(MatrixHelper::Identity), mInterpolationEnabled(interpolationEnabled), mIsPlayingClip(false), mIsClipLooped(true)
{
	mFinalTransforms.resize(model.m_bones.size());
}

const Mesh& AnimationPlayer::GetModel() const
{
	return *mModel;
}

const AnimationClip* AnimationPlayer::CurrentClip() const
{
	return mCurrentClip;
}

float AnimationPlayer::CurrentTime() const
{
	return mCurrentTime;
}

UINT AnimationPlayer::CurrentKeyframe() const
{
	return mCurrentKeyframe;
}

const std::vector<XMFLOAT4X4>& AnimationPlayer::BoneTransforms() const
{
	return mFinalTransforms;
}

bool AnimationPlayer::InterpolationEnabled() const
{
	return mInterpolationEnabled;
}

bool AnimationPlayer::IsPlayingClip() const
{
	return mIsPlayingClip;
}

bool AnimationPlayer::IsClipLooped() const
{
	return mIsClipLooped;
}

void AnimationPlayer::SetInterpolationEnabled(bool interpolationEnabled)
{
	mInterpolationEnabled = interpolationEnabled;
}

void AnimationPlayer::StartClip(AnimationClip& clip)
{
	mCurrentClip = &clip;
	mCurrentTime = 0.0f;
	mCurrentKeyframe = 0;
	mIsPlayingClip = true;

	XMMATRIX inverseRootTransform = XMMatrixInverse(&XMMatrixDeterminant(mModel->RootNode()->TransformMatrix()), mModel->RootNode()->TransformMatrix());
	XMStoreFloat4x4(&mInverseRootTransform, inverseRootTransform);
	GetBindPose(*(mModel->RootNode()));
}

void AnimationPlayer::PauseClip()
{
	mIsPlayingClip = false;
}

void AnimationPlayer::ResumeClip()
{
	if (mCurrentClip != nullptr)
	{
		mIsPlayingClip = true;
	}
}

void AnimationPlayer::Update(float time)
{
	if (mIsPlayingClip)
	{
		assert(mCurrentClip != nullptr);

		mCurrentTime += time * mCurrentClip->TicksPerSecond();
		if (mCurrentTime >= mCurrentClip->Duration())
		{
			if (mIsClipLooped)
			{
				mCurrentTime = 0.0f;
			}
			else
			{
				mIsPlayingClip = false;
				return;
			}
		}

		if (mInterpolationEnabled)
		{
			GetInterpolatedPose(mCurrentTime, *(mModel->RootNode()));
		}
		else
		{
			GetPose(mCurrentTime, *(mModel->RootNode()));
		}
	}
}

void AnimationPlayer::SetCurrentKeyFrame(UINT keyframe)
{
	mCurrentKeyframe = keyframe;
	GetPoseAtKeyframe(mCurrentKeyframe, *(mModel->RootNode()));
}

void AnimationPlayer::GetBindPoseBottomUp(SceneNode& sceneNode)
{
	XMMATRIX toRootTransform = sceneNode.TransformMatrix();

	SceneNode* parentNode = sceneNode.Parent();
	while (parentNode != nullptr)
	{
		toRootTransform = toRootTransform * parentNode->TransformMatrix();
		parentNode = parentNode->Parent();
	}

	Bone* bone = sceneNode.As<Bone>();
	if (bone != nullptr)
	{
		XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() *  toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
	}

	for (SceneNode* childNode : sceneNode.Children())
	{
		GetBindPoseBottomUp(*childNode);
	}
}

void AnimationPlayer::GetBindPose(SceneNode& sceneNode)
{
	XMMATRIX toParentTransform = sceneNode.TransformMatrix();
	XMMATRIX toRootTransform = (sceneNode.Parent() != nullptr ? toParentTransform * XMLoadFloat4x4(&(mToRootTransforms.at(sceneNode.Parent()))) : toParentTransform);
	XMStoreFloat4x4(&(mToRootTransforms[&sceneNode]), toRootTransform);

	Bone* bone = sceneNode.As<Bone>();
	if (bone != nullptr)
	{
		XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() * toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
	}

	for (SceneNode* childNode : sceneNode.Children())
	{
		GetBindPose(*childNode);
	}
}

void AnimationPlayer::GetPose(float time, SceneNode& sceneNode)
{
	XMFLOAT4X4 toParentTransform;
	Bone* bone = sceneNode.As<Bone>();
	if (bone != nullptr)
	{
		mCurrentKeyframe = mCurrentClip->GetTransform(time, *bone, toParentTransform);
	}
	else
	{
		toParentTransform = sceneNode.Transform();
	}

	XMMATRIX toRootTransform = (sceneNode.Parent() != nullptr ? XMLoadFloat4x4(&toParentTransform) * XMLoadFloat4x4(&(mToRootTransforms.at(sceneNode.Parent()))) : XMLoadFloat4x4(&toParentTransform));
	XMStoreFloat4x4(&(mToRootTransforms[&sceneNode]), toRootTransform);

	if (bone != nullptr)
	{
		XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() * toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
	}

	for (SceneNode* childNode : sceneNode.Children())
	{
		GetPose(time, *childNode);
	}
}

void AnimationPlayer::GetPoseAtKeyframe(UINT keyframe, SceneNode& sceneNode)
{
	XMFLOAT4X4 toParentTransform;
	Bone* bone = sceneNode.As<Bone>();
	if (bone != nullptr)
	{
		mCurrentClip->GetTransformAtKeyframe(keyframe, *bone, toParentTransform);
	}
	else
	{
		toParentTransform = sceneNode.Transform();
	}

	XMMATRIX toRootTransform = (sceneNode.Parent() != nullptr ? XMLoadFloat4x4(&toParentTransform) * XMLoadFloat4x4(&(mToRootTransforms.at(sceneNode.Parent()))) : XMLoadFloat4x4(&toParentTransform));
	XMStoreFloat4x4(&(mToRootTransforms[&sceneNode]), toRootTransform);

	if (bone != nullptr)
	{
		XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() * toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
	}

	for (SceneNode* childNode : sceneNode.Children())
	{
		GetPoseAtKeyframe(keyframe, *childNode);
	}
}

void AnimationPlayer::GetInterpolatedPose(float time, SceneNode& sceneNode)
{
	XMFLOAT4X4 toParentTransform;
	Bone* bone = sceneNode.As<Bone>();
	if (bone != nullptr)
	{
		mCurrentClip->GetInteropolatedTransform(time, *bone, toParentTransform);
	}
	else
	{
		toParentTransform = sceneNode.Transform();
	}

	XMMATRIX toRootTransform = (sceneNode.Parent() != nullptr ? XMLoadFloat4x4(&toParentTransform) * XMLoadFloat4x4(&(mToRootTransforms.at(sceneNode.Parent()))) : XMLoadFloat4x4(&toParentTransform));
	XMStoreFloat4x4(&(mToRootTransforms[&sceneNode]), toRootTransform);

	if (bone != nullptr)
	{
		XMStoreFloat4x4(&(mFinalTransforms[bone->Index()]), bone->OffsetTransformMatrix() * toRootTransform * XMLoadFloat4x4(&mInverseRootTransform));
	}

	for (SceneNode* childNode : sceneNode.Children())
	{
		GetInterpolatedPose(time, *childNode);
	}
}