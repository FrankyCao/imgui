#include "Lut3D.h"
#define DECLARE_ALIGNED(n,t,v)      t __attribute__ ((aligned (n))) v

#include "Lut3D_Shader.h"
#include "ImVulkanShader.h"
#include <algorithm>
#include "SDR709_HDR2020_HLG.h"
#include "SDR709_HDR2020_PQ.h"
#include "HDR2020_HLG_SDR709.h"
#include "HDR2020_PQ_SDR709.h"
#include "HDR2020PQ_HDR2020HLG.h"
#include "HDR2020HLG_HDR2020PQ.h"

#define MAX_LEVEL 256
#define MAX_LINE_SIZE 512
#define NEXT_LINE(loop_cond) do {                           \
    if (!fgets(line, sizeof(line), f)) {                    \
        fprintf(stderr, "Unexpected EOF\n");                \
        return -1;                                          \
    }                                                       \
} while (loop_cond)

static inline float clipf_c(float a, float amin, float amax)
{
    if      (a < amin) return amin;
    else if (a > amax) return amax;
    else               return a;
}

static inline int _isspace(int c) 
{
    return (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' ||
            c == '\v');
}

static inline int skip_line(const char *p)
{
    while (*p && _isspace(*p))
        p++;
    return !*p || *p == '#';
}

static inline int size_mult(size_t a, size_t b, size_t *r)
{
    size_t t = a * b;
    /* Hack inspired from glibc: don't try the division if nelem and elsize
     * are both less than sqrt(SIZE_MAX). */
    if ((a | b) >= ((size_t)1 << (sizeof(size_t) * 4)) && a && t / a != b)
        return -1;
    *r = t;
    return 0;
}

static inline void * malloc_array(size_t nmemb, size_t size)
{
    size_t result;
    if (size_mult(nmemb, size, &result) < 0)
        return NULL;
    return malloc(result);
}

static void sanitize_name(char* name)
{
    for (std::size_t i = 0; i < strlen(name); i++)
    {
        if (!isalnum(name[i]))
        {
            name[i] = '_';
        }
    }
}

static std::string path_to_varname(const char* path, bool sanitize = true)
{
    const char* lastslash = strrchr(path, '/');
    const char* name = lastslash == NULL ? path : lastslash + 1;

    std::string varname = name;
    if (sanitize)
        sanitize_name((char*)varname.c_str());

    std::transform(varname.begin(), varname.end(), varname.begin(), ::toupper);
    return varname;
}

static std::string path_to_name(const char* path, int up_low = 0)
{
    const char* lastslash = strrchr(path, '/');
    const char* name = lastslash == NULL ? path : lastslash + 1;

    std::string varname = name;
    std::string prefix = varname.substr(0, varname.rfind("."));

    if (up_low == 1)
        std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper);
    else if (up_low == 2)
        std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::tolower);
    return prefix;
}

