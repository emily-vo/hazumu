#pragma once

#include "stdafx.h"
#include "RaytracingHlslCompat.h"

using namespace DirectX;
using String = std::string;

class Mesh {
public:
	Mesh(String name);

	static std::unique_ptr<Mesh> LoadFromFile(const String &filename, const String &name);

	String name;
	std::vector<Vertex> vertices;
	std::vector<Index> indices;
};