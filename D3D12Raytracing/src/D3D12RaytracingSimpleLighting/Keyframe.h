#pragma once

#include "stdafx.h"
#include "BoneAnimation.h"

using namespace DirectX;

class Keyframe
{
	friend class BoneAnimation;

public:
	float Time() const;
	const XMFLOAT3& Translation() const;
	const XMFLOAT4& RotationQuaternion() const;
	const XMFLOAT3& Scale() const;

	XMVECTOR TranslationVector() const;
	XMVECTOR RotationQuaternionVector() const;
	XMVECTOR ScaleVector() const;

	XMMATRIX Transform() const;

private:
	Keyframe(float time, const XMFLOAT3& translation, const XMFLOAT4& rotationQuaternion, const XMFLOAT3& scale);

	Keyframe();
	Keyframe(const Keyframe& rhs);
	Keyframe& operator=(const Keyframe& rhs) = delete;

	float mTime;
	XMFLOAT3 mTranslation;
	XMFLOAT4 mRotationQuaternion;
	XMFLOAT3 mScale;
};
