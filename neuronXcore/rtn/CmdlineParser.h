// ======================================================================== //
// Copyright 2018-2020 The Contributors                                     //
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


#ifndef  CMDLINE_PARSER_H_
#define  CMDLINE_PARSER_H_

#include "material.h"

#include <string>
#include <vector>

#ifndef Log
#define Log  printf
#endif

namespace rtn {
	struct CmdlineArgs
	{
		std::string method = "basic"; //instance,basic,phantom,obbphantom
		int curveSegments = 1; // split b-spline curves into segments 1,2,4, or 8 segments
		int spp = 4;
		int shadeMode = 0;
		int pathDepth = 0;
		float radius = 0.00005f;
		float bg = 0.1f;
		int   nCircleSamples = 420;
		float cameraDistScale = 0.5;
		float dendriteScale = 1.f;
		float synapseScale = 1.f;
		float maxWeightScale = 1.f;
		float minWeightScale = 0.1f;

		struct {
			vec3f vp = vec3f(0.f);
			vec3f vu = vec3f(0.f);
			vec3f vi = vec3f(0.f);
			float fov = 70.f;
			float lens_radius = 0.f;
			float focal_distance = 1.f;
			int mode = 0; /* 0: fly mode, 1: inspect mode, 2: FPS mode, 3: free mode */
		} camera;
		vec2i windowSize = vec2i(1280, 720);
		bool measure = false;

		int maxVoltage = -1000;
		int minVoltage = 1000;
		bool enable_emissive = false;
		float emissive_intensity = 0.3f;
		bool enable_light_decay = true;

		bool worm_auto_screenshot = false;
		
		DisneyMaterial material;

		std::vector<std::string> dataDirectories;
		std::vector<std::string> geomFiles;
		std::vector<std::string> voltageFiles;
		std::vector<std::string> synapseFiles;
		std::vector<std::string> materialFiles;
		std::vector<std::string> triangleFiles;
		std::vector<std::string> curveFiles;

		std::string screenshotPath = "./screenshots";

		int volSz = 1998;
		int accum = 10;
		int maxAccum{ -1 };
	};

	void usage();

	void parse_arguments(int argc, char** argv, CmdlineArgs& cmdline_args);

	void parse_directories(CmdlineArgs& cmdline_args);
}
#endif

