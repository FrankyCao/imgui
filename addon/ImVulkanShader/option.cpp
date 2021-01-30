#include "option.h"

//#include "cpu.h"

namespace ImVulkan 
{
Option::Option()
{
    blob_allocator = 0;
    workspace_allocator = 0;

    blob_vkallocator = 0;
    workspace_vkallocator = 0;
    staging_vkallocator = 0;
    pipeline_cache = 0;

    use_bf16_storage = false;

    use_fp16_packed = false;
    use_fp16_storage = false;
    use_fp16_arithmetic = false;
    use_int8_packed = false;
    use_int8_storage = false;
    use_int8_arithmetic = false;

    use_subgroup_basic = false;
    use_subgroup_vote = false;
    use_subgroup_ballot = false;
    use_subgroup_shuffle = false;

    use_image_storage = false;
    use_tensor_storage = false;

    use_packing_layout = false;
    use_shader_pack8 = false;
}

} // namespace ImVulkan
