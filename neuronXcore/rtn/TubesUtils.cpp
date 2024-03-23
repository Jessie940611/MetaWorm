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

#include "TubesUtils.h"
#include "profiler.h"
#include "material.h"

#include <cstddef>
#include <fstream>
#include <sstream>

/*
	The three dimensional structure of a neuron can be represented in a SWC format (Cannon et al., 1998). SWC is a simple Standardized format.
	Each line has 7 fields encoding data for a single neuronal compartment:
	an integer number as compartment identifier
	type of neuronal compartment
	0 - undefined
	1 - soma
	2 - axon
	3 - basal dendrite
	4 - apical dendrite
	5 - custom (user-defined preferences)
	6 - unspecified neurites
	7 - glia processes
	x coordinate of the compartment
	y coordinate of the compartment
	z coordinate of the compartment
	radius of the compartment
	parent compartment

	Every compartment has only one parent and the parent compartment for the first point in each file is always -1 (if the file does not include
	the soma information then the originating point of the tree will be connected to a parent of -1). The index for parent compartments are
	always less than child compartments. Loops and unconnected branches are excluded. All trees should originate from the soma and have parent
	type 1 if the file includes soma information. Soma can be a single point or more than one point. When the soma is encoded as one line in
	the SWC, it is interpreted as a "sphere". When it is encoded by more than 1 line, it could be a set of tapering cylinders (as in some
	pyramidal cells) or even a 2D projected contour ("circumference").
*/

namespace rtn {
	using namespace owl;
	using namespace owl::common;

#if _WIN32
	inline double drand48() { return double(rand()) / RAND_MAX; }
#endif

	std::string getExt(const std::string &fileName)
	{
		int pos = fileName.rfind(".");
		if (pos == fileName.npos)
			return "";
		return fileName.substr(pos);
	}

	std::string getName(const std::string &fileName)
	{
		#ifdef WIN32
		int startPos = fileName.rfind("\\");
		#endif
		#ifdef __linux__
		int startPos = fileName.rfind("/");
		#endif
		int endPos = fileName.rfind(".");
		if (endPos == fileName.npos)
			return fileName.substr(startPos + 1);
		return fileName.substr(startPos + 1, endPos - startPos - 1);
	}

