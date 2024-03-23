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

#include "rtn/device/TubesGeom.h"
#include "rtn/device/PerRayData.h"
#include "rtn/device/RayGenData.h"
#include "rtn/device/Camera.h"
#include "rtn/device/disney_bsdf.h"

#include <optix_device.h> // Only for test, can be remove later. 

#define HIGH -24.0f
#define MID1 -42.0f
#define MID2 -60.0f
#define LOW -100.0f
#define DELTA 18.0f

namespace rtn {
	namespace device {

		__device__ inline
			vec4f ToneMap(const vec4f& c, float limit)
		{
			float luminance = 0.3f * c.x + 0.6f * c.y + 0.1f * c.z;

			vec4f col = c * 1.0f / (1.0f + luminance / limit);
			return vec4f(col.x, col.y, col.z, 1.0f);
		}

		__device__ inline
			vec4f LinearToSRGB(const vec4f& c)
		{
			const float kInvGamma = 1.0f / 2.2f;
			return vec4f(powf(c.x, kInvGamma), powf(c.y, kInvGamma), powf(c.z, kInvGamma), c.w);
		}


		static inline __device__
			int32_t make_8bit(const float f)
		{
			return min(255, max(0, int(f * 256.f)));
		}

		static     inline __device__
			int32_t make_rgba8(const vec4f color)
		{
			return
				(make_8bit(color.x) << 0) +
				(make_8bit(color.y) << 8) +
				(make_8bit(color.z) << 16) + 
				(make_8bit(color.w) << 24);
		}

		inline __device__ vec3f random_in_unit_sphere(Random& rnd) {
			vec3f p;
			do {
				p = 2.0f * vec3f(rnd(), rnd(), rnd()) - vec3f(1.f, 1.f, 1.f);
			} while (dot(p, p) >= 1.0f);
			return p;
		}

		inline __device__  vec3f toneMapping(const vec3f& color)
		{
			float a = 2.51f;
			float b = 0.03f;
			float c = 2.43f;
			float d = 0.59f;
			float e = 0.14f;
			return saturate((color*(a*color + b)) / (color*(color*color + d) + e));
		}


		inline __device__
			vec3f vmin(vec3f x, vec3f y)
		{
			vec3f res = vec3f(0.0f);
			res.x = min(x.x, y.x);
			res.y = min(x.y, y.y);
			res.z = min(x.z, y.z);
			return res;
		}


		inline __device__
			vec3f vmax(vec3f x, vec3f y)
		{
			vec3f res = vec3f(0.0f);
			res.x = max(x.x, y.x);
			res.y = max(x.y, y.y);
			res.z = max(x.z, y.z);
			return res;
		}

		inline __device__
			vec3f vpow(vec3f x, float y)
		{
			vec3f res = vec3f(0.0f);
			res.x = pow(x.x, y);
			res.y = pow(x.y, y);
			res.z = pow(x.z, y);
			return res;
		}

		inline __device__
			vec3f linear2srgb(vec3f col)
		{
			col = vmax(6.10352e-5, col);
			return vmin(col * 12.92f, vpow(vmax(col, 0.00313067), 1.0 / 2.4) * 1.055f - 0.055f);
		}


		//inline __device__
		//	vec3f vpow(vec3f x, float y)
		//{
		//	vec3f res = vec3f(0.0f);
		//	res.x = pow(x.x, y);
		//	res.y = pow(x.y, y);
		//	res.z = pow(x.z, y);
		//	return res;
		//}

		// inline __device__  vec3f linear2srgb(const vec3f& color)
		//{
		//	return vpow(color, 1.f / 2.2f);
		//}


		inline __device__
			void pathTrace(const RayGenData& self, owl::Ray& ray, Random& rnd, PerRayData& prd)
		{
			prd.attenuation = vec3f(1.0f);
			prd.radiance = vec3f(0.0f);
			prd.depth = 0;
			prd.transparentDepth = 0;
			prd.alpha = 0.0f;
			prd.fs = &self.frameStateBuffer[0];
			prd.voltagesColor = self.voltagesColor;
			prd.voltagesDeriv = self.voltagesDeriv;
			prd.rnd = &rnd;
			//prd.voltageColorMap = self.voltageColorMap;
			prd.done = false;
			prd.world = self.world;
			const int pathDepth = prd.fs->pathDepth;
			for (int depth = 0; depth < pathDepth; depth++) /* iterative version of recursion */
			{
				owl::traceRay(self.world, ray, prd, OPTIX_RAY_FLAG_DISABLE_ANYHIT);
				if (prd.done == true) 
				{
					//if (depth > 0) 
					break;
				}
				
				prd.depth++;
				depth = prd.depth;
				ray = owl::Ray(prd.hit_point, normalize(prd.wi_sampled), 1e-3f, 1e6f);
			}
		}

