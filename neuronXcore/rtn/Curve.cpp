#include <iostream>
#include <fstream>
#include "Curve.h"
#include "rtn/profiler.h"

namespace rtn {
	float calcCubicCurveLength(const vec3f& coeA, const vec3f& coeB, const vec3f& coeC, const vec3f coeD)
	{
		float length = 0.0f;
		for (int i = 1; i <= 100; i++)
		{
			float t0 = (i - 1) * 0.01f;
			float t1 = i * 0.01f;
			vec3f startP = coeD + t0 * (coeC + t0 * (coeB + t0 * coeA));
			vec3f endP = coeD + t1 * (coeC + t1 * (coeB + t1 * coeA));
			vec3f line = startP - endP;
			length += sqrtf(dot(line, line));
		}
		return length;
	}

	Curves::SP Curves::loadCurves(std::vector<std::string> fileNames) noexcept
	{
		Curves::SP curves = Curves::SP(new Curves);

		for (int i = 0; i < fileNames.size(); i++)
		{
			std::string fileName = fileNames[i];

			utility::CPUProfiler profiler();
			std::cout << OWL_TERMINAL_BLUE
				<< "#try to load curves:\t"
				<< fileName.c_str()
				<< OWL_TERMINAL_DEFAULT
				<< std::endl;

			std::ifstream fin(fileName);
			std::string line;
			vec3f coefficients[4] = { 0 };
			vec4f radius = { 0 };
			vec3f color = { 0 };
			float length = 0.0f;

			while (std::getline(fin, line))
			{
				if (line.find_first_of("#") != std::string::npos) continue;
				std::istringstream iss(line);
				iss >> coefficients[0].x >> coefficients[0].y >> coefficients[0].z
					>> coefficients[1].x >> coefficients[1].y >> coefficients[1].z
					>> coefficients[2].x >> coefficients[2].y >> coefficients[2].z
					>> coefficients[3].x >> coefficients[3].y >> coefficients[3].z
					>> radius.x >> radius.y >> radius.z >> radius.w
					>> color.x >> color.y >> color.z;

				for (size_t i = 0; i < 4; i++)
				{
					curves->coefficients.push_back(coefficients[i]);
				}

				curves->radius.push_back(radius);
				//color
				curves->curveColor.push_back(color);
				//length
				length = calcCubicCurveLength(coefficients[0], coefficients[1], coefficients[2], coefficients[3]);
				curves->length.push_back(length);

				curves->bounds.extend(vec3f(coefficients[3].x, coefficients[3].y, coefficients[3].z));

			}
		}		

		return curves;
	}

}