	bool Tubes::loadGeom(std::string geomFile)
	{
		if (getExt(geomFile) == ".swc")
		{
			std::ifstream geomIn(geomFile);
			std::string line;

			unsigned int primitive;
			vec3f vertex;
			float radius;
			int prevPrim;
			unsigned int type;

			while (std::getline(geomIn, line))
			{
				if (line.find_first_of("#") != std::string::npos) continue;

				std::stringstream iss(line);
				iss >> primitive >> type >> vertex.x >> vertex.y >> vertex.z >> radius >> prevPrim;

				this->vertices.push_back(vertex);
				this->endPointRadii.push_back(radius);				
				this->types.push_back(type);
				this->bounds.extend(vertex);

				if (prevPrim == -1)
				{
					this->prevPrims.push_back(0);
				}
				else
				{
					this->prevPrims.push_back((unsigned int) prevPrim);
				}
			}
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Tubes::loadVoltage(std::string voltageFile)
	{
		if (getExt(voltageFile) == ".vtg")
		{
			FILE* fp = nullptr;
			fp = fopen(voltageFile.c_str(), "rb");
			if (fp == NULL)
			{
				return false;
			}
			
			int startTime = 0;
			int endTime;
			// fread(&startTime, sizeof(int), 1, fp);
			fread(&endTime, sizeof(int), 1, fp);

			if (this->maxTime < endTime) this->maxTime = endTime;

			int timeSpan = endTime - startTime;

			bool uniformTimeSpan = (timeSpan != 0);

			float* voltageVals = new float[timeSpan];

			for (int i = 0; i < this->vertices.size(); i++)
			{
				std::vector<float> voltagePerPrim;

				if (!uniformTimeSpan)
				{					
					fread(&startTime, sizeof(int), 1, fp);
					fread(&endTime, sizeof(int), 1, fp);

					if (this->maxTime < endTime) this->maxTime = endTime;

					timeSpan = endTime - startTime;
				}

				this->voltageStart.push_back(startTime);
				this->voltageEnd.push_back(endTime);

				if (!feof(fp))
				{
					fread(voltageVals, sizeof(float), timeSpan, fp); // read more. 
				}
				else
				{
					std::cout << " The end of the file. \n"
						<< "Total read : "
						<< "i : " << i << " " << std::endl;
				}

				for (int j = 0; j < timeSpan; j++)
				{
					voltagePerPrim.push_back(voltageVals[j]);
					if (this->maxVoltage < voltageVals[j]) this->maxVoltage = voltageVals[j];
					if (this->minVoltage > voltageVals[j]) this->minVoltage = voltageVals[j];
				}

				this->voltages.push_back(voltagePerPrim);
				this->haveVoltage.push_back(true);
			}

			delete[] voltageVals;

			this->haveVoltages = true;

			return true;
		}
		else
		{
			for (int i = 0; i < this->vertices.size(); i++)
			{
				std::vector<float> voltagePerPrim(1, 0.0f);
				this->voltages.push_back(voltagePerPrim);
				this->haveVoltage.push_back(false);
				this->voltageStart.push_back(0);
				this->voltageEnd.push_back(1);
			}
			return false;
		}
	}

	float Tubes::weightToScale(float weight)
	{
		float t = (weight - 0.1f) / 9.9f;
		return minScale + t * (maxScale - minScale);
	}

	bool Tubes::loadSynapse(std::string synapseFile)
	{	
		if (getExt(synapseFile) == ".snp")
		{
			std::ifstream synapseIn(synapseFile);
			std::string line;
			unsigned int prePrim;
			unsigned int postPrim;
			unsigned int type;
			float weight;

			while (std::getline(synapseIn, line))
			{
				std::stringstream iss(line);
				iss >> prePrim >> postPrim >> type >> weight;

				/*this->synapse.prePrim.push_back(prePrim);
				this->synapse.postPrim.push_back(postPrim);
				this->synapse.type.push_back(type + 8);
				this->synapse.weight.push_back(weight);*/

				float scale = weightToScale(weight);
				float preRadius = scale * 0.6f;
				float postRadius = scale * 1.5f;

				vec3f preSynapse = 0.5f * this->vertices[prePrim - 1] + 0.5f * this->vertices[postPrim - 1];
				vec3f postSynapse = 0.5f * this->vertices[prePrim - 1] + 0.5f * this->vertices[postPrim - 1];				

				this->vertices.push_back(preSynapse);
				this->vertices.push_back(postSynapse);
				this->endPointRadii.push_back(preRadius);
				this->endPointRadii.push_back(postRadius);
				this->prevPrims.push_back(prePrim);
				this->prevPrims.push_back(postPrim);
				this->types.push_back(type + 5);
				this->types.push_back(type + 5);

				std::vector<float> preVoltages, postVoltages;

				for (int i = 0; i < this->voltages[prePrim - 1].size(); i++)
				{
					preVoltages.push_back(this->voltages[prePrim - 1][i]);
				}
				for (int i = 0; i < this->voltages[postPrim - 1].size(); i++)
				{
					postVoltages.push_back(this->voltages[postPrim - 1][i]);
				}

				this->voltages.push_back(preVoltages);
				this->voltages.push_back(postVoltages);

				int preStart = this->voltageStart[prePrim - 1];
				int postStart = this->voltageStart[postPrim - 1];
				int preEnd = this->voltageEnd[prePrim - 1];
				int postEnd = this->voltageEnd[postPrim - 1];
				
				this->voltageStart.push_back(preStart);
				this->voltageStart.push_back(postStart);
				this->voltageEnd.push_back(preEnd);
				this->voltageEnd.push_back(postEnd);
			}

			return true;
		}
		else
		{
			return false;
		}
	}

	/* Haixin Ma: currently no error check for materials */

	/*bool Tubes::loadMaterial(std::string materialFile)
	{
		
		DisneyMaterial material = load_material(materialFile);
		this->materials.push_back(material);
		return true;
	}*/

	void Tubes::mergeTubes(Tubes::SP tube)
	{
		unsigned int tubeSize = this->vertices.size();

		this->vertices.insert(this->vertices.end(), tube->vertices.begin(), tube->vertices.end());
		this->endPointRadii.insert(this->endPointRadii.end(), tube->endPointRadii.begin(), tube->endPointRadii.end());
		this->voltages.insert(this->voltages.end(), tube->voltages.begin(), tube->voltages.end());
		this->haveVoltage.insert(this->haveVoltage.end(), tube->haveVoltage.begin(), tube->haveVoltage.end());
		this->types.insert(this->types.end(), tube->types.begin(), tube->types.end());
		this->voltageStart.insert(this->voltageStart.end(), tube->voltageStart.begin(), tube->voltageStart.end());
		this->voltageEnd.insert(this->voltageEnd.end(), tube->voltageEnd.begin(), tube->voltageEnd.end());
		/*this->materials.insert(this->materials.end(), tube->materials.begin(), tube->materials.end());
		this->synapse.type.insert(this->synapse.type.end(), tube->synapse.type.begin(), tube->synapse.type.end());
		this->synapse.weight.insert(this->synapse.weight.end(), tube->synapse.weight.begin(), tube->synapse.weight.end());*/

		unsigned int prevPrim, postPrim;

		for (int i = 0; i < tube->prevPrims.size(); i++)
		{
			prevPrim = tube->prevPrims[i];
			if (prevPrim != 0) prevPrim += tubeSize;

			this->prevPrims.push_back(prevPrim);

			
		}

		/*for (int i = 0; i < tube->synapse.prePrim.size(); i++)
		{
			prevPrim = tube->synapse.prePrim[i] + tubeSize;
			postPrim = tube->synapse.postPrim[i] + tubeSize;

			this->synapse.prePrim.push_back(prevPrim);
			this->synapse.postPrim.push_back(postPrim);
		}*/

		if (tube->haveVoltage[0])
		{
			if (this->maxVoltage < tube->maxVoltage) this->maxVoltage = tube->maxVoltage;
			if (this->minVoltage > tube->minVoltage) this->minVoltage = tube->minVoltage;
			if (this->maxTime < tube->maxTime) this->maxTime = tube->maxTime;
			this->haveVoltages = true;
		}	

		this->bounds.extend(tube->bounds);
	}

	Tubes::SP Tubes::load(std::vector<std::string> &geomFiles,
		std::vector<std::string> &voltageFiles,
		std::vector<std::string> &synapseFiles,
		std::vector<std::string> &materialFiles)
	{
		Tubes::SP result = nullptr;

		for (int i = 0; i < geomFiles.size(); i++)
		{
			std::string geomFile = geomFiles[i];

			Tubes::SP tube = Tubes::SP(new Tubes);

			if (tube->loadGeom(geomFile))
			{
				std::cout << "finished reading geometry file: " << geomFile << std::endl;
			}
			else
			{
				std::cout << "WARNING: unable to read geometry file: " << geomFile << " invalid data." << std::endl;
				std::cout << "WARNING: searching for another file." << std::endl;
				continue;
			}

			bool hasVoltage = false;
			bool hasSynapse = false;
			bool hasMaterial = false;

			std::string geomName = getName(geomFile);

			for (int j = 0; j < voltageFiles.size(); j++)
			{
				std::string voltageFile = voltageFiles[j];
				std::string voltageName = getName(voltageFile);
				
				if (geomName == voltageName)
				{
					if (tube->loadVoltage(voltageFile))
					{
						std::cout << "finished reading voltage file: " << voltageFile << std::endl;
					}
					else
					{
						std::cout << "WARNING: unable to read voltage file: " << voltageFile << " invalid data." << std::endl;
						std::cout << "WARNING: emissive light for this part will be invalid." << std::endl;
					}
					hasVoltage = true;					
					break;
				}
			}

			if (!hasVoltage)
			{
				std::cout << "WARNING: voltage file related to " << geomName << " not found." << std::endl;
				std::cout << "WARNING: emissive light for this part will be invalid." << std::endl;
				for (int i = 0; i < tube->vertices.size(); i++)
				{
					std::vector<float> voltagePerPrim(1, 0.0f);
					tube->voltages.push_back(voltagePerPrim);
					tube->haveVoltage.push_back(false);
					tube->voltageStart.push_back(0);
					tube->voltageEnd.push_back(1);
				}
			}

			for (int j = 0; j < synapseFiles.size(); j++)
			{
				std::string synapseFile = synapseFiles[j];
				std::string synapseName = getName(synapseFile);
				if (geomName == synapseName)
				{
					if (tube->loadSynapse(synapseFile))
					{
						std::cout << "finished reading synapse file: " << synapseFile << std::endl;

					}
					else
					{
						std::cout << "WARNING: unable to read synapse file: " << synapseFile << " invalid extension." << std::endl;
						std::cout << "WARNING: synapse for this part will be empty." << std::endl;
					}
					hasSynapse = true;
					break;
				}
			}

			if (!hasSynapse)
			{
				std::cout << "WARNING: synapse file related to " << geomName << " not found." << std::endl;
				std::cout << "WARNING: synapse for this part will be empty." << std::endl;
			}

			/* Haixin Ma: currently using uniform materials for neuron and synapse */

			/*for (int j = 0; j < materialFiles.size(); j++)
			{
				std::string materialFile = materialFiles[j];
				std::string materialName = getName(materialFile);
				if (geomName == materialName)
				{
					if (tube->loadMaterial(materialFile))
					{
						std::cout << "finished reading material file: " << materialFile << std::endl;

					}
					else
					{
						std::cout << "WARNING: unable to read material file: " << materialFile << " invalid data." << std::endl;
						std::cout << "WARNING: using default material for " << geomFile << std::endl;
					}
					hasMaterial = true;
					break;
				}
			}

			if (!hasMaterial)
			{
				std::cout << "WARNING: material file related to " << geomName << " not found." << std::endl;
				std::cout << "WARNING: using default material for " << geomFile << std::endl;
			}*/

			if (!result)
			{
				result = tube;
			}
			else
			{
				result->mergeTubes(tube);
			}
		}

		if (!result)
		{
			std::cout << "WARNING: failed to read tubes." << std::endl;
		}

		return result;
	}

	

#if 0 /* Haixin Ma: old codes here */

	bool Tubes::loadColors(std::vector<std::vector<float>>& primColorsVec, const std::string rec_voltagesFile)
	{
		FILE* fp = nullptr;
		fp = fopen(rec_voltagesFile.c_str(), "rb");
		if (fp == NULL)
		{
			std::cout << "Open file Failed at: "
				<< rec_voltagesFile
				<< std::endl;
			return false;
		}

		int rec_len, nrec, nlast, len;
		int nparam; // To min xuan: try to debug this. @Lei

		try
		{
			fread(&nparam, sizeof(int), 1, fp);
			fread(&nrec, sizeof(int), 1, fp);
			fread(&rec_len, sizeof(int), 1, fp);
			fread(&nlast, sizeof(int), 1, fp);


			len = rec_len * nrec + nlast;
			float* rec_vals = new float[nparam + 1];  // add 1 , just in case.
			fread(rec_vals, sizeof(float), nparam, fp);

			//primColorsVec.reserve(len);
			primColorsVec.resize(0);
			for (int i = 0; i < len; i++)
			{
				std::vector<float> aa;
				aa.reserve(nparam);
				aa.resize(0);
				for (int j = 0; j < nparam; j++)
				{
					float val = rec_vals[j];
					if (val > vol_fmax)
						vol_fmax = val;
					if (val < vol_fmin)
						vol_fmin = val;
					aa.emplace_back(val);
				}
				primColorsVec.emplace_back(aa);

				//fseek(fp, nparam, 1); //relocated the file pointer. 
				if (!feof(fp)) //
				{
					fread(rec_vals, sizeof(float), nparam, fp); // read more. 
				}
				else
				{
					std::cout << " The end of the file. \n"
						<< "Total read : "
						<< "i : " << i << " " << std::endl;
				}
			}
			delete[] rec_vals;

			int errorCheck = primColorsVec.at(0).size();
		}
		catch (std::out_of_range)
		{
			fclose(fp);
			fp = fopen(rec_voltagesFile.c_str(), "rb");

			fscanf(fp, "%d%d%d%d\n", &nparam, &rec_len, &nrec, &nlast);


			len = rec_len * nrec + nlast;
			float* rec_vals = new float[nparam + 1];  // add 1 , just in case.
			fread(rec_vals, sizeof(float), nparam, fp);

			//primColorsVec.reserve(len);
			primColorsVec.resize(0);
			for (int i = 0; i < len; i++)
			{
				std::vector<float> aa;
				aa.reserve(nparam);
				aa.resize(0);
				for (int j = 0; j < nparam; j++)
				{
					float val = rec_vals[j];
					if (val > vol_fmax)
						vol_fmax = val;
					if (val < vol_fmin)
						vol_fmin = val;
					aa.emplace_back(val);
				}
				primColorsVec.emplace_back(aa);

				//fseek(fp, nparam, 1); //relocated the file pointer. 
				if (!feof(fp)) //
				{
					fread(rec_vals, sizeof(float), nparam, fp); // read more. 
				}
				else
				{
					std::cout << " The end of the file. \n"
						<< "Total read : "
						<< "i : " << i << " " << std::endl;
				}
			}
			delete[] rec_vals;
		}

		if (primColorsVec.size() == 0)
		{
			std::cout << "Open file Failed at: "
				<< rec_voltagesFile
				<< std::endl;
			return false;
		}

		std::cout
			<< "loaded  voltage file with : \n"
			// << "\t len   : " << len << std::endl
			<< "\t nparam: " << nparam << std::endl;

		std::cout
			<< "Please make sure nparam: " << nparam
			<< "\t is not oversized (too large  to be fitin in one array)"
			<< std::endl;

		std::cout << nparam << " " << rec_len << " " << nrec << " " << nlast << std::endl;

		std::cout
			<< "loaded  voltage file: \n"
			<< "\t voltages: " << (primColorsVec[0].size()) << std::endl
			<< "\t nodes   : " << (primColorsVec.size()) << std::endl
			<< "\t max voltage: " << vol_fmax << std::endl
			<< "\t min voltage: " << vol_fmin << std::endl
			<< std::endl;

		return true;
	}

	Tubes::SP trySWC_folder_binary(const std::string& fileName, float tubeSacle = 1.f)
	{
		utility::CPUProfiler profiler("Load SWC Folder");
		const std::string ext = getExt(fileName);
		if (ext.size() < 5 && ext.size() > 0)
			return nullptr;

		//< Expect fileName is a directory with suffix "\\" on windows.

		Tubes::SP neurons = std::make_shared<Tubes>();
#ifdef WIN32
		std::string vertexFile = fileName + "\\vertex";
		std::string indicesFile = fileName + "\\indices";
		std::string rec_voltagesFile = fileName + "\\rec_voltages";
#endif
#ifdef __linux__
		std::string vertexFile = fileName + "/vertex";
		std::string indicesFile = fileName + "/indices";
		std::string rec_voltagesFile = fileName + "/rec_voltages";
#endif

		std::ifstream vertexin(vertexFile, std::ios_base::binary);
		std::ifstream indicesin(indicesFile, std::ios_base::binary);

		bool volAvailability = neurons->loadColors(neurons->primColorsVec, rec_voltagesFile);

		if (!volAvailability)
		{
			return nullptr;
		}

		box3f bounds;
		std::string line;
		std::vector<vec3f> positions;
		std::vector<float> radii;
		std::vector<unsigned> colorsID;

		while (std::getline(vertexin, line))
		{
			int colorId;
			vec3f pos;
			float ra;
			if (line.find_first_of("#") != std::string::npos) continue;
			std::istringstream iss(line);
			iss >> pos.x >> pos.y >> pos.z >> ra >> colorId;
			positions.push_back(pos);
			radii.push_back(ra * tubeSacle);
			colorsID.push_back(colorId);
			bounds.extend(pos);
		}
		neurons->_bounds = bounds;

		//std::cout 
		//    << "load .swc-like folder file: \n"
		//    << "\t vertex: " << (positions.size()) << std::endl
		//    << "\t bounds: " << bounds << std::endl;

		while (std::getline(indicesin, line))
		{
			int i, j;
			if (line.find_first_of("#") != std::string::npos) continue;
			std::istringstream iss(line);
			iss >> i >> j;
#if 0
			neurons->lineStrips.emplace_back();
			neurons->lineStrips.back().push_back({ positions[i],radii[i],colorsID[i] });
			neurons->lineStrips.back().push_back({ positions[j],radii[j],colorsID[j] });
#else
			neurons->lineStrips.emplace_back(std::vector<Tubes::Vertex>(2));
			neurons->lineStrips.back()[0] = { positions[i],radii[i],colorsID[i] };
			neurons->lineStrips.back()[1] = { positions[j],radii[j],colorsID[j] };
#endif
		}

		std::cout
			<< "load .swc-like folder file: \n"
			<< "\t vertex: " << (positions.size()) << std::endl
			<< "\t tubes : " << (neurons->lineStrips.size()) << std::endl
			<< "\t bounds: " << bounds << std::endl
			<< "\t spans : " << bounds.span() << std::endl
			<< std::endl;

		return neurons;
	}

	//['r','g','b','c','m','y','k','g','DarkGray']  
	unsigned int neuronColorMapping(const unsigned type)
	{
		vec3f col(0.0);
		/* soma white, axon white, dentrites either blue or red */
		// switch (type) {
		//   case 1: col = vec3f(1.0f); break; // soma
		//   case 2: col = vec3f(1.0, 1.0, 1.0); break; // axon
		//   case 3: col = vec3f(1.0, 0.0, 0.0); break; // basal dendrite
		//   case 4: col = vec3f(0.0, 0.0, 1.0); break; // apical dendrite
		//   default: col = vec3f(0.0, 0.0, 0.0); break;
		// }

		/* soma white, axon white, dentrites either blue or red */
		switch (type) {
		case 1: col = vec3f(1.0f); break; // soma
		case 2: col = vec3f(1.0, 1.0, 1.0); break; // axon
		case 3: col = vec3f(156.0 / 255.0, 15.0 / 255.0, 12.0 / 255.0); break; // basal dendrite
		case 4: col = vec3f(10.0 / 255.0, 32.0 / 255.0, 156.0 / 255.0); break; // apical dendrite
		default: col = vec3f(0.0, 0.0, 0.0); break;
		}

		/* soma red, all others blue */
		// switch (type) {
		//   case 1: col = vec3f(1.0, 0.0, 0.0); break; // soma
		//   case 2: col = vec3f(0.2, .2, 1.0); break; // axon
		//   case 3: col = vec3f(0.2, .2, 1.0); break; // basal dendrite
		//   case 4: col = vec3f(0.2, .2, 1.0); break; // apical dendrite
		//   default: col = vec3f(0.2, .2, 1.0); break;
		// }

		// case 0: col = vec3f(256.0/255.0,231.0/255.0,37.0/255.0); break;
		// case 1: col = vec3f(158.0/255.0,217.0/255.0,58.0/255.0); break;
		// case 2: col = vec3f(137.0/255.0,213.0/255.0,72.0/255.0); break;
		// case 3: col = vec3f(214.0/255.0,210.0/255.0,80.0/255.0); break;
		// case 4: col = vec3f(71.0/255.0,193.0/255.0,110.0/255.0); break;
		// case 5: col = vec3f(62.0/255.0,73.0/255.0,137.0/255.0); break;
		// case 6: col = vec3f(62.0/255.0,73.0/255.0,137.0/255.0); break;
		// case 7: col = vec3f(72.0/255.0,29.0/255.0,111.0/255.0); break;
		// case 8: col = vec3f(69.0/255.0,5.0/255.0,89.0/255.0); break;
		// default: col = vec3f(71.0/255.0,193.0/255.0,110.0/255.0); break;
		// case 5: col = vec3f(58.0/255.0,186.0/255.0,118.0/255.0)
		// case 6: col = vec3f(35.0/255.0,169.0/255.0,131.0/255.0)
		// case 7: col = vec3f(30.0/255.0,157.0/255.0,137.0/255.0)
		// case 8: col = vec3f(42.0/255.0,120.0/255.0,142.0/255.0)
		// case 0: col = vec3f(.85, .17, .19); break;
		// case 1: col = vec3f(1.0, 0.0, 0.0); break;
		// case 2: col = vec3f(0.0, 0.0, 1.0); break;
		// case 3: col = vec3f(0.0, 1.0, 0.0); break;
		// case 4: col = vec3f(1.0, 0.0, 1.0); break;
		// case 5: col = vec3f(1.0, 1.0, 0.0); break;
		// case 6: col = vec3f(0.0, 1.0, 1.0); break;
		// case 7: col = vec3f(0.2, 0.2, 0.2); break;
		// case 8: col = vec3f(0.85, 0.05, 0.11); break;
		// default: col = vec3f(0.86, 0.11, 0.26);
		typedef unsigned char u_char;
		u_char r = (u_char)clamp((u_char)(col.x * 255.f), (u_char)0, (u_char)255);
		u_char g = (u_char)clamp((u_char)(col.y * 255.f), (u_char)0, (u_char)255);
		u_char b = (u_char)clamp((u_char)(col.z * 255.f), (u_char)0, (u_char)255);
		// nvm: don't know why, but white becomes green when "|"ing 255 << 24...
		return (r | (g << 8) | (b << 16));// | (u_char(255) << 24); 
	}

	// try parsing a 'procedural://' pseudo-url specifier:
	Tubes::SP tryProcedural(const std::string &pseudoURL)
	{
		int numLinks = 1000;
		vec2f lengthInterval = vec2f(0.5f, 3.5f);
		vec2f radiusInterval = vec2f(0.2f, 0.02f);

		int numRecognized
			= sscanf(pseudoURL.c_str(), "procedural://%i:%f,%f:%f,%f",
				&numLinks,
				&lengthInterval.x, &lengthInterval.y,
				&radiusInterval.x, &radiusInterval.y);
		PING;
		PRINT(numRecognized);
		if (numRecognized == 1 || numRecognized == 3 || numRecognized == 5)
			return Tubes::procedural(numLinks, lengthInterval, radiusInterval);

		return nullptr;
	}

	/*! try '.clusters' file format - will reject if file does not end with
		".slbin" extension

		clusters file format is what's written by clusterSegmenmts tools,
		and so far is ONLY PROTOTYPICAL

		Format as follows:

		FILE = {
		int32 numClusters;
		Cluster[numClusters];
		}

		Cluster = {
		linear3f xfm;
		int32 numSegments;
		Segments[numSegments];
		}

		Segment = {
		vec3f A
		vec3f B
		int   trackID
		int   segID;
		}

		currently this parser will only keep the N largest ones, for
		debugging
	*/
	//Tubes::SP tryClusters(const std::string &fileName)
	//{
	//const std::string ext = getExt(fileName);
	//if (ext != ".clusters")
	//	return nullptr;

	//std::cout << "recognized 'clusters' file format" << std::endl;
	//struct Segment {
	//	vec3f A, B;
	//	int trackID,segID;
	//};

	//Tubes::SP tubes = std::make_shared<Tubes>();

	//std::ifstream in(fileName,std::ios::binary);

	//// read N tracks
	//int numClusters;
	//in.read((char*)&numClusters,sizeof(numClusters));
	//PRINT(numClusters);
	//for (int clusterID=0;clusterID<numClusters;clusterID++) {
	//	linear3f toLocal;
	//	in.read((char*)&toLocal,sizeof(toLocal));
	//	int numSegs;
	//	in.read((char*)&numSegs,sizeof(numSegs));
	//	std::cout << "cluster " << clusterID << " : " << numSegs << " segments" << std::endl;
	//	std::vector<Segment> segments;
	//	for (int segID=0;segID<numSegs;segID++) {
	//	Segment seg;
	//	in.read((char*)&seg,sizeof(seg));
	//	segments.push_back(seg);
	//	}
	//	// if (segments.size() == 1)
	//	{
	//	for (auto seg : segments) {
	//		Tubes::Link link;
	//		link.pos    = seg.A;
	//		link.col    = unsigned(-1);
	//		link.rad    = 1.f;
	//		link.prev   = -1;
	//		tubes->links.push_back(link);
	//		link.prev = tubes->links.size()-1;
	//		link.pos = seg.B;
	//		tubes->links.push_back(link);

	//		// if (segments.size() > 200) {
	//		//   PING;
	//		//   PRINT(seg.A);
	//		//   PRINT(seg.B);
	//		// }
	//	}
	//	}
	//	// PRINT(tubes->links.size());
	//}

	//PING; PRINT(tubes->links.size());
	//// no timplemneted
	//return tubes;
	//}

	/*! try '.slbin' file format - will reject if file does not end with
		".slbin" extension

		the SLBin file format encodes a list of 'tracks' or stream lines, as follows:

		FILE = {
		int numTracks;
		Track[numTracks];
		int numPoints;
		vec3f point[numPoints];
		}

		with Track = {
		int begin;
		int count;
		}

		where 'begin' is a offset in the points[] array, and 'count' is the
		number of control points (starting at that offset) that go into the
		given track. (ie, for individual cylinders 'count' would always be
		'2', for anything longer than two we get a 'track' where
		points[offset] is the start point, and each following of the count
		points coonects to its predecessor.

	*/
	Tubes::SP trySLBin(const std::string &fileName,
		float radius)
	{
#if 1
		std::cout << ".slbin not yet ported...." << std::endl;
		return nullptr;
		// throw std::runtime_error("not yet ported....");
#else
		const std::string ext = getExt(fileName);
		if (ext != ".slbin")
			return nullptr;

		const char *subSampleEnv = getenv("TUBES_SUBSAMPLE");
		const int subSample
			= subSampleEnv
			? atoi(subSampleEnv)
			: 1;

		Tubes::SP tubes = std::make_shared<Tubes>();
		tubes->radius = radius;
		std::ifstream in(fileName, std::ios::binary);

		// read N tracks
		int numTracks;
		in.read((char*)&numTracks, sizeof(numTracks));
		std::vector<std::pair<int, int>> tracks(numTracks);
		in.read((char*)tracks.data(), tracks.size() * sizeof(tracks[0]));

		// read N points
		int numPoints;
		in.read((char*)&numPoints, sizeof(numPoints));
		std::vector<vec3f> points(numPoints);
		in.read((char*)points.data(), points.size() * sizeof(points[0]));

		if (radius <= 0.f) {
			radius = .1f;
			std::cout << "#rtx-neurons.slbin: no (or invalid) radius specified, defaulting to " << radius << std::endl;
		}
		int numSegments = 0;
		int trackID = 0;
		for (auto track : tracks) {
			int begin = track.first;
			int count = track.second;
			if (count == 1)
				continue;

			bool useThese = drand48()*subSample < 1.f;
			for (int i = 1; i < count; i++) {
				Tubes::Link link;
				link.pos0 = points[begin + i - 1];
				link.pos1 = points[begin + i];
				link.col = unsigned(-1);
				link.rad0 = radius;
				link.rad1 = radius;
				if (useThese)
					tubes->links.push_back(link);
			}
			if (useThese)
				numSegments += (count - 1);
			trackID++;
		}
		std::cout << OWL_TERMINAL_GREEN
			<< "#rtx-neurons: loaded .slbin file with "
			<< prettyNumber(tracks.size()) << " stream lines with "
			<< prettyNumber(numSegments) << " segments and "
			<< prettyNumber(numPoints) << " points/links"
			<< OWL_TERMINAL_DEFAULT
			<< std::endl;

		// no timplemneted
		return tubes;
#endif
	}

	Tubes::SP tryBCC(const std::string &fileName, float radius)
	{
		const std::string ext = getExt(fileName);
		if (ext != ".bcc")
			return nullptr;

		Tubes::SP tubes = std::make_shared<Tubes>();
		tubes->radius = radius;
		std::ifstream in(fileName, std::ios::binary);

		std::vector<BSpline> curves;
#if 0
		// Some dummies for now
		curves.push_back({ {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.5f}, {2.0f, 0.5f, 0.0f}, {3.0f, 0.0f, 0.0f}} });
		curves.push_back({ {{0.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}, {2.0f, 2.0f, 0.0f}, {3.0f, 3.0f, 0.0f}} });
		curves.push_back({ {{0.0f, 0.0f, 0.0f}, {0.0f, 20.0f, 1.0f}, {0.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 3.0f}} });
		curves.push_back({ {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 2.0f}, {0.0f, 0.0f, 3.0f}} });
		curves.push_back({ {{-1.0f, 2.0f, 0.0f}, {0.0f, -2.0f, 0.0f}, {1.0f, 4.0f, 0.0f}, {2.0f, -4.0f, 0.0f}} });
		curves.push_back({ {{-5.0f, 2.0f, 0.0f}, {0.0f, -0.5f, 1.0f}, {1.0f, 4.0f, 0.0f}, {2.0f, -4.0f, 8.0f}} });
		curves.push_back({ {{0.0f, 10.0f, 0.0f}, {0.0f, 11.0f, 11.0f}, {0.0f, -11.0f, 0.0f}, {0.0f, -10.0f, 0.0f}} });
		curves.push_back({ {{3.0f, 8.0f, 0.0f}, {0.0f, 15.0f, 11.0f}, {0.0f, -11.0f, 0.0f}, {7.0f, -10.0f, 0.0f}} });
		curves.push_back({ {{-10.0f, 20.0f, 0.0f}, {0.0f, -20.0f, 0.0f}, {1.0f, 4.0f, 0.0f}, {2.0f, -4.0f, 0.0f}} });
		curves.push_back({ {{0.0f, 20.0f, 0.0f}, {1.0f, 2.0f, 4.5f}, {2.0f, 3.5f, 0.0f}, {3.0f, 4.0f, 0.0f}} });
#endif
		// Ignore any header - these are uniformly parameterized Catmull Rom curves
		in.seekg(8, std::ios::beg);

		uint64_t numCurves = 0;
		in.read((char*)&numCurves, sizeof(numCurves));
		uint64_t numControlPoints = 0;
		in.read((char*)&numControlPoints, sizeof(numControlPoints));
		std::cout << numCurves << ' ' << numControlPoints << '\n';

		// Jump  straight to the loc containing the curves
		in.seekg(64, std::ios::beg);

		curves.resize(numCurves);

		for (size_t i = 0; i < numCurves; ++i)
		{
			int32_t numCurvePoints = 0;
			in.read((char*)&numCurvePoints, sizeof(numCurvePoints));
			numCurvePoints = std::abs(numCurvePoints);
			std::vector<vec3f> curvePoints(numCurvePoints);
			in.read((char*)curvePoints.data(), curvePoints.size() * sizeof(vec3f));

			for (size_t j = 0; j < numCurvePoints / 4; ++j)
			{
				vec3f p0 = curvePoints[j * 4];
				vec3f p1 = curvePoints[j * 4 + 1];
				vec3f p2 = curvePoints[j * 4 + 2];
				vec3f p3 = curvePoints[j * 4 + 3];

				float d1 = length(p1 - p0);
				float d2 = length(p2 - p1);
				float d3 = length(p3 - p2);

				vec3f w0 = p1;
				vec3f w1 = (p2 - p0 + 6.f*p1) / 6.f;
				vec3f w2 = (p1 - p3 + 6.f*p2) / 6.f;
				vec3f w3 = p2;
				curves.push_back({ {w0,w1,w2,w3} });
			}
		}

		for (auto& curve : curves) {
			tubes->lineStrips.emplace_back();
			tubes->lineStrips.back().resize(4);
			for (int i = 0; i < 4; ++i) {
				tubes->lineStrips.back()[i].pos = curve[i];
				tubes->lineStrips.back()[i].rad = radius;
				tubes->lineStrips.back()[i].col = unsigned(-1);
				tubes->_bounds.extend(curve[i]);
			}
		}

		return tubes;
	}

