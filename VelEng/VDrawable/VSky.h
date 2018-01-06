#pragma once

#include <memory>

#include "vMesh.h"


namespace Vel
{
	
	class Skybox : public Mesh
	{
	public:
		Skybox(const  std::shared_ptr<TextureCube> &texture);
		void SetVAO();
	private:
		void BindAdditionalDrawingOptions() final;
		void UnbindAdditionalDrawingOptions() final;
		std::shared_ptr<Texture> _skyTex;
	};
}