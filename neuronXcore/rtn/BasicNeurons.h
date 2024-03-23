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

#include "rtn/OptixTubes.h"
#include "rtn/Curve.h"

namespace rtn {
	using device::QuadLightGeom;
	using rtn::Curves;

	/*! simple/default varaint in which each tube/link becomes a user
			geometry with a world-space AABB
	*/
	struct BasicNeurons : public OWLTubes
	{
		BasicNeurons();

		void build(Tubes::SP rtn, Triangles::SP triangles, std::shared_ptr<Curves> curves, const QuadLightGeom& light) override;

	private:
		OWLGroup buildTubes(Tubes::SP rtn);
		OWLGroup buildQuad(Tubes::SP rtn);
		OWLGroup buildTriangles(Triangles::SP triModel);
		OWLGroup buildQuadLight(const QuadLightGeom& light);
		OWLGroup buildCurves(Curves::SP curves);
	};

}



