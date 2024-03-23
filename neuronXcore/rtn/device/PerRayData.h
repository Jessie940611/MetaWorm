// ======================================================================== //
// Copyright 2018-2019 The Contributors                                            //
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

 // To reduce the PerRayData GPU memory size, we probably need to disable this.
#define TRIMESH_COLOR

#include "rtn/device/common.h"
#include "rtn/device/FrameState.h"

namespace rtn {
	namespace device {

		typedef owl::common::LCG<16> Random;

		struct PerRayData {
			/*! primitive ID, -1 means 'no hit', otherwise this is either the
				 link (meshID=-1) or triangle mesh (meshID=0) */
			int primID;
			/*! ID of the triangle mehs we hit; if -1, this means we hit the
					tubes - eventullay this should allow to support multiple
					meshes with different materials */
			int meshID;
			int depth;

			int transparentDepth;
			/*! distance to hit point */
			float t;

			/* interpolated surface color, if available */
			OptixTraversableHandle world;
			FrameState *fs;
			Random     *rnd;
			float      *voltagesColor;
			float      *voltagesDeriv;
			// vec3uc *voltageColorMap;

			/* geometry normal at intersection.*/
			vec3f Ng;
			vec3f attenuation;
			vec3f radiance;
			vec3f hit_point;
			vec3f wi_sampled;
			float relativeTubePosition;
			float alpha;

			bool done;
		};

	}
}

