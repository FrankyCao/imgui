#include "ImVulkanShader.h"
#include "imgui_impl_vulkan.h"

namespace ImGui
{
static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

#if IMGUI_RENDERING_VULKAN
ImTextureID ImVulkanImageToImTexture(const VkImageMat& image_vk)
{
    ImTextureVk texture = new ImTextureVK();
    VkAllocator* _allocator = (VkAllocator*)image_vk.allocator;
    const VulkanDevice *device = _allocator->vkdev;
    texture->textureImage = image_vk.image();
    texture->textureView = image_vk.imageview();
    texture->textureSampler = *(device->immutable_texelfetch_sampler());
    texture->textureImageMemory = nullptr;
    texture->extra_image = true;
    texture->textureDescriptor = ImGui_ImplVulkan_AddTexture(texture->textureSampler, texture->textureView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    return (ImTextureID)texture;
}
#endif

void ImMatToImVulkanMat(const ImMat &src, VkMat &dst)
{
    Option opt;
    const VkAllocator* allocator = (VkAllocator*)src.allocator;
    const VulkanDevice* vkdev = allocator->vkdev;
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    VkCompute cmd(vkdev);
    cmd.record_clone(src, dst, opt);
    cmd.submit_and_wait();
    vkdev->reclaim_blob_allocator(opt.blob_vkallocator);
    vkdev->reclaim_staging_allocator(opt.staging_vkallocator);
}

void ImVulkanVkMatToImMat(const VkMat &src, ImMat &dst)
{
    Option opt;
    const VkAllocator* allocator = (VkAllocator*)src.allocator;
    const VulkanDevice* vkdev = allocator->vkdev;
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    dst.create_like(src);
    VkCompute cmd(vkdev);
    cmd.record_download(src, dst, opt);
    cmd.submit_and_wait();
    vkdev->reclaim_blob_allocator(opt.blob_vkallocator);
    vkdev->reclaim_staging_allocator(opt.staging_vkallocator);
}

void ImVulkanVkMatToVkImageMat(const VkMat &src, VkImageMat &dst)
{
    Option opt;
    const VkAllocator* allocator = (VkAllocator*)src.allocator;
    const VulkanDevice* vkdev = allocator->vkdev;
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    VkCompute cmd(vkdev);
    cmd.record_upload(src, dst, opt);
    cmd.submit_and_wait();
    vkdev->reclaim_blob_allocator(opt.blob_vkallocator);
    vkdev->reclaim_staging_allocator(opt.staging_vkallocator);
}
} // namespace ImGui