	Tubes::SP tryPBRT(const std::string &fileName, float radius)
	{
		const std::string ext = getExt(fileName);
		if (ext != ".pbrt")
			return nullptr;

		Tubes::SP tubes = std::make_shared<Tubes>();
		tubes->radius = radius;
		std::ifstream in(fileName, std::ios::binary);

		std::vector<BSpline> curves;

		std::string line;
		while (std::getline(in, line))
		{
			BSpline curve;
			char ignore[256];
			sscanf(line.c_str(), "Shape \"curve\" \"string type\" [ \"cylinder\" ] \"point P\" [ %f %f %f %f %f %f %f %f %f %f %f %f %s",
				&curve[0].x, &curve[0].y, &curve[0].z,
				&curve[1].x, &curve[1].y, &curve[1].z,
				&curve[2].x, &curve[2].y, &curve[2].z,
				&curve[3].x, &curve[3].y, &curve[3].z,
				ignore);
			curves.push_back(curve);
		}

		for (auto& curve : curves) {
			tubes->lineStrips.emplace_back();
			tubes->lineStrips.back().resize(4);
			for (int i = 0; i < 4; ++i) {
				tubes->lineStrips.back()[i].pos = curve[i];
				tubes->lineStrips.back()[i].rad = radius;
				tubes->lineStrips.back()[i].col = unsigned(-1);
				tubes->_bounds.extend(curve[i]);
			}
		}

		return tubes;
	}

