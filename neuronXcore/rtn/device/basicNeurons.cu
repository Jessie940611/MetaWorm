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

#include "rtn/device/TriangleMesh.h"
#include "rtn/device/Camera.h"
#include "rtn/device/roundedCone.h"

#include "rtn/device/disney_bsdf.h"

#include <optix_device.h> // Only for test, can be remove later. 

#define HIGH 10.0f
#define LOW -105.0f
#define DELTA 0.001f

#ifndef M_PIf
#define M_PIf       3.14159265358979323846f
#endif
namespace rtn {
	namespace device {

		__forceinline__ __device__ void cosine_sample_hemisphere(float u1, float u2, vec3f& p)
		{
			// Uniformly sample disk.
			const float r = sqrtf(u1);
			const float phi = 2.0f * M_PIf * u2;
			p.x = r * cosf(phi);
			p.y = r * sinf(phi);

			// Project up to hemisphere.
			p.z = sqrtf(fmaxf(0.0f, 1.0f - p.x * p.x - p.y * p.y));
		}
		struct Onb
		{
			__forceinline__ __device__ Onb(const vec3f& normal)
			{
				m_normal = normal;

				if (fabs(m_normal.x) > fabs(m_normal.z))
				{
					m_binormal.x = -m_normal.y;
					m_binormal.y = m_normal.x;
					m_binormal.z = 0;
				}
				else
				{
					m_binormal.x = 0;
					m_binormal.y = -m_normal.z;
					m_binormal.z = m_normal.y;
				}

				m_binormal = normalize(m_binormal);
				m_tangent = cross(m_binormal, m_normal);
			}

			__forceinline__ __device__ void inverse_transform(vec3f& p) const
			{
				p = p.x * m_tangent + p.y * m_binormal + p.z * m_normal;
			}

			vec3f m_tangent;
			vec3f m_binormal;
			vec3f m_normal;
		};

		static inline __device__ vec3f voltage_mapping(float voltage, float maxVol, float minVol, float inhibition, float resting, float excitation, float firing)
		{
			float span = maxVol - minVol;
			float r, ri, rr, re, rf;
			r = (voltage - minVol) / span;
			ri = (inhibition - minVol) / span;
			rr = (resting - minVol) / span;
			re = (excitation - minVol) / span;
			rf = (firing - minVol) / span;
			float rt, ir, ig, ib;

			if (r <= 0.0f + 0.00001f)
			{
				ir = 0.0f;
				ig = 0.0f;
				ib = 0.0f;
			}
			if (0.0f + 0.00001f < r && r <= ri)
			{
				rt = r / ri;
				ir = 0.0f;
				ig = 0.0f;
				ib = 0.0f + 1.0f * rt;
			}
			if (ri < r && r <= rr)
			{
				rt = (r - ri) / (rr - ri);
				ir = 0.0f + 0.1f * rt;
				ig = 0.0f + 0.1f * rt;
				ib = 1.0f - 0.9f * rt;
			}
			if (rr < r && r <= re)
			{
				rt = (r - rr) / (re - rr);
				ir = 0.1f + 0.9f * rt;
				ig = 0.1f - 0.1f * rt;
				ib = 0.1f - 0.1f * rt;
			}
			if (re < r && r <= rf)
			{
				rt = (r - re) / (rf - re);
				ir = 1.0f;
				ig = 0.0f + 1.0f * rt;
				ib = 0.0f;
			}
			if (rf < r && r <= 1.0f - 0.00001f)
			{
				rt = (r - rf) / (1.0f - rf);
				ir = 1.0f;
				ig = 1.0f;
				ib = 0.0f + 1.0f * rt;
			}
			if (1.0f - 0.00001f < r)
			{
				ir = 1.0f;
				ig = 1.0f;
				ib = 1.0f;
			}

			vec3f emissiveColor(ir / 1.01f, ig / 1.01f, ib / 1.01f);
			return emissiveColor;
		}

