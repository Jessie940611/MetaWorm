// ======================================================================== //
// Copyright 2018-2019 The Contributors                                     //
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

#include "rtn/OptixTubes.h"
#include "rtn/device/RayGenData.h"
#include "rtn/device/TubesGeom.h"
#include "profiler.h"
#include <map> 
#include <fstream> 
#include <cuda_runtime.h>


#include "samples/common/3rdParty/stb/stb_image.h"

#ifdef WIN32
#define ENABLE_MULTI_GPU 0
#else
#define ENABLE_MULTI_GPU 1
#endif
namespace rtn {

	using device::RayGenData;
	using device::MissData;
	using device::FrameState;

	extern "C" const char embedded_common_programs[];

	OWLTubes::OWLTubes()
	{
	//#if ENABLE_MULTI_GPU
	//	int gpus[] = { 0, 1, 2, 3 };
	//	context = owlContextCreate(nullptr, 4); //< all GPUs used
	//#else
	//	context = owlContextCreate(nullptr, 1); //< only use GPU 0;
	//#endif
	//	int numGPUsFound = owlGetDeviceCount(context);

#if 1
		int numGPUsFound;
		cudaGetDeviceCount(&numGPUsFound);
		std::cout << "Cuda found " << numGPUsFound << " GPUs in device" << std::endl;
#else
		int numGPUsFound = 1;
#endif

		context = owlContextCreate(nullptr);
		int numContextGPUsFound = owlGetDeviceCount(context);
		printf("Context created;\n owl found %d GPUs \n", numContextGPUsFound);

		owlContextSetRayTypeCount(context, 2);
		module = owlModuleCreate(context, embedded_common_programs);
		frameStateBuffer = owlDeviceBufferCreate(context, OWL_USER_TYPE(FrameState), 1, nullptr);

		// -------------------------------------------------------
		// set up miss prog 
		// -------------------------------------------------------
		OWLVarDecl missProgVars[] =
		{
			{ "topColor",          OWL_FLOAT3,   OWL_OFFSETOF(MissData,topColor)},
			{ "bottomColor",       OWL_FLOAT3,   OWL_OFFSETOF(MissData,bottomColor)},
			{ /* sentinel to mark end of list */ }
		};
		// ........... create object  ............................
		this->missProg = owlMissProgCreate(context, module, "miss_program", sizeof(MissData), missProgVars, -1);
		owlMissProgSet3f(missProg, "topColor", { 0.06f, 0.11f, 0.26f });
		owlMissProgSet3f(missProg, "bottomColor", { 0.25f, 0.25f, 0.25f });

		OWLMissProg missProgShadow = owlMissProgCreate(context, module, "miss_program_shadow", 0, nullptr, -1);

		// -------------------------------------------------------
		// set up ray gen program
		// -------------------------------------------------------
		OWLVarDecl rayGenVars[] = {
			{ "deviceIndex",     OWL_DEVICE, OWL_OFFSETOF(RayGenData,deviceIndex)},
			{ "deviceCount",     OWL_INT,    OWL_OFFSETOF(RayGenData,deviceCount)},
			{ "world",           OWL_GROUP,  OWL_OFFSETOF(RayGenData,world)},
			{ "fbSize",          OWL_INT2,   OWL_OFFSETOF(RayGenData,fbSize)},
			{ "colorBuffer",     OWL_RAW_POINTER, OWL_OFFSETOF(RayGenData,colorBufferPtr)},
			{ "accumBuffer",     OWL_BUFPTR, OWL_OFFSETOF(RayGenData,accumBufferPtr)},
			{ "frameStateBuffer",OWL_BUFPTR, OWL_OFFSETOF(RayGenData,frameStateBuffer)},		
			{ "voltagesColor",   OWL_BUFPTR, OWL_OFFSETOF(RayGenData,voltagesColor)},
			{ "voltagesDeriv",   OWL_BUFPTR, OWL_OFFSETOF(RayGenData,voltagesDeriv)},
			{ /* sentinel to mark end of list */ }
		};

		// ........... create object  ............................
		this->rayGen = owlRayGenCreate(context, module, "raygen_program", sizeof(RayGenData), rayGenVars, -1);

		owlRayGenSetBuffer(rayGen, "frameStateBuffer", frameStateBuffer);
		owlRayGenSet1i(rayGen, "deviceCount", numGPUsFound);

		//---------  bind textures, before owl upgrade using buffer instead---------------//

		/*auto splitFunc = [](const std::string& str)
		{
			size_t found;
			found = str.find_last_of("/\\");
			return str.substr(0, found);
		};*/

		/* Haixin Ma: currently lut is not used */

		/*std::string voltageColorMapPath = splitFunc(__FILE__) + "//lut.png";
		vec2i res;
		int comp;

		unsigned char* image = stbi_load(voltageColorMapPath.c_str(),
			&res.x, &res.y, &comp, STBI_rgb);

		assert(image && "No LUT Image Found!");

		assert(res.x > 0 && res.y > 0);

		voltageColorMapSize = res.x * res.y;
		voltageColorMap = owlDeviceBufferCreate(context, OWL_USER_TYPE(vec3uc), res.x * res.y, image);;
		owlRayGenSetBuffer(rayGen, "voltageColorMap", voltageColorMap);

		voltageColorMap = owlTexture2DCreate(context, OWL_TEXEL_FORMAT_RGBA8, texelX, texelY, texels.data(), OWL_TEXTURE_NEAREST);
		owlParamsSetTexture();*/
		
	}

