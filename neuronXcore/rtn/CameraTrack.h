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

#ifndef  CAMERA_TRACK_H_
#define  CAMERA_TRACK_H_

#include <vector>

#include "owl/common/math/box.h"

#ifndef Log
#define Log  printf
#endif

using namespace owl::common;

namespace rtn
{

#define PI 3.1415936f

	class CameraTrack
	{
	public:

		virtual vec3f get_camera_position(int index) const = 0;
	};

	//template <typename T>
	class SphereTrack : public CameraTrack
	{

	private:
		int _nSamples;
		std::vector<vec3f> _samples;
		box3f _sceneBounds;
	public:
		SphereTrack(const box3f &bound, int samples)
			: _nSamples(samples)
			, _sceneBounds(bound)
		{
			float r = _sceneBounds.span().z;
			float theta = 0.5f * PI;
			for (int i = 0; i < _nSamples; ++i)
			{
				float alpha = float(i) / float(_nSamples);
				float phi = alpha * (2.f * PI);
				vec3f p;
				p.x = r * sin(theta) * cos(phi);
				p.z = r * sin(theta) * sin(phi);
				p.y = r * cos(theta);
				p = p + _sceneBounds.center();
				_samples.emplace_back(p);
			}
		}

		vec3f get_camera_position(int index) const override
		{
			return _samples[index];
		}
	};
}

#endif
