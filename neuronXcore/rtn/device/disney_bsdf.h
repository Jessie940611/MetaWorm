#pragma once

#ifndef M_PIF
#define M_PIF 3.14159265358979323846f
#endif

#ifndef M_1_PIF
#define M_1_PIF 0.318309886183790671538f
#endif

#include "common.h"

namespace rtn {
	namespace device {

		__device__ float pow2(float x) {
			return x * x;
		}

		__device__ float lerp(float x, float y, float s) {
			return x * (1.f - s) + y * s;
		}

		__device__ owl::vec3f lerp(const owl::vec3f x, const owl::vec3f &y, float s) {
			return x * (1.f - s) + y * s;
		}

		__device__ float luminance(const owl::vec3f &c) {
			return 0.2126f * c.x + 0.7152f * c.y + 0.0722f * c.z;
		}

		__device__ owl::vec3f reflect(const owl::vec3f &i, const owl::vec3f &n) {
			return i - 2.f * n * dot(i, n);
		}

		__device__ owl::vec3f refract_ray(const owl::vec3f &i, const owl::vec3f &n, float eta) {
			float n_dot_i = dot(n, i);
			float k = 1.f - eta * eta * (1.f - n_dot_i * n_dot_i);
			if (k < 0.f) {
				return owl::vec3f(0.f);
			}
			return eta * i - (eta * n_dot_i + sqrtf(k)) * n;
		}

		__device__ void ortho_basis(owl::vec3f &v_x, owl::vec3f &v_y, const owl::vec3f &n) {
			v_y = make_float3(0.f, 0.f, 0.f);

			if (n.x < 0.6f && n.x > -0.6f) {
				v_y.x = 1.f;
			}
			else if (n.y < 0.6f && n.y > -0.6f) {
				v_y.y = 1.f;
			}
			else if (n.z < 0.6f && n.z > -0.6f) {
				v_y.z = 1.f;
			}
			else {
				v_y.x = 1.f;
			}
			v_x = normalize(cross(v_y, n));
			v_y = normalize(cross(n, v_x));
		}

		/* Disney BSDF functions, for additional details and examples see:
		 * - https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
		 * - https://www.shadertoy.com/view/XdyyDd
		 * - https://github.com/wdas/brdf/blob/master/src/brdfs/disney.brdf
		 * - https://schuttejoe.github.io/post/disneybsdf/
		 *
		 * Variable naming conventions with the Burley course notes:
		 * V -> w_o
		 * L -> w_i
		 * H -> w_h
		 */

		__device__ bool same_hemisphere(const owl::vec3f &w_o, const owl::vec3f &w_i, const owl::vec3f &n) {
			return dot(w_o, n) * dot(w_i, n) > 0.f;
		}

		// Sample the hemisphere using a cosine weighted distribution,
		// returns a vector in a hemisphere oriented about (0, 0, 1)
		__device__ owl::vec3f cos_sample_hemisphere(owl::vec2f u) {
			owl::vec2f s = 2.f * u - owl::vec2f(1.f);
			owl::vec2f d;
			float radius = 0.f;
			float theta = 0.f;
			if (s.x == 0.f && s.y == 0.f) {
				d = s;
			}
			else {
				if (fabs(s.x) > fabs(s.y)) {
					radius = s.x;
					theta = M_PIF / 4.f * (s.y / s.x);
				}
				else {
					radius = s.y;
					theta = M_PIF / 2.f - M_PIF / 4.f * (s.x / s.y);
				}
			}
			d = radius * owl::vec2f(cos(theta), sin(theta));
			return owl::vec3f(d.x, d.y, sqrtf(max(0.f, 1.f - d.x * d.x - d.y * d.y)));
		}

		__device__ owl::vec3f spherical_dir(float sin_theta, float cos_theta, float phi) {
			return owl::vec3f(sin_theta * cos(phi), sin_theta * sin(phi), cos_theta);
		}

		__device__ float power_heuristic(float n_f, float pdf_f, float n_g, float pdf_g) {
			float f = n_f * pdf_f;
			float g = n_g * pdf_g;
			return (f * f) / (f * f + g * g);
		}

