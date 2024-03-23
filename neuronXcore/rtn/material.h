#pragma once

#include <string>
#include "device/common.h"

namespace rtn {
    // The material file format is similar to MTL, but with tags
    // matching the disney material. Comments can be prefixed with #,
    // and material params are listed as "key value". For example:
    // 
    // # a cool material
    // base_color 1 0 0
    // metallic 0
    // specular 1
    // roughness 0.5
    // specular_tint 0.2
    DisneyMaterial load_material(const std::string &file);
}

