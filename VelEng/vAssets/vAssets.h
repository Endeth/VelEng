#pragma once

#include <memory>

#include "external/glm/glm.hpp"
#include "vGeo/vGeo.h"
#include "vGraphics/vDrawable/vScene.h"
#include "vGraphics/vDrawable/vModel.h"

namespace Vel
{
	template <typename Asset>
	class AssetMap
	{
	public:
		std::shared_ptr<Asset> Get( const std::string &path )
		{
			if( _assets.find( path.c_str() ) == _assets.end() )
			{
				auto asset = std::make_shared<Asset>( path );
				_assets[path.c_str()] = asset;
				return asset;
			}
			return _assets[path.c_str()];
		}
	private:
		std::unordered_map<std::string, std::shared_ptr<Asset>> _assets;
	};

	class AssetsManager
	{
	public:
		AssetsManager();
		~AssetsManager();
		AssetsManager( const AssetsManager& ) = delete;
		AssetsManager& operator=( const AssetsManager& ) = delete;
		AssetsManager( const AssetsManager&& ) = delete;
		AssetsManager& operator=( const AssetsManager&& ) = delete;

		std::shared_ptr<Scene> GetScene( const std::string& path );
		std::shared_ptr<Model> GetModel( const std::string& path );
        std::shared_ptr<Texture> GetTexture( const std::string& path );
        
        void LoadSound(); //TODO implement sounds

	protected:
		AssetMap<Scene> scenes;
		AssetMap<Model> _models;
		AssetMap<Texture> _textures;
	};
}