		__device__ float schlick_weight(float cos_theta) {
			return pow(saturate(1.f - cos_theta), 5.f);
		}

		// Complete Fresnel Dielectric computation, for transmission at ior near 1
		// they mention having issues with the Schlick approximation.
		// eta_i: material on incident side's ior
		// eta_t: material on transmitted side's ior
		__device__ float fresnel_dielectric(float cos_theta_i, float eta_i, float eta_t) {
			float g = pow2(eta_t) / pow2(eta_i) - 1.f + pow2(cos_theta_i);
			if (g < 0.f) {
				return 1.f;
			}
			return 0.5f * pow2(g - cos_theta_i) / pow2(g + cos_theta_i)
				* (1.f + pow2(cos_theta_i * (g + cos_theta_i) - 1.f) / pow2(cos_theta_i * (g - cos_theta_i) + 1.f));
		}

		// D_GTR1: Generalized Trowbridge-Reitz with gamma=1
		// Burley notes eq. 4
		__device__ float gtr_1(float cos_theta_h, float alpha) {
			if (alpha >= 1.f) {
				return M_1_PIF;
			}
			float alpha_sqr = alpha * alpha;
			return M_1_PIF * (alpha_sqr - 1.f) / (log(alpha_sqr) * (1.f + (alpha_sqr - 1.f) * cos_theta_h * cos_theta_h));
		}

		// D_GTR2: Generalized Trowbridge-Reitz with gamma=2
		// Burley notes eq. 8
		__device__ float gtr_2(float cos_theta_h, float alpha) {
			float alpha_sqr = alpha * alpha;
			return M_1_PIF * alpha_sqr / pow2(1.f + (alpha_sqr - 1.f) * cos_theta_h * cos_theta_h);
		}

		// D_GTR2 Anisotropic: Anisotropic generalized Trowbridge-Reitz with gamma=2
		// Burley notes eq. 13
		__device__ float gtr_2_aniso(float h_dot_n, float h_dot_x, float h_dot_y, owl::vec2f alpha) {
			return M_1_PIF / (alpha.x * alpha.y
				* pow2(pow2(h_dot_x / alpha.x) + pow2(h_dot_y / alpha.y) + h_dot_n * h_dot_n));
		}

		__device__ float smith_shadowing_ggx(float n_dot_o, float alpha_g) {
			float a = alpha_g * alpha_g;
			float b = n_dot_o * n_dot_o;
			return 1.f / (n_dot_o + sqrtf(a + b - a * b));
		}

		__device__ float smith_shadowing_ggx_aniso(float n_dot_o, float o_dot_x, float o_dot_y, owl::vec2f alpha) {
			return 1.f / (n_dot_o + sqrtf(pow2(o_dot_x * alpha.x) + pow2(o_dot_y * alpha.y) + pow2(n_dot_o)));
		}

		// Sample a reflection direction the hemisphere oriented along n and spanned by v_x, v_y using the random samples in s
		__device__ owl::vec3f sample_lambertian_dir(const owl::vec3f &n, const owl::vec3f &v_x, const owl::vec3f &v_y, const owl::vec2f &s) {
			const owl::vec3f hemi_dir = normalize(cos_sample_hemisphere(s));
			return hemi_dir.x * v_x + hemi_dir.y * v_y + hemi_dir.z * n;
		}

		// Sample the microfacet normal vectors for the various microfacet distributions
		__device__ owl::vec3f sample_gtr_1_h(const owl::vec3f &n, const owl::vec3f &v_x, const owl::vec3f &v_y, float alpha, const owl::vec2f &s) {
			float phi_h = 2.f * M_PIF * s.x;
			float alpha_sqr = alpha * alpha;
			float cos_theta_h_sqr = (1.f - pow(alpha_sqr, 1.f - s.y)) / (1.f - alpha_sqr);
			float cos_theta_h = sqrtf(cos_theta_h_sqr);
			float sin_theta_h = 1.f - cos_theta_h_sqr;
			owl::vec3f hemi_dir = normalize(spherical_dir(sin_theta_h, cos_theta_h, phi_h));
			return hemi_dir.x * v_x + hemi_dir.y * v_y + hemi_dir.z * n;
		}

