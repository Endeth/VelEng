#pragma once

#include "../VOpenGL/glew.h"
#include "../VOpenGL/glm/glm.hpp"
#include "../VGeo/VGeo.h"
#include "../VDrawable/VScene.h"
#include "../VDrawable/VModel.h"

#include <memory>

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