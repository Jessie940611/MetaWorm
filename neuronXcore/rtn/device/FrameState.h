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

#include "rtn/device/common.h"
#include "rtn/device/TubesGeom.h"

namespace rtn {
	namespace device {

		using namespace owl;
		using namespace owl::common;

		enum ShaderMode
		{
			No_Lighting = 0,
			Direct_Lighting = 1,
			Full_Lighting = 2,
		};

		/*! keeps all information required for the raygen program to
			render one frame */
		struct FrameState {
			/*! camera setup ...*/
			vec3f camera_screen_du;
			vec3f camera_screen_dv;
			vec3f camera_screen_00;
			vec3f camera_lens_center;
			vec3f camera_lens_du;
			vec3f camera_lens_dv;
			float camera_lens_radius = 0.0;

			int frameID;
			/*! accumulation id, for progressive refinement */
			int accumID;
			/* max accumulation, control the effect of refinement and
			the speed of changes(eg. voltage changing), -1 means infinite,
			the higher maxAccum, the better refinement but slower changes */
			int maxAccum{ -1 };
			/*! shading mode, in case we support different modes */
			int shadeMode{ 2 };
			/*! path tracing path depth */
			int pathDepth{ 3 };
			int samplesPerPixel{ 1 };

			bool enable_point_light{ 0 };
			bool enable_quad_light{ 1 };
			bool enable_voltage_deriv{ 0 };
			bool enable_mesh_transparency{ 0 };
			float mesh_transparency{ 0.9 };
			int max_transparency_depth{ 7 };

			bool enable_light_decay{ 1 };
			float max_decay_distance{ 2e4f };
			float light_decay_index{ 1.03f };

			/*! quad light intensity */
			float point_light_intensity = 1.f;
			vec3f point_light_pos{ 300.f, 350.f, 0 };
			vec3f point_light_color{ 1.f, 1.f, 1.f };
			vec3f ground_color{ .1f, .1f, .1f };

			vec3f synapse_type0_color { 1.f, 0.f, 0.f };
			vec3f synapse_type1_color { 0.f, 1.f, 0.f };
			vec3f synapse_type2_color { 0.f, 0.f, 1.f };

			QuadLightGeom quad_light;
			DisneyMaterial material;		

			/* SDF settings */
			bool useSDF{ false };
			int maxMarchIteration{ 500 };

			/* voltage settings */
			int voltageSequenceLength{ 0 }; // or total time
			int step{ 1 };
			float epslon{ 1e-3f };
			float voltage_base{ .433f };
			float voltage_threshold{ .001f };
			bool enable_emissive{ 0 };
			float emissive_intensity{ 0.3f };
			float inhibition{ -10.0f };
			float resting{ 0.0f };
			float excitation{ 10.0f };
			float firing{ 20.0f };
			float excitationDeriv{ 0.1f };
			float firingDeriv{ 0.2f };
			int voltageColorMapSize = 0;
			float minVoltage = -110;
			float maxVoltage = 20;
		};

	}
}