	/*! procedurally generate some linked tubes,so we have "something"
		to test.  numTubes --- maximum supported size on 1080 card:
		29000000.  lengthInterval radiusInterval
	*/
	Tubes::SP Tubes::procedural(int numTubes,
		vec2f lengthInterval,
		vec2f radiusInterval)
	{
#if 1
		std::cout << "procedural method not yet ported...." << std::endl;
		return nullptr;
		//throw std::runtime_error("not yet ported....");
#else
		PING;
		PRINT(lengthInterval);
		PRINT(radiusInterval);

		Tubes::SP tubes = std::make_shared<Tubes>();
		float size = powf((float)numTubes, 1.f / 3.f);
		Link firstNode;
		firstNode.pos = size * vec3f((float)drand48(),
			(float)drand48(),
			(float)drand48());
		firstNode.rad = radiusInterval.x;
		firstNode.prev = -1;
		tubes->links.push_back(firstNode);

		for (int i = 0; i < numTubes; i++) {
			Link comp;
			comp.prev = i;
			float randomLen
				= lengthInterval.x + ((float)lengthInterval.y - lengthInterval.x) * drand48();
			float randomRadius
				= tubes->links[i].rad
				+ radiusInterval.y * ((float)drand48() * 2.f - 1.f);

			comp.pos
				= tubes->links[i].pos
				+ randomDirection() * randomLen;
			comp.rad = std::max(.1f*radiusInterval.x, fabsf(randomRadius));

			tubes->links.push_back(comp);
		}
		std::cout << "generated " << numTubes << " tubes as instructured" << std::endl;
		return tubes;
#endif
	}