namespace ImGui 
{
LUT3D_vulkan::LUT3D_vulkan(int default_model, int interpolation, int gpu)
{
    if (default_model < 0 || default_model > NO_DEFAULT)
    {
        return;
    }
    if (init(interpolation, gpu) != 0)
    {
        return;
    }

    switch (default_model)
    {
        case SDR709_HDRHLG:
            lutsize = SDR709_HDR2020_HLG_SIZE;
            lut = (void *)sdr709_hdr2020_hlg_lut;
            scale.r = SDR709_HDR2020_HLG_R_SCALE;
            scale.g = SDR709_HDR2020_HLG_G_SCALE;
            scale.b = SDR709_HDR2020_HLG_B_SCALE;
            scale.a = SDR709_HDR2020_HLG_A_SCALE;
            break;
        case SDR709_HDRPQ:
            lutsize = SDR709_HDR2020_PQ_SIZE;
            lut = (void *)sdr709_hdr2020_pq_lut;
            scale.r = SDR709_HDR2020_PQ_R_SCALE;
            scale.g = SDR709_HDR2020_PQ_G_SCALE;
            scale.b = SDR709_HDR2020_PQ_B_SCALE;
            scale.a = SDR709_HDR2020_PQ_A_SCALE;
            break;
        case HDRHLG_SDR709:
            lutsize = HDR2020_HLG_SDR709_SIZE;
            lut = (void *)hdr2020_hlg_sdr709_lut;
            scale.r = HDR2020_HLG_SDR709_R_SCALE;
            scale.g = HDR2020_HLG_SDR709_G_SCALE;
            scale.b = HDR2020_HLG_SDR709_B_SCALE;
            scale.a = HDR2020_HLG_SDR709_A_SCALE;
            break;
        case HDRPQ_SDR709:
            lutsize = HDR2020_PQ_SDR709_SIZE;
            lut = (void *)hdr2020_pq_sdr709_lut;
            scale.r = HDR2020_PQ_SDR709_R_SCALE;
            scale.g = HDR2020_PQ_SDR709_G_SCALE;
            scale.b = HDR2020_PQ_SDR709_B_SCALE;
            scale.a = HDR2020_PQ_SDR709_A_SCALE;
            break;
        case HDRHLG_HDRPQ:
            lutsize = HDR2020HLG_HDR2020PQ_SIZE;
            lut = (void *)hdr2020hlg_hdr2020pq_lut;
            scale.r = HDR2020HLG_HDR2020PQ_R_SCALE;
            scale.g = HDR2020HLG_HDR2020PQ_G_SCALE;
            scale.b = HDR2020HLG_HDR2020PQ_B_SCALE;
            scale.a = HDR2020HLG_HDR2020PQ_A_SCALE;
            break;
        case HDRPQ_HDRHLG:
            lutsize = HDR2020PQ_HDR2020HLG_SIZE;
            lut = (void *)hdr2020pq_hdr2020hlg_lut;
            scale.r = HDR2020PQ_HDR2020HLG_R_SCALE;
            scale.g = HDR2020PQ_HDR2020HLG_G_SCALE;
            scale.b = HDR2020PQ_HDR2020HLG_B_SCALE;
            scale.a = HDR2020PQ_HDR2020HLG_A_SCALE;
            break;
        default:
            break;
    }

    ImMat lut_cpu;
    lut_cpu.create_type(lutsize, lutsize * 4, lutsize, (void *)lut, IM_DT_FLOAT32);
    VkTransfer tran(vkdev);
    tran.record_upload(lut_cpu, lut_gpu, opt, false);
    tran.submit_and_wait();
    from_file = false;
}

LUT3D_vulkan::LUT3D_vulkan(std::string lut_path, int interpolation, int gpu)
{
    int ret = 0;
    ret = parse_cube(lut_path);
    if (ret != 0 || lutsize == 0)
    {
        if (lut)
        {
            free(lut);
            lut = nullptr;
        }
        return;
    }
    if (init(interpolation, gpu) != 0)
    {
        return;
    }

    ImMat lut_cpu;
    lut_cpu.create_type(lutsize, lutsize * 4, lutsize, (void *)lut, IM_DT_FLOAT32);
    VkTransfer tran(vkdev);
    tran.record_upload(lut_cpu, lut_gpu, opt, false);
    tran.submit_and_wait();
    from_file = true;
}

LUT3D_vulkan::~LUT3D_vulkan()
{
    if (vkdev)
    {
        if (cmd) { delete cmd; cmd = nullptr; }
        if (opt.blob_vkallocator) { vkdev->reclaim_blob_allocator(opt.blob_vkallocator); opt.blob_vkallocator = nullptr; }
        if (opt.staging_vkallocator) { vkdev->reclaim_staging_allocator(opt.staging_vkallocator); opt.staging_vkallocator = nullptr; }
    }
    if (lut && from_file) { free(lut); lut = nullptr; }
}

int LUT3D_vulkan::init(int interpolation, int gpu)
{
    vkdev = get_gpu_device(gpu);
    if (vkdev == NULL) return -1;
    opt.blob_vkallocator = vkdev->acquire_blob_allocator();
    opt.staging_vkallocator = vkdev->acquire_staging_allocator();
    opt.use_image_storage = false;
    opt.use_fp16_arithmetic = true;
    opt.use_fp16_storage = true;
    cmd = new VkCompute(vkdev);
    std::vector<vk_specialization_type> specializations(0);
    std::vector<uint32_t> spirv_data;
    compile_spirv_module(LUT3D_data, opt, spirv_data);
    pipeline_lut3d = new Pipeline(vkdev);
    pipeline_lut3d->set_optimal_local_size_xyz(16, 16, 1);
    pipeline_lut3d->create(spirv_data.data(), spirv_data.size() * 4, specializations);
    cmd->reset();
    interpolation_mode = interpolation;
    return 0;
}

int LUT3D_vulkan::allocate_3dlut(int size)
{
    int i;
    if (size < 2 || size > MAX_LEVEL) 
    {
        fprintf(stderr, "Too large or invalid 3D LUT size\n");
        return -1;
    }

    if (lut && from_file)
    {
        free(lut);
        lut = nullptr;
    }
    lut = (rgbvec *)malloc_array(size * size * size, sizeof(float) * 4);
    if (!lut)
        return -1;

    lutsize = size;
    return 0;
}

int LUT3D_vulkan::parse_cube(std::string lut_file)
{
    FILE *f = fopen(lut_file.c_str(), "r");
    if (f == NULL) return -1;
    char line[MAX_LINE_SIZE];
    float min[3] = {0.0, 0.0, 0.0};
    float max[3] = {1.0, 1.0, 1.0};

    while (fgets(line, sizeof(line), f)) 
    {
        if (!strncmp(line, "LUT_3D_SIZE", 11)) 
        {
            int ret, i, j, k;
            const int size = strtol(line + 12, NULL, 0);
            const int size2 = size * size;

            ret = allocate_3dlut(size);
            if (ret < 0)
            {
                fclose(f);
                return ret;
            }

            for (k = 0; k < size; k++) 
            {
                for (j = 0; j < size; j++) 
                {
                    for (i = 0; i < size; i++) 
                    {
                        rgbvec *table_lut = (rgbvec *)lut;
                        rgbvec *vec = &table_lut[i * size2 + j * size + k];
                        do 
                        {
try_again:
                            NEXT_LINE(0);
                            if (!strncmp(line, "DOMAIN_", 7)) 
                            {
                                float *vals = NULL;
                                if      (!strncmp(line + 7, "MIN ", 4)) vals = min;
                                else if (!strncmp(line + 7, "MAX ", 4)) vals = max;
                                if (!vals)
                                {
                                    fclose(f);
                                    return -1;
                                }
                                sscanf(line + 11, "%f %f %f", vals, vals + 1, vals + 2);
                                //fprintf(stderr, "min: %f %f %f | max: %f %f %f\n", min[0], min[1], min[2], max[0], max[1], max[2]);
                                goto try_again;
                            } 
                            else if (!strncmp(line, "TITLE", 5)) 
                            {
                                goto try_again;
                            }
                        } while (skip_line(line));
                        if (sscanf(line, "%f %f %f", &vec->r, &vec->g, &vec->b) != 3)
                        {
                            fclose(f);
                            return -1;
                        }
                    }
                }
            }
            break;
        }
    }
    scale.r = clipf_c(1. / (max[0] - min[0]), 0.f, 1.f);
    scale.g = clipf_c(1. / (max[1] - min[1]), 0.f, 1.f);
    scale.b = clipf_c(1. / (max[2] - min[2]), 0.f, 1.f);
    scale.a = 1.f;
    fclose(f);
    // For dump lut file to header files
    //write_header_file("/Users/dicky/Desktop/SDR709_HDR2020_HLG.h");
    return 0;
}

void LUT3D_vulkan::write_header_file(std::string filename)
{
    if (!lut) return;
    rgbvec* _lut = (rgbvec *)lut;
    FILE * fp = fopen(filename.c_str(), "w");
    if (!fp) return;
    std::string include_guard_var = path_to_varname(filename.c_str());
    std::string guard_var_up = path_to_name(filename.c_str(), 1);
    std::string guard_var_low = path_to_name(filename.c_str(), 2);
    fprintf(fp, "#ifndef __LUT_INCLUDE_GUARD_%s__\n", include_guard_var.c_str());
    fprintf(fp, "#define __LUT_INCLUDE_GUARD_%s__\n", include_guard_var.c_str());
    fprintf(fp, "\n");
    fprintf(fp, "#define %s_SIZE %d\n", guard_var_up.c_str(), lutsize);
    fprintf(fp, "\n");
    fprintf(fp, "#define %s_R_SCALE %f\n", guard_var_up.c_str(), scale.r);
    fprintf(fp, "#define %s_G_SCALE %f\n", guard_var_up.c_str(), scale.g);
    fprintf(fp, "#define %s_B_SCALE %f\n", guard_var_up.c_str(), scale.b);
    fprintf(fp, "#define %s_A_SCALE %f\n", guard_var_up.c_str(), scale.a);
    fprintf(fp, "\n");
    int _lutsize = lutsize * lutsize * lutsize;
    fprintf(fp, "DECLARE_ALIGNED(32, const static rgbvec, %s_lut)[%d] = {\n", guard_var_low.c_str(), _lutsize);
    for (int x = 0; x < _lutsize; x++)
    {
        if (x % 4 == 0)
            fprintf(fp, "\n\t");
        if (x < _lutsize - 1)
            fprintf(fp, "{%f, %f, %f, %f}, ", _lut[x].r, _lut[x].g, _lut[x].b, 0.f);
        else
            fprintf(fp, "{%f, %f, %f, %f}", _lut[x].r, _lut[x].g, _lut[x].b, 0.f);
    }

    fprintf(fp, "\n};\n");
    fprintf(fp, "\n");
    fprintf(fp, "#endif /* __LUT_INCLUDE_GUARD_%s__ */\n", include_guard_var.c_str());
    fclose(fp);
}

void LUT3D_vulkan::upload_param(const VkMat& src, VkMat& dst)
{
    std::vector<VkMat> bindings(9);
    if      (dst.type == IM_DT_INT8)     bindings[0] = dst;
    else if (dst.type == IM_DT_INT16)    bindings[1] = dst;
    else if (dst.type == IM_DT_FLOAT16)  bindings[2] = dst;
    else if (dst.type == IM_DT_FLOAT32)  bindings[3] = dst;

    if      (src.type == IM_DT_INT8)     bindings[4] = src;
    else if (src.type == IM_DT_INT16)    bindings[5] = src;
    else if (src.type == IM_DT_FLOAT16)  bindings[6] = src;
    else if (src.type == IM_DT_FLOAT32)  bindings[7] = src;
    bindings[8] = lut_gpu;
    std::vector<vk_constant_type> constants(12);
    constants[0].i = src.w;
    constants[1].i = src.h;
    constants[2].i = src.c;
    constants[3].i = src.color_format;
    constants[4].i = src.type;
    constants[5].i = dst.w;
    constants[6].i = dst.h;
    constants[7].i = dst.c;
    constants[8].i = dst.color_format;
    constants[9].i = dst.type;
    constants[10].i = interpolation_mode;
    constants[11].i = lut_gpu.w;
    cmd->record_pipeline(pipeline_lut3d, bindings, constants, dst);
}

void LUT3D_vulkan::filter(const ImMat& src, ImMat& dst)
{
    if (!vkdev || !pipeline_lut3d || lut_gpu.empty() || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type);

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);
    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, out_gpu);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void LUT3D_vulkan::filter(const ImMat& src, VkMat& dst)
{
    if (!vkdev || !pipeline_lut3d || lut_gpu.empty() || !cmd)
    {
        return;
    }

    dst.create_type(src.w, src.h, 4, dst.type, opt.blob_vkallocator);

    VkMat in_gpu;
    cmd->record_clone(src, in_gpu, opt);

    upload_param(in_gpu, dst);

    cmd->submit_and_wait();
    cmd->reset();
}

void LUT3D_vulkan::filter(const VkMat& src, ImMat& dst)
{
    if (!vkdev || !pipeline_lut3d || lut_gpu.empty() || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type);

    VkMat out_gpu;
    out_gpu.create_like(dst, opt.blob_vkallocator);

    upload_param(src, out_gpu);

    // download
    cmd->record_clone(out_gpu, dst, opt);
    cmd->submit_and_wait();
    cmd->reset();
}

void LUT3D_vulkan::filter(const VkMat& src, VkMat& dst)
{
    if (!vkdev || !pipeline_lut3d || lut_gpu.empty() || !cmd)
    {
        return;
    }
    dst.create_type(src.w, src.h, 4, dst.type, opt.blob_vkallocator);

    upload_param(src, dst);

    cmd->submit_and_wait();
    cmd->reset();
}
} //namespace ImGui 