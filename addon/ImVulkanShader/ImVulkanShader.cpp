#include "ImVulkanShader.h"
#include "imgui_impl_vulkan.h"
#include "imvk_mat_shader.h"

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
    ImTextureVk texture = new ImTextureVK("Texture From VkImage");
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

void ImVulkanShaderClear()
{
    destroy_gpu_instance();
}

float ImVulkanPeak(VulkanDevice* vkdev, int loop, int count_mb, int cmd_loop, int storage_type, int arithmetic_type, int packing_type)
{
    const int count = count_mb * 1024 * 1024;
    int elempack = packing_type == 0 ? 1 : packing_type == 1 ? 4 : 8;
    if (!vkdev->info.support_fp16_storage() && storage_type == 2)
    {
        return -233;
    }
    if (!vkdev->info.support_fp16_arithmetic() && arithmetic_type == 1)
    {
        return -233;
    }
    double max_gflops = 0;
    ImGui::Option opt;
    opt.use_fp16_packed = storage_type == 1;
    opt.use_fp16_storage = storage_type == 2;
    opt.use_fp16_arithmetic = arithmetic_type == 1;
    opt.use_shader_pack8 = packing_type == 2;

    // setup pipeline
    ImGui::Pipeline pipeline(vkdev);
    {
        int local_size_x = std::min(128, std::max(32, (int)vkdev->info.subgroup_size()));
        pipeline.set_local_size_xyz(local_size_x, 1, 1);
        std::vector<ImGui::vk_specialization_type> specializations(2);
        specializations[0].i = count;
        specializations[1].i = loop;
        // glsl to spirv
        // -1 for omit the tail '\0'
        std::vector<uint32_t> spirv;
        if (packing_type == 0)
        {
            ImGui::compile_spirv_module(glsl_p1_data, opt, spirv);
        }
        if (packing_type == 1)
        {
            ImGui::compile_spirv_module(glsl_p4_data, opt, spirv);
        }
        if (packing_type == 2)
        {
            ImGui::compile_spirv_module(glsl_p8_data, opt, spirv);
        }

        pipeline.create(spirv.data(), spirv.size() * 4, specializations);
    }

    ImGui::VkAllocator* allocator = vkdev->acquire_blob_allocator();

    // prepare storage
    {
        ImGui::VkMat a;
        ImGui::VkMat b;
        ImGui::VkMat c;
        {
            if (opt.use_fp16_packed || opt.use_fp16_storage)
            {
                a.create(count, (size_t)(2u * elempack), elempack, allocator);
                b.create(count, (size_t)(2u * elempack), elempack, allocator);
                c.create(count, (size_t)(2u * elempack), elempack, allocator);
            }
            else
            {
                a.create(count, (size_t)(4u * elempack), elempack, allocator);
                b.create(count, (size_t)(4u * elempack), elempack, allocator);
                c.create(count, (size_t)(4u * elempack), elempack, allocator);
            }
        }

        // encode command
        ImGui::VkCompute cmd(vkdev);
        for (int i = 0; i < cmd_loop; i++)
        {
            {
                std::vector<ImGui::VkMat> bindings(3);
                bindings[0] = a;
                bindings[1] = b;
                bindings[2] = c;
                std::vector<ImGui::vk_constant_type> constants(0);
                cmd.record_pipeline(&pipeline, bindings, constants, c);
            }

            // time this
            {
                double t0 = ImGui::get_current_time();

                int ret = cmd.submit_and_wait();
                if (ret != 0)
                {
                    vkdev->reclaim_blob_allocator(allocator);
                    return -1;
                }

                double time = ImGui::get_current_time() - t0;
                const double mac = (double)count * (double)loop * 8 * elempack * 2;
                double gflops = mac / time / 1000000000;
                max_gflops += gflops;
            }
            cmd.flash();
        }
        cmd.reset();
    }
    vkdev->reclaim_blob_allocator(allocator);
    return max_gflops / cmd_loop;
}

