// ======================================================================== //
// Copyright 2018-2020 The Contributors                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "owl/common/math/AffineSpace.h"
#include "owl/common/math/random.h"
#include <owl/owl.h>

namespace rtn {
	using namespace owl;
	using namespace owl::common;

	struct DisneyMaterial {
		vec3f base_color = vec3f(0.8f);
		float metallic = 0.f;

		float specular = 0.f;
		float roughness = 1.0f;
		float specular_tint = 0.f;
		float anisotropy = 0.f;

		float sheen = 0.f;
		float sheen_tint = 0.0f;
		float clearcoat = 0.0f;
		float clearcoat_gloss = 0.0f;

		float ior = 1.45f;
		float specular_transmission = 0.f;
	};

	namespace device {
	}
}
