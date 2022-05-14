#include <immat.h>
#include <iostream>

static void print_mat(std::string name, ImGui::ImMat & mat)
{
    std::cout << name << std::endl << "[" << std::endl;
    if (mat.dims == 1)
    {
        for (int w = 0; w < mat.w; w++)
        {
            switch(mat.type)
            {
                case IM_DT_INT8:    std::cout << mat.at<int8_t> (w) << " "; break;
                case IM_DT_INT16:   std::cout << mat.at<int16_t>(w) << " "; break;
                case IM_DT_INT32:   std::cout << mat.at<int32_t>(w) << " "; break;
                case IM_DT_INT64:   std::cout << mat.at<int64_t>(w) << " "; break;
                case IM_DT_FLOAT32: std::cout << mat.at<float>  (w) << " "; break;
                case IM_DT_FLOAT64: std::cout << mat.at<double> (w) << " "; break;
                case IM_DT_FLOAT16: std::cout << im_float16_to_float32(mat.at<uint16_t>  (w)) << " "; break;
                default: break;
            }
        }
    }
    else if (mat.dims == 2)
    {
        for (int h = 0; h < mat.h; h++)
        {
            std::cout << "    [ ";
            for (int w = 0; w < mat.w; w++)
            {
                switch(mat.type)
                {
                    case IM_DT_INT8:    std::cout << mat.at<int8_t> (w, h) << " "; break;
                    case IM_DT_INT16:   std::cout << mat.at<int16_t>(w, h) << " "; break;
                    case IM_DT_INT32:   std::cout << mat.at<int32_t>(w, h) << " "; break;
                    case IM_DT_INT64:   std::cout << mat.at<int64_t>(w, h) << " "; break;
                    case IM_DT_FLOAT32: std::cout << mat.at<float>  (w, h) << " "; break;
                    case IM_DT_FLOAT64: std::cout << mat.at<double> (w, h) << " "; break;
                    case IM_DT_FLOAT16: std::cout << im_float16_to_float32(mat.at<uint16_t>  (w, h)) << " "; break;
                    default: break;
                }
            }
            std::cout << "]" << std::endl;
        }
    }
    else if (mat.dims == 3)
    {
        for (int c = 0; c < mat.c; c++)
        {
            std::cout << "  [ " << std::endl;
            for (int h = 0; h < mat.h; h++)
            {
                std::cout << "    [ ";
                for (int w = 0; w < mat.w; w++)
                {
                    switch(mat.type)
                    {
                        case IM_DT_INT8:    std::cout << mat.at<int8_t> (w, h, c) << " "; break;
                        case IM_DT_INT16:   std::cout << mat.at<int16_t>(w, h, c) << " "; break;
                        case IM_DT_INT32:   std::cout << mat.at<int32_t>(w, h, c) << " "; break;
                        case IM_DT_INT64:   std::cout << mat.at<int64_t>(w, h, c) << " "; break;
                        case IM_DT_FLOAT32: std::cout << mat.at<float>  (w, h, c) << " "; break;
                        case IM_DT_FLOAT64: std::cout << mat.at<double> (w, h, c) << " "; break;
                        case IM_DT_FLOAT16: std::cout << im_float16_to_float32(mat.at<uint16_t>  (w, h, c)) << " "; break;
                        default: break;
                    }
                }
                std::cout << "]" << std::endl;
            }
            std::cout << "  ]" << std::endl;
        }
    }
    
    std::cout << "]" << std::endl;
}

