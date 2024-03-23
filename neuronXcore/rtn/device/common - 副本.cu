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

#define HIGH -24.0f//37.1453f//-24.0f
#define MID1 -42.0f
#define MID2 -60.0f
#define LOW  -78.0f//-90.2629f//-100.0f
#define DELTA 18.0f

//:37.1453 Min : -90.2629

namespace rtn {
	namespace device {


		inline __device__ vec3f short_rainbow_rgb(float v)
		{
			v = clamp(v,LOW, HIGH);
			v = (v - LOW) / (HIGH - LOW);

			float a = (1.f - v) / 0.25f;	//invert and group
			int X = floor(a);	//this is the integer part
			int Y = floor(255 * (a - X)); //fractional part from 0 to 255
			int r, g, b;

			switch (X)
			{
				case 0: r = 255; g = Y; b = 0; break;
				case 1: r = 255 - Y; g = 255; b = 0; break;
				case 2: r = 0; g = 255; b = Y; break;
				case 3: r = 0; g = 255 - Y; b = 255; break;
				case 4: r = 0; g = 0; b = 255; break;
			}
			return vec3f(r/255.f, g/255.f, b/255.f);
	    }

		// Lei : change this for a better vol --> color.
		inline __device__ vec3f vol2rgb(float v) {
			float red, green, blue;
			if (v > HIGH)
			{
				red = green = 1.0f;
				blue = (v - HIGH) / DELTA;
				if (blue > 1.0f)
				{
					blue = 1.0f;
				}
			}
			else if (v > MID1)
			{
				red = 1.0f;
				green = (v - MID1) / DELTA;
				blue = 0.0f;
			}
			else if (v > MID2)
			{
				red = (v - MID2) / DELTA;
				green = (MID1 - v) / (2 * DELTA);
				blue = 0.0f;
			}
			else if (v > LOW)
			{
				red = 0.0f;
				green = (v - LOW) / (2 * DELTA);
				blue = (MID2 - v) / DELTA;
			}
			else
			{
				red = green = 0.0f;
				blue = 1.0 - (LOW - v) / DELTA;
				if (blue < 0.0f)
				{
					blue = 0.0f;
				}
			}
			return vec3f(red, green, blue);
		}

		inline __device__
			int32_t make_8bit(const float f)
		{
			return min(255, max(0, int(f * 256.f)));
		}

		inline __device__
			int32_t make_rgba8(const vec4f color)
		{
			return
				(make_8bit(color.x) << 0) +
				(make_8bit(color.y) << 8) +
				(make_8bit(color.z) << 16);
		}

		inline __device__ vec3f random_in_unit_sphere(Random& rnd) {
			vec3f p;
			do {
				p = 2.0f * vec3f(rnd(), rnd(), rnd()) - vec3f(1.f, 1.f, 1.f);
			} while (dot(p, p) >= 1.0f);
			return p;
		}

		// ------------------------------------------------------------------
		// actual tubes stuff
		// ------------------------------------------------------------------

		inline __device__
			vec3f missColor(const Ray& ray, vec3f top_color, vec3f bottom_color)
		{
			const vec2i pixelID = owl::getLaunchIndex();
			const float t = pixelID.y / (float)optixGetLaunchDimensions().y;
			const vec3f c = (1.0f - t) * top_color + t * bottom_color;
			return c;
		}


