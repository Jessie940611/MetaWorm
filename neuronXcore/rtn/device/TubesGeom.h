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

#include "rtn/device/common.h"

namespace rtn {
	namespace device {

		/*! the device-side tubes geometry */
		struct TubesGeom {
			/*! list of all control points (a buffer) */
			vec3f    *vertices;
			/*! list of radii per end point (a buffer) */
			float    *endPointRadii;
			/*! list of previous primitive (a buffer) */
			unsigned int *prevPrims;
			/*! list of node types (a buffer) */
			unsigned int *types;

			float* dendriteScale;
			float* synapseScale;
		};

		struct TrianglesGeomData
		{
			vec3f* color;
			vec3i* index;
			vec3f* vertex;
		};

		struct QuadGeom
		{
			vec3f  p;
			vec3f  v1;
			vec3f  v2;
		};

		struct QuadLightGeom
		{
			vec3f  p;
			vec3f  v1;
			vec3f  v2;
			vec3f  color;
			float  intensity;
		};

		struct CurveGeom
		{
			vec3f* coefficients;
			vec4f* radius;
			vec3f* curveColor;
			float* length;
		};


	}
}
