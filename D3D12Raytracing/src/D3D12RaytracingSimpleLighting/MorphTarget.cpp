#include "MorphTarget.h"

#include <fstream>
#include <streambuf>
#include <rapidjson/document.h>
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>
#include <glm/gtx/common.hpp>

#include "cudaSkinning.h"

using namespace rapidjson;

MorphTarget::MorphTarget(const std::string &filename) :
	initVertices(),
	targets(),
	activeVertices(),
	indices(),
	duration(3.f)
{
	std::ifstream file(filename);
	std::stringstream buffer;
	buffer << file.rdbuf();
	std::string json = buffer.str();

	Document d;
	d.Parse(json.c_str());

	assert(d.HasMember("vertices"));
	assert(d.HasMember("morphTargets"));
	assert(d.HasMember("faces"));

	Value &vertices = d["vertices"];
	assert(vertices.IsArray());
	for (SizeType i = 0; i < vertices.Size(); i += 3) {
		float x = vertices[i + 0].GetFloat();
		float y = vertices[i + 1].GetFloat();
		float z = vertices[i + 2].GetFloat();
		XMFLOAT3 pos(x, y, z);
		XMFLOAT3 nor(1, 0, 0);
		initVertices.push_back({ pos, nor });
	}

	Value &morphTargets = d["morphTargets"];
	assert(morphTargets.IsArray());
	for (SizeType i = 0; i < morphTargets.Size(); ++i) {
		Value &t = morphTargets[i];
		assert(t.HasMember("vertices"));
		Value &v = t["vertices"];
		std::string s = t["name"].GetString();
		std::vector<Vertex> targetVertices;
		for (SizeType j = 0; j < v.Size(); j += 3) {
			float x = v[j + 0].GetFloat();
			float y = v[j + 1].GetFloat();
			float z = v[j + 2].GetFloat();
			XMFLOAT3 pos(x, y, z);
			XMFLOAT3 nor(0, 0, 1);
			targetVertices.push_back({ pos, nor });
		}
		targets.push_back(std::move(targetVertices));
	}

	Value &faces = d["faces"];
	assert(faces.IsArray());
	for (SizeType i = 0; i < faces.Size(); ++i) {
		unsigned int face = faces[i++].GetInt();
		bool isQuad = (face & 1) != 0;
		bool hasMaterial = (face & 2) != 0;
		bool hasFaceVertexUV = (face & 8) != 0;
		bool hasFaceNormal = (face & 16) != 0;
		bool hasFaceVertexNormal = (face & 32) != 0;
		bool hasFaceColor = (face & 64) != 0;
		bool hasFaceVertexColor = (face & 128) != 0;

		if (isQuad) {
			indices.push_back(faces[i].GetInt());
			indices.push_back(faces[i + 1].GetInt());
			indices.push_back(faces[i + 3].GetInt());
			indices.push_back(faces[i + 1].GetInt());
			indices.push_back(faces[i + 2].GetInt());
			indices.push_back(faces[i + 3].GetInt());
			i += 4;
			if (hasMaterial) { i++; }
			if (hasFaceVertexUV) { i += 4; }
			if (hasFaceNormal) { i++; }
			if (hasFaceVertexNormal) { i += 4; }
			if (hasFaceColor) { i++; }
			if (hasFaceVertexColor) { i += 4; }
		} else {
			indices.push_back(faces[i].GetInt());
			indices.push_back(faces[i + 1].GetInt());
			indices.push_back(faces[i + 2].GetInt());
			i += 3;
			if (hasMaterial) { i++; }
			if (hasFaceVertexUV) { i += 3; }
			if (hasFaceNormal) { i++; }
			if (hasFaceVertexNormal) { i += 3; }
			if (hasFaceColor) { i++; }
			if (hasFaceVertexColor) { i += 3; }
		}
		i--;
	}
	activeVertices = initVertices;
}

void MorphTarget::Update(float time) {
	const bool UseCPU = true;
	if (UseCPU) {
		UpdateCPU(time);
	} else {
		UpdateGPU(time);
	}
}

void MorphTarget::UpdateCPU(float time) {
	// n targets, duration seconds for all n targets
	// transition from one target to next is 24 / n seconds
	time = glm::fmod(time, duration);
	activeVertices.clear();
	float transitionTime = duration / targets.size();
	float inBetween = time / transitionTime;
	float alpha = glm::fmod(time, transitionTime) / transitionTime;
	int target1 = glm::floor(inBetween);
	int target2 = target1 + 1 == targets.size() ? 0 : target1 + 1;
	auto &targets_1 = targets[target1];
	auto &targets_2 = targets[target2];
	for (int i = 0; i < targets_1.size(); ++i) {
		Vertex &v1 = targets_1[i];
		Vertex &v2 = targets_2[i];
		glm::vec3 pos1(v1.position.x, v1.position.y, v1.position.z);
		glm::vec3 pos2(v2.position.x, v2.position.y, v2.position.z);
		glm::vec3 lerpPos = glm::lerp(pos1, pos2, alpha);
		XMFLOAT3 newPos { lerpPos.x, lerpPos.y, lerpPos.z };
		activeVertices.push_back({ newPos, v1.normal });
	}
	//activeVertices = targets_1;
}

void MorphTarget::UpdateGPU(float time) {
	time = glm::fmod(time, duration);
	activeVertices.clear();
	float transitionTime = duration / targets.size();
	float inBetween = time / transitionTime;
	float alpha = glm::fmod(time, transitionTime) / transitionTime;
	int target1 = glm::floor(inBetween);
	int target2 = target1 + 1 == targets.size() ? 0 : target1 + 1;
	auto &targets_1 = targets[target1];
	auto &targets_2 = targets[target2];
	//cudaMorph(int numVerts, cudaVertex *target1, cudaVertex *target2, cudaVertex *vertsOut, const float alpha);
	cudaVertex *newVertices = new cudaVertex[initVertices.size()];
	Vertex *updatedVerts = reinterpret_cast<Vertex *>(newVertices);
	cudaMorph(initVertices.size(), reinterpret_cast<cudaVertex *>(targets_1.data()), reinterpret_cast<cudaVertex *>(targets_2.data()), newVertices, alpha);
	activeVertices = std::vector<Vertex>(updatedVerts, updatedVerts + initVertices.size());
	delete[] newVertices;
}