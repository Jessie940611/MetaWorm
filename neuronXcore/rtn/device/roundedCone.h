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

#include "disney_bsdf.h"

namespace rtn {
	namespace device {

		inline __device__
			float sign(float& val)
		{
			return val < 0.0f ? -1.0f : 1.0f;
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

		inline __device__
			vec3f uIntToVec3f(int32_t irgba)
		{
			vec3f rgb;
			rgb.z = uint_as_float(irgba >> 16 & 0xFF) / 255;
			rgb.y = uint_as_float(irgba >> 8 & 0xFF) / 255;
			rgb.x = uint_as_float(irgba & 0xFF) / 255;
			return rgb;
		}

		inline __device__
			int32_t vec3fToUInt(vec3f t)
		{
			t.x = min(0.f, max(t.x, 1.0f));   // clamp to [0.0, 1.0]
			t.y = min(0.f, max(t.y, 1.0f));
			t.z = min(0.f, max(t.z, 1.0f));
			return (int32_t(t.z * 255) << 16) | (int32_t(t.y * 255) << 8) | int32_t(t.x * 255);
		}

		inline __device__ vec3f random_in_unit_sphere(Random& rnd) {
			vec3f p;
			do {
				p = 2.0f * vec3f(rnd(), rnd(), rnd()) - vec3f(1, 1, 1);
			} while (dot(p, p) >= 1.0f);
			return p;
		}

		/*inline __device__ float lerp(const float a, const float b, const float factor)
		{
			return a * (1 - factor) + factor * b;

		}

		inline __device__ vec3f lerp(const vec3f& a, const vec3f& b, const float factor)
		{
			return a * (1 - factor) + factor * b;
		}*/

		inline __device__ float clamp(float x, float lowerlimit, float upperlimit) {
			if (x < lowerlimit)
				x = lowerlimit;
			if (x > upperlimit)
				x = upperlimit;
			return x;
		}

		inline __device__ float smoothstep(float x) {
			// Scale, bias and saturate x to 0..1 range
			x = clamp((x - 0) / 1, 0.0, 1.0);
			// Evaluate polynomial
			return x * x * (3 - 2 * x);
		}

		inline __device__ float sdConePill(const vec3f& p, const vec3f p0, const vec3f p1,
			const float r0, const float r1, bool useSigmoid)
		{
			const vec3f v = p1 - p0;
			const vec3f w = p - p0;

			// distance to p0 along cone axis
			const float c1 = dot(w, v);
			if (c1 <= 0)
				return length(p - p0) - r0;

			// cone length
			const float c2 = dot(v, v);
			if (c2 <= c1)
				return length(p - p1) - r1;

			const float b = c1 / c2;
			const vec3f Pb = p0 + b * v;

			const float thickness = useSigmoid ? lerp(r0, r1, smoothstep(b)) : lerp(r0, r1, b);

			return length(p - Pb) - thickness;
		}

		inline __device__ float opSmoothUnion(float d1, float d2, float k)
		{
			float h = max(k - abs(d1 - d2), 0.0);
			return min(d1, d2) - h * h*0.25 / k;
			//float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
			//return mix( d2, d1, h ) - k*h*(1.0-h);
		}

		inline __device__ float sdSphere(const vec3f& p, const vec3f c, float r)
		{
			return length(p - c) - r;
		}

		/*inline __device__ bool intersectSdfSphere(const owl::Ray& ray, const float radius, const vec3f center, float& hitPoint)
		{

			vec3f p = ray.origin + ray.direction;
			float t = 0;
			for (int i = 0; i < MAX_MARCH_ITERATION; i++)
			{

				float distance = sdSphere(p, center, radius);

				if (distance < 10)
				{
					hitPoint = t;
					return true;
				}
				else
				{
					t += distance;
				}

				p = ray.origin + t * ray.direction;

			}
			return false;

		}*/

		inline __device__ void primToTube(const TubesGeom& self, int primID, vec3f& pa, vec3f& pb, float& ra, float& rb)
		{
			if (self.types[primID] == 1)
			{
				pa = self.vertices[primID];
				pb = self.vertices[primID];
				ra = self.endPointRadii[primID];
				rb = self.endPointRadii[primID];
				// ra = self.endPointRadii[primID] * 0.5f;
				// rb = self.endPointRadii[primID] * 0.5f;
			}
			else
			{
				const unsigned int prevPrim = self.prevPrims[primID] - 1;

				pa = self.vertices[prevPrim];
				pb = self.vertices[primID];
				ra = self.endPointRadii[prevPrim];
				rb = self.endPointRadii[primID];

				// if (self.types[prevPrim] == 1)
				// {
				// 	// ra = self.endPointRadii[prevPrim] * 0.5f;
				// 	// rb = (self.endPointRadii[primID] + self.endPointRadii[prevPrim]) * 0.125f;
				// 	//ra = self.endPointRadii[prevPrim] * 0.5f;
				// 	//rb = (self.endPointRadii[primID] + self.endPointRadii[prevPrim]) * 0.5f;
				// 	ra = self.endPointRadii[primID];
				// 	rb = self.endPointRadii[primID];
				// }
				// else
				// {
				// 	const unsigned int prevPrev = self.prevPrims[prevPrim] - 1;

				// 	if (self.types[prevPrev] == 1)
				// 	{
				// 		ra = (self.endPointRadii[prevPrev] + self.endPointRadii[prevPrim])* 0.125f;
				// 	}
				// 	else
				// 	{
				// 		ra = self.endPointRadii[prevPrim];
				// 	}

				// 	rb = self.endPointRadii[primID];
				// }
			}
			if (self.types[primID] == 2)
			{
				if (self.types[self.prevPrims[primID] - 1] == 1)
				{
					rb *= self.dendriteScale[0];
				}
				else
				{
					ra *= self.dendriteScale[0];
					rb *= self.dendriteScale[0];
				}
			}
			if (self.types[primID] == 5 || self.types[primID] == 6 || self.types[primID] == 7)
			{
				//vec3f ab = pb - pa;
				//pa += 1.5f * ab;
				//pb -= 1.5f * ab;
				ra *= self.dendriteScale[0];
				rb *= self.synapseScale[0];
			}
			//ra *= 0.01f;
			//rb *= 0.01f;
		}

		inline __device__ float calcSdfDistance(vec3f p, const owl::Ray& ray, const TubesGeom& self, int primID, bool useSigmoid)
		{
			vec3f pa, pb;
			float ra, rb;

			primToTube(self, primID, pa, pb, ra, rb);

			float d = sdConePill(p, pa, pb, ra, rb, 1);
			return d;

		}

		inline __device__ float sminPoly(const float d1, const float d2, const float k)
		{
			float h = clamp(0.5 + 0.5*(d2 - d1) / k, 0.0, 1.0);
			return lerp(d2, d1, h) - k * h*(1.0 - h);

		}

		inline __device__ vec3f computeSdfNormal(vec3f p, const owl::Ray& ray, const TubesGeom& self, int primID, bool useSigmoid)
		{
			// todo for mathen: set pixel dependent value
			const float e = 0.1f;
			// tetrahedron techni_vecque (4 evaluations)
			const vec3f k0 = vec3f(1.0f, -1.0f, -1.0f), k1 = vec3f(-1.0f, -1.0f, 1.0f),
				k2 = vec3f(-1.0f, 1.0f, -1.0f), k3 = vec3f(1.0f, 1.0f, 1.0f);

			return normalize(k0 * calcSdfDistance(p + e * k0, ray, self, primID, useSigmoid) +
				k1 * calcSdfDistance(p + e * k1, ray, self, primID, useSigmoid) +
				k2 * calcSdfDistance(p + e * k2, ray, self, primID, useSigmoid) +
				k3 * calcSdfDistance(p + e * k3, ray, self, primID, useSigmoid));

		}

		inline __device__ float sdfDistance(vec3f p, const owl::Ray& ray, const TubesGeom self, int primID, bool useSigmoid)
		{

			float d = calcSdfDistance(p, ray, self, primID, useSigmoid);

			// for(int i =0; i< 5; i++)
			// {
			//   //if(primID >2850 ) printf("primNeighbors %d\n", primID);
			//   int neighborID = self.primNeighbors[primID * 5  + i];
			//   if(neighborID >= 0)
			//   {
			//       float nd = calcSdfDistance(p, ray, self, neighborID, useSigmoid);
			//       d = sminPoly(d, nd, 0.1f);
			//   }

			// }

			return d;

		}

		inline __device__ bool intersectSdfRoundConeWithNeighbors(const owl::Ray& ray, const TubesGeom self, int primID,
			float & hitPoint, vec3f& normal, bool useSigmoid, int& maxMarchIteration)
		{
			bool sdfSign = true;
			float t = ray.tmin;
			float omega = 1.2f;
			float prevRadius = .0f;
			float stepLength = .0f;
			float candidateError = 1000000.0f;
			float candidate_t = 0;
			int i = 0;
			for (i = 0; i < maxMarchIteration; i++)
			{
				vec3f p = ray.origin + t * ray.direction;

				float signedRadius = sdfSign * sdfDistance(p, ray, self, primID, useSigmoid);
				float radius = abs(signedRadius);
				bool sorFail = (omega > 1.f && (radius + prevRadius) < stepLength);

				if (sorFail)
				{
					stepLength -= omega * stepLength;
					omega = 1.f;
				}
				else
				{
					stepLength = signedRadius * omega;
				}

				prevRadius = radius;

				float error = radius / t;

				if (!sorFail && error < candidateError)
				{
					candidate_t = t;
					candidateError = error;
				}


				if (!sorFail && (error < 0.0001))
					break;

				t += stepLength;

			}

			hitPoint = candidate_t;

			normal = computeSdfNormal(ray.origin + t * ray.direction, ray, self, primID, useSigmoid);

			if (t > ray.tmax || i > maxMarchIteration - 1) return false;
			return true;

		}

		//inline __device__ bool intersectSdfRoundCone(const owl::Ray& ray, const float startPoint,
		//	const float endPoint, float& hitPoint, const vec3f& p0, const vec3f& p1,
		//	const float r0, const float r1, vec3f& normal, bool useSigmoid)
		//{
		//	//todo：get boundingbox intersection point
		//	bool sdfSign = true;
		//	float t = startPoint;
		//	float omega = 1.2f;
		//	float prevRadius = .0f;
		//	float stepLength = .0f;
		//	float candidateError = 1000000.0f;
		//	float candidate_t = 0;
		//	int i = 0;
		//	for (i = 0; i < MAX_MARCH_ITERATION; i++)
		//	{
		//		vec3f p = ray.origin + t * ray.direction;

		//		float signedRadius = sdfSign * sdConePill(p, p0, p1, r0, r1, useSigmoid);
		//		float radius = abs(signedRadius);
		//		bool sorFail = (omega > 1.f && (radius + prevRadius) < stepLength);

		//		if (sorFail)
		//		{
		//			stepLength -= omega * stepLength;
		//			omega = 1.f;
		//		}
		//		else
		//		{
		//			stepLength = signedRadius * omega;
		//		}

		//		prevRadius = radius;

		//		float error = radius / t;

		//		if (!sorFail && error < candidateError)
		//		{
		//			candidate_t = t;
		//			candidateError = error;
		//		}

		//		if (!sorFail && (error < 0.0001))
		//			break;

		//		t += stepLength;

		//	}

		//	hitPoint = candidate_t;
		//	if (t > endPoint || i > MAX_MARCH_ITERATION - 1) return false;
		//	return true;

		//}

		inline __device__
			bool intersectSphere(const vec3f   pa,
				const float   ra,
				const owl::Ray ray,
				float& hit_t,
				vec3f& isec_normal)
		{
			// #if STATS_ON
			//       STATS_COUNT_LINK_TEST();
			// #endif
			const vec3f  oc = ray.origin - pa;
			const float  a = dot((vec3f)ray.direction, (vec3f)ray.direction);
			const float  b = dot(oc, (vec3f)ray.direction);
			const float  c = dot(oc, oc) - ra * ra;
			const float  discriminant = b * b - a * c;

			if (discriminant < 0.f) return false;

			{
				float temp = (-b - sqrtf(discriminant)) / a;
				if (temp < hit_t && temp > ray.tmin) {
					hit_t = temp;
					isec_normal = ray.origin + hit_t * ray.direction - pa;
					return true;
				}
			}

			{
				float temp = (-b + sqrtf(discriminant)) / a;
				if (temp < hit_t && temp > ray.tmin) {
					hit_t = temp;
					isec_normal = ray.origin + hit_t * ray.direction - pa;
					return true;
				}
			}
			return false;
		}


		//Correct the intersection program.
		/*! ray-cylinder intersector : ref code from shadertoy.com/view/4lcSRn */
		inline __device__ bool intersectCylinder(const vec3f   pa,
			const vec3f   pb,
			const float   ra,
			const owl::Ray ray,
			float &hit_t,
			vec3f &isec_normal)
		{
#if STATS_ON
			make sure stats are off...
				STATS_COUNT_LINK_TEST();
#endif
			/*
				lei: apply the same numerical accuracy trick in this intersector as ingo did. Only tested on my side (windows) for now
			*/
# if 1
			vec3f ro = ray.origin;
			float minDist = max(0.f, min(length(pa - ro) - ra * 2, length(pb - ro) - ra * 2));
			ro = ro + minDist * ray.direction;

			const vec3f  ba = pb - pa;
			const vec3f  oc = ro - pa;

			float baba = dot(ba, ba);
			float bard = dot(ba, ray.direction);
			float baoc = dot(ba, oc);

			float k2 = baba - bard * bard;
			float k1 = baba * dot(oc, ray.direction) - baoc * bard;
			float k0 = baba * dot(oc, oc) - baoc * baoc - ra * ra * baba;

			float h = k1 * k1 - k2 * k0;

			if (h < 0.0f) return false;

			h = sqrtf(h);
			float t = (-k1 - h) / k2;

			// body
			float y = baoc + t * bard;
			if (y > 0.0f && y < baba) {
				hit_t = minDist + t;
				isec_normal = (oc + t * ray.direction - ba * y / baba) / ra;
				return true;
			}

			// caps
			t = (((y < 0.0f) ? 0.0f : baba) - baoc) / bard;
			if (fabsf(k1 + k2 * t) < h)
			{
				hit_t = minDist + t;
				isec_normal = ba * sign(y) / baba;
				return true;
			}
			return false;
#else
			const vec3f  ba = pb - pa;
			const vec3f  oc = ray.origin - pa;

			float baba = dot(ba, ba);
			float bard = dot(ba, ray.direction);
			float baoc = dot(ba, oc);

			float k2 = baba - bard * bard;
			float k1 = baba * dot(oc, ray.direction) - baoc * bard;
			float k0 = baba * dot(oc, oc) - baoc * baoc - ra * ra * baba;

			float h = k1 * k1 - k2 * k0;

			if (h < 0.f) return false;

			h = sqrtf(h);
			float t = (-k1 - h) / k2;

			// body
			float y = baoc + t * bard;
			if (y > 0.0f && y < baba) {
				hit_t = t;
				isec_normal = (oc + t * ray.direction - ba * y / baba) / ra;
				return true;
			}

			// caps
			t = (((y < 0.0f) ? 0.0f : baba) - baoc) / bard;
			if (fabsf(k1 + k2 * t) < h)
			{
				hit_t = t;
				isec_normal = ba * sign(y) / baba;
				return true;
			}
			return false;
#endif
		}


		inline __device__ vec3f Hermite(const float t, const vec3f& a, const vec3f&b, const vec3f& c, const vec3f& d)
		{

			float t2 = t * t;
			float t3 = t2 * t;

			return t3 * a + t2 * b + t * c + d;

		}


		inline __device__ float cubicPolynomial(const float t, vec4f coef)
		{
			return coef.x * t * t * t + coef.y * t * t + coef.z * t + coef.w;
		}



		/* ray - rounded cone intersection. */
		inline __device__
			bool intersectRoundedCone(
				const vec3f  pa, const vec3f  pb,
				const float  ra, const float  rb,
				const owl::Ray ray,
				float& hit_t,
				vec3f& isec_normal)
		{
#if 1
			/* iw, mar 1st, 2020 - this is the version that uses the
				 numerical accuracy trick of firs tmoving the ray origin
				 closer to the object, then computing distance (which is
				 numerically unstable for large distances), and then moving
				 the origin back _after_ the dsitance has been found. this
				 fixes some rpetty egregious numerical issues we've seen, BUT
				 i'm not 100% sure what happens in cases where we're close to
				 the object, etc (it migth be that the "if < 0" etc tests will
				 then reject a hitpointe _before_ we can move the origin back
				 .... i don't _think_ that happens, but .... huh. */
			vec3f ro = ray.origin;
			const vec3f &rd = ray.direction;
			float minDist = max(0.f, min(length(pa - ro) - ra, length(pb - ro) - rb));
			ro = ro + minDist * rd;

			vec3f  ba = pb - pa;
			vec3f  oa = ro - pa;
			vec3f  ob = ro - pb;
			float  rr = ra - rb;
			float  m0 = dot(ba, ba);
			float  m1 = dot(ba, oa);
			float  m2 = dot(ba, rd);
			float  m3 = dot(rd, oa);
			float  m5 = dot(oa, oa);
			float  m6 = dot(ob, rd);
			float  m7 = dot(ob, ob);

			float d2 = m0 - rr * rr;

			float k2 = d2 - m2 * m2;
			float k1 = d2 * m3 - m1 * m2 + m2 * rr * ra;
			float k0 = d2 * m5 - m1 * m1 + m1 * rr * ra * 2.0f - m0 * ra * ra;

			float h = k1 * k1 - k0 * k2;
			if (h < 0.0f) return false;
			float t = (-sqrtf(h) - k1) / k2;

			float y = m1 - ra * rr + t * m2;
			if (y > 0.0f && y < d2)
			{
				hit_t = minDist + t;
				isec_normal = (d2 * (oa + t * rd) - ba * y);
				return true;
			}

			// Caps. 
			float h1 = m3 * m3 - m5 + ra * ra;
			if (h1 > 0.0f)
			{
				t = -m3 - sqrtf(h1);
				hit_t = minDist + t;
				isec_normal = ((oa + t * rd) / ra);
				return true;
			}

			// Haixin Ma: bottom cap for neuron
			float h2 = m6 * m6 - m7 + rb * rb;
			if (h2 > 0.0f)
			{
				t = -m6 - sqrtf(h2);
				hit_t = minDist + t;
				isec_normal = ((ob + t * rd) / rb);
				return true;
			}
			return false;

#else
			const vec3f &ro = ray.origin;
			const vec3f &rd = ray.direction;

			vec3f  ba = pb - pa;
			vec3f  oa = ro - pa;
			vec3f  ob = ro - pb;
			float  rr = ra - rb;
			float  m0 = dot(ba, ba);
			float  m1 = dot(ba, oa);
			float  m2 = dot(ba, rd);
			float  m3 = dot(rd, oa);
			float  m5 = dot(oa, oa);
			float  m6 = dot(ob, rd);
			float  m7 = dot(ob, ob);

			float d2 = m0 - rr * rr;

			float k2 = d2 - m2 * m2;
			float k1 = d2 * m3 - m1 * m2 + m2 * rr * ra;
			float k0 = d2 * m5 - m1 * m1 + m1 * rr * ra * 2.0f - m0 * ra * ra;

			float h = k1 * k1 - k0 * k2;
			if (h < 0.0f) return false;
			float t = (-sqrtf(h) - k1) / k2;

			float y = m1 - ra * rr + t * m2;
			if (y > 0.0f && y < d2)
			{
				hit_t = t;
				isec_normal = (d2 * (oa + t * rd) - ba * y);
				return true;
			}

			// Caps. 
			float h1 = m3 * m3 - m5 + ra * ra;
			if (h1 > 0.0f)
			{
				t = -m3 - sqrtf(h1);
				hit_t = t;
				isec_normal = ((oa + t * rd) / ra);
				return true;
			}
			return false;
#endif
		}

		inline __device__ 
			bool intersectCappedCone(
				const vec3f  pa, const vec3f  pb,
				const float  ra, const float  rb,
				const owl::Ray ray,
				float& hit_t,
				vec3f& isec_normal)
		{
			vec3f ro = ray.origin;
			const vec3f &rd = ray.direction;
			float minDist = max(0.f, min(length(pa - ro) - ra, length(pb - ro) - rb));
			ro = ro + minDist * rd;

			vec3f  ba = pb - pa;
			vec3f  oa = ro - pa;
			vec3f  ob = ro - pb;
			float  rr = ra - rb;
			float  m0 = dot(ba, ba);
			float  m1 = dot(ba, oa);
			float  m2 = dot(ba, rd);
			float  m3 = dot(rd, oa);
			float  m5 = dot(oa, oa);
			float  m6 = dot(ob, rd);
			float  m7 = dot(ob, ob);

			float d2 = m0 - rr * rr;

			float k2 = d2 - m2 * m2;
			float k1 = d2 * m3 - m1 * m2 + m2 * rr * ra;
			float k0 = d2 * m5 - m1 * m1 + m1 * rr * ra * 2.0f - m0 * ra * ra;

			float h = k1 * k1 - k0 * k2;
			if (h < 0.0f) return false;
			float t = (-sqrtf(h) - k1) / k2;

			float y = m1 - ra * rr + t * m2;
			if (y > 0.0f && y < d2)
			{
				hit_t = minDist + t;
				isec_normal = (d2 * (oa + t * rd) - ba * y);
				return true;
			}

			// Caps. 
			t = -m1 / dot(rd, ba);
			vec3f otd = ro + t * rd - pa;
			if (sqrtf(dot(otd, otd)) < ra)
			{
				hit_t = minDist + t;
				isec_normal = normalize(-ba);
				return true;
			}

			t = -dot(ba, ob) / dot(rd, ba);
			otd = ro + t * rd - pb;
			if (sqrtf(dot(otd, otd)) < rb)
			{
				hit_t = minDist + t;
				isec_normal = normalize(ba);
				return true;
			}

			return false;
		}

		inline __device__
			float calcCurveLength(const vec3f& coeA, const vec3f& coeB, const vec3f& coeC, const vec3f& coeD)
		{
			float length = 0.0f;
			int piece = 60;
			float deltaT = 1.0f / float(piece);
			for (int i = 1; i <= piece; i++)
			{
				vec3f startP = Hermite((i - 1) *  deltaT, coeA, coeB, coeC, coeD);
				vec3f endP = Hermite(i * deltaT, coeA, coeB, coeC, coeD);
				vec3f line = startP - endP;
				length += sqrtf(dot(line, line));
			}
			return length;
		}

		inline __device__
			int getSegmentNumber(float length, float distance, float radius)
		{
			// todo: when distance to camera increase, decrease segment number
			if (distance < 100) distance = 100.0f;
			float segf = length / radius / distance * 100;
			int segi = 4;

			if (floor(segf) > 4) segi = floor(segf);
			else segi = 4;

			if (floor(segf) > 100) segi = 100;

			return segi;

		}

		inline __device__
			vec3f vectorTransform(vec3f v, vec3f basisX, vec3f basisY, vec3f basisZ)
		{

			vec3f res = vec3f(dot(v, basisX), dot(v, basisY), dot(v, basisZ));

			return res;
		}

		inline __device__
			float lengthXY(vec3f v)
		{
			return sqrtf(v.x * v.x + v.y *v.y);
		}

		inline __device__
			bool intersectHermiteCone(const vec3f& coeA, const vec3f& coeB, const vec3f& coeC, const vec3f& coeD, const PerRayData& prd, const owl::Ray& ray,
				float& tmp_hit_t, vec3f& normal, const vec3f& pa, const vec3f& pb, const vec4f& rad, const float& length)
		{

			//method1 : adaptive segment detection
			{
				float minT = 0.0f;
				float minD = 100000.0f;
				//const int seg = getSegmentNumber(ray.t);

				float r0 = cubicPolynomial(0.0f, rad);
				float r1 = cubicPolynomial(1.0f, rad);

				int seg = 4;
				float deltaT = 0.25f;

				if (rad.w > 10)
				{
					seg = 1;
					deltaT = 1.0f;
				}
				else
				{
					seg = getSegmentNumber(length, prd.t, 2 * (r0 + r1));
					deltaT = 1.0f / float(seg);
				}

				for (int i = 1; i <= seg; i++)
				{
					float hit_t;
					vec3f startP = Hermite((i - 1) *  deltaT, coeA, coeB, coeC, coeD);
					vec3f endP = Hermite(i * deltaT, coeA, coeB, coeC, coeD);
					float ra = (1 - (i - 1) * deltaT) * r0 + (i - 1) * deltaT * r1;
					float rb = (1 - i * deltaT) * r0 + i * deltaT * r1;
					bool has_hit = intersectRoundedCone(startP, endP, ra, rb, ray, hit_t, normal);
					if (has_hit)
					{
						tmp_hit_t = hit_t;
						return true;
					}

				}

				return false;
			}


			/*method 2: frenet frame or project curve to plane orthogonal to ray direction
			 paper: GPU-based Raycasting of Hermite Spline Tubes
						: Exploiting Budan-Fourier and Vincent’s Theorems for Ray Tracing 3D Bézier Curves
			*/
			//{
			//    vec3f sP = Hermite( 0.0f, coeA, coeB, coeC, coeD);
			//    vec3f eP = Hermite( 1.0f, coeA, coeB, coeC, coeD);
			//    vec3f dx  = normalize(cross(sP - eP, ray.direction));
			//    vec3f dy = normalize(cross(dx, ray.direction));
			//    vec3f dz = normalize(ray.direction);
			//    //basis : dx, dy, ray.dir

			//    //T[3x3] * (a, b, c, d)* (1, t, t^2, t^3)^t, where T is transform matrix that transfer hermite curve to ray space
			//    vec3f coARay = vectorTransform(coeA, dx, dy, dz);          
			//    vec3f coBRay = vectorTransform(coeB, dx, dy, dz);
			//    vec3f coCRay = vectorTransform(coeC, dx, dy, dz);
			//    vec3f coDRay = vectorTransform(coeD, dx, dy, dz);

			//
			//    vec3f originRay = vectorTransform(ray.origin, dx, dy, dz);


			//    //
			//    const int seg = 100;
			//    const float deltaT = 0.01f;
			//    for(int i = 1; i <=seg; i++)
			//    {
			//      vec3f startP = Hermite((i-1) * deltaT, coARay, coBRay, coCRay, coDRay) - originRay;
			//      vec3f endP = Hermite(i * deltaT, coARay, coBRay, coCRay, coDRay) - originRay;
			//      float d0 = lengthXY(startP);
			//      float d1 = lengthXY(endP);

			//      //printf("value x:\t%f, y:\t%f, z:\t%f, r:\t%f", startP.x, startP.y, startP.z, ra);
			//      if(d0 >= ra && d1 <=ra )
			//      {
			//        //float alpha = d0/(d0+d1);
			//        //float beta = d1/(d0+d1); 
			//       // tmp_hit_t = alpha*(i*deltaT) + beta*((i-1)*deltaT);
			//        return true;
			//      }
			//      else if(d0 <= ra)
			//      {

			//        //tmp_hit_t = (i-1)*deltaT;
			//        return true;

			//      }            
			//    }
			//    return false;
			//}
		}

		/*! use that same intersect program in both inst and user
				versions; only difference is which prim ID we are going to
				use */
		inline __device__
			void intersectProgram(int primID)
		{
			const auto& self
				= owl::getProgramData<TubesGeom>();

			owl::Ray ray(optixGetWorldRayOrigin(),
				optixGetWorldRayDirection(),
				optixGetRayTmin(),
				optixGetRayTmax());

			PerRayData &prd = owl::getPRD<PerRayData>();
			auto fs = prd.fs;

			float tmp_hit_t = ray.tmax;

			/*const vec3f pa = self.vertices[primID * 2];
			const vec3f pb = self.vertices[primID * 2 + 1];
			const float ra = self.endPointRadii[primID * 2];
			const float rb = self.endPointRadii[primID * 2 + 1];*/

			vec3f pa, pb;
			float ra, rb;

			primToTube(self, primID, pa, pb, ra, rb);

			vec3f normal;

			if (!fs->useSDF)
			{
				if (pa == pb)
				{
					if (intersectSphere(pb, rb, ray, tmp_hit_t, normal)) {
						if (optixReportIntersection(tmp_hit_t, 0)) {
							prd.primID = primID;
							prd.meshID = -1;
							prd.t = tmp_hit_t;
							prd.Ng = normal;
							prd.hit_point = ray.origin + prd.t * ray.direction;
							//float alpha = dot(normalize(prd.hit_point - pb), normalize(pa - pb));
						}
					}
				}
				//if (ra == rb)
				//{
				//	if (intersectCylinder(pa, pb, ra, ray, tmp_hit_t, normal)) {
				//		if (optixReportIntersection(tmp_hit_t, 0)) {
				//			prd.primID = primID;
				//			prd.meshID = -1;
				//			prd.t = tmp_hit_t;
				//			prd.Ng = normal;
				//			prd.hit_point = ray.origin + prd.t * ray.direction;
				//			//float alpha = dot(normalize(prd.hit_point - pb), normalize(pa - pb));
				//		}
				//	}
				//}
				else
				{
					if (intersectRoundedCone(pa, pb, ra, rb, ray, tmp_hit_t, normal)) {
						if (optixReportIntersection(tmp_hit_t, 0)) {
							prd.primID = primID;
							prd.meshID = -1;
							prd.t = tmp_hit_t;
							prd.Ng = normal;
							prd.hit_point = ray.origin + prd.t * ray.direction;
							//float alpha = dot(normalize(prd.hit_point - pb), normalize(pa - pb));

							float da, db;
							da = sqrtf(dot(prd.hit_point - pa, prd.hit_point - pa)) - ra;
							db = sqrtf(dot(prd.hit_point - pb, prd.hit_point - pb)) - ra;
							prd.relativeTubePosition = db / (da + db);
						}
					}
				}
				
			}
			if (fs->useSDF)
			{
				//if (intersectSdfRoundCone(ray, ray.tmin, ray.tmax, tmp_hit_t, pa, pb, ra, rb, normal, true)){
				if (intersectSdfRoundConeWithNeighbors(ray, self, primID, tmp_hit_t, normal, true, fs->maxMarchIteration)) {
					if (optixReportIntersection(tmp_hit_t, 0))
					{
						prd.primID = primID;
						prd.meshID = -1;
						prd.t = tmp_hit_t;
						prd.Ng = normal;
						prd.hit_point = ray.origin + prd.t * ray.direction;
						//prd.materialID = self.materialID[primID];
					}
				}
			}
		}

		inline __device__
			void intersectCurveProgram(int primID)
		{
			const auto& self
				= owl::getProgramData<CurveGeom>();

			owl::Ray ray(optixGetWorldRayOrigin(),
				optixGetWorldRayDirection(),
				optixGetRayTmin(),
				optixGetRayTmax());

			PerRayData &prd = owl::getPRD<PerRayData>();

			float tmp_hit_t = ray.tmax;

			const vec3f coeA = self.coefficients[primID * 4];
			const vec3f coeB = self.coefficients[primID * 4 + 1];
			const vec3f coeC = self.coefficients[primID * 4 + 2];
			const vec3f coeD = self.coefficients[primID * 4 + 3];

			const vec3f pa = Hermite(1.0f, coeA, coeB, coeC, coeD);
			const vec3f pb = Hermite(0.0f, coeA, coeB, coeC, coeD);

			const vec4f rad = self.radius[primID];

			const float length = self.length[primID];

			vec3f normal;

			// if (intersectRoundedCone(pa, pb, ra,rb, ray, tmp_hit_t, normal)) {
			//   if (optixReportIntersection(tmp_hit_t, 0)) {
			//     prd.primID = primID;
			//     prd.meshID = -1;
			//     prd.t = tmp_hit_t;
			//     prd.Ng = normal;
			//     prd.hit_point = ray.origin + prd.t * ray.direction;
			//     //float alpha = dot(normalize(prd.hit_point - pb), normalize(pa - pb));
			//   }
			// }

			if (intersectHermiteCone(coeA, coeB, coeC, coeD, prd, ray, tmp_hit_t, normal, pa, pb, rad, length))
			{
				if (optixReportIntersection(tmp_hit_t, 0))
				{
					prd.primID = primID;
					prd.meshID = -1;
					prd.t = tmp_hit_t;
					prd.Ng = normal;
					prd.hit_point = ray.origin + prd.t * ray.direction;
					//float alpha = dot(normalize(prd.hit_point - pb), normalize(pa - pb));
				}
			}
		}

	}
}
