#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Mesh::Mesh(String name) :
	name(name),
	vertices(),
	indices() {}

Mesh::Mesh(Mesh &&other) :
	name(std::move(other.name)),
	vertices(std::move(other.vertices)),
	indices(std::move(other.indices)) {}

std::unique_ptr<Mesh> Mesh::LoadFromAiMesh(aiMesh *mesh) {
	auto m = std::make_unique<Mesh>(mesh->mName.C_Str());
	aiMesh *currMesh = mesh;
	if (!(currMesh->HasPositions()) || !(currMesh->HasFaces())) { return nullptr; }

	int idxOffset = m->vertices.size();

	// Load vertex attributes
	for (int j = 0; j < currMesh->mNumVertices; ++j) {
		aiVector3D pos = currMesh->mVertices[j];
		aiVector3D nor = currMesh->HasNormals() ? currMesh->mNormals[j] : aiVector3D(0, 0, 1);
		m->vertices.push_back({ { pos.x, pos.y, pos.z },
								{ nor.x, nor.y, nor.z } });
	}

	for (int j = 0; j < currMesh->mNumFaces; ++j) {
		aiFace face = currMesh->mFaces[j];
		if (face.mNumIndices != 3) {
			throw std::exception();
		}
		m->indices.push_back(idxOffset + face.mIndices[0]);
		m->indices.push_back(idxOffset + face.mIndices[1]);
		m->indices.push_back(idxOffset + face.mIndices[2]);
	}

	// Load skeleton information

	return m;
}