		static inline __device__ vec3f voltage_mapping_deriv(float deriv, float maxVol, float minVol, float excitation, float firing)
		{
			float maxDeriv = maxVol - minVol;
			float r = deriv / maxDeriv;
			float ir, ig, ib;
			
			if (r <= 0.00001f)
			{
				ir = 0.1f;
				ig = 0.1f;
				ib = 0.1f;
			}
			if (0.00001f < r && r <= excitation)
			{
				ir = 0.1f + 0.9f * r / excitation;
				ig = 0.1f;
				ib = 0.1f;
			}
			if (excitation < r && r <= firing)
			{
				ir = 1.0f;
				ig = 0.1f + 0.9f * (r - excitation) / (firing - excitation);
				ib = 0.1f;
			}
			if (firing < r && r <= 1.0f - 0.00001f)
			{
				ir = 1.0f;
				ig = 1.0f;
				ib = 0.1f + 0.9f * (r - firing) / (1.0f - firing);
			}
			if (1.0f - 0.00001f < r)
			{
				ir = 1.0f;
				ig = 1.0f;
				ib = 1.0f;
			}

			vec3f emissiveColor(ir / 1.01f, ig / 1.01f, ib / 1.01f);
			return emissiveColor;
		}

		//                                                                      // 
		//************************ BasicNeurons ********************************//
		//                                                                      // 
		OPTIX_INTERSECT_PROGRAM(BasicNeurons)()
		{
			intersectProgram(optixGetPrimitiveIndex());
		}

		// Cylinder boundingBox    
		OPTIX_BOUNDS_PROGRAM(BasicNeurons)(const void *geomData, box3f &primBounds, const int primID)
		{
			const TubesGeom &self = *(const TubesGeom*)geomData;

			//vec3f pa = self.vertices[primID * 2];
			//vec3f ra = self.endPointRadii[primID * 2];
			//primBounds = box3f().including(pa - ra)//self.radius)
			//	.including(pa + ra);//self.radius);

			//vec3f pb = self.vertices[primID * 2 + 1];
			//vec3f rb = self.endPointRadii[primID * 2 + 1];
			//primBounds
			//	= primBounds
			//	.including(pb - rb)//self.radius)
			//	.including(pb + rb);//self.radius);

			vec3f pa, pb;
			float ra, rb;

			primToTube(self, primID, pa, pb, ra, rb);

			primBounds = box3f().including(pa - ra).including(pa + ra).including(pb - rb).including(pb + rb);
		}