		__device__ owl::vec3f sample_gtr_2_h(const owl::vec3f &n, const owl::vec3f &v_x, const owl::vec3f &v_y, float alpha, const owl::vec2f &s) {
			float phi_h = 2.f * M_PIF * s.x;
			float cos_theta_h_sqr = (1.f - s.y) / (1.f + (alpha * alpha - 1.f) * s.y);
			float cos_theta_h = sqrtf(cos_theta_h_sqr);
			float sin_theta_h = 1.f - cos_theta_h_sqr;
			owl::vec3f hemi_dir = normalize(spherical_dir(sin_theta_h, cos_theta_h, phi_h));
			return hemi_dir.x * v_x + hemi_dir.y * v_y + hemi_dir.z * n;
		}

		__device__ owl::vec3f sample_gtr_2_aniso_h(const owl::vec3f &n, const owl::vec3f &v_x, const owl::vec3f &v_y, const owl::vec2f &alpha, const owl::vec2f &s) {
			float x = 2.f * M_PIF * s.x;
			owl::vec3f w_h = sqrtf(s.y / (1.f - s.y)) * (alpha.x * cos(x) * v_x + alpha.y * sin(x) * v_y) + n;
			return normalize(w_h);
		}

		__device__ float lambertian_pdf(const owl::vec3f &w_i, const owl::vec3f &n) {
			float d = dot(w_i, n);
			if (d > 0.f) {
				return d * M_1_PIF;
			}
			return 0.f;
		}

		__device__ float gtr_1_pdf(const owl::vec3f &w_o, const owl::vec3f &w_i, const owl::vec3f &n, float alpha) {
			if (!same_hemisphere(w_o, w_i, n)) {
				return 0.f;
			}
			owl::vec3f w_h = normalize(w_i + w_o);
			float cos_theta_h = dot(n, w_h);
			float d = gtr_1(cos_theta_h, alpha);
			return d * cos_theta_h / (4.f * dot(w_o, w_h));
		}

		__device__ float gtr_2_pdf(const owl::vec3f &w_o, const owl::vec3f &w_i, const owl::vec3f &n, float alpha) {
			if (!same_hemisphere(w_o, w_i, n)) {
				return 0.f;
			}
			owl::vec3f w_h = normalize(w_i + w_o);
			float cos_theta_h = dot(n, w_h);
			float d = gtr_2(cos_theta_h, alpha);
			return d * cos_theta_h / (4.f * dot(w_o, w_h));
		}

		__device__ float gtr_2_transmission_pdf(const owl::vec3f &w_o, const owl::vec3f &w_i, const owl::vec3f &n,
			float alpha, float ior)
		{
			if (same_hemisphere(w_o, w_i, n)) {
				return 0.f;
			}
			bool entering = dot(w_o, n) > 0.f;
			float eta_o = entering ? 1.f : ior;
			float eta_i = entering ? ior : 1.f;
			owl::vec3f w_h = normalize(w_o + w_i * eta_i / eta_o);
			float cos_theta_h = fabs(dot(n, w_h));
			float i_dot_h = dot(w_i, w_h);
			float o_dot_h = dot(w_o, w_h);
			float d = gtr_2(cos_theta_h, alpha);
			float dwh_dwi = o_dot_h * pow2(eta_o) / pow2(eta_o * o_dot_h + eta_i * i_dot_h);
			return d * cos_theta_h * fabs(dwh_dwi);
		}

		__device__ float gtr_2_aniso_pdf(const owl::vec3f &w_o, const owl::vec3f &w_i, const owl::vec3f &n,
			const owl::vec3f &v_x, const owl::vec3f &v_y, const owl::vec2f alpha)
		{
			if (!same_hemisphere(w_o, w_i, n)) {
				return 0.f;
			}
			owl::vec3f w_h = normalize(w_i + w_o);
			float cos_theta_h = dot(n, w_h);
			float d = gtr_2_aniso(cos_theta_h, fabs(dot(w_h, v_x)), fabs(dot(w_h, v_y)), alpha);
			return d * cos_theta_h / (4.f * dot(w_o, w_h));
		}

