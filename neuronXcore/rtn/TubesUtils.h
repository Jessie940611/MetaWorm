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

#pragma once

#include "device/common.h"
#include "device/TubesGeom.h"
// std
#include <vector>
#include <memory>

#ifndef Log
#define Log  printf
#endif

namespace rtn {

	/*! the entire set of tubes, including all links - everything we
		wnat to render */
	struct Tubes {
		typedef std::shared_ptr<Tubes> SP;

		/*! list of all control points */
		std::vector<vec3f> vertices;
		/*! list of radii per end point */
		std::vector<float> endPointRadii;
		/*! list of previous primitive */
		std::vector<unsigned int> prevPrims;
		/*! list of type per primitive*/
		std::vector<unsigned int> types;
		/*! list of voltages per primitive */
		std::vector<std::vector<float>> voltages;
		/*! list of voltage existance per primitive */
		std::vector<bool> haveVoltage;
		/*! list of start time per primitive */
		std::vector<unsigned int> voltageStart;
		/*! list of end time per primitive */
		std::vector<unsigned int> voltageEnd;
		/*! list of materials per primitive */
		std::vector<DisneyMaterial> materials;
		/*! bounding box of the whole set of tubes */
		box3f bounds;
		/*! maximum and minimum voltage data */
		float maxVoltage = -10000.0f;
		float minVoltage = 10000.0f;
		/*! max time of voltage data */
		unsigned int maxTime = 0;
		bool haveVoltages = false;
		float dendriteScale = 1.0f;
		float synapseScale = 1.0f;
		
		struct Synapse
		{
			std::vector<unsigned int> prePrim;
			std::vector<unsigned int> postPrim;
			std::vector<unsigned int> type;
			std::vector<float> weight; /*! trained weight */
		} synapse;

		float maxScale = 1.0f;
		float minScale = 0.1f;

		bool loadGeom(std::string geomFile);

		bool loadVoltage(std::string voltageFile);

		float weightToScale(float weight);

		bool loadSynapse(std::string synapseFile);

		// bool loadMaterial(std::string materialFile);

		void mergeTubes(Tubes::SP tube);

		static Tubes::SP load(std::vector<std::string> &geomFiles,
			std::vector<std::string> &voltageFiles,
			std::vector<std::string> &synapseFiles,
			std::vector<std::string> &materialFiles);

#if 0
		static Tubes::SP load(const std::string &fileName);

		/*! load several files
			@see load(const std::string&,float) for more info
		 */
		static Tubes::SP load(const std::vector<std::string> &fileNames);

		/*! load several files */
		bool loadColors(std::vector< std::vector<float>> & primColorsVec, const std::string fileName);

		/*! load several files */
		bool loadColors_m1(std::vector< std::vector<float>>& primColorsVec, const std::string fileName);

		/*! load several files */
		bool randomColors(std::vector< std::vector<float>>& primColorsVec, int vec_n, int vec_len = 1);

		/*!
			procedurally generate some linked tubes,so we have "something"
			to test.  numTubes --- maximum supported size on 1080 card:
			29000000.  lengthInterval radiusInterval
		*/
		static Tubes::SP procedural(int numTubes = 1000,
			vec2f lengthInterval = vec2f(0.5f, 3.5f),
			vec2f radiusInterval = vec2f(0.2f, 0.02f));

		/*! returns the stored line strips as a list of individual links/lines */
		HostGeom asLinks() const;

		/*! returns the stored line strips as cubic bsplines */
		HostGeom asCubicSplines() const;

		/* returns the stored raw data */
		HostGeom asRaw() const;

		/*! returns world space bounding box of the scene; to be used for
			the viewer to automatically set camera and motion speed */
		virtual box3f getBounds() const;

		/*! split bspline curves into 1,2,4, or 8 curves */
		int        curveSegments{ 1 };
#endif
	};

	

}
