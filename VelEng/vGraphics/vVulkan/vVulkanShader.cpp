#include <fstream>

#include "vGeo/vGeo.h"

#include "vVulkanCommands.h"
#include "vVulkanShader.h"

namespace Vel
{
	ShaderDescription::ShaderDescription( VkShaderModule vertex, VkShaderModule fragment )
	{

		vertexShader = CreateShaderModule( "shaders/shader.vert.spv" ); //TODO proper shader loading
		fragmentShader = CreateShaderModule( "shaders/shader.frag.spv" );

		shaderStageInfo[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo[0].pNext = nullptr;
		shaderStageInfo[0].flags  0;
		shaderStageInfo[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStageInfo[0].module = vertexShader;
		shaderStageInfo[0].pName = "main";
		shaderStageInfo[0].pSpecializationInfo = nullptr;

		shaderStageInfo[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageInfo[1].pNext = nullptr;
		shaderStageInfo[1].flags  0;
		shaderStageInfo[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStageInfo[1].module = fragmentShader;
		shaderStageInfo[1].pName = "main";
		shaderStageInfo[1].pSpecializationInfo = nullptr;

		vertexInputBindingDescription.binding = 0;
		vertexInputBindingDescription.stride = sizeof( VertexUVColor );
		vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		vertexInputAttributeDescription[0].location = 0;
		vertexInputAttributeDescription[0].binding = 0;
		vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexInputAttributeDescription[0].offset = offsetof( struct VertexUVColor, position );
		vertexInputAttributeDescription[1].location = 1;
		vertexInputAttributeDescription[1].binding = 0;
		vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		vertexInputAttributeDescription[1].offset = offsetof( struct VertexUVColor, color );
		vertexInputAttributeDescription[2].location = 2;
		vertexInputAttributeDescription[2].binding = 0;
		vertexInputAttributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
		vertexInputAttributeDescription[2].offset = offsetof( struct VertexUVColor, UV );

		vertexInputStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateInfo.pNext = nullptr;
		vertexInputStateInfo.flags = 0;
		vertexInputStateInfo.vertexBindingDescriptionCount = 1;
		vertexInputStateInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
		vertexInputStateInfo.vertexAttributeDescriptionCount = 3;

		std::array<VkDescriptorSetLayoutBinding, 3> descriptorSetsLayoutsBindings;
		descriptorSetsLayoutsBindings[0].binding = 0;
		descriptorSetsLayoutsBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetsLayoutsBindings[0].descriptorCount = 1;
		descriptorSetsLayoutsBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		descriptorSetsLayoutsBindings[0].pImmutableSamplers = nullptr;

		descriptorSetsLayoutsBindings[1].binding = 1;
		descriptorSetsLayoutsBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorSetsLayoutsBindings[1].descriptorCount = 1;
		descriptorSetsLayoutsBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		descriptorSetsLayoutsBindings[1].pImmutableSamplers = nullptr;

		descriptorSetsLayoutsBindings[2].binding = 2; //TODO check whether bindings in different shader have to be exclusive
		descriptorSetsLayoutsBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSetsLayoutsBindings[2].descriptorCount = 1;
		descriptorSetsLayoutsBindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorSetsLayoutsBindings[2].pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo;
		descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCreateInfo.pNext = nullptr;
		descriptorSetLayoutCreateInfo.flags = 0;
		descriptorSetLayoutCreateInfo.bindingCount = descriptorSetsLayoutsBindings.size();
		descriptorSetLayoutCreateInfo.pBindings = descriptorSetsLayoutsBindings.data();

		CheckResult( vkCreateDescriptorSetLayout( VulkanCommon::Device, &descriptorSetLayoutCreateInfo, nullptr, &dscSetLayout ), "failed to create descriptor set" ); //TODO move this to a place where it can be reused

		VkDescriptorBufferInfo descriptorBufferInfo;
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = sizeof( CameraMatrices ); //TODO i dont think 0 and 1 both need whole cameramatrices.range

		VkDescriptorImageInfo descriptorImageInfo;
		descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		descriptorImageInfo.imageView = sampledImage.imageView;
		descriptorImageInfo.sampler = Samplers.GetSampler( VulkanSamplers::Type::BasicSampler );

		std::array<VkWriteDescriptorSet, 3> writeDescriptorSets; //TODO this might be best done in shaderdescription, define writedescriptor in constructor, update (given right buffers) from shaderinstance
		writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[0].pNext = nullptr;
		writeDescriptorSets[0].dstBinding = 0;
		writeDescriptorSets[0].dstSet = nullptr;
		writeDescriptorSets[0].dstArrayElement = 0;
		writeDescriptorSets[0].descriptorCount = 1;
		writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[0].pImageInfo = nullptr;
		writeDescriptorSets[0].pBufferInfo = &descriptorBufferInfo;
		writeDescriptorSets[0].pTexelBufferView = nullptr;

		writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[1].pNext = nullptr;
		writeDescriptorSets[1].dstBinding = 1;
		writeDescriptorSets[1].dstSet = nullptr;
		writeDescriptorSets[1].dstArrayElement = 0;
		writeDescriptorSets[1].descriptorCount = 1;
		writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSets[1].pImageInfo = nullptr;
		writeDescriptorSets[1].pBufferInfo = &descriptorBufferInfo;
		writeDescriptorSets[1].pTexelBufferView = nullptr;

		writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSets[2].pNext = nullptr;
		writeDescriptorSets[2].dstBinding = 2;
		writeDescriptorSets[2].dstSet = nullptr;
		writeDescriptorSets[2].dstArrayElement = 0;
		writeDescriptorSets[2].descriptorCount = 1;
		writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescriptorSets[2].pImageInfo = &descriptorImageInfo;
		writeDescriptorSets[2].pBufferInfo = nullptr;
		writeDescriptorSets[2].pTexelBufferView = nullptr;
	}

	ShaderDescription::~ShaderDescription()
	{
		DestroyShaderModules();
		vkDestroyDescriptorSetLayout( VulkanCommon::Device, dscSetLayout, nullptr );
		dscSetLayout = VK_NULL_HANDLE;
	}

	VkShaderModule ShaderDescription::CreateShaderModule( const char *filepath ) //TODO want to use SPIRV-CROSS for reflections - might want to use it here and store modules with their info
	{
		VkShaderModule shader = VK_NULL_HANDLE;
		std::ifstream file( filepath, std::ios::binary );
		if( !file.fail() ) //TODO add fail
		{
			std::streampos begin, end;
			begin = file.tellg();
			file.seekg( 0, std::ios::end );
			end = file.tellg();

			std::vector<char> shaderCode( static_cast<size_t>( end - begin ) );
			file.seekg( 0, std::ios::beg );
			file.read( shaderCode.data(), end - begin );
			file.close();

			if( shaderCode.size() > 0 )
			{
				VkShaderModuleCreateInfo shaderModuleCreateInfo;
				shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
				shaderModuleCreateInfo.pNext = nullptr;
				shaderModuleCreateInfo.flags = 0;
				shaderModuleCreateInfo.codeSize = shaderCode.size();
				shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>( shaderCode.data() );

				CheckResult( vkCreateShaderModule( VulkanCommon::Device, &shaderModuleCreateInfo, nullptr, &shader ), "failed to create shader" );
			}
		}

		return shader;
	}
	bool ShaderDescription::DestroyShaderModules()
	{
		if( vertexShader != VK_NULL_HANDLE )
		{
			vkDestroyShaderModule( VulkanCommon::Device, vertexShader, nullptr );
			vertexShader = VK_NULL_HANDLE;
		}
		if( fragmentShader != VK_NULL_HANDLE )
		{
			vkDestroyShaderModule( VulkanCommon::Device, fragmentShader, nullptr );
			fragmentShader = VK_NULL_HANDLE;
		}
	}
}