#if IMGUI_BUILD_EXAMPLE
static std::string print_result(float gflops)
{
    if (gflops == -1)
            return "  error";

    if (gflops == -233)
        return "  not supported";

    if (gflops == 0)
        return "  not tested";

    if (gflops > 1000)
        return "  " + std::to_string(gflops / 1000.0) + " TFLOPS";
    return "  " + std::to_string(gflops) + " GFLOPS";
}

void ImVulkanTestWindow(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    ImGui::Begin(name, p_open, flags);
    static int loop_count = 200;
    static int block_count = 20;
    static int cmd_count = 1;
    static float fp32[8] = {0.f};
    static float fp32v4[8] = {0.f};
    static float fp32v8[8] = {0.f};
    static float fp16pv4[8] = {0.f};
    static float fp16pv8[8] = {0.f};
    static float fp16s[8] = {0.f};
    static float fp16sv4[8] = {0.f};
    static float fp16sv8[8] = {0.f};
    int device_count = get_gpu_count();
    for (int i = 0; i < device_count; i++)
    {
        ImGui::VulkanDevice* vkdev = ImGui::get_gpu_device(i);
        uint32_t driver_version = vkdev->info.driver_version();
        uint32_t api_version = vkdev->info.api_version();
        int device_type = vkdev->info.type();
        std::string driver_ver = std::to_string(VK_VERSION_MAJOR(driver_version)) + "." + 
                                std::to_string(VK_VERSION_MINOR(driver_version)) + "." +
                                std::to_string(VK_VERSION_PATCH(driver_version));
        std::string api_ver =   std::to_string(VK_VERSION_MAJOR(api_version)) + "." + 
                                std::to_string(VK_VERSION_MINOR(api_version)) + "." +
                                std::to_string(VK_VERSION_PATCH(api_version));
        std::string device_name = vkdev->info.device_name();
        uint32_t gpu_memory_budget = vkdev->get_heap_budget();
        ImGui::Text("%uMB", gpu_memory_budget);
        ImGui::Text("Device[%d]", i);
        ImGui::Text("Driver:%s", driver_ver.c_str());
        ImGui::Text("   API:%s", api_ver.c_str());
        ImGui::Text("  Name:%s", device_name.c_str());
        ImGui::Text("Memory:%uMB", gpu_memory_budget);
        ImGui::Text("Device Type:%s", device_type == 0 ? "Discrete" : device_type == 1 ? "Integrated" : device_type == 2 ? "Virtual" : "CPU");
        std::string buffon_label = "Perf Test##" + std::to_string(i);
        if (ImGui::Button(buffon_label.c_str(), ImVec2(120, 20)))
        {
            int _loop_count = device_type == 0 ? loop_count : loop_count / 5;
            fp32[i]     = ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 0, 0, 0);
            fp32v4[i]   = ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 0, 0, 1);
            fp32v8[i]   = ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 0, 0, 2);
            fp16pv4[i]  = ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 1, 1, 1);
            fp16pv8[i]  = ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 1, 1, 2);
            fp16s[i]    = ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 2, 1, 0);
            fp16sv4[i]  = ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 2, 1, 1);
            fp16sv8[i]  = ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 2, 1, 2);
        }
        ImGui::Text(" FP32 Scalar :%s", print_result(fp32[i]).c_str());
        ImGui::Text("   FP32 Vec4 :%s", print_result(fp32v4[i]).c_str());
        ImGui::Text("   FP32 Vec8 :%s", print_result(fp32v8[i]).c_str());
        ImGui::Text("  FP16p Vec4 :%s", print_result(fp16pv4[i]).c_str());
        ImGui::Text("  FP16p Vec8 :%s", print_result(fp16pv8[i]).c_str());
        ImGui::Text("FP16s Scalar :%s", print_result(fp16s[i]).c_str());
        ImGui::Text("  FP16s Vec4 :%s", print_result(fp16sv4[i]).c_str());
        ImGui::Text("  FP16s Vec8 :%s", print_result(fp16sv8[i]).c_str());
        
        ImGui::Separator();
    }
    ImGui::End();
}
#endif

} // namespace ImGui
