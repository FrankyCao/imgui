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
#if 1
    texture->textureDescriptor = ImGui_ImplVulkan_AddTexture(texture->textureSampler, texture->textureView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
#else
    VkResult err;
    VkDescriptorPool descriptor_pool;
    // Create Descriptor Pool
    {
        VkDescriptorPoolSize pool_sizes[] =
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };
        VkDescriptorPoolCreateInfo pool_info = {};
        pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
        pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
        pool_info.pPoolSizes = pool_sizes;
        err = vkCreateDescriptorPool(device->vkdevice(), &pool_info, 0, &descriptor_pool);
        check_vk_result(err);
    }

    // Create Descriptor SetLayout:
    VkDescriptorSetLayout descriptorSetLayout;
    {
        VkSampler samplers[1] = { texture->textureSampler };
        VkDescriptorSetLayoutBinding binding[1] = {};
        binding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        binding[0].descriptorCount = 1;
        binding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        binding[0].pImmutableSamplers = samplers;
        VkDescriptorSetLayoutCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = 1;
        info.pBindings = binding;
        VkResult err = vkCreateDescriptorSetLayout(device->vkdevice(), &info, 0, &descriptorSetLayout);
        check_vk_result(err);
    }

    // Create Descriptor Set:
    //VkDescriptorSet descriptor_set;
    {
        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &descriptorSetLayout;
        err = vkAllocateDescriptorSets(device->vkdevice(), &alloc_info, &texture->textureDescriptor);
        check_vk_result(err);
    }

        // Update the Descriptor Set:
    {
        VkDescriptorImageInfo desc_image[1] = {};
        desc_image[0].sampler = texture->textureSampler;
        desc_image[0].imageView = texture->textureView ;
        desc_image[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkWriteDescriptorSet write_desc[1] = {};
        write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc[0].dstSet = texture->textureDescriptor;
        write_desc[0].descriptorCount = 1;
        write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_desc[0].pImageInfo = desc_image;
        vkUpdateDescriptorSets(device->vkdevice(), 1, write_desc, 0, NULL);
    }
#endif
    return (ImTextureID)texture;
}

}