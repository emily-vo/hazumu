#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Model::Model(String name) :
	name(name),
	meshes() {}

std::unique_ptr<Model> Model::LoadFromFile(const String &filename, const String &name) {
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(filename,
											 aiProcessPreset_TargetRealtime_MaxQuality
	);

	if (!scene) { throw std::exception("could not read file"); }
	if (scene->mNumMeshes == 0) { throw std::exception("file contains no meshes"); }

	auto m = std::make_unique<Model>(name);
	for (int i = 0; i < scene->mNumMeshes; ++i) {
		aiMesh *currMesh = scene->mMeshes[i];
		auto mesh = Mesh::LoadFromAiMesh(currMesh);
		if (mesh) {
			m->meshes.push_back(std::move(mesh));
		}
	}
	importer.FreeScene();
	return m;
}