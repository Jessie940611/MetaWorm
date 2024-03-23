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

#include "CmdlineParser.h"

#include "external/filesystem.hpp"

namespace rtn
{

	void usage()
	{
		Log("usage:   neuronXcore.exe [option] -user input-data-location "
			"\ncommand line arguments: "
			"\n\t -help, show this message and exit "
			"\n\t -bg v, specify background light intensiy in range [0,1] "
			"\n\t -win w h, specify image resolution "
			"\n\t -data path-to-all-data-location "
			"\n\t -morp path-to-neuron-morphology-data-location, used for detail neuron visualization "
			"\n\t -obj path-to-triangle-mesh-data-location, used for brain section visualization "
			"\n\t -curve path-to-polynomial-curve-data-location, used for large scale neuron visualization "
			"\nexamples:"
			"\n\t .\\neuronXcore.exe  -win 1280 720 -data anypath "
			"\n\t .\\neuronXcore.exe  -win 1280 720 -bg 0.161 -morp anypath"
			"\n\t .\\neuronXcore.exe  -win 1280 720 -morp anypath -obj anypath -curve anypath --camera-focal-distance 0.85 --camera-lens-radius 1.2 "
			"\n\t "
			"\n\t for more detail, see user guide md in ./dynamicneuronx/neuronXcore/example "
		);
		exit(1);
	}

	void parse_arguments(int argc, char **argv, CmdlineArgs &cmdline_args)
	{
		for (int i = 1; i < argc; i++)
		{
			const std::string arg = argv[i];
			if (arg == "-data")
			{
				cmdline_args.dataDirectories.push_back(argv[++i]);
			}
			else if (arg == "-morp" || arg == "-geom") {
				cmdline_args.geomFiles.push_back(argv[++i]);
			}
			else if (arg == "-vol" || arg == "-voltage") {
				cmdline_args.voltageFiles.push_back(argv[++i]);
			}
			else if (arg == "-snp" || arg == "-synapse") {
				cmdline_args.synapseFiles.push_back(argv[++i]);
			}
			else if (arg == "-mat") {
				cmdline_args.materialFiles.push_back(argv[++i]);
			}
			else if (arg == "-triangle" || arg == "-obj") {
				cmdline_args.triangleFiles.push_back(argv[++i]);
			}
			else if (arg == "-curve") {
				cmdline_args.curveFiles.push_back(argv[++i]);
			}
			else if (arg == "--screenshot" || arg == "-ss")
			{
				cmdline_args.screenshotPath = argv[++i];
			}
			else if (arg == "-win" || arg == "-size") {
				cmdline_args.windowSize.x = std::atoi(argv[++i]);
				cmdline_args.windowSize.y = std::atoi(argv[++i]);
			}
			else if (arg == "-bg" || arg == "-backgroud") {
				cmdline_args.bg = std::atof(argv[++i]);
			}
			else if (arg == "--radius" || arg == "-r") {
				cmdline_args.radius = std::stof(argv[++i]);
				PRINT(cmdline_args.radius);
			}
			else if (arg == "--camera") {
				cmdline_args.camera.vp.x = std::atof(argv[++i]);
				cmdline_args.camera.vp.y = std::atof(argv[++i]);
				cmdline_args.camera.vp.z = std::atof(argv[++i]);
				cmdline_args.camera.vi.x = std::atof(argv[++i]);
				cmdline_args.camera.vi.y = std::atof(argv[++i]);
				cmdline_args.camera.vi.z = std::atof(argv[++i]);
				cmdline_args.camera.vu.x = std::atof(argv[++i]);
				cmdline_args.camera.vu.y = std::atof(argv[++i]);
				cmdline_args.camera.vu.z = std::atof(argv[++i]);
			}
			else if (arg == "--camera-fov") {
				cmdline_args.camera.fov = std::atof(argv[++i]);
			}
			else if (arg == "--camera-focal-distance") {
				cmdline_args.camera.focal_distance = std::atof(argv[++i]);
			}
			else if (arg == "--camera-lens-radius") {
				cmdline_args.camera.lens_radius = std::atof(argv[++i]);
			}		
			else if (arg == "-spp") {
				cmdline_args.spp = std::stoi(argv[++i]);
			}
			else if (arg == "-help") {
				usage();
			}
			else if (arg == "-circle") {
				cmdline_args.nCircleSamples = std::stoi(argv[++i]);
			}
			else if (arg == "-circleDist") {
				cmdline_args.cameraDistScale = std::stof(argv[++i]);
			}
			else if (arg == "--dendrite-scale") {
				cmdline_args.dendriteScale = std::stof(argv[++i]);
			}
			else if (arg == "--synapse-scale") {
				cmdline_args.synapseScale = std::stof(argv[++i]);
			}
			else if (arg == "--max-weight-scale") {
				cmdline_args.maxWeightScale = std::stof(argv[++i]);
			}
			else if (arg == "--min-weight-scale") {
				cmdline_args.minWeightScale = std::stof(argv[++i]);
			}
			else if (arg == "--auto-accum") {
				cmdline_args.accum = std::stoi(argv[++i]);
			}
			else if (arg == "--curve-segments" || arg == "-cs") {
				cmdline_args.curveSegments = std::atoi(argv[++i]);
			}
			else if (arg == "-sm" || arg == "--shade-mode") {
				cmdline_args.shadeMode = std::atoi(argv[++i]);
			}
			else if (arg == "--rec-depth" || arg == "-rd") {
				cmdline_args.pathDepth = std::atoi(argv[++i]);
			}			
			else if (arg == "-measure" || arg == "--measure") {
				cmdline_args.measure = true;
			}			
			else if (arg == "-volsz") {
				cmdline_args.volSz = std::atoi(argv[++i]);
			}			
			else if (arg == "--max-accum") {
				cmdline_args.maxAccum = std::atoi(argv[++i]);
			}
			else if (arg == "--enable-emissive") {
				cmdline_args.enable_emissive = true;
			}
			else if (arg == "--disable-light-decay") {
				cmdline_args.enable_light_decay = false;
			}
			else if (arg == "--emissive-intensity") {
				cmdline_args.emissive_intensity = std::atof(argv[++i]);
			}
			else if (arg == "--max-voltage") {
				cmdline_args.maxVoltage = std::atoi(argv[++i]);
			}
			else if (arg == "--min-voltage") {
				cmdline_args.minVoltage = std::atoi(argv[++i]);
			}
			else if (arg == "--worm-auto-screenshot") {
				cmdline_args.worm_auto_screenshot = true;
			}
			else
			{
				Log("unknown argument:%s\n", arg.c_str());
				usage();
			}
		}
	}

