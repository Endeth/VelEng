#pragma once

#include <memory>

#include "glm/glm.hpp"
#include "VTextures/VTexture.h"

namespace Vel
{
	class VMaterial 
	{
	public:
		VMaterial(const std::shared_ptr<VTexture>& diffuse, const std::shared_ptr<VTexture>& specular, GLfloat& shininess);
		virtual void SetTexturesUnits();
		virtual void BindMaterial();
		virtual void UnbindMaterial();
	private:
		std::shared_ptr<VTexture> _diffuse;
		std::shared_ptr<VTexture> _specular;
		GLfloat _shininess;
	};

	class VEmissiveMaterial : public VMaterial
	{
	public:
		VEmissiveMaterial(const std::shared_ptr<VTexture>& diffuse, const std::shared_ptr<VTexture>& specular, const std::shared_ptr<VTexture>& emission, GLfloat& shininess);
		void BindMaterial() final;
		void UnbindMaterial() final;
	protected:
		std::shared_ptr<VTexture> _emission;
	};
}