		OPTIX_CLOSEST_HIT_PROGRAM(BasicNeurons)()
		{
			owl::Ray ray(optixGetWorldRayOrigin(), optixGetWorldRayDirection(), optixGetRayTmin(), optixGetRayTmax());
			const auto& self = owl::getProgramData<TubesGeom>();
			auto& prd = owl::getPRD<PerRayData>();

			const FrameState* fs = prd.fs;
			const auto epslon = fs->epslon;

			const auto wo = -ray.direction;
			auto N = prd.Ng;
			N = normalize(N);
			if (dot(N, wo) < epslon)
			{
				prd.done = true;
				return;
			}

			Random& rnd = *prd.rnd;

			const unsigned int prevID = self.prevPrims[prd.primID];
			const unsigned int type = self.types[prd.primID];
			
			float voltage, voltagePre, voltageCurrent, deriv, derivPre, derivCurrent;

			const float relative = prd.relativeTubePosition;

			if (prevID == 0)
			{
				voltage = prd.voltagesColor[prd.primID];
				deriv = prd.voltagesDeriv[prd.primID];
			}
			else
			{
				voltagePre = prd.voltagesColor[self.prevPrims[prd.primID] - 1];
				voltageCurrent = prd.voltagesColor[prd.primID];
				
				voltage = voltagePre * relative + voltageCurrent * (1.0f - relative);

				derivPre = prd.voltagesDeriv[self.prevPrims[prd.primID] - 1];
				derivCurrent = prd.voltagesDeriv[prd.primID];

				deriv = derivPre * relative + derivCurrent * (1.0f - relative);
			}

			float v = (voltage - fs->minVoltage) / (fs->maxVoltage - fs->minVoltage);

			const auto voltage_threshold = fs->voltage_threshold;

			DisneyMaterial material = fs->material;

			ShaderMode shadeMode = (ShaderMode)fs->shadeMode;
			if (shadeMode == ShaderMode::No_Lighting) // always emissive
			{
				if (fs->enable_voltage_deriv)
				{
					prd.radiance += voltage_mapping_deriv(deriv, fs->maxVoltage, fs->minVoltage, fs->excitationDeriv, fs->firingDeriv) * fs->emissive_intensity;
				}
				else
				{
					prd.radiance += voltage_mapping(voltage, fs->maxVoltage, fs->minVoltage, fs->inhibition, fs->resting, fs->excitation, fs->firing) * fs->emissive_intensity;
				}				
				prd.done = true;
			}
			else if (shadeMode == ShaderMode::Direct_Lighting)
			{
				if (prd.depth >= 1)
				{
					prd.done = true;
					return;
				}

				if (v > fs->voltage_threshold)
				{
					vec3f emissive_color;

					if (fs->enable_voltage_deriv)
					{
						emissive_color = voltage_mapping_deriv(deriv, fs->maxVoltage, fs->minVoltage, fs->excitationDeriv, fs->firingDeriv) * fs->emissive_intensity;
					}
					else
					{
						emissive_color = voltage_mapping(voltage, fs->maxVoltage, fs->minVoltage, fs->inhibition, fs->resting, fs->excitation, fs->firing) * fs->emissive_intensity;
					}

					float intensity = 1.0f;

					if (fs->enable_light_decay)
					{
						const auto max_decay_distance = 256.f;
						float a = (max_decay_distance - prd.t) / max_decay_distance;
						float b = a <= 0.f ? 0.f : a * a;
						intensity *= b;
					}

					if (type == 5) emissive_color = fs->synapse_type0_color;
					if (type == 6) emissive_color = fs->synapse_type1_color;
					if (type == 7) emissive_color = fs->synapse_type2_color;

					if (type == 5 || type == 6 || type == 7)
					{
						prd.radiance += emissive_color * dot(N, wo) * intensity;
					}
					else if (fs->enable_emissive)
					{
						prd.radiance += emissive_color * fs->emissive_intensity * dot(N, wo) * intensity;
					}

					// material.base_color = emissive_color;
				}

				vec3f v_x, v_y;
				ortho_basis(v_x, v_y, N);
				float wi_pdf;
				vec3f sampled_wi;
				vec3f albedo = sample_disney_brdf(material, N, wo, v_x, v_y, rnd, sampled_wi, wi_pdf);

				if (wi_pdf < epslon || (albedo.x < epslon && albedo.y < epslon && albedo.z < epslon))
				{
					prd.done = true;
					return;
				}
				prd.attenuation *= (albedo * max(dot(sampled_wi, N), epslon) / wi_pdf);
				prd.wi_sampled = sampled_wi;

			}
			else if (shadeMode == ShaderMode::Full_Lighting)
			{
				if (v > fs->voltage_threshold)
				{
					//auto emissivePre = voltage_mapping(voltagePre, fs->maxVoltage, fs->minVoltage, fs->inhibition, fs->resting, fs->excitation, fs->firing);
					//auto emissiveCurrent = voltage_mapping(voltageCurrent, fs->maxVoltage, fs->minVoltage, fs->inhibition, fs->resting, fs->excitation, fs->firing);
					//auto emissive_color = emissivePre * relative + emissiveCurrent * (1.0f - relative);;
					
					//auto emissive_color = voltage_mapping(voltage, fs->maxVoltage, fs->minVoltage, fs->inhibition, fs->resting, fs->excitation, fs->firing);

					//auto emissive_color = 0.5f * (emissive_color1 + emissive_color2);
					vec3f emissive_color;

					if (fs->enable_voltage_deriv)
					{
						emissive_color = voltage_mapping_deriv(deriv, fs->maxVoltage, fs->minVoltage, fs->excitationDeriv, fs->firingDeriv) * fs->emissive_intensity;
					}
					else
					{
						emissive_color = voltage_mapping(voltage, fs->maxVoltage, fs->minVoltage, fs->inhibition, fs->resting, fs->excitation, fs->firing) * fs->emissive_intensity;
					}

					float intensity = 1.0f;

					if (fs->enable_light_decay)
					{
						const auto max_decay_distance = 256.f;
						float a = (max_decay_distance - prd.t) / max_decay_distance;
						float b = a <= 0.f ? 0.f : a * a;
						intensity *= b;
					}

					if (type == 5) emissive_color = fs->synapse_type0_color;
					if (type == 6) emissive_color = fs->synapse_type1_color;
					if (type == 7) emissive_color = fs->synapse_type2_color;

					if (type == 5 || type == 6 || type == 7)
					{
						prd.radiance += emissive_color * dot(N, wo) * intensity;
					}
					else if (fs->enable_emissive)
					{
						prd.radiance += emissive_color * fs->emissive_intensity * dot(N, wo) * intensity;
					}
					
					// material.base_color = emissive_color;
				}

				vec3f v_x, v_y;
				ortho_basis(v_x, v_y, N);
				float wi_pdf;
				vec3f sampled_wi;
				vec3f albedo = sample_disney_brdf(material, N, wo, v_x, v_y, rnd, sampled_wi, wi_pdf);

				if (wi_pdf < epslon || (albedo.x < epslon && albedo.y < epslon && albedo.z < epslon))
				{
					prd.done = true;
					return;
				}
				prd.attenuation *= albedo * max(dot(sampled_wi, N), epslon) / wi_pdf;
				prd.wi_sampled = sampled_wi;

			}
		}