	/*
		this only for the polyline style obj. not the triangle obj file

	*/
	Tubes::SP tryOBJ(const std::string& fileName, float radius = 0.00005)
	{
		const std::string ext = getExt(fileName);
		if (ext != ".obj")
			return nullptr;
		Tubes::SP tubes = std::make_shared<Tubes>();

		std::ifstream in(fileName);
		vec3f Pos;
		std::string line;
		char Ty;

		std::vector<vec3f> positions;
		std::vector<unsigned>   colors;
		// the previous line segment to build up a linked list over segments
		std::pair<int, int> prevSegment = { -1, -1 };
		std::size_t segmentCount = 0;

		tubes->radius = radius;
		while (std::getline(in, line)) {
			if (line.find_first_of("#") != std::string::npos)
				continue;
			if (line.find_first_of("O") != std::string::npos)
				continue;
			std::istringstream iss(line);
			if (line.find_first_of("v") != std::string::npos) {
				vec3f pt;
				iss >> Ty >> pt.x >> pt.y >> pt.z;
				positions.push_back(pt);
				if (!iss.eof()) {
					vec3f col;
					iss >> col.x >> col.y >> col.z;
					unsigned r = (unsigned)clamp(col.x*255.f, 0.f, 255.f);
					unsigned g = (unsigned)clamp(col.y*255.f, 0.f, 255.f);
					unsigned b = (unsigned)clamp(col.z*255.f, 0.f, 255.f);
					colors.push_back(r | (g << 8) | (b << 16) | (255 << 24));
				}
				else {
					colors.push_back(unsigned(-1));
				}
			}
			else if (line[0] == 'l') {
				std::pair<int, int> segment;
				iss >> Ty >> segment.first >> segment.second;
				segment.first--;
				segment.second--;
				if (segment.first >= positions.size())
					continue;
				if (segment.second >= positions.size())
					continue;
				segmentCount++;
				// assert(segment.first >= 0);
				// assert(segment.first < tubes->links.size());
				// assert(segment.second >= 0);
				// assert(segment.second < tubes->links.size());
				// tubes->links[segment.second].prev = segment.first;
				if (prevSegment.second != segment.first) {
					tubes->lineStrips.emplace_back();
					tubes->lineStrips.back().push_back({ positions[segment.first],tubes->radius,colors[segment.first] });
					tubes->_bounds.extend(positions[segment.first]);
				}
				tubes->lineStrips.back().push_back({ positions[segment.second],tubes->radius,colors[segment.second] });
				tubes->_bounds.extend(positions[segment.second]);
				prevSegment = segment;
			}
		}

		std::cout << OWL_TERMINAL_GREEN
			<< "#rtx-neurons: loaded .obj file with "
			<< prettyNumber(tubes->lineStrips.size()) << " stream lines "
			// << prettyNumber(lines.size()) << " segments and "
			<< segmentCount << " points/links"
			<< OWL_TERMINAL_DEFAULT
			<< std::endl;

		return tubes;
	}

