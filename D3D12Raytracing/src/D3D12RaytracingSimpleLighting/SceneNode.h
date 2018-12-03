#pragma once
#include "stdafx.h"
#include "RaytracingHlslCompat.h"
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "RTTI.h"
using namespace DirectX;
using String = std::string;

class SceneNode : public RTTI
{
	RTTI_DECLARATIONS(SceneNode, RTTI)
public:
	const std::string& Name() const;
	SceneNode* Parent();
	std::vector<SceneNode*>& Children();
	const XMFLOAT4X4& Transform() const;
	XMMATRIX TransformMatrix() const;

	void SetParent(SceneNode* parent);

	void SetTransform(XMFLOAT4X4& transform);
	void SetTransform(CXMMATRIX transform);

	SceneNode(const std::string& name);
	SceneNode(const std::string& name, const XMFLOAT4X4& transform);

protected:
	std::string mName;
	SceneNode* mParent;
	std::vector<SceneNode*> mChildren;
	XMFLOAT4X4 mTransform;

private:
	SceneNode();
	SceneNode(const SceneNode& rhs);
	SceneNode& operator=(const SceneNode& rhs) = delete;
};