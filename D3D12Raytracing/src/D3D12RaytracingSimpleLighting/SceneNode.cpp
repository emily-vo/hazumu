#include "SceneNode.h"
#include "MatrixHelper.h"

RTTI_DEFINITIONS(SceneNode)

SceneNode::SceneNode(const std::string& name)
	: mName(name), mParent(nullptr), mChildren(), mTransform(MatrixHelper::Identity)
{
}

SceneNode::SceneNode(const std::string& name, const XMFLOAT4X4& transform)
	: mName(name), mParent(nullptr), mChildren(), mTransform(transform) {
}

const std::string& SceneNode::Name() const
{
	return mName;
}

SceneNode* SceneNode::Parent()
{
	return mParent;
}

std::vector<SceneNode*>& SceneNode::Children()
{
	return mChildren;
}

const XMFLOAT4X4& SceneNode::Transform() const
{
	return mTransform;
}

XMMATRIX SceneNode::TransformMatrix() const
{
	return XMLoadFloat4x4(&mTransform);
}

void SceneNode::SetParent(SceneNode* parent)
{
	mParent = parent;
}

void SceneNode::SetTransform(XMFLOAT4X4& transform)
{
	mTransform = transform;
}

void SceneNode::SetTransform(CXMMATRIX transform)
{
	XMFLOAT4X4 t;
	XMStoreFloat4x4(&t, transform);

	SetTransform(t);
}