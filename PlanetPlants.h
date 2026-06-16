#pragma once

#include "Planet.h"
#include <memory>
#include "graphics/RendererStruct.h"

class PlanetPlants
{
public:
	PlanetPlants(int seed);
	
	void cull(const tim::Camera&, std::vector<ObjectInstance>&, std::vector<ObjectInstance>&);

	void createTree(int sizeCategorie, int number);
	void populatePlant(Planet&, size_t plantIndex, int nbInstance);

private:
	int _seed;

	struct Plant
	{
		struct Lod
		{
			Lod(const Lod&) = default;
			Lod& operator=(const Lod&) = default;

			std::shared_ptr<MeshBuffers> meshs[2];
			float distance;
		};
		std::vector<Lod> lods;
		Sphere boundingSphere;
	};

	std::mutex _treeVectorMutex;
	std::vector<Plant> _plants;

	struct Instance
	{
		tim::mat4 transform;
		MaterialParameter material[2];
		size_t indexPlant;
	};

	std::vector<Instance> _instances;
};