		//                                                                      // 
		//************************ TriangleMesh ********************************//
		//                                                                      //
		OPTIX_CLOSEST_HIT_PROGRAM(TriangleMesh)()
		{
			PerRayData& prd = owl::getPRD<PerRayData>();
			const TrianglesGeomData& self = owl::getProgramData<TrianglesGeomData>();

			const int   primID = optixGetPrimitiveIndex();
			const vec3i index = self.index[primID];

			// compute normal:
			const vec3f& A = self.vertex[index.x];
			const vec3f& B = self.vertex[index.y];
			const vec3f& C = self.vertex[index.z];
			vec3f Ng = normalize(cross(B - A, C - A));

			// const vec3f rayDir = optixGetWorldRayDirection();
			// prd = (.2f + .8f * fabs(dot(rayDir, Ng))) * self.color;

			const float u = optixGetTriangleBarycentrics().x;
			const float v = optixGetTriangleBarycentrics().y;

			const auto wo = -1.0f * (vec3f)optixGetWorldRayDirection();
			if (dot(wo, Ng) < 0.0f) Ng = -1.0f * Ng;

			prd.Ng = Ng;
			prd.hit_point = u * A + v * B + (1 - u - v) * C;

			const FrameState* fs = prd.fs;
			const auto epslon = fs->epslon;
			DisneyMaterial material = fs->material;
			Random& rnd = *prd.rnd;

			vec3f col = (self.color)
				? ((1.f - u - v) * self.color[index.x]
					+ u * self.color[index.y]
					+ v * self.color[index.z])
				: vec3f(-1.0);

			if (fs->enable_mesh_transparency)
			{
				switch (prd.transparentDepth) // currently hard coded
				{
					case 0: material.base_color = vec3f(0.2f, 0.5f, 0.2f); break;
					case 1: material.base_color = vec3f(0.2f, 0.5f, 0.2f); break;
					case 2: material.base_color = vec3f(0.1f, 1.0f, 0.1f); break;
					case 3: material.base_color = vec3f(0.1f, 1.0f, 0.1f); break;
					case 4: material.base_color = vec3f(0.1f, 0.1f, 0.5f); break;
					case 5: material.base_color = vec3f(0.1f, 0.1f, 0.5f); break;
					case 6: material.base_color = vec3f(0.0f, 0.0f, 1.0f); break;
					case 7: material.base_color = vec3f(0.0f, 0.0f, 1.0f); break;
					case 8: material.base_color = vec3f(0.5f, 0.5f, 0.1f); break;
					case 9: material.base_color = vec3f(0.5f, 0.5f, 0.1f); break;
					case 10: material.base_color = vec3f(1.0f, 1.0f, 0.0f); break;
					case 11: material.base_color = vec3f(1.0f, 1.0f, 0.0f); break;
					case 12: material.base_color = vec3f(0.5f, 0.1f, 0.1f); break;
					case 13: material.base_color = vec3f(0.5f, 0.1f, 0.1f); break;
					case 14: material.base_color = vec3f(1.0f, 0.0f, 0.0f); break;
				}
			}
			else
			{
				material.base_color = col;
				//material.base_color = vec3f(0.2f, 0.2f, 0.2f);
			}

			
			vec3f v_x, v_y;
			ortho_basis(v_x, v_y, Ng);
			float wi_pdf;
			vec3f sampled_wi;
			vec3f albedo = sample_disney_brdf(material, Ng, wo, v_x, v_y, rnd, sampled_wi, wi_pdf);

			if (wi_pdf < epslon || (albedo.x < epslon && albedo.y < epslon && albedo.z < epslon))
			{
				prd.done = true;
				return;
			}

			if (fs->enable_mesh_transparency)
			{
				prd.attenuation = (albedo * max(dot(sampled_wi, Ng), epslon) / wi_pdf);
			}
			else
			{
				prd.attenuation *= (albedo * max(dot(sampled_wi, Ng), epslon) / wi_pdf);
			}		

			if (prd.transparentDepth >= fs->max_transparency_depth * 2)
			{
				prd.radiance += prd.attenuation;
				//prd.radiance += material.base_color;
				prd.done = true;
				return;
			}

			float transmit = rnd();
			if (fs->enable_mesh_transparency)
			{
				if (transmit < fs->mesh_transparency)
				{
					prd.wi_sampled = -wo;
					prd.transparentDepth += 1;
					prd.depth -= 1;
				}
				else
				{
					if (prd.transparentDepth % 2 == 0)
					{
						prd.wi_sampled = sampled_wi;
						prd.radiance += prd.attenuation;
						//prd.radiance += material.base_color;
						prd.done = true;
						return;
					}
					if (prd.transparentDepth % 2 == 1)
					{
						prd.wi_sampled = -wo;
						prd.transparentDepth += 1;
						prd.depth -= 1;
					}
				}				
			}
			else
			{
				prd.wi_sampled = sampled_wi;
			}
			
		}

