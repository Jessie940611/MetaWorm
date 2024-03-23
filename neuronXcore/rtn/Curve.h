#pragma once

#include <vector>
#include "owl/common/math/AffineSpace.h"
#include "string.h"

namespace rtn {

	using namespace owl;
	using namespace owl::common;

	enum CurveType
	{
		Hermite //default
	};

	struct Curves
	{
		typedef std::shared_ptr<Curves> SP;

		//cubic curve coefficients
		std::vector<vec3f> coefficients;
		//curve radius
		std::vector<vec4f> radius;
		//curve color
		std::vector<vec3f> curveColor;
		//curve length
		std::vector<float> length;

		box3f bounds;

		//load all curve splines to memory
		static Curves::SP loadCurves(std::vector<std::string> fileNames) noexcept;

	};

	/*class CurveSpline
	{
	public:
		CurveSpline(const vec3f co[], const vec4f* r, const vec3f* col)
		{
			memcpy(coefficient, co, sizeof(vec3f) * 4);
			memcpy(radius, r, sizeof(vec4f));
			memcpy(curveColor, col, sizeof(vec3f));
		};
		~CurveSpline() {};
	private:
		const CurveType type = Hermite;
		vec3f coefficient[4];
		vec4f *radius;
		vec3f *curveColor;
		float *length;
	};*/

}