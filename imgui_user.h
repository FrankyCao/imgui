#pragma once

#ifndef NO_STB_IMAGE
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#define STBIDEF inline
#define STBIWDEF inline
#define STBIRDEF inline
#include "stb_image.h"
#include "stb_image_write.h"
#include "stb_image_resize.h"
#endif