		//                                                                      // 
		//************************ Curves ***************************************//
		//                                                                      // 
		inline __device__
			void  getHermiteBoundingBox(const vec3f coA, const vec3f coB, const vec3f coC, const vec3f coD, vec3f& re0, vec3f& re1)
		{
			// derivative of cubic function
			vec3f a = 3.0f * coA;
			vec3f b = 2.0f * coB;
			vec3f c = coC;

			vec3f part = 4.0f * b * b - 12.0f * a * c;
			vec3f partSolution = vec3f(sqrtf(part.x), sqrtf(part.y), sqrtf(part.z));
			// todo : if a equals zero , add small value 
			re0 = (-2.0f * b + partSolution) / (6.0f * a);
			re1 = (-2.0f * b - partSolution) / (6.0f * a);

		}

		OPTIX_INTERSECT_PROGRAM(Curves)()
		{
			intersectCurveProgram(optixGetPrimitiveIndex());
		}

		// Cylinder boundingBox    
		OPTIX_BOUNDS_PROGRAM(Curves)(const void  *geomData, box3f       &primBounds, const int    primID)
		{
			const CurveGeom &self = *(const CurveGeom*)geomData;

			const vec3f coA = self.coefficients[primID * 4];
			const vec3f coB = self.coefficients[primID * 4 + 1];
			const vec3f coC = self.coefficients[primID * 4 + 2];
			const vec3f coD = self.coefficients[primID * 4 + 3];

			const vec4f rad = self.radius[primID];

			vec3f pa = Hermite(0.0f, coA, coB, coC, coD);
			float ra = cubicPolynomial(0.0f, rad);

			vec3f pb = Hermite(1.0f, coA, coB, coC, coD);
			float rb = cubicPolynomial(1.0f, rad);

			vec3f re0, re1;
			getHermiteBoundingBox(coA, coB, coC, coD, re0, re1);

			// add end point to bounding box
			primBounds = box3f().including(pa - ra).including(pa + ra);
			primBounds = primBounds.including(pb - rb).including(pb + rb);

			if (re0.x > 0 && re0.x < 1)
			{
				float t = re0.x;
				vec3f p = Hermite(t, coA, coB, coC, coD);
				primBounds = primBounds.including(p - rb).including(p + rb);
			}
			if (re0.y > 0 && re0.y < 1)
			{
				float t = re0.y;
				vec3f p = Hermite(t, coA, coB, coC, coD);
				primBounds = primBounds.including(p - rb).including(p + rb);
			}
			if (re0.z > 0 && re0.z < 1)
			{
				float t = re0.z;
				vec3f p = Hermite(t, coA, coB, coC, coD);
				primBounds = primBounds.including(p - rb).including(p + rb);
			}

			if (re1.x > 0 && re1.x < 1)
			{
				float t = re1.x;
				vec3f p = Hermite(t, coA, coB, coC, coD);
				primBounds = primBounds.including(p - rb).including(p + rb);
			}
			if (re1.y > 0 && re1.y < 1)
			{
				float t = re1.y;
				vec3f p = Hermite(t, coA, coB, coC, coD);
				primBounds = primBounds.including(p - rb).including(p + rb);
			}
			if (re1.z > 0 && re1.z < 1)
			{
				float t = re1.z;
				vec3f p = Hermite(t, coA, coB, coC, coD);
				primBounds = primBounds.including(p - rb).including(p + rb);
			}

		}

