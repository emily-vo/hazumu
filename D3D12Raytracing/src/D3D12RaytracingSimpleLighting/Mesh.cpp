#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Mesh::Mesh(String name) :
	name(name),
	vertices(),
	indices() {}

std::unique_ptr<Mesh> Mesh::LoadFromFile(const String &filename, const String &name) {
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(filename,
											 aiProcessPreset_TargetRealtime_MaxQuality
	);

	if (!scene) { throw std::exception(); }

	auto m = std::make_unique<Mesh>(name);
	for (int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh *currMesh = scene->mMeshes[i];
		if (!(currMesh->HasPositions()) || !(currMesh->HasFaces())) { continue; }

		int idxOffset = m->vertices.size();
		// Load vertex positions
		for (int j = 0; j < currMesh->mNumVertices; ++j) {
			aiVector3D pos = currMesh->mVertices[j];
			aiVector3D nor = currMesh->HasNormals() ? currMesh->mNormals[j] : aiVector3D(0, 0, 1);
			m->vertices.push_back({ { pos.x, pos.y, pos.z },
									{ nor.x, nor.y, nor.z } });
			//m->vertices.push_back(Vector3(pos.x, pos.y, pos.z));
		}

		// Load vertex normals
		if (currMesh->HasNormals()) {
			for (int j = 0; j < currMesh->mNumVertices; ++j) {
				aiVector3D nor = currMesh->mNormals[j];
				//m->normals.push_back(Vector3(nor.x, nor.y, nor.z));
			}
		}

		// Load vertex uvs
		if (currMesh->HasTextureCoords(0)) {
			for (int j = 0; j < currMesh->mNumVertices; ++j) {
				//aiVector3D *uv = currMesh->mTextureCoords[j];
				// TODO: figure out how to get tex coords
			}
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
	}
	return m;
}
