// ======================================================================== //
// Copyright 2018-2022 The Contributors                                     //
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

#include "BasicNeurons.h"
#include "CmdlineParser.h"
#include "Curve.h"
#include "OptixTubes.h"
#include "profiler.h"
#include "Triangles.h"
#include "TubesUtils.h"
#include "Viewer.h"
#include "SimulationWindow.h"
#include "interact.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <iostream>
#include <sstream>
#include <thread>


namespace rtn
{
	// void simulation(SimulationWindow* simWindow)
	// {
	// 	int argct;
	// 	char** argvt;
		
	// 	glutInit(&argct, argvt);
	// 	simWindow->InitWindow(1920, 1080, "MetaWorm");
	// 	glutMainLoop();
	// }

	extern "C" int main(int argc, char **argv)
	{
		CmdlineArgs cmdline_args;
		std::stringstream ss;
		parse_arguments(argc, argv, cmdline_args);

		utility::CPUProfiler::begin();

		if (cmdline_args.dataDirectories.size() > 0)
		{
			parse_directories(cmdline_args);
		}

		// ------------------------------------------------------------------
		// load input data
		// ------------------------------------------------------------------
		Tubes::SP      neurons = nullptr;
		Triangles::SP triangles = nullptr;
		neurons = Tubes::load(cmdline_args.geomFiles, cmdline_args.voltageFiles, cmdline_args.synapseFiles, cmdline_args.materialFiles);
		neurons->dendriteScale = cmdline_args.dendriteScale;
		neurons->synapseScale = cmdline_args.synapseScale;
		neurons->maxScale = cmdline_args.maxWeightScale;
		neurons->minScale = cmdline_args.minWeightScale;
		/*if (neurons)
		{
			neurons->curveSegments = cmdline_args.curveSegments;
		}*/

		if (cmdline_args.triangleFiles.size() > 0)
		{
			for (auto &name : cmdline_args.triangleFiles)
			{
				Log("loading OBJ :%s\n", name.c_str());
			}
			triangles = Triangles::load(cmdline_args.triangleFiles, cmdline_args.material.base_color);
		}

		/***
		load curves
		***/
		Curves::SP curves = nullptr;

		if (cmdline_args.curveFiles.size() > 0)
		{
			curves = Curves::loadCurves(cmdline_args.curveFiles);
		}

		// ------------------------------------------------------------------
		// creating backend, and ask backend to build its data
		// ------------------------------------------------------------------
		OWLTubes *owlTubes = nullptr;

		if (cmdline_args.method == "basic")
		{
			owlTubes = new BasicNeurons;
			Log("created basic neurons back-end!\n");
		}
		else
		{
			Log("unknown back-end: %s!\n", cmdline_args.method.c_str());
			exit(1);
		}

		box3f sceneBounds;
		if (neurons)
		{
			sceneBounds.extend(neurons->bounds);
		}
		if (triangles)
		{
			sceneBounds.extend(triangles->bounds);
		}
		if (curves)
		{
			sceneBounds.extend(curves->bounds);
		}

		vec3f model_span = sceneBounds.span();

		ss << "Scene info: \n"
			<< "\t span: " << model_span << std::endl
			<< std::endl;


		vec3f camera_pos = sceneBounds.center() + vec3f(-1.f, .0f, +1.f) * model_span;
		vec3f camera_target = sceneBounds.center();
		float camera_fov = 70.f;

		ss << "camera setting: \n"
			<< "\t pos: " << camera_pos << std::endl
			<< "\t to : " << camera_target << std::endl
			<< "\t fov: " << camera_fov << std::endl
			<< std::endl;

		Log("%s", ss.str().c_str());
		ss.str() = "";

		//    vec2f A(sceneBounds.lower.x, sceneBounds.lower.z), B(sceneBounds.lower.x, sceneBounds.upper.z), C(sceneBounds.upper.x, sceneBounds.upper.z);
		//    vec2f D = (A + B + C) * 0.333f;
		//
		//    A = A - (A - D) * 0.9f + D * 0.1f;
		//    B = B - (B - D) * 0.9f + D * 0.1f;
		//    C = C - (C - D) * 0.9f + D * 0.1f;
		//
		//   //float area_light_height = model_span.y * 9.f;
		//    float area_light_height = sceneBounds.center().y + model_span.y * 2.05;
		//    QuadLightGeom light =
		//    {
		//#if 0
		//        //< right
		//        { 3430.f,3580.f,2750.f },
		//        { 3430.f,3580.f, -2750.f }, //u
		//        { 3430.f,-3850.f,2750.f }, //v
		//#endif
		//        {  A.x, area_light_height, A.y   },
		//        {  B.x, area_light_height, B.y }, //v
		//        {  C.x, area_light_height, C.y  }, //u
		//        { 1.f,1.f,1.f },
		//        1.f
		//    };
		float area_light_height = sceneBounds.center().y + model_span.y * 1.2f;
		QuadLightGeom light =
		{
			{ sceneBounds.lower.x, area_light_height, sceneBounds.lower.z },
			{ sceneBounds.lower.x, area_light_height, sceneBounds.upper.z },
			{ sceneBounds.upper.x, area_light_height, sceneBounds.lower.z },
			{1.0f, 1.0f, 1.0f},
			1.0f
		};

		owlTubes->setModel(neurons, triangles, curves, light);

		std::cout << utility::CPUProfiler::end() << std::endl;
		
		// ------------------------------------------------------------------
		// create glfw based viewer
		// ------------------------------------------------------------------
		int width = 800;
		int height = 450;
		if (cmdline_args.windowSize != vec2i(0))
		{
			width = cmdline_args.windowSize.x;
			height = cmdline_args.windowSize.y;
		}
		TubesViewer widget(*owlTubes, "owlTubesViwer", vec2i(width, height));

		Py_Initialize();
		wchar_t **_argv = (wchar_t **)PyMem_Malloc(sizeof(wchar_t *) * argc);
		// for (int i = 0; i < argc; i++)
		// {
		// 	wchar_t *arg = Py_DecodeLocale(argv[i], NULL);
		// 	_argv[i] = arg;
		// }

		PySys_SetArgv(argc, _argv);
		boost::python::numpy::initialize();
		Interact::InitTubesViewer(&widget);
		widget.simulationWindow = new SimulationWindow("c302");

		// std::cout << "simulation window initialized" << std::endl;

		widget.simulationWindow->autoScreenShot = cmdline_args.worm_auto_screenshot;

		//std::thread wormsim(simulation, widget.simulationWindow);
		//wormsim.detach();

		widget.frameState.material = cmdline_args.material;
		widget.frameState.quad_light = light;
		widget.frameState.camera_lens_radius = cmdline_args.camera.lens_radius;
		// widget.frameState.voltageColorMapSize = owlTubes->getVoltageColorMapSize();
		widget.frameState.enable_emissive = cmdline_args.enable_emissive;
		widget.frameState.maxAccum = cmdline_args.maxAccum;
		widget.frameState.enable_light_decay = cmdline_args.enable_light_decay;
		widget.frameState.emissive_intensity = cmdline_args.emissive_intensity;
		widget.frameState.samplesPerPixel = cmdline_args.spp;
		widget.anim_accum = cmdline_args.accum;
		widget.bg_bottom_color = vec3f(cmdline_args.bg);
		widget.bg_top_color = vec3f(cmdline_args.bg + 0.2f > 1.f ? 1.f : cmdline_args.bg + 0.2f);
		widget.nCircleSamples = cmdline_args.nCircleSamples;
		widget.cameraDistScale = cmdline_args.cameraDistScale;

		widget.camera_pos = camera_pos;
		widget.camera_target = camera_target;
		widget.model_span = model_span;
		widget.camera_fov = camera_fov;

		if (neurons)
		{
			widget.rtn = neurons;
			widget.frameState.maxVoltage = neurons->maxVoltage;
			widget.frameState.minVoltage = neurons->minVoltage;
			widget.frameState.inhibition = 0.8 * neurons->minVoltage + 0.2 * neurons->maxVoltage;
			widget.frameState.resting = 0.6 * neurons->minVoltage + 0.4 * neurons->maxVoltage;
			widget.frameState.excitation = 0.4 * neurons->minVoltage + 0.6 * neurons->maxVoltage;
			widget.frameState.firing = 0.2 * neurons->minVoltage + 0.8 * neurons->maxVoltage;

			widget.sceneBounds.extend(neurons->bounds);
			if (neurons->maxTime > 0)
			{
				widget.frameState.voltageSequenceLength = neurons->maxTime;
			}
			else
			{
				widget.frameState.voltageSequenceLength = 1;
			}
		}
		if (triangles)
		{
			widget.triangles = triangles;
			widget.sceneBounds.extend(triangles->bounds);
		}
		if (curves)
		{
			widget.sceneBounds.extend(curves->bounds);
		}

		//Log("- set bg color : %f\n", cmdline_args.bg);
		//Log("- set circle camera : samples=%d, dist=%f\n", cmdline_args.nCircleSamples, cmdline_args.cameraDistScale);

		widget.setWorldScale(length(sceneBounds.span())); // world scale for owlviewer and its camera
		widget.worldScale = length(sceneBounds.span()); //world scale for tubesviewer and its camera

		widget.enableFreeMode();

		if (cmdline_args.camera.vu != vec3f(0.f))
		{
			widget.setCameraOrientation(/*origin   */cmdline_args.camera.vp,
				/*lookat   */cmdline_args.camera.vi,
				/*up-vector*/cmdline_args.camera.vu,
				/*fovy(deg)*/cmdline_args.camera.fov);
			widget.setCameraOptions(cmdline_args.camera.fov, cmdline_args.camera.focal_distance);
			/*Log("Not expected path, exit !!\n");
			exit(1);*/
		}
		else
		{
			widget.setCameraOrientation(/*origin   */camera_pos,
				/*lookat   */camera_target,
				/*up-vector*/vec3f(0.f, 1.f, 0.f),
				/*fovy(deg)*/camera_fov);
		}
		
		widget.showAndRun();
	}

}
