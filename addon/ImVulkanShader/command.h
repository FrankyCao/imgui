#pragma once
#include "platform.h"
#include "allocator.h"
#include "buffer.h"
#include <vulkan/vulkan.h>

namespace ImVulkan 
{
class Pipeline;
class VkComputePrivate;
class VkCompute
{
public:
    explicit VkCompute(const VulkanDevice* vkdev);
    virtual ~VkCompute();

public:
    void record_upload(const ImageBuffer& src, VkImageBuffer& dst, const Option& opt);

    void record_upload(const ImageBuffer& src, VkImageMat& dst, const Option& opt);

    void record_download(const VkImageBuffer& src, ImageBuffer& dst, const Option& opt);

    void record_download(const VkImageMat& src, ImageBuffer& dst, const Option& opt);

    void record_buffer_to_image(const VkImageBuffer& src, VkImageMat& dst, const Option& opt);

    void record_image_to_buffer(const VkImageMat& src, VkImageBuffer& dst, const Option& opt);

    void record_clone(const ImageBuffer& src, VkImageBuffer& dst, const Option& opt);

    void record_clone(const ImageBuffer& src, VkImageMat& dst, const Option& opt);

    void record_clone(const VkImageBuffer& src, ImageBuffer& dst, const Option& opt);

    void record_clone(const VkImageMat& src, ImageBuffer& dst, const Option& opt);

    void record_clone(const VkImageBuffer& src, VkImageBuffer& dst, const Option& opt);

    void record_clone(const VkImageMat& src, VkImageMat& dst, const Option& opt);

    void record_clone(const VkImageBuffer& src, VkImageMat& dst, const Option& opt);

    void record_clone(const VkImageMat& src, VkImageBuffer& dst, const Option& opt);

    void record_pipeline(const Pipeline* pipeline, const std::vector<VkImageBuffer>& bindings, const std::vector<vk_constant_type>& constants, const VkImageBuffer& dispatcher);

    void record_pipeline(const Pipeline* pipeline, const std::vector<VkImageMat>& bindings, const std::vector<vk_constant_type>& constants, const VkImageMat& dispatcher);

    void record_pipeline(const Pipeline* pipeline, const std::vector<VkImageBuffer>& buffer_bindings, const std::vector<VkImageMat>& image_bindings, const std::vector<vk_constant_type>& constants, const VkImageBuffer& dispatcher);
    void record_pipeline(const Pipeline* pipeline, const std::vector<VkImageBuffer>& buffer_bindings, const std::vector<VkImageMat>& image_bindings, const std::vector<vk_constant_type>& constants, const VkImageMat& dispatcher);
    void record_pipeline(const Pipeline* pipeline, const std::vector<VkImageBuffer>& buffer_bindings, const std::vector<VkImageMat>& image_bindings, const std::vector<vk_constant_type>& constants, const ImageBuffer& dispatcher);

#if SHADER_BENCHMARK
    void record_write_timestamp(uint32_t query);
#endif // SHADER_BENCHMARK

    int submit_and_wait();

    int reset();

    void flash();

#if SHADER_BENCHMARK
    int create_query_pool(uint32_t query_count);

    int get_query_pool_results(uint32_t first_query, uint32_t query_count, std::vector<uint64_t>& results);
#endif // SHADER_BENCHMARK

protected:
    const VulkanDevice* vkdev;

    void barrier_readwrite(const VkImageBuffer& binding);
    void barrier_readwrite(const VkImageMat& binding);
    void barrier_readonly(const VkImageMat& binding);

private:
    VkComputePrivate* const d;
};

class VkTransferPrivate;
class VkTransfer
{
public:
    explicit VkTransfer(const VulkanDevice* vkdev);
    virtual ~VkTransfer();

public:
    void record_upload(const ImageBuffer& src, VkImageBuffer& dst, const Option& opt, bool flatten = true);

    void record_upload(const ImageBuffer& src, VkImageMat& dst, const Option& opt);

    int submit_and_wait();

protected:
    const VulkanDevice* vkdev;

private:
    VkTransferPrivate* const d;
};

} // namespace ImVulkan

