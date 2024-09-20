#include "ui/VelImgui.h"
#include "rendering/VulkanTypes.h"

void Vel::Imgui::Init(ImGui_ImplVulkan_InitInfo& initInfo, SDL_Window* window)
{
	ImGui::CreateContext();

	ImGui_ImplSDL2_InitForVulkan(window);
	ImGui_ImplVulkan_Init(&initInfo);
	ImGui_ImplVulkan_CreateFontsTexture();
}

void Vel::Imgui::PrepareFrame(std::function<void(void)> windowPopulate)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplSDL2_NewFrame();

	ImGui::NewFrame();

	bool isVisible = ImGui::Begin("Uniforms");
	if (isVisible)
		windowPopulate();
	ImGui::End();

	ImGui::Render();
}

void Vel::Imgui::HandleSDLEvent(SDL_Event* sdlEvent)
{
	ImGui_ImplSDL2_ProcessEvent(sdlEvent);
}

void Vel::Imgui::Draw(VkCommandBuffer cmd, VkImageView targetImageView, VkExtent2D &swapchainExtent)
{
	VkRenderingAttachmentInfo colorAttachment {
		.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
		.pNext = nullptr,
		.imageView = targetImageView,
		.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE
	};

	VkRenderingInfo renderInfo {
		.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
		.pNext = nullptr,
		.renderArea = { VkOffset2D { 0, 0 }, swapchainExtent },
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachment,
		.pDepthAttachment = nullptr,
		.pStencilAttachment = nullptr,
	};

	vkCmdBeginRendering(cmd, &renderInfo);

	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd); //TODO CRITICAL fix

	vkCmdEndRendering(cmd);
}

void Vel::Imgui::Cleanup()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}