		OPTIX_CLOSEST_HIT_PROGRAM(Curves)()
		{
			const auto& self = owl::getProgramData<CurveGeom>();
			auto& prd = owl::getPRD<PerRayData>();
			const int primID = optixGetPrimitiveIndex();
			owl::Ray ray(optixGetWorldRayOrigin(), optixGetWorldRayDirection(), optixGetRayTmin(), optixGetRayTmax());

			const FrameState* fs = prd.fs;
			const auto epslon = fs->epslon;

			const auto wo = -ray.direction;
			auto Ng = prd.Ng;
			Ng = normalize(Ng);
			if (dot(Ng, wo) < epslon)
			{
				prd.done = true;
				return;
			}

			Random& rnd = *prd.rnd;

			DisneyMaterial material = fs->material;
			//const auto & base_color = material.base_color;
			material.base_color = self.curveColor[primID];

			vec3f v_x, v_y;
			ortho_basis(v_x, v_y, Ng);
			float wi_pdf;
			vec3f sampled_wi;
			vec3f albedo = sample_disney_brdf(material, Ng, wo, v_x, v_y, rnd, sampled_wi, wi_pdf);

			if (wi_pdf < epslon || (albedo.x < epslon && albedo.y < epslon && albedo.z < epslon))
			{
				prd.done = true;
				return;
			}

			prd.attenuation *= (albedo * max(dot(sampled_wi, Ng), epslon) / wi_pdf);
			prd.wi_sampled = sampled_wi;

		}



		//                                                                      // 
		//************************ Quad Light **********************************//
		//                                                                      // 
		static inline __device__
			bool in_quad_light_shadow(OptixTraversableHandle world, const FrameState* fs,
				const vec3f& hit_point, vec3f& light_normal, vec3f &shadow_ray_direction, Random& rnd)
		{
			float r1 = rnd();
			float r2 = rnd();
			const vec3f light_origin = fs->quad_light.p;
			const vec3f light_u = fs->quad_light.v1 - light_origin;
			const vec3f light_v = fs->quad_light.v2 - light_origin;
			const vec3f light_p = light_origin + r1 * light_u * .1f + r2 * light_v * .1f;
			light_normal = normalize(cross(light_u, light_v)); //< important
			shadow_ray_direction = (light_p - hit_point);

			const auto shadow_ray_distance = length(shadow_ray_direction);

			Ray shadow_ray = owl::Ray(hit_point, normalize(shadow_ray_direction), 1e-3f, shadow_ray_distance - 1e-3f);
			PerRayData prd_shadow;
			prd_shadow.primID = -1;
			owl::traceRay(world, shadow_ray, prd_shadow, OPTIX_RAY_FLAG_DISABLE_CLOSESTHIT);
			if (prd_shadow.t < shadow_ray_distance - fs->epslon) // hit something, in shadow
			{
				return true;
			}
			return false;
		}

