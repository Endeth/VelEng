#pragma once

#include <memory>

#include "external/glm/glm.hpp"
#include "vGeo/vGeo.h"
#include "vDrawable/vScene.h"
#include "vDrawable/vModel.h"

namespace Vel
{

	class AssetsManager
	{
	public:
		AssetsManager();
		~AssetsManager();
		AssetsManager(const AssetsManager&) = delete;
		AssetsManager& operator=(const AssetsManager&) = delete;
		AssetsManager(const AssetsManager&&) = delete;
		AssetsManager& operator=(const AssetsManager&&) = delete;

		std::shared_ptr<Scene> LoadScene(const char* path);
		std::shared_ptr<Scene> LoadScene(const std::string& path);

		std::shared_ptr<Model> LoadModel(const char* path);
		std::shared_ptr<Model> LoadModel(const std::string& path);
		
        std::shared_ptr<Texture> LoadTexture(const char* path);
        std::shared_ptr<Texture> LoadTexture(const std::string& path);
        
        void LoadSound(); //TODO implement sounds

	protected:
		//std::unique_ptr<Assimp::Importer> _importer;
	};
}