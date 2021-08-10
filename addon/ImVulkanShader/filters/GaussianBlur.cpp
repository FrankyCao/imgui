#include "GaussianBlur.h"
#include "ImVulkanShader.h"

GaussianBlur::GaussianBlur(int gpu)
    : ImGui::Filter2DS_vulkan(gpu)
{
    prepare_kernel();
}

GaussianBlur::~GaussianBlur()
{
}

void GaussianBlur::prepare_kernel()
{
    int ksize = blurRadius * 2 + 1;
    if (sigma <= 0.0f) 
    {
        sigma = ((ksize - 1) * 0.5 - 1) * 0.3 + 0.8;
    }
    double scale = 1.0f / (sigma * sigma * 2.0);
    //double cons = scale / M_PI;
    double sum = 0.0;

    //kernel.create(ksize, ksize, size_t(4u), 1);
    kernel.create(ksize, size_t(4u), 1);
    for (int i = 0; i < ksize; i++) 
    {
        int x = i - (ksize - 1) / 2;
        //kernel.at<float>(i, j) = cons * exp(-scale * (x * x + y * y));
        kernel.at<float>(i) = exp(-scale * (x * x));
        sum += kernel.at<float>(i);
    }

    sum = 1.0 / sum;
    kernel *= (float)(sum);
    ImGui::VkTransfer tran(vkdev);
    tran.record_upload(kernel, vk_kernel, opt, false);
    tran.submit_and_wait();

    xksize = yksize = ksize;
    xanchor = yanchor = blurRadius;
}

void GaussianBlur::SetParam(int _blurRadius, float _sigma)
{
    if (blurRadius != _blurRadius || sigma != _sigma)
    {
        blurRadius = _blurRadius;
        sigma = _sigma;
        prepare_kernel();
    }
}
