#pragma once

#include <vector>
#include <memory>
#include <filesystem>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

namespace Vel
{
    template <typename Res>
    class Asset
    {
    public:
        Asset(std::filesystem::path& path);

    private:
        void LoadAsset();
        void UnloadAsset();

        Res resource;

        std::atomic<uint32_t> counter;
        std::filesystem::path filePath;
    };
}