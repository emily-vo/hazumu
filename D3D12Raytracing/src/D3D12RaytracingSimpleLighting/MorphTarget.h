#pragma once

#include "stdafx.h"

#include "RaytracingHlslCompat.h"

class MorphTarget {
public:
	MorphTarget(const std::string &filename);

	void Update(float time);

	std::vector<Vertex> initVertices;
	std::vector<std::vector<Vertex>> targets;
	std::vector<Vertex> activeVertices;
	std::vector<Index> indices;
	float duration;
private:
	void UpdateCPU(float time);
	void UpdateGPU(float time);
};