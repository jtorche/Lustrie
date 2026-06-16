#include "PlanetTextureManager.h"
#include <memory>

using namespace tim;

PlanetTextureManager::PlanetTextureManager(int seed) : _seed(seed)
{
	_textures = std::make_unique<TexturePool>(16);
}