	void OWLTubes::setModel(Tubes::SP rtn, Triangles::SP triangles, std::shared_ptr<Curves> curve, const QuadLightGeom &light)
	{

		build(rtn, triangles, curve, light);

		owlRayGenSetGroup(rayGen, "world", world);

		std::cout << "building programs" << std::endl;
		owlBuildPrograms(context);
		std::cout << "building pipeline" << std::endl;
		owlBuildPipeline(context);
		std::cout << "building SBT" << std::endl;
		owlBuildSBT(context);
	}

	// Move this to the child class.
	/*! the setup code for the 'all tubes are user-geom cylinders'
		variant. todo: add one with using instance transforms */
	//void OWLTubes::buildModel(Tubes::SP model)
	//{
	//	linkBuffer
	//	= owlDeviceBufferCreate(context,
	//	OWL_USER_TYPE(model->links[0]),
	//	model->links.size(),
	//	model->links.data());
	//	OWLVarDecl tubesVars[] = {
	//	{ "links",  OWL_BUFPTR, OWL_OFFSETOF(TubesGeom,links)},
	//	{ "radius", OWL_FLOAT , OWL_OFFSETOF(TubesGeom,radius)},
	//	{ /* sentinel to mark end of list */ }
	//	};

	//	OWLGeomType tubesType
	//	= owlGeomTypeCreate(context,
	//						OWL_GEOMETRY_USER,
	//						sizeof(TubesGeom),
	//						tubesVars,-1);
	//	owlGeomTypeSetBoundsProg(tubesType,module,
	//							"tubes_bounds");
	//	owlGeomTypeSetIntersectProg(tubesType,0,module,
	//								"tubes_intersect");
	//	owlGeomTypeSetClosestHit(tubesType,0,module,
	//							"tubes_closest_hit");

	//	OWLGeom geom = owlGeomCreate(context,tubesType);
	//	owlGeomSetPrimCount(geom,model->links.size());
	//  
	//	owlGeomSetBuffer(geom,"links",linkBuffer);
	//	owlGeomSet1f(geom,"radius",model->radius);
	//  
	//	this->world = owlUserGeomGroupCreate(context,1,&geom);
	//	owlBuildPrograms(context);
	//	owlGroupBuildAccel(this->world);
	//}

	void OWLTubes::render()
	{
		owlRayGenLaunch2D(rayGen, fbSize.x, fbSize.y);
	}

	void OWLTubes::resizeFrameBuffer(const vec2i &newSize, uint32_t* colorbuf)
	{
		fbSize = newSize;
		if (!accumBuffer)
			accumBuffer = owlDeviceBufferCreate(context, OWL_FLOAT4, fbSize.x*fbSize.y, nullptr);
		owlBufferResize(accumBuffer, fbSize.x*fbSize.y);
		owlRayGenSetBuffer(rayGen, "accumBuffer", accumBuffer);
		owlRayGenSet1i(rayGen, "deviceCount", owlGetDeviceCount(context));

		owlRayGenSet1ul(rayGen, "colorBuffer", (uint64_t)colorbuf);
		owlRayGenSet2i(rayGen, "fbSize", fbSize.x, fbSize.y);
	}

	void OWLTubes::updateBgColors(const vec3f &top, const vec3f &bottom)
	{
		owlMissProgSet3f(missProg, "topColor", { top.x, top.y, top.z });
		owlMissProgSet3f(missProg, "bottomColor", { bottom.x, bottom.y, bottom.z });
		owlBuildSBT(context);
	}

	void OWLTubes::updateFrameState(FrameState &fs)
	{
		owlBufferUpload(frameStateBuffer, &fs);
	}

	void OWLTubes::updatevoltagesColor(float* primColorsData)
	{
		owlBufferUpload(voltagesColorBuffer, primColorsData);
	}

	void OWLTubes::updatevoltagesDeriv(float* derivatives)
	{
		owlBufferUpload(voltagesDerivBuffer, derivatives);
	}

}
