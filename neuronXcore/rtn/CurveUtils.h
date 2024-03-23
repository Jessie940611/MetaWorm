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
#include "TubesUtils.h"
// std
#include <array>
#include <cstddef>
#include <utility>
#include <vector>

namespace rtn {
  
  typedef std::array<vec3f,4> BSpline;

  // Split b-spline curves
  static std::pair<BSpline,BSpline> splitBSplineImpl(const BSpline& curve)
  {
    std::pair<BSpline,BSpline> result;
  
    vec3f p0 = curve[0];
    vec3f p1 = curve[1];
    vec3f p2 = curve[2];
    vec3f p3 = curve[3];

    vec3f q0 = (p0 + p1) / 2.0f;
    vec3f q1 = (p1 + p2) / 2.0f;
    vec3f q2 = (p2 + p3) / 2.0f;

    vec3f r0 = (q0 + q1) / 2.0f;
    vec3f r1 = (q1 + q2) / 2.0f;

    vec3f s0 = (r0 + r1) / 2.0f;
 
    result.first  = {{p0, q0, r0, s0}};
    result.second = {{s0, r1, q2, p3}};

    return result;
  };

  template <std::size_t NumCurves>
  static void splitBspline(std::array<BSpline,NumCurves>& dst, const BSpline& src)
  {
    static_assert(NumCurves >= 1 && (NumCurves & (NumCurves-1)) == 0, "Can only split into PoT num curves!!");

    // TODO: really recurse :-)
    auto p0 = splitBSplineImpl(src);
    auto p00 = splitBSplineImpl(p0.first);
    auto p01 = splitBSplineImpl(p0.second);
    auto p000 = splitBSplineImpl(p00.first);
    auto p001 = splitBSplineImpl(p00.second);
    auto p010 = splitBSplineImpl(p01.first);
    auto p011 = splitBSplineImpl(p01.second);

    if (NumCurves == 1) {
      dst[0] = src;
    } else if (NumCurves == 2) {
      dst[0] = p0.first;
      dst[1] = p0.second;
    } else if (NumCurves == 4) {
      dst[0] = p00.first;
      dst[1] = p00.second;
      dst[2] = p01.first;
      dst[3] = p01.second;
    } else if (NumCurves == 8) {

      dst[0] = p000.first;
      dst[1] = p000.second;
      dst[2] = p001.first;
      dst[3] = p001.second;
      dst[4] = p010.first;
      dst[5] = p010.second;
      dst[6] = p011.first;
      dst[7] = p011.second;

    }
  }

  template <std::size_t NumCurves>
  static Tubes::HostGeom splitBSplineLinks(const Tubes::HostGeom &input)
  {
    static_assert(NumCurves >= 1 && (NumCurves & (NumCurves-1)) == 0, "Can only split into PoT num curves!!");

    Tubes::HostGeom result;

    for (std::size_t i=0; i<input.vertices.size(); i+=4) {
      BSpline curve{{input.vertices[i],input.vertices[i+1],input.vertices[i+2],input.vertices[i+3]}};
      std::array<BSpline,NumCurves> splitCurves;
      splitBspline(splitCurves,curve);
      for (std::size_t j=0; j<NumCurves; ++j) {
        for (std::size_t k=0; k<4; ++k) {
          result.vertices.push_back(splitCurves[j][k]);
        }
        // TODO: assume these are per prim, otherwise
        // we'd have to interpolate these as well..
        result.endPointRadii.push_back(input.endPointRadii[i/4]);
        result.endPointRadii.push_back(input.endPointRadii[i/4]);
        result.primColors.push_back(input.primColors[i/4]);
      }
    }

    return result;
  }

}
