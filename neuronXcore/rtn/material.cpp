#include <fstream>
#include <sstream>
#include <owl/common/math/vec.h>
#include "device/common.h"

namespace rtn {

	DisneyMaterial load_material(const std::string &file)
	{
		std::ifstream fin(file.c_str());
		DisneyMaterial mat;
		std::string line;
		while (std::getline(fin, line)) {
			if (line.empty() || line[0] == '#') {
				continue;
			}

			std::stringstream ss(line);
			std::string param;
			float val = 0;
			ss >> param;

			if (param == "base_color") {
				ss >> mat.base_color.x >> mat.base_color.y >> mat.base_color.z;
			}
			else if (param == "metallic") {
				ss >> mat.metallic;
			}
			else if (param == "specular") {
				ss >> mat.specular;
			}
			else if (param == "roughness") {
				ss >> mat.roughness;
			}
			else if (param == "specular_tint") {
				ss >> mat.specular_tint;
			}
			else if (param == "anisotropy") {
				ss >> mat.anisotropy;
			}
			else if (param == "sheen") {
				ss >> mat.sheen;
			}
			else if (param == "sheen_tint") {
				ss >> mat.sheen_tint;
			}
			else if (param == "clearcoat") {
				ss >> mat.clearcoat;
			}
			else if (param == "clearcoat_gloss") {
				ss >> mat.clearcoat_gloss;
			}
			else if (param == "ior") {
				ss >> mat.ior;
			}
			else if (param == "specular_transmission") {
				ss >> mat.specular_transmission;
			}
		}
		return mat;
	}

}