	Tubes::SP trySWCraw(const std::string& fileName)
	{
		const std::string ext = getExt(fileName);
		if (ext != ".swc")
			return nullptr;
		Tubes::SP neurons = std::make_shared<Tubes>();

		std::ifstream in(fileName);
		int idx, type, prev;
		vec3f Pos;
		float ra;
		std::string line;

		while (std::getline(in, line))
		{
			if (line.find_first_of("#") != std::string::npos)
				continue;
			std::istringstream iss(line);
			iss >> idx >> type >> Pos.x >> Pos.y >> Pos.z >> ra >> prev;
			neurons->hostGeom.vertices.push_back(Pos);
			neurons->hostGeom.endPointRadii.push_back(ra);
			if (prev == -1)
			{
				neurons->hostGeom.prevPrims.push_back(0);
			}
			else
			{
				neurons->hostGeom.prevPrims.push_back(prev);
			}
			neurons->hostGeom.postPrims.push_back(0);
			neurons->hostGeom.primColors.push_back(neuronColorMapping(type));
			neurons->_bounds.extend(Pos);

			if (prev > 0)
			{
				const int prevPost = neurons->hostGeom.postPrims[prev - 1];

				if (prevPost == 0)
				{
					neurons->hostGeom.postPrims[prev - 1] = idx;
				}
			}

		}

		/*for (int i = 0; i < neurons->hostGeom.prevPrims.size(); i++)
		{
			int prev1 = neurons->hostGeom.prevPrims[i];
			int post = neurons->hostGeom.postPrims[i];
			std::cout << "before " << i + 1 << " " << prev1 << " " << post << std::endl;
		}

		for (int i = 0; i < neurons->hostGeom.prevPrims.size(); i++)
		{
			int prev1 = neurons->hostGeom.prevPrims[i];
			int post = neurons->hostGeom.postPrims[i];
			std::cout << "after " << i + 1 << " " << prev1 << " ";
			if (prev1 > 0)
			{
				const int prevPost = neurons->hostGeom.postPrims[prev1 - 1];
				std::cout << prevPost << " ";
				if (prevPost == 0)
				{
					neurons->hostGeom.postPrims[prev1 - 1] = i + 1;
					std::cout << neurons->hostGeom.postPrims[prev1 - 1];
				}
			}
			std::cout << std::endl;
		}*/

		return neurons;
	}