		__device__ owl::vec3f disney_diffuse(const DisneyMaterial &mat, const owl::vec3f &n,
			const owl::vec3f &w_o, const owl::vec3f &w_i)
		{
#if FAST_SHADING
			return mat.base_color * M_1_PIF;
#else
			owl::vec3f w_h = normalize(w_i + w_o);
			float n_dot_o = fabs(dot(w_o, n));
			float n_dot_i = fabs(dot(w_i, n));
			float i_dot_h = dot(w_i, w_h);
			float fd90 = 0.5f + 2.f * mat.roughness * i_dot_h * i_dot_h;
			float fi = schlick_weight(n_dot_i);
			float fo = schlick_weight(n_dot_o);
			return mat.base_color * M_1_PIF * lerp(1.f, fd90, fi) * lerp(1.f, fd90, fo);
#endif
		}

		__device__ owl::vec3f disney_microfacet_isotropic(const DisneyMaterial &mat, const owl::vec3f &n,
			const owl::vec3f &w_o, const owl::vec3f &w_i)
		{
			owl::vec3f w_h = normalize(w_i + w_o);
			float lum = luminance(mat.base_color);
			owl::vec3f tint = lum > 0.f ? mat.base_color / lum : owl::vec3f(1.f);
			owl::vec3f spec = lerp(mat.specular * 0.08f * lerp(owl::vec3f(1.f), tint, mat.specular_tint), mat.base_color, mat.metallic);

			float alpha = max(0.001f, mat.roughness * mat.roughness);
			float d = gtr_2(dot(n, w_h), alpha);
			owl::vec3f f = lerp(spec, owl::vec3f(1.f), schlick_weight(dot(w_i, w_h)));
			float g = smith_shadowing_ggx(dot(n, w_i), alpha) * smith_shadowing_ggx(dot(n, w_o), alpha);
			return d * f * g;
		}

		__device__ owl::vec3f disney_microfacet_transmission_isotropic(const DisneyMaterial &mat, const owl::vec3f &n,
			const owl::vec3f &w_o, const owl::vec3f &w_i)
		{
			float o_dot_n = dot(w_o, n);
			float i_dot_n = dot(w_i, n);
			if (o_dot_n == 0.f || i_dot_n == 0.f) {
				return owl::vec3f(0.f);
			}
			bool entering = o_dot_n > 0.f;
			float eta_o = entering ? 1.f : mat.ior;
			float eta_i = entering ? mat.ior : 1.f;
			owl::vec3f w_h = normalize(w_o + w_i * eta_i / eta_o);

			float alpha = max(0.001f, mat.roughness * mat.roughness);
			float d = gtr_2(fabs(dot(n, w_h)), alpha);

			float f = fresnel_dielectric(fabs(dot(w_i, n)), eta_o, eta_i);
			float g = smith_shadowing_ggx(fabs(dot(n, w_i)), alpha) * smith_shadowing_ggx(fabs(dot(n, w_o)), alpha);

			float i_dot_h = dot(w_i, w_h);
			float o_dot_h = dot(w_o, w_h);

			float c = fabs(o_dot_h) / fabs(dot(w_o, n)) * fabs(i_dot_h) / fabs(dot(w_i, n))
				* pow2(eta_o) / pow2(eta_o * o_dot_h + eta_i * i_dot_h);

			return mat.base_color * c * (1.f - f) * g * d;
		}

		__device__ owl::vec3f disney_microfacet_anisotropic(const DisneyMaterial &mat, const owl::vec3f &n,
			const owl::vec3f &w_o, const owl::vec3f &w_i, const owl::vec3f &v_x, const owl::vec3f &v_y)
		{
			owl::vec3f w_h = normalize(w_i + w_o);
			float lum = luminance(mat.base_color);
			owl::vec3f tint = lum > 0.f ? mat.base_color / lum : owl::vec3f(1.f);
			owl::vec3f spec = lerp(mat.specular * 0.08f * lerp(owl::vec3f(1.f), tint, mat.specular_tint), mat.base_color, mat.metallic);

			float aspect = sqrtf(1.f - mat.anisotropy * 0.9f);
			float a = mat.roughness * mat.roughness;
			owl::vec2f alpha = owl::vec2f(max(0.001f, a / aspect), max(0.001f, a * aspect));
			float d = gtr_2_aniso(dot(n, w_h), fabs(dot(w_h, v_x)), fabs(dot(w_h, v_y)), alpha);
			owl::vec3f f = lerp(spec, owl::vec3f(1.f), schlick_weight(dot(w_i, w_h)));
			float g = smith_shadowing_ggx_aniso(dot(n, w_i), fabs(dot(w_i, v_x)), fabs(dot(w_i, v_y)), alpha)
				* smith_shadowing_ggx_aniso(dot(n, w_o), fabs(dot(w_o, v_x)), fabs(dot(w_o, v_y)), alpha);
			return d * f * g;
		}

