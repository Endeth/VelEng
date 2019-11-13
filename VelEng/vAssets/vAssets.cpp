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
	std::shared_ptr<Model> AssetsManager::GetModel(const std::string &path )
	{
		//return _models.Get( path );
		return std::make_shared<Model>();
	}
}