		OPTIX_INTERSECT_PROGRAM(QuadLight)()
		{
			owl::Ray ray(optixGetWorldRayOrigin(), optixGetWorldRayDirection(), optixGetRayTmin(), optixGetRayTmax());

			const auto& geo = owl::getProgramData<QuadLightGeom>();
			auto& prd = owl::getPRD<PerRayData>();

			auto anchor = geo.p;
			auto v1 = geo.v1;
			auto v2 = geo.v2;
			vec3f u = v1 - anchor;
			vec3f v = v2 - anchor;
			float u_length = length(u);
			float v_length = length(v);
			vec3f n = normalize(cross(u, v));
			float dt = dot(ray.direction, n);
			float t = (dot(n, anchor) - dot(n, ray.origin)) / dt;
			if (t > ray.tmin && t < ray.tmax)
			{
				vec3f p = ray.origin + ray.direction * t;
				vec3f vi = p - anchor;
				float a1 = dot(u, vi) / u_length;
				float a2 = dot(v, vi) / v_length;
				if (a1 >= 0 && a1 <= u_length && a2 >= 0 && a2 <= v_length)
				{
					if (optixReportIntersection(t, 0))
					{
						//prd.primID = 0;
						prd.meshID = 1;
						prd.t = t;
						prd.Ng = n;
					}
				}
			}
		}

		OPTIX_BOUNDS_PROGRAM(QuadLight)(const void  *geomData, box3f &primBounds, const int primID)
		{
			const QuadLightGeom &self = *(const QuadLightGeom*)geomData;
			auto anchor = self.p;
			auto v1 = self.v1;
			auto v2 = self.v2;
			const auto tv1 = v1 - anchor;
			const auto tv2 = v2 - anchor;
			const auto p00 = anchor;
			const auto p01 = anchor + tv1;
			const auto p10 = anchor + tv2;
			const auto p11 = anchor + tv1 + tv2;

			primBounds = box3f();
			primBounds.extend(p00);
			primBounds.extend(p01);
			primBounds.extend(p10);
			primBounds.extend(p11);
		}

		OPTIX_CLOSEST_HIT_PROGRAM(QuadLight)()
		{
			//const auto& light = owl::getProgramData<QuadLightGeom>();
			auto& prd = owl::getPRD<PerRayData>();
			const auto epslon = prd.fs->epslon;
			const auto max_decay_distance = prd.fs->max_decay_distance;
			const auto light_decay_index = prd.fs->light_decay_index;
			const auto light_decay_enable = prd.fs->enable_light_decay;
			const auto color = prd.fs->quad_light.color;
			const auto ray_distance = prd.t;
			auto intensity = prd.fs->quad_light.intensity;

			if (prd.depth > 0 && light_decay_enable)
			{
				float a = (max_decay_distance - ray_distance) / max_decay_distance;
				float b = a <= epslon ? epslon : pow(a, light_decay_index);
				intensity *= b;
			}
			prd.radiance += (color * intensity) * prd.attenuation;
			prd.done = true;
		}

		//                                                                      // 
		//************************ Point Light *********************************//
		//                                                                      // 
		static inline __device__
			bool in_point_light_shadow(OptixTraversableHandle world, const FrameState* fs,
				const vec3f& hit_point, vec3f &shadow_ray_direction, Random& rnd)
		{
			shadow_ray_direction = (fs->point_light_pos - hit_point);
			const float shadow_ray_distance = length(shadow_ray_direction);

			Ray shadow_ray = owl::Ray(hit_point, normalize(shadow_ray_direction), 1e-3f, shadow_ray_distance - 1e-3f);
			PerRayData prd_shadow;
			prd_shadow.primID = -1;
			owl::traceRay(world, shadow_ray, prd_shadow, OPTIX_RAY_FLAG_DISABLE_CLOSESTHIT);
			if (prd_shadow.t < shadow_ray_distance - fs->epslon) // hit something, in shadow
			{
				return true;
			}
			return false;
		}