		/*! the actual ray generation program - note this has no formal
			function parameters, but gets its paramters throught the 'pixelID'
			and 'pixelBuffer' variables/buffers declared above */
		OPTIX_RAYGEN_PROGRAM(raygen_program)()
		{
			const RayGenData& self = owl::getProgramData<RayGenData>();
			const vec2i pixelID = owl::getLaunchIndex();
			const vec2i launchDim = owl::getLaunchDims();

			if (pixelID.x >= self.fbSize.x) return;
			if (pixelID.y >= self.fbSize.y) return;
			const int pixelIdx = pixelID.x + self.fbSize.x * pixelID.y;

			// for multi-gpu check 
			if (((pixelID.x >> 5) % self.deviceCount) != self.deviceIndex) return;

			const FrameState* fs = &self.frameStateBuffer[0];
			const int accumID = fs->accumID;
			const int spp = fs->samplesPerPixel;
			int pixel_index = pixelID.y * launchDim.x + pixelID.x;
			Random rnd(pixel_index, accumID);

			vec4f col(0.f);
			PerRayData prd;
			for (int s = 0; s < spp; s++)
			{
				vec2f pixelSample = vec2f(pixelID) + vec2f(rnd(), rnd());
				owl::Ray ray = Camera::generateRay(*fs, pixelSample, rnd);
				pathTrace(self, ray, rnd, prd);
				col += vec4f(prd.radiance, prd.alpha);
			}
			col = col / float(spp);

			const int maxAccum = fs->maxAccum;

			vec4f perFrameColor;

			if (maxAccum <= 0)
			{
				if (accumID > 0)
				{
					col = col + (vec4f)self.accumBufferPtr[pixelIdx];
				}
				self.accumBufferPtr[pixelIdx] = col;
				perFrameColor = col / (accumID + 1.f);
			}
			else /* smooth color change for situations with fixed camera */
			{
				const int accumStatus = min(accumID, maxAccum);
				if (accumID == 0)
				{
					self.accumBufferPtr[pixelIdx] = col;
				}
				if (accumID > 0 && accumID < maxAccum)
				{
					col = col + (float)accumID * (vec4f)self.accumBufferPtr[pixelIdx];
					self.accumBufferPtr[pixelIdx] = col / (accumID + 1.f);
				}
				if (accumID >= maxAccum)
				{
					col = col + (float)maxAccum * (vec4f)self.accumBufferPtr[pixelIdx];
					self.accumBufferPtr[pixelIdx] = col / ((float)maxAccum + 1.f);
				}
				perFrameColor = col / (accumStatus + 1.f);
			}

			vec3f hdrColor = vec3f(perFrameColor.x, perFrameColor.y, perFrameColor.z);
			vec3f ldrColor = toneMapping(hdrColor);
			vec3f sRGBColor = linear2srgb(ldrColor);

			uint32_t rgba = make_rgba8(vec4f(sRGBColor, perFrameColor.w));
			//uint32_t rgba = make_rgba8(col);

			self.colorBufferPtr[pixelIdx] = rgba;
		}

		OPTIX_MISS_PROGRAM(miss_program)()
		{
			const auto& miss_data = owl::getProgramData<MissData>();
			const vec3f top = miss_data.topColor;
			const vec3f bottom = miss_data.bottomColor;
			const vec3f ray_dir = optixGetWorldRayDirection();
			const float t = ray_dir.y * 0.5f + 0.5f; // Transform the y component from [-1.0f, 1.0f] to [0.0f, 1.0f]
			const vec3f c = lerp(top, bottom, t);

			PerRayData& prd = owl::getPRD<PerRayData>();
			prd.done = true;
			if (prd.depth == 0)
			{
				prd.radiance += c;
				prd.alpha = 0.f;
			}
			else
			{
				//prd.radiance += prd.attenuation * c * 1.618f; 
				prd.radiance += prd.attenuation * c * 0.618f;
				prd.alpha = 1.0f;
			}
		}

		OPTIX_MISS_PROGRAM(miss_program_shadow)()
		{ }
	}
}

