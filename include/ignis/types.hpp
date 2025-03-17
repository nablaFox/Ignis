#pragma once

#include <stdint.h>

#define IGNIS_STORAGE_BUFFER_BINDING 0
#define IGNIS_UNIFORM_BUFFER_BINDING 1
#define IGNIS_IMAGE_SAMPLER_BINDING 2

#define IGNIS_INVALID_BUFFER_ID UINT32_MAX
#define IGNIS_INVALID_IMAGE_ID UINT32_MAX

typedef uint32_t BufferId;
typedef uint32_t ImageId;
