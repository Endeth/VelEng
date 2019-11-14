#pragma once

#include "vVulkanCommon.h"
#include "external/glfw/glfw3.h"
#include "external/glm/glm.hpp"

#include "vGraphics/vDrawable/vModel.h"
#include "vVulkanBuffer.h"
#include "vVulkanCommands.h"
#include "vVulkanDevice.h"
#include "vVulkanSwapChain.h"
#include "vVulkanRenderpass.h"
#include "vVulkanImage.h"

#ifdef _DEBUG
#include "vVulkanDebug.h"
#endif

namespace Vel
{
    class VulkanRenderer : public NonCopyable
    {
    public:
        void Init( GLFWwindow *window );
        void Destroy();
		void CreateSwapchain( GLFWwindow *window );
		void CreateCommandBuffers();
		void DestroyCommandBuffers();
		void RecordCommandBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void LoadOBJ();
		void CreateStagingBuffer();
		void CreateBuffer();
		void CreateImage();
		void CreateDepthImage();
		void CreateUniformBuffers();
		void UpdateUniformBuffers( uint32_t imageIndex );
		void UpdateCamera( glm::mat4 &view, glm::mat4 &proj );
		void Draw();

        Swapchain swapchain;
		RenderPass renderPass;

		VkCommandPool commandPoolGraphics = VK_NULL_HANDLE; //TODO get rid of this test cmdpool
		VkCommandPool commandPoolTransfer = VK_NULL_HANDLE; //TODO get rid of this test cmdpool
		VkDescriptorSetLayout descriptorSetLayout; //TODO move descriptors to a single class ( shader -> attributes + descriptors )
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE; // test dsptrPool
		std::vector<VkDescriptorSet> descriptorSets;
		std::vector<VkCommandBuffer> commandBuffers;
		VkCommandBuffer transferCommand;
		VulkanBuffer vertexBuffer;
		VulkanBuffer indexBuffer;
		VulkanBuffer stagingBuffer;
		VulkanImage sampledImage;
		VulkanImage depthImage;
		std::vector<VulkanBuffer> uniformBuffers;

        bool enableValidationLayers = false;
		CameraMatrices internalCamera;


		std::shared_ptr<Model> _testingModel;
		std::shared_ptr<Material> _testingMaterial;
    };
}