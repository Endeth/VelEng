#pragma once

#include <memory>

#include "external/glm/glm.hpp"
#include "vGraphics/vVulkan/vVulkanPipeline.h"

namespace Vel
{
	class MaterialPipeline
	{
	public:
		MaterialPipeline();

		void Create( VkRenderPass renderPass, VkDescriptorSetLayout dscSetLayout );
		void Bind();
		void Unbind();
		const VkPipeline GetPipeline() const;
	private:
		Pipeline pipeline;
	};
}

