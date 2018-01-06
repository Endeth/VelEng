
#include "VAssets.h"
#include "../VOpenGL/assimp/mesh.h"
#include "../VOpenGL/assimp/scene.h"
#include "../VOpenGL/assimp/postprocess.h"
#include "../VOpenGL/assimp/Importer.hpp"

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