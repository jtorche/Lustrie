#pragma once

#include "API.h"
#include <memory>
#include "TexturePool.h"

class Material
{
	friend class Graphics;
	friend class dx12::Renderer;

public:
	Material(const Material&) = default;
	Material& operator=(const Material&) = default;

	std::shared_ptr<TexturePool> texturePool() const;

private:
	Material() = default;

private:
	std::shared_ptr<dx12::RootSignature> _signature;
	std::shared_ptr<dx12::PipelineState> _pipeline;
	std::shared_ptr<TexturePool> _textures;
};

inline std::shared_ptr<TexturePool> Material::texturePool() const { return _textures; }