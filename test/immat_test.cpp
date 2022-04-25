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
    ImGui::ImMat A, B, C;
    A.create_type(4, 4, IM_DT_FLOAT32);
    B.create_type(4, 4, IM_DT_FLOAT32);

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

    C = A * B;
    print_mat("C", C);

    auto t = A.t();
    print_mat("A.t", t);

    ImGui::ImMat dot_mat = A * t;
    print_mat("A *  A.t", dot_mat);

    ImGui::ImMat add_mat = A / 2.0f;
    print_mat("A / 2", add_mat);

    auto n = A.randn<float>(0.f, 5.f);
    print_mat("A.randn", n);

    auto _det = n.det<float>();
    std::cout << "A.randn.i.det=" << _det << std::endl;

    auto i = n.inv<float>();
    print_mat("A.randn.i", n);

    return 0;
}