	Tubes::SP trySWC(const std::string& fileName)
	{
		const std::string ext = getExt(fileName);
		if (ext != ".swc")
			return nullptr;
		Tubes::SP neurons = std::make_shared<Tubes>();

		float currentRadius = 0.0f;
		float lastRadius = 0.0f;
		int somaNum = 1;
		int searchNum = 1;
		std::ifstream temp(fileName);
		std::string tempLine;
		while (std::getline(temp, tempLine))
		{
			if (tempLine.find_first_of("#") != std::string::npos)
				continue;
			std::istringstream iss(tempLine);
			if (currentRadius <= 0.000001f)
			{
				iss >> currentRadius >> currentRadius >> currentRadius >> currentRadius >> currentRadius >> currentRadius;
				lastRadius = currentRadius;
				continue;
			}
			iss >> currentRadius >> currentRadius >> currentRadius >> currentRadius >> currentRadius >> currentRadius;
			if (currentRadius - lastRadius < 0.00001f && lastRadius - currentRadius < 0.00001f)
			{
				somaNum += 1;
			}
			searchNum += 1;
			if (searchNum > somaNum)
			{
				break;
			}
		}
		std::cout << somaNum << std::endl;
		std::ifstream in(fileName);
		int idx, type, prev;
		vec3f Pos;
		float ra;
		std::string line;
		std::vector<vec3f> positions;
		std::vector<float> radii;
		bool somaHasMerged = false;
		vec3f centeroid;
		float radius;

		if (somaNum > 1)
		{
			while (std::getline(in, line))
			{
				if (line.find_first_of("#") != std::string::npos)
					continue;
				std::istringstream iss(line);
				iss >> idx >> type >> Pos.x >> Pos.y >> Pos.z >> ra >> prev;

				if (type == 1)
				{
					positions.push_back(Pos);
					radii.push_back(ra);
					continue;
				}

				// a soma part should have three samples 
				if (!somaHasMerged && positions.size() == somaNum && radii.size() == somaNum)
				{
					somaHasMerged = true;
					centeroid = std::accumulate(positions.begin(), positions.end(), vec3f(0.0f)) / float(somaNum);//computeCentroid(positions);
					radius = 0.5f * std::accumulate(radii.begin(), radii.end(), 0.0f) / float(somaNum);

					positions.clear();
					radii.clear();
					positions.push_back(centeroid);
					radii.push_back(radius);
					neurons->lineStrips.emplace_back();
					neurons->lineStrips.back().push_back({ centeroid, radius, neuronColorMapping(type) });
					neurons->lineStrips.back().push_back({ centeroid, radius, neuronColorMapping(type) });
					neurons->_bounds.extend(Pos);
				}
				positions.push_back(Pos);
				radii.push_back(ra);
				if (prev > 1) prev -= somaNum - 1;
				neurons->lineStrips.emplace_back();
				neurons->lineStrips.back().push_back({ Pos, ra, neuronColorMapping(type) });
				neurons->lineStrips.back().push_back({ positions[prev - 1], radii[prev - 1], neuronColorMapping(type) });
				neurons->_bounds.extend(Pos);
			}
		}
		else
		{
			while (std::getline(in, line))
			{
				if (line.find_first_of("#") != std::string::npos)
					continue;
				std::istringstream iss(line);
				iss >> idx >> type >> Pos.x >> Pos.y >> Pos.z >> ra >> prev;
				positions.push_back(Pos);
				radii.push_back(ra);

				if (prev < 0)
				{
					neurons->lineStrips.emplace_back();
					neurons->lineStrips.back().push_back({ Pos, ra, neuronColorMapping(type) });
					neurons->lineStrips.back().push_back({ Pos, ra, neuronColorMapping(type) });
					neurons->_bounds.extend(Pos);
				}
				else
				{
					neurons->lineStrips.emplace_back();
					neurons->lineStrips.back().push_back({ Pos, ra, neuronColorMapping(type) });
					neurons->lineStrips.back().push_back({ positions[prev - 1], radii[prev - 1], neuronColorMapping(type) });
					neurons->_bounds.extend(Pos);
				}
			}
		}

		std::cout << OWL_TERMINAL_GREEN
			<< "#rtx-neurons: loaded .swc file with "
			<< prettyNumber(neurons->lineStrips.size()) << " stream lines "
			// << prettyNumber(numSegments) << " segments and "
			// << prettyNumber(numPoints) << " points/links"
			<< OWL_TERMINAL_DEFAULT
			<< std::endl;
		// no timplemneted
		return neurons;
	}



