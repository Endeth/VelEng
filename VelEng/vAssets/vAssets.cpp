#include "external/assimp/mesh.h"
#include "external/assimp/scene.h"
#include "external/assimp/postprocess.h"
#include "external/assimp/Importer.hpp"
#include "vAssets.h"

namespace Vel
{
    AssetsManager::AssetsManager()
    {
        //TODO database of assets
    }
    AssetsManager::~AssetsManager()
    {
    }
	std::shared_ptr<Model> AssetsManager::LoadModel(const char * path)
	{
		return std::shared_ptr<Model>();
	}
}