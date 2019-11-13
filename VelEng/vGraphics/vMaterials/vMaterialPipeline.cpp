#include "vMaterialPipeline.h"

namespace Vel
{
	MaterialPipeline::MaterialPipeline()
	{
		constexpr auto test = sizeof( glm::mat4 );
	}

	void MaterialPipeline::Create( VkRenderPass renderPass, VkDescriptorSetLayout dscSetLayout )
	{
		pipeline.Create( renderPass, dscSetLayout );
	}

	void MaterialPipeline::Bind()
	{
	}

	void MaterialPipeline::Unbind()
	{
	}

	const VkPipeline MaterialPipeline::GetPipeline() const
	{
		return pipeline.Get();
	}
}
