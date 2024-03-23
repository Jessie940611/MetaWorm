// ======================================================================== //
// Copyright 2019-2020 The Collaborators                                    //
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

#include "Triangles.h"
#include "rtn/device/FrameState.h"
#include "TubesUtils.h"
#include "owl/owl.h"
#include "rtn/Curve.h"

namespace rtn {

	using device::QuadLightGeom;

	/*! the entire set of tubes, including all links - everything we
		want to render */
	struct OWLTubes {
		OWLTubes();
		/*! build owl-model (OWLGeoms, OWLGroup, etc) that we can ray
				trace against */
		virtual void build(
			/*! the tubes to build over, usign either
			onne group of user prims, or one inst per tube */
			Tubes::SP rtn,
			/*! the triangle mesh(es) */
			Triangles::SP triangles, 
			/*! the curve */
			std::shared_ptr<Curves> curve, 
			/* quad lights */
			const QuadLightGeom &light) = 0;

		/*! this should only ever once get called... it'll call the
		virtual buildModel(), and then set up the SBT, raygen, etc */
		void setModel(Tubes::SP rtn, Triangles::SP triModel, std::shared_ptr<Curves>curve, const QuadLightGeom &light);

		void resizeFrameBuffer(const vec2i &newSize, uint32_t* colorbuf);
		void updateFrameState(device::FrameState &fs);

		void updateBgColors(const vec3f& top, const vec3f& bottom);
		void updatevoltagesColor(float* primColors);
		void updatevoltagesDeriv(float* derivatives);

		void render();

		// inline int getVoltageColorMapSize() { return voltageColorMapSize; }

		// helper function that turns a triangle model into a owl geometry
		

		/*! size of current frame buffer */
		vec2i fbSize{ -1,-1 };
		OWLContext context = 0;
		OWLModule module = 0;
		OWLBuffer frameStateBuffer = 0;

		OWLBuffer accumBuffer = 0;
		OWLGroup  world = 0;
		OWLRayGen rayGen = 0;
		OWLMissProg missProg = 0;

		/* buffer for tubes */
		OWLBuffer vertices = 0;
		OWLBuffer endPointRadii = 0;
		OWLBuffer prevPrims = 0;
		OWLBuffer types = 0;
		OWLBuffer dendriteScale = 0;
		OWLBuffer synapseScale = 0;
		OWLBuffer voltagesColorBuffer = 0;
		OWLBuffer voltagesDerivBuffer = 0;
		// OWLBuffer voltageColorMap = 0;

		// voltagecolormap buffer size
		// int voltageColorMapSize = 0;

		/* buffer for curve */
		OWLBuffer coefficients = 0;;
		OWLBuffer radius = 0;
		OWLBuffer curveColor = 0;
		OWLBuffer length = 0;
	};

}