		inline __device__
			vec3f pathTrace(const RayGenData& self,
				owl::Ray& ray,
				Random& rnd,
				PerRayData& prd)
		{
			vec3f attenuation = 1.f;

			const FrameState* fs = &self.frameStateBuffer[0];
			int pathDepth = fs->pathDepth;

			vec3f ambientLight(fs->ambient /*1.8f*/);

			/* ambientLight *= fs->frameID / 60;*/

			 /* code for tubes */
			if (pathDepth <= 1)
			{
				prd.primID = -1;
				owl::traceRay(/*accel to trace against*/self.world,
					/*the ray to trace*/ ray,
					/*prd*/prd,
					OPTIX_RAY_FLAG_DISABLE_ANYHIT);

				if (prd.primID < 0)
					return missColor(ray, vec3f(0.5f), vec3f(fs->bg));

				vec3f N = prd.Ng;
				if (dot(N, (vec3f)ray.direction) > 0.f)
					N = -N;
				N = normalize(N);

				// hardcoded albedo for now:
				vec3f albedo;

				// Tractography colors
				if (fs->shadeMode == 1 && prd.meshID < 0)
				{
					unsigned colorID = self.primColors[prd.primID];
					colorID = clamp(colorID, (unsigned)0, (unsigned)12085);
					float rgba = self.voltagesColor[colorID];
					float val = rgba ;
					albedo = vol2rgb(val);
					//albedo = short_rainbow_rgb(val);
					// move this color stuff to CH program!?
					// TODO: this assumes that primitives are lines!!
					//vec3f pa = self.vertices[prd.primID*2];
					//vec3f pb = self.vertices[prd.primID*2+1];
					 //albedo = abs(normalize(pa - pb)); //vec3f(0.628, 0.85, 0.511);
				}
				else
				{
					// Random colors
					albedo = randomColor(1 + prd.primID);
				}
				vec3f color = albedo * (.2f + .6f * fabsf(dot(N, (vec3f)ray.direction)));
				return color;
			}

			// could actually swtich material based on meshID ...
			DisneyMaterial material = fs->material;
			/* iterative version of recursion, up to depth 50 */
			for (int depth = 0; true; depth++)
			{
				prd.primID = -1;
				owl::traceRay(/*accel to trace against*/self.world,
					/*the ray to trace*/ ray,
					/*prd*/prd,
					OPTIX_RAY_FLAG_DISABLE_ANYHIT);

				if (prd.primID == -1)
				{
					// miss...
					if (depth == 0)
						return missColor(ray, vec3f(0.06f, 0.11f, 0.26f), vec3f(fs->bg));

#if FAST_SHADING
					return attenuation * ambientLight;
#else
					float phi = atan2(ray.direction.z, ray.direction.x); // azimuth 
					float theta = acos(ray.direction.y / length(ray.direction)); // elevation
					const float half_width = M_PIF / 8.f;
					const float elevation = M_PIF / 4.f; // from 0 to PI
					const float azimuth = M_PIF / 2.f;  // from 0 to 2PI
					if (theta > (elevation - half_width) && theta < (elevation + half_width)
						&& phi >(azimuth - half_width) && phi < (azimuth + half_width)) {
						return attenuation * owl::vec3f(fs->light);
					}
					else {
						return attenuation * owl::vec3f(ambientLight / 2.f);
					}
#endif
				}

				vec3f N = normalize(prd.Ng);
				const vec3f w_o = -ray.direction;
				if (dot(N, w_o) < 0.f)
				{
					N = -N;
				}

				if (prd.meshID >= 0)
				{
					/* kinda hacky, check if mesh has vertex colors. if not, use material base color. */
					if (!(prd.color.x < 0))
					{
						material.base_color = prd.color;
					}
				}
				else
				{
					// Tractography colors
					if (fs->shadeMode == 1)
					{
						// TODO: this assumes that primitives are lines!!
						//vec3f pa = self.vertices[prd.primID*2];
						//vec3f pb = self.vertices[prd.primID*2+1];
						//material.base_color = abs(normalize(pa - pb)); //vec3f(0.628, 0.85, 0.511);
						unsigned colorID = self.primColors[prd.primID];
						colorID = clamp(colorID, (unsigned)0, (unsigned)12085);
						float rgba = self.voltagesColor[colorID];
						float val = rgba;
						material.base_color = vol2rgb(val);
						//material.base_color = short_rainbow_rgb(val);
						//material.base_color = vec3f((rgba & 0xff) / 255.f,
						//    ((rgba >> 8) & 0xff) / 255.f,
						//    ((rgba >> 16) & 0xff) / 255.f);
						
					}
					// Other shade modes here...
					else if (fs->shadeMode == 2)
					{
						if (self.primColors[prd.primID] != unsigned(-1))
						{
							unsigned rgba = self.primColors[prd.primID]; // ignore alpha for now
							material.base_color = vec3f((rgba & 0xff) / 255.f,
								((rgba >> 8) & 0xff) / 255.f,
								((rgba >> 16) & 0xff) / 255.f);
						}
						else
							material.base_color = vec3f(35.0f / 255.0f, 60.0f / 255.0f, 43.0f / 255.0f);
					}
				}

				owl::vec3f v_x, v_y;
				ortho_basis(v_x, v_y, N);
				// pdf and dir are set by sampling the BRDF
				float pdf;
				vec3f scattered_direction;
				vec3f albedo = sample_disney_brdf(material, N, w_o, v_x, v_y, rnd, scattered_direction, pdf);

				const vec3f scattered_origin = ray.origin + prd.t * ray.direction;
				ray = owl::Ray(/* origin   : */ scattered_origin,
					/* direction: */ scattered_direction,
					/* tmin     : */ 1e-3f,
					/* tmax     : */ 1e+8f);

				if (depth >= pathDepth || pdf == 0.f || albedo == owl::vec3f(0.f))
				{
					// ambient term:
					return owl::vec3f(0.f);//attenuation * ambientLight;
				}

				attenuation *= albedo * fabs(dot(scattered_direction, N)) / pdf;
			}
		}

		OPTIX_MISS_PROGRAM(miss_program)()
		{
			/*! nothing to do - we initialize prd before trace */
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

			// for multi-gpu: only render every deviceCount'th column of 32 pixels:
			if (((pixelID.x / 32) % self.deviceCount) != self.deviceIndex)
				return;

			uint64_t clock_begin = clock64();
			const FrameState* fs = &self.frameStateBuffer[0];
			int pixel_index = pixelID.y * launchDim.x + pixelID.x;
			vec4f col(0.f);
			Random rnd(pixel_index, fs->accumID );

			PerRayData prd;

			for (int s = 0; s < fs->samplesPerPixel; s++)
			{
				vec2f pixelSample = vec2f(pixelID) + vec2f(rnd(), rnd());
				float u = float(pixelID.x + rnd());
				float v = float(pixelID.y + rnd());
				owl::Ray ray = Camera::generateRay(*fs, pixelSample, rnd);
				col += vec4f(pathTrace(self, ray, rnd, prd), 1);
			}
			col = col / float(fs->samplesPerPixel);

			uint64_t clock_end = clock64();
			if (fs->heatMapEnabled)
			{
				float t = (clock_end - clock_begin) * fs->heatMapScale;
				if (t >= 256.f * 256.f * 256.f)
					col = vec4f(1, 0, 0, 1);
				else
				{
					uint64_t ti = uint64_t(t);
					col.x = ((ti >> 16) & 255) / 255.f;
					col.y = ((ti >> 8) & 255) / 255.f;
					col.z = ((ti >> 0) & 255) / 255.f;
				}
			}

			if (fs->accumID > 0)
				col = col + (vec4f)self.accumBufferPtr[pixelIdx];
			self.accumBufferPtr[pixelIdx] = col;

			uint32_t rgba = make_rgba8(col / (fs->accumID + 1.f));
			self.colorBufferPtr[pixelIdx] = rgba;
		}

	}
}