		//                                                                      // 
		//************************ Ground **************************************//
		//                                                                      // 
		OPTIX_INTERSECT_PROGRAM(Ground)()
		{
			owl::Ray ray(optixGetWorldRayOrigin(), optixGetWorldRayDirection(), optixGetRayTmin(), optixGetRayTmax());

			const auto& self = owl::getProgramData<QuadGeom>();
			auto& prd = owl::getPRD<PerRayData>();

			auto anchor = self.p;
			auto v1 = self.v1;
			auto v2 = self.v2;
			vec3f u = v1 - anchor;
			vec3f v = v2 - anchor;
			float u_length = length(u);
			float v_length = length(v);
			vec3f n = normalize(cross(u, v));
			float dt = dot(ray.direction, n);
			float t = (dot(n, anchor) - dot(n, ray.origin)) / dt;
			if (t > ray.tmin && t < ray.tmax)
			{
				vec3f p = ray.origin + ray.direction * t;
				vec3f vi = p - anchor;
				float a1 = dot(u, vi) / u_length;
				float a2 = dot(v, vi) / v_length;
				if (a1 >= 0 && a1 <= u_length && a2 >= 0 && a2 <= v_length)
				{
					if (optixReportIntersection(t, 0))
					{
						//prd.primID = 0;
						prd.meshID = 1;
						prd.t = t;
						prd.Ng = n;
					}
				}
			}
		}

		OPTIX_BOUNDS_PROGRAM(Ground)(const void  *geomData, box3f &primBounds, const int primID)
		{
			const QuadGeom &self = *(const QuadGeom*)geomData;
			auto anchor = self.p;
			auto v1 = self.v1;
			auto v2 = self.v2;
			const auto tv1 = v1 - anchor;
			const auto tv2 = v2 - anchor;
			const auto p00 = anchor;
			const auto p01 = anchor + tv1;
			const auto p10 = anchor + tv2;
			const auto p11 = anchor + tv1 + tv2;

			primBounds = box3f();
			primBounds.extend(p00);
			primBounds.extend(p01);
			primBounds.extend(p10);
			primBounds.extend(p11);
		}

		OPTIX_CLOSEST_HIT_PROGRAM(Ground)()
		{
			owl::Ray ray(optixGetWorldRayOrigin(), optixGetWorldRayDirection(), optixGetRayTmin(), optixGetRayTmax());
			auto& prd = owl::getPRD<PerRayData>();

			Random& rnd = *prd.rnd;
			const vec3f albedo = prd.fs->ground_color;
			const vec3f hit_point = ray.origin + prd.t * ray.direction;
			prd.hit_point = hit_point;

			const vec3f N = prd.Ng;
			vec3f wi; //< sampled direction

			Onb onb(N);
			float r1 = rnd();
			float r2 = rnd();
			cosine_sample_hemisphere(r1, r2, wi);
			onb.inverse_transform(wi);

			prd.attenuation *= albedo;
			prd.wi_sampled = wi;

			vec3f shadow_ray_dir;
			vec3f light_normal;

			const auto epslon = prd.fs->epslon;
			const auto max_decay_distance = prd.fs->max_decay_distance;
			const auto light_decay_index = prd.fs->light_decay_index;
			const auto light_decay_enable = prd.fs->enable_light_decay;

			if (prd.depth == 0)
			{
				auto enable_quad_light = prd.fs->enable_quad_light;
				if (enable_quad_light && !in_quad_light_shadow(prd.world, prd.fs, hit_point, light_normal, shadow_ray_dir, rnd))
				{
					float NdotL_hit_point = dot(N, shadow_ray_dir);
					float NdotL_light = dot(light_normal, -shadow_ray_dir);
					if (NdotL_light > epslon && NdotL_hit_point > epslon)
					{
						auto intensity = prd.fs->point_light_intensity;
						const auto shadow_ray_distance = length(shadow_ray_dir);
						if (light_decay_enable)
						{
							float a = (max_decay_distance - shadow_ray_distance) / max_decay_distance;
							float b = a <= 0.f ? 0.f : pow(a, light_decay_index);
							intensity *= b;
						}
						prd.radiance += intensity * albedo;
					}
				}

				auto enable_point_light = prd.fs->enable_point_light;
				if (enable_point_light && !in_point_light_shadow(prd.world, prd.fs, hit_point, shadow_ray_dir, rnd))
				{
					float NdotL_hit_point = dot(N, shadow_ray_dir);
					if (NdotL_hit_point > epslon)
					{
						auto intensity = prd.fs->point_light_intensity;
						const auto shadow_ray_distance = length(shadow_ray_dir);
						if (light_decay_enable)
						{
							float a = (max_decay_distance - shadow_ray_distance) / max_decay_distance;
							float b = a <= 0.f ? 0.f : pow(a, light_decay_index);
							intensity *= b;
						}
						prd.radiance += intensity * albedo;
					}
				}
			}

		}

	}

}