		__device__ float disney_clear_coat(const DisneyMaterial &mat, const owl::vec3f &n,
			const owl::vec3f &w_o, const owl::vec3f &w_i)
		{
			owl::vec3f w_h = normalize(w_i + w_o);
			float alpha = lerp(0.1f, 0.001f, mat.clearcoat_gloss);
			float d = gtr_1(dot(n, w_h), alpha);
			float f = lerp(0.04f, 1.f, schlick_weight(dot(w_i, n)));
			float g = smith_shadowing_ggx(dot(n, w_i), 0.25f) * smith_shadowing_ggx(dot(n, w_o), 0.25f);
			return 0.25f * mat.clearcoat * d * f * g;
		}

		__device__ owl::vec3f disney_sheen(const DisneyMaterial &mat, const owl::vec3f &n,
			const owl::vec3f &w_o, const owl::vec3f &w_i)
		{
			owl::vec3f w_h = normalize(w_i + w_o);
			float lum = luminance(mat.base_color);
			owl::vec3f tint = lum > 0.f ? mat.base_color / lum : owl::vec3f(1.f);
			owl::vec3f sheen_color = lerp(owl::vec3f(1.f), tint, mat.sheen_tint);
			float f = schlick_weight(dot(w_i, n));
			return f * mat.sheen * sheen_color;
		}

		__device__ owl::vec3f disney_brdf(const DisneyMaterial &mat, const owl::vec3f &n,
			const owl::vec3f &w_o, const owl::vec3f &w_i, const owl::vec3f &v_x, const owl::vec3f &v_y)
		{
#if FAST_SHADING
			return disney_diffuse(mat, n, w_o, w_i);
#endif
			if (!same_hemisphere(w_o, w_i, n)) {
				if (mat.specular_transmission > 0.f) {
					owl::vec3f spec_trans = disney_microfacet_transmission_isotropic(mat, n, w_o, w_i);
					return spec_trans * (1.f - mat.metallic) * mat.specular_transmission;
				}
				return owl::vec3f(0.f);
			}

			float coat = disney_clear_coat(mat, n, w_o, w_i);
			owl::vec3f sheen = disney_sheen(mat, n, w_o, w_i);
			owl::vec3f diffuse = disney_diffuse(mat, n, w_o, w_i);
			owl::vec3f gloss;
			if (mat.anisotropy == 0.f) {
				gloss = disney_microfacet_isotropic(mat, n, w_o, w_i);
			}
			else {
				gloss = disney_microfacet_anisotropic(mat, n, w_o, w_i, v_x, v_y);
			}
			return (diffuse + sheen) * (1.f - mat.metallic) * (1.f - mat.specular_transmission) + gloss + coat;
		}

		__device__ float disney_pdf(const DisneyMaterial &mat, const owl::vec3f &n,
			const owl::vec3f &w_o, const owl::vec3f &w_i, const owl::vec3f &v_x, const owl::vec3f &v_y)
		{
#if FAST_SHADING
			return lambertian_pdf(w_i, n);
#else
			float alpha = max(0.001f, mat.roughness * mat.roughness);
			float aspect = sqrtf(1.f - mat.anisotropy * 0.9f);
			owl::vec2f alpha_aniso = owl::vec2f(max(0.001f, alpha / aspect), max(0.001f, alpha * aspect));

			float clearcoat_alpha = lerp(0.1f, 0.001f, mat.clearcoat_gloss);

			float diffuse = lambertian_pdf(w_i, n);
			float clear_coat = gtr_1_pdf(w_o, w_i, n, clearcoat_alpha);

			float n_comp = 3.f;
			float microfacet = 0.f;
			float microfacet_transmission = 0.f;
			if (mat.anisotropy == 0.f) {
				microfacet = gtr_2_pdf(w_o, w_i, n, alpha);
			}
			else {
				microfacet = gtr_2_aniso_pdf(w_o, w_i, n, v_x, v_y, alpha_aniso);
			}
			if (mat.specular_transmission > 0.f) {
				n_comp = 4.f;
				microfacet_transmission = gtr_2_transmission_pdf(w_o, w_i, n, alpha, mat.ior);
			}
			return (diffuse + microfacet + microfacet_transmission + clear_coat) / n_comp;
#endif
		}

