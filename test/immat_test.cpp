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
    int width = 4;
    int height = 4;
    int channel = 3;
    ImGui::ImMat MatrixA;
    MatrixA.create_type(width, height, channel, IM_DT_FLOAT32);
    print_mat("MatrixA", MatrixA);

    for (int z = 0; z < MatrixA.c; z++)
    {
        for (int y = 0; y < MatrixA.h; y++)
        {
            for (int x = 0; x < MatrixA.w; x++)
            {
                MatrixA.at<float>(x,y,z) = y * MatrixA.w + x + 1;
            }
        }
    }

    print_mat("MatrixA", MatrixA);

    auto t = MatrixA.t();
    print_mat("MatrixA.t", t);

    ImGui::ImMat dot_mat = MatrixA * t;
    print_mat("MatrixA *  MatrixA.t", dot_mat);

    ImGui::ImMat add_mat = MatrixA / 2.0f;
    print_mat("MatrixA / 2", add_mat);

    return 0;
}