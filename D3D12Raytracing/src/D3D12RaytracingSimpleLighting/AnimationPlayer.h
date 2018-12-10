#pragma once

#include "stdafx.h"
#include "Mesh.h"
#include "AnimationClip.h"

using namespace DirectX;

class AnimationPlayer
{

public:
	AnimationPlayer(Mesh& model, bool interpolationEnabled = true);

	const Mesh& GetModel() const;
	const AnimationClip* CurrentClip() const;
	float CurrentTime() const;
	UINT CurrentKeyframe() const;
	const std::vector<XMFLOAT4X4>& BoneTransforms() const;

	bool InterpolationEnabled() const;
	bool IsPlayingClip() const;
	bool IsClipLooped() const;

	void SetInterpolationEnabled(bool interpolationEnabled);

	void StartClip(AnimationClip& clip);
	void PauseClip();
	void ResumeClip();
	virtual void Update(float time);
	void SetCurrentKeyFrame(UINT keyframe);

private:
	AnimationPlayer();
	AnimationPlayer(const AnimationPlayer& rhs);
	AnimationPlayer& operator=(const AnimationPlayer& rhs) = delete;

	void GetBindPose(SceneNode& sceneNode);
	void GetBindPoseBottomUp(SceneNode& sceneNode);
	void GetPose(float time, SceneNode& sceneNode);
	void GetPoseAtKeyframe(UINT keyframe, SceneNode& sceneNode);
	void GetInterpolatedPose(float time, SceneNode& sceneNode);

	Mesh* mModel;
	AnimationClip* mCurrentClip;
	float mCurrentTime;
	UINT mCurrentKeyframe;
	std::map<SceneNode*, XMFLOAT4X4> mToRootTransforms;
	std::vector<XMFLOAT4X4> mFinalTransforms;
	XMFLOAT4X4 mInverseRootTransform;
	bool mInterpolationEnabled;
	bool mIsPlayingClip;
	bool mIsClipLooped;
};