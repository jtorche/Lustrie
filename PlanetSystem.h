#pragma once

#include "PlanetGrass.h"
#include <memory>
#include "PlanetPlants.h"

class PlanetSystem
{
public:
	static constexpr int TEX_PLANET = 0;
	static constexpr int NB_TEX_PLANET = 1;

	static constexpr int TEX_PLANET_GRASS = TEX_PLANET + NB_TEX_PLANET;

	static constexpr int TEX_PLANET_PLANT_TRUNK = TEX_PLANET_GRASS+1;
	static constexpr int NB_TEX_PLANET_PLANT_TRUNK = 1;

	static constexpr int TEX_PLANET_PLANT_LEAF = TEX_PLANET_PLANT_TRUNK + NB_TEX_PLANET_PLANT_TRUNK;
	static constexpr int NB_TEX_PLANET_PLANT_LEAF = 1;

	std::unique_ptr<Material> planetMaterial;
	std::vector<std::shared_ptr<Material>> grassMaterial;

	std::unique_ptr<Planet> planet;
	std::vector<std::unique_ptr<PlanetGrass>> grassOnPlanet;

	std::unique_ptr<Material> plantMaterial;
	std::unique_ptr<Material> leafMaterial;
	std::unique_ptr<PlanetPlants> plants;
};