	void parse_directories(CmdlineArgs& cmdline_args)
	{
		std::string path, extension;
		for (auto directory : cmdline_args.dataDirectories)
		{
			std::cout << "try to regist path: " << path << std::endl;

			for (const auto& filePath : ghc::filesystem::recursive_directory_iterator(directory))
			{
				path = filePath.path();
				
				int extPos = path.find_last_of(".");
				if (extPos != std::string::npos)
				{
					extension = path.substr(extPos);
				}
				else
				{
					extension = "";
				}
				
				if (extension == ".swc")
				{
					std::cout << "registing geometry path: " << path << std::endl;
					cmdline_args.geomFiles.push_back(path);
				}
				else if (extension == ".vtg")
				{
					std::cout << "registing voltage path: " << path << std::endl;
					cmdline_args.voltageFiles.push_back(path);
				}
				else if (extension == ".snp")
				{
					std::cout << "registing synapse path: " << path << std::endl;
					cmdline_args.synapseFiles.push_back(path);
				}
				else if (extension == ".mat")
				{
					std::cout << "registing material path: " << path << std::endl;
					cmdline_args.materialFiles.push_back(path);
				}
				if (extension == ".obj")
				{
					std::cout << "registing triangle path: " << path << std::endl;
					cmdline_args.triangleFiles.push_back(path);
				}
				if (extension == ".crv")
				{
					std::cout << "registing curve path: " << path << std::endl;
					cmdline_args.curveFiles.push_back(path);
				}
				
			}
		}
	}

} // ::tubes