		/* Sample a component of the Disney BRDF, returns the sampled BRDF color,
		 * ray reflection direction (w_i) and sample PDF.
		 */
		__device__ owl::vec3f sample_disney_brdf(const DisneyMaterial &mat, const owl::vec3f &n,
			const owl::vec3f &w_o, const owl::vec3f &v_x, const owl::vec3f &v_y, Random &rng,
			owl::vec3f &w_i, float &pdf)
		{
#if FAST_SHADING
			owl::vec2f samples = owl::vec2f(rng(), rng());
			w_i = sample_lambertian_dir(n, v_x, v_y, samples);
			pdf = lambertian_pdf(w_i, n);
			return disney_diffuse(mat, n, w_o, w_i);
#else
			int component = 0;
			if (mat.specular_transmission == 0.f) {
				component = rng() * 3.f;
				component = clamp(component, 0, 2);
			}
			else {
				component = rng() * 4.f;
				component = clamp(component, 0, 3);
			}

			owl::vec2f samples = owl::vec2f(rng(), rng());
			if (component == 0) {
				// Sample diffuse component
				w_i = sample_lambertian_dir(n, v_x, v_y, samples);
			}
			else if (component == 1) {
				owl::vec3f w_h;
				float alpha = max(0.001f, mat.roughness * mat.roughness);
				if (mat.anisotropy == 0.f) {
					w_h = sample_gtr_2_h(n, v_x, v_y, alpha, samples);
				}
				else {
					float aspect = sqrtf(1.f - mat.anisotropy * 0.9f);
					owl::vec2f alpha_aniso = owl::vec2f(max(0.001f, alpha / aspect), max(0.001f, alpha * aspect));
					w_h = sample_gtr_2_aniso_h(n, v_x, v_y, alpha_aniso, samples);
				}
				w_i = reflect(-w_o, w_h);

				// Invalid reflection, terminate ray
				if (!same_hemisphere(w_o, w_i, n)) {
					pdf = 0.f;
					w_i = owl::vec3f(0.f);
					return owl::vec3f(0.f);
				}
			}
			else if (component == 2) {
				// Sample clear coat component
				float alpha = lerp(0.1f, 0.001f, mat.clearcoat_gloss);
				owl::vec3f w_h = sample_gtr_1_h(n, v_x, v_y, alpha, samples);
				w_i = reflect(-w_o, w_h);

				// Invalid reflection, terminate ray
				if (!same_hemisphere(w_o, w_i, n)) {
					pdf = 0.f;
					w_i = owl::vec3f(0.f);
					return owl::vec3f(0.f);
				}
			}
			else {
				// Sample microfacet transmission component
				float alpha = max(0.001f, mat.roughness * mat.roughness);
				owl::vec3f w_h = sample_gtr_2_h(n, v_x, v_y, alpha, samples);
				if (dot(w_o, w_h) < 0.f) {
					w_h = -w_h;
				}
				bool entering = dot(w_o, n) > 0.f;
				w_i = refract_ray(-w_o, w_h, entering ? 1.f / mat.ior : mat.ior);

				// Invalid refraction, terminate ray
				if (w_i == owl::vec3f(0.f)) {
					pdf = 0.f;
					return owl::vec3f(0.f);
				}
			}
			pdf = disney_pdf(mat, n, w_o, w_i, v_x, v_y);
			return disney_brdf(mat, n, w_o, w_i, v_x, v_y);
#endif
		}

	}
}
