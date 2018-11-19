#pragma once

#include "stdafx.h"
#include "RaytracingHlslCompat.h"
#include <assimp/scene.h>


using namespace DirectX;
using String = std::string;

class Mesh {
public:
	Mesh(String name);
	Mesh(Mesh &&other);

	static std::unique_ptr<Mesh> LoadFromAiMesh(aiMesh *mesh);

	String name;
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
};