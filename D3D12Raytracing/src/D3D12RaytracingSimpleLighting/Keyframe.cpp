#include "Keyframe.h"

Keyframe::Keyframe(float time, const XMFLOAT3& translation, const XMFLOAT4& rotationQuaternion, const XMFLOAT3& scale)
	: mTime(time), mTranslation(translation), mRotationQuaternion(rotationQuaternion), mScale(scale)
{
}

float Keyframe::Time() const
{
	return mTime;
}

const XMFLOAT3& Keyframe::Translation() const
{
	return mTranslation;
}

const XMFLOAT4& Keyframe::RotationQuaternion() const
{
	return mRotationQuaternion;
}

const XMFLOAT3& Keyframe::Scale() const
{
	return mScale;
}

XMVECTOR Keyframe::TranslationVector() const
{
	return XMLoadFloat3(&mTranslation);
}

XMVECTOR Keyframe::RotationQuaternionVector() const
{
	return XMLoadFloat4(&mRotationQuaternion);
}

XMVECTOR Keyframe::ScaleVector() const
{
	return XMLoadFloat3(&mScale);
}

XMMATRIX Keyframe::Transform() const
{
	static XMVECTOR rotationOrigin = XMVectorZero();

	return XMMatrixAffineTransformation(ScaleVector(), rotationOrigin, RotationQuaternionVector(), TranslationVector());
}
