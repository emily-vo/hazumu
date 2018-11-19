#pragma once

#include "Mesh.h"

class Model {
public:
	Model(String name);

	static std::unique_ptr<Model> LoadFromFile(const String &filename, const String &name);

	String name;
	std::vector<std::unique_ptr<Mesh>> meshes;
};