int main(int argc, char ** argv)
{
    int mw = 4;
    int mh = 4;
    ImGui::ImMat A, B, C;
    A.create_type(mw, mh, IM_DT_FLOAT32);
    B.create_type(mw, mh, IM_DT_FLOAT32);

    for (int y = 0; y < A.h; y++)
    {
        for (int x = 0; x < A.w; x++)
        {
            A.at<float>(x,y) += y * A.w + x + 1;
        }
    }

    for (int y = 0; y < B.h; y++)
    {
        for (int x = 0; x < B.w; x++)
        {
            B.at<float>(x,y) = y * B.w + x + 1;
        }
    }

    print_mat("A", A);
    print_mat("B", B);

    // scalar math
    C = B + 2.f;
    print_mat("C=B+2", C);

    B += 2.f;
    print_mat("B+=2", B);

    C = B - 2.f;
    print_mat("C=B-2", C);

    B -= 2.f;
    print_mat("B-=2", B);

    C = A * 2.0f;
    print_mat("C=A*2", C);

    A *= 2.0f;
    print_mat("A*=2", A);

    C = A / 2.0f;
    print_mat("C=A/2", C);

    A /= 2.0f;
    print_mat("A/=2", A);

    // mat math
    C = A + B;
    print_mat("C=A+B", C);

    A += B;
    print_mat("A+=B", A);

    C = A - B;
    print_mat("C=A-B", C);

    A -= B;
    print_mat("A-=B", A);

    C = A * B;
    print_mat("C=A*B", C);

    A *= B;
    print_mat("A*=B", A);

    C = A.clip(200, 500);
    print_mat("C=A.clip(200,500)", C);

    // mat tranform
    auto t = A.t();
    print_mat("A.t", t);

    // mat setting
    auto e = A.eye(1.f);
    print_mat("A.eye", e);

    auto n = A.randn<float>(0.f, 5.f);
    print_mat("A.randn", n);

    // mat matrix math
    C = n.inv<float>();
    print_mat("C=A.randn.i", C);

    // fp16
    ImGui::ImMat A16, B16, C16;
    A16.create_type(mw, mh, IM_DT_FLOAT16);
    B16.create_type(mw, mh, IM_DT_FLOAT16);
    for (int y = 0; y < A16.h; y++)
    {
        for (int x = 0; x < A16.w; x++)
        {
            A16.at<uint16_t>(x,y) = im_float32_to_float16(y * A16.w + x + 1);
        }
    }
    for (int y = 0; y < B16.h; y++)
    {
        for (int x = 0; x < B16.w; x++)
        {
            B16.at<uint16_t>(x,y) = im_float32_to_float16(y * B16.w + x + 1);
        }
    }

    print_mat("A16", A16);
    print_mat("B16", B16);

    // fp16 scalar math
    C16 = B16 + 2.f;
    print_mat("C16=B16+2", C16);

    B16 += 2.f;
    print_mat("B16+=2", B16);

    C16 = B16 - 2.f;
    print_mat("C16=B16-2", C16);

    B16 -= 2.f;
    print_mat("B16-=2", B16);

    C16 = A16 * 2.0f;
    print_mat("C16=A16*2", C16);

    A16 *= 2.0f;
    print_mat("A16*=2", A16);

    C16 = A16 / 2.0f;
    print_mat("C16=A16/2", C16);

    A16 /= 2.0f;
    print_mat("A16/=2", A16);

    // mat math
    C16 = A16 + B16;
    print_mat("C16=A16+B16", C16);

    A16 += B16;
    print_mat("A16+=B16", A16);

    C16 = A16 - B16;
    print_mat("C16=A16-B16", C16);

    A16 -= B16;
    print_mat("A16-=B16", A16);

    C16 = A16 * B16;
    print_mat("C16=A16*B16", C16);

    A16 *= B16;
    print_mat("A16*=B16", A16);

    C16 = A16.clip(200, 500);
    print_mat("C16=A16.clip(200,500)", C16);

    // mat tranform
    auto t16 = A16.t();
    print_mat("A16.t", t16);

    // mat setting
    auto e16 = A16.eye(1.f);
    print_mat("A16.eye", e16);

    auto n16 = A16.randn<float>(0.f, 5.f);
    print_mat("A16.randn", n16);

    // mat matrix math
    //C16 = n16.inv<float>();
    //print_mat("C16=A16.randn.i", C16);

    // test mat draw
    ImGui::ImMat D;
    D.create_type(256, 256, 4, IM_DT_INT8);
    D.fill(0);
    D.elempack = 4;
    float cx = D.w * 0.5f, cy = D.h * 0.5f;
    for (int j = 0; j < 5; j++) 
    {
        float r1 = 255 * (j + 0.5f) * 0.085f;
        float r2 = 255 * (j + 1.5f) * 0.085f;
        float t = j * M_PI / 64.0f, r = (j + 1) * 0.5f;
        for (int i = 1; i <= 64; i++, t += 2.0f * M_PI / 64.0f)
        {
            float ct = cosf(t), st = sinf(t);
            D.draw_line(cx + r1 * ct, cy - r1 * st, cx + r2 * ct, cy - r2 * st, 2, (int8_t)255, (int8_t)255, (int8_t)255, (int8_t)255);
        }
    }

    return 0;
}