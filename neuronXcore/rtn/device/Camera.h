// ======================================================================== //
// Copyright 2018-2022 The Contributors                                            //
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

#include "rtn/device/FrameState.h"

namespace rtn {
	namespace device {

		struct Camera {
			static __device__ owl::Ray generateRay(const FrameState &fs,
				const vec2f &pixelSample,
				Random &rnd)
			{
				vec3f p(0.f);
				if (fs.camera_lens_radius > 0.0) {
					do {
						p = 2.0f*vec3f(rnd(), rnd(), 0.f) - vec3f(1.f, 1.f, 0.f);
					} while (dot(p, p) >= 1.0f);
				}

				const vec3f rd = fs.camera_lens_radius * p;
				const vec3f lens_offset = fs.camera_screen_du * rd.x + fs.camera_screen_dv * rd.y;
				const vec3f origin = fs.camera_lens_center + lens_offset;
				const vec3f direction
					= fs.camera_screen_00
					+ pixelSample.x * fs.camera_screen_du
					+ pixelSample.y * fs.camera_screen_dv
					- lens_offset
					;

				return owl::Ray(/* origin   : */ origin,
					/* direction: */ normalize(direction),
					/* tmin     : */ 1e-6f,
					/* tmax     : */ 1e8f);
			}
		};

	}

}