	Tubes::SP Tubes::load(const std::string& fileName)
	{


		/* try '*.swc' */
		if (Tubes::SP tubes = trySWC(fileName))
		{
			return tubes;
		}

		std::cout << "could not load/create input '" << fileName << "'" << std::endl;

		return nullptr;

#if 0 /* Haixin Ma: these are old codes, 
			some might need to be updated,
			others would be abandoned. */

			/* try custom swc like files with voltage */
		if (Tubes::SP tubes = trySWC_folder(fileName))
		{
			return tubes;
		}

		// try polyline "obj"
		if (Tubes::SP tubes = tryOBJ(fileName, radius))
			return tubes;

		// try '*.pbrt'
		if (Tubes::SP tubes = tryPBRT(fileName, radius))
			return tubes;

		// try '*.clusters'
		if (Tubes::SP tubes = tryClusters(fileName)) {
			if (radius > 0.f) tubes->radius = radius;
			return tubes;
		}

		if (Tubes::SP tubes = trySLTxt(fileName))
			return tubes;

		// try '*.slbin'
		if (Tubes::SP tubes = trySLBin(fileName, radius))
			return tubes;

		// try '*.bcc'
		if (Tubes::SP tubes = tryBCC(fileName, radius))
			return tubes;

		// try 'procedural://' specifier
		if (Tubes::SP tubes = tryProcedural(fileName))
			return tubes;

		// this is code for random *particle* generation
		float radius = 1.f;
		Tubes::SP tubes = std::make_shared<Tubes>();
		int numParticles = 200;
		if (sscanf(fileName.c_str(), "%i.random", &numParticles))
			std::cout << "using " << numParticles << " as instructured on cmdline" << std::endl;

		float size = powf(numParticles, 1.f / 3.f);

		for (int i = 0; i < numParticles; i++) {
			vec3f pos = size * 2.f * vec3f(drand48(), drand48(), drand48());
			Link comp = { pos,radius,-1 };

			// comp.r = .01f+.08f*drand48();
			tubes->links.push_back(comp);
		}
		tubes->radius = radius;
		return tubes;
#endif
	}

	/*! load several files */
	Tubes::SP Tubes::load(const std::vector<std::string>& fileNames, float radius, unsigned volSz, float tubeScale)
	{
		Tubes::SP result = nullptr;
		for (std::size_t i = 0; i < fileNames.size(); ++i)
		{
			Tubes::SP tubes = load(fileNames[i], radius, volSz, tubeScale);
			if (tubes != nullptr)
			{
				if (result != nullptr)
				{

					auto ls = result->lineStrips[result->lineStrips.size() - 1];
					/* for those files that dont support voltages */
					unsigned int lastColorId = ls[ls.size() - 1].col + 1;
					for (auto& lineStrip : tubes->lineStrips)
					{
						for (auto& vertex : lineStrip)
						{
							vertex.col = lastColorId;
						}
					}

					result->lineStrips.insert(result->lineStrips.end(), tubes->lineStrips.begin(), tubes->lineStrips.end());

					result->_bounds.extend(tubes->_bounds);
				}
				else
				{
					result = tubes;
				}
			}
			else
			{
				//throw std::runtime_error("could not load/create input from file no. '" + std::to_string(i) + "'");
			}
		}
		return result;
	}



	Tubes::HostGeom Tubes::asLinks() const
	{
		static Tubes::HostGeom *result = nullptr;
		if (result == nullptr)
		{
			result = new Tubes::HostGeom;
			for (const LineStrip &ls : lineStrips)
			{
				for (ptrdiff_t i = 0; i < ptrdiff_t(ls.size()) - 1; ++i)
				{
					Vertex v0 = ls[i];
					Vertex v1 = ls[i + 1];
					(*result).vertices.push_back(v0.pos);
					(*result).vertices.push_back(v1.pos);
					(*result).endPointRadii.push_back(v0.rad);
					(*result).endPointRadii.push_back(v1.rad);
					(*result).primColors.push_back(v0.col);
				}
			}
		}
		return *result;
	}

	Tubes::HostGeom Tubes::asCubicSplines() const
	{
		Tubes::HostGeom result;

		for (const LineStrip &ls : lineStrips) {
			for (size_t i = 0; i < ls.size(); i += 3) {
				Vertex v0 = ls[i];
				Vertex v1 = ls[std::min(ptrdiff_t(i) + 1, ptrdiff_t(ls.size()) - 1)];
				Vertex v2 = ls[std::min(ptrdiff_t(i) + 2, ptrdiff_t(ls.size()) - 1)];
				Vertex v3 = ls[std::min(ptrdiff_t(i) + 3, ptrdiff_t(ls.size()) - 1)];
				result.vertices.push_back(v0.pos);
				result.vertices.push_back(v1.pos);
				result.vertices.push_back(v2.pos);
				result.vertices.push_back(v3.pos);
				result.endPointRadii.push_back(v0.rad);
				result.endPointRadii.push_back(v3.rad);
				result.primColors.push_back(v0.col);

				// Make sure when we're at the end of the
				// curve we don't pad that last control
				// point in the loop with three additional,
				// degenerate points
				if (i + 4 == ls.size())
					break;
			}
		}

		std::cout << "num curves (before split): " << result.vertices.size() / 4 << '\n';

		// Reshetov splits into 8 curves (might be too much for us)
		if (curveSegments == 2)
			result = splitBSplineLinks<2>(result);
		else if (curveSegments == 4)
			result = splitBSplineLinks<4>(result);
		else if (curveSegments == 8)
			result = splitBSplineLinks<8>(result);
		else if (curveSegments != 1)
			throw std::runtime_error("invalid --curve-segments, must be 1,2,4, or 8!");

		std::cout << "num curves (after split) : " << result.vertices.size() / 4 << '\n';

		return result;
	}

	Tubes::HostGeom Tubes::asRaw() const
	{
		return this->hostGeom;
	}

	/*! returns world space bounding box of the scene; to be used for
		the viewer to automatically set camera and motion speed */
	box3f Tubes::getBounds() const
	{
		return box3f(_bounds.lower - vec3f(1.0f), _bounds.upper + vec3f(1.0f));
	}

#endif

}
