#pragma once

#include "core\ImageAlgorithm.h"

namespace tim
{

    class PerlinNoise
    {
    public:
        PerlinNoise(uint32_t numLayer, uint32_t firstLayerSize, int seed=42);
        ~PerlinNoise();

        PerlinNoise(const PerlinNoise&) = default;
        PerlinNoise& operator=(const PerlinNoise&) = default;

        ImageAlgorithm<float> generate(uivec2) const;

    private:
        std::vector<ImageAlgorithm<float>> _layers;

    private:
        static ImageAlgorithm<float> genNoise(uint32_t, int seed, vec2 boundary = vec2(0,1));
    };

}

