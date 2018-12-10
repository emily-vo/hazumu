#pragma once

#include <vector>
#include "stdafx.h"

using namespace DirectX;

struct aiNodeAnim;

class Mesh;
class Bone;
class Keyframe;

class BoneAnimation
{
	friend class AnimationClip;

public:
	~BoneAnimation();

	Bone& GetBone();
	const std::vector<Keyframe*> Keyframes() const;

	UINT GetTransform(float time, XMFLOAT4X4& transform) const;
	void GetTransformAtKeyframe(UINT keyframeIndex, XMFLOAT4X4& transform) const;
	void GetInteropolatedTransform(float time, XMFLOAT4X4& transform) const;

private:
	BoneAnimation(Mesh& model, aiNodeAnim& nodeAnim);

	BoneAnimation();
	BoneAnimation(const BoneAnimation& rhs);
	BoneAnimation& operator=(const BoneAnimation& rhs);

	UINT FindKeyframeIndex(float time) const;

	Mesh* mModel;
	Bone* mBone;
	std::vector<Keyframe*> mKeyframes;
};
