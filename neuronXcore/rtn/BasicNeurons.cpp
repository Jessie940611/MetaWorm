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

#include "BasicNeurons.h"
#include "profiler.h"

namespace rtn {

	using device::TubesGeom;
	using device::TrianglesGeomData;
	using device::QuadGeom;
	using device::QuadLightGeom;
	using device::CurveGeom;

	extern "C" const char embedded_neurons_programs[];

	BasicNeurons::BasicNeurons()
	{
		module = owlModuleCreate(context, embedded_neurons_programs);
	}

	/*! the setup code for the 'all tubes are user-geom cylinders'
		variant. todo: add one with using instance transforms */
	void BasicNeurons::build(Tubes::SP rtn, Triangles::SP triangles, std::shared_ptr<Curves> curves, const QuadLightGeom& light)
	{
		utility::CPUProfiler profiler("BasicNeurons::build");

		std::vector<OWLGroup> rootGroups;

		if (rtn)
		{
			std::cout << "building tubes" << std::endl;
			OWLGroup tubesGroup = buildTubes(rtn);
			if (tubesGroup) rootGroups.push_back(tubesGroup);

			std::cout << "building quad" << std::endl;
			OWLGroup quadGroup = buildQuad(rtn);
			if (quadGroup) rootGroups.push_back(quadGroup);
		}

		if (triangles)
		{
			std::cout << "building triangles" << std::endl;
			OWLGroup triangleGroup = buildTriangles(triangles);
			if (triangleGroup) rootGroups.push_back(triangleGroup);
		}

		if (curves)
		{
			std::cout << "building curves" << std::endl;
			OWLGroup curveGroup = buildCurves(curves);
			if (curveGroup) rootGroups.push_back(curveGroup);
		}

		std::cout << "building quadlight" << std::endl;
		OWLGroup quadLightGroup = buildQuadLight(light);
		if (quadLightGroup) rootGroups.push_back(quadLightGroup);

		this->world = owlInstanceGroupCreate(context, rootGroups.size());
		for (int i = 0; i < rootGroups.size(); i++)
			owlInstanceGroupSetChild(this->world, i, rootGroups[i]);

		owlGroupBuildAccel(this->world);
	}

	OWLGroup BasicNeurons::buildTubes(Tubes::SP rtn)
	{
		std::cout << "initializing voltage sequence." << std::endl;
		std::vector<float> initVoltageSequence;
		for (int i = 0; i < rtn->voltages.size(); i++)
		{
			initVoltageSequence.push_back(rtn->voltages[i][0]);
		}
		std::vector<float> initZeroData(initVoltageSequence.size(), 0.f);
		voltagesColorBuffer = owlDeviceBufferCreate(context, OWL_USER_TYPE(float), initVoltageSequence.size(), initVoltageSequence.data());;
		voltagesDerivBuffer = owlDeviceBufferCreate(context, OWL_USER_TYPE(float), initZeroData.size(), initZeroData.data());
		owlRayGenSetBuffer(rayGen, "voltagesColor", voltagesColorBuffer);
		owlRayGenSetBuffer(rayGen, "voltagesDeriv", voltagesDerivBuffer);

		vertices
			= owlDeviceBufferCreate(context,
				OWL_USER_TYPE(rtn->vertices[0]),
				rtn->vertices.size(),
				rtn->vertices.data());

		endPointRadii
			= owlDeviceBufferCreate(context,
				OWL_USER_TYPE(rtn->endPointRadii[0]),
				rtn->endPointRadii.size(),
				rtn->endPointRadii.data());

		prevPrims
			= owlDeviceBufferCreate(context,
				OWL_USER_TYPE(rtn->prevPrims[0]),
				rtn->prevPrims.size(),
				rtn->prevPrims.data());

		/*postPrims
			= owlDeviceBufferCreate(context,
				OWL_USER_TYPE(rtn->postPrims[0]),
				rtn->postPrims.size(),
				rtn->postPrims.data());*/

		types
			= owlDeviceBufferCreate(context,
				OWL_USER_TYPE(rtn->types[0]),
				rtn->types.size(),
				rtn->types.data());
		
		dendriteScale
			= owlDeviceBufferCreate(context,
				OWL_USER_TYPE(rtn->dendriteScale),
				1,
				&rtn->dendriteScale);
		
		synapseScale
			= owlDeviceBufferCreate(context,
				OWL_USER_TYPE(rtn->synapseScale),
				1,
				&rtn->synapseScale);

		OWLVarDecl tubesVars[] = {
			{ "vertices",      OWL_BUFPTR, OWL_OFFSETOF(TubesGeom,vertices)},
			{ "endPointRadii", OWL_BUFPTR, OWL_OFFSETOF(TubesGeom,endPointRadii)},
			{ "prevPrims",     OWL_BUFPTR, OWL_OFFSETOF(TubesGeom,prevPrims)},
			{ "types",         OWL_BUFPTR, OWL_OFFSETOF(TubesGeom,types)},
			{ "dendriteScale", OWL_BUFPTR, OWL_OFFSETOF(TubesGeom,dendriteScale)},
			{ "synapseScale",  OWL_BUFPTR, OWL_OFFSETOF(TubesGeom,synapseScale)},
			{ /* sentinel to mark end of list */ }
		};

		OWLGeomType tubesType = owlGeomTypeCreate(context, OWL_GEOMETRY_USER, sizeof(TubesGeom), tubesVars, -1);
		owlGeomTypeSetBoundsProg(tubesType, module, "BasicNeurons");
		owlGeomTypeSetIntersectProg(tubesType, 0, module, "BasicNeurons");
		owlGeomTypeSetClosestHit(tubesType, 0, module, "BasicNeurons");

		OWLGeom geom = owlGeomCreate(context, tubesType);
		owlGeomSetPrimCount(geom, rtn->vertices.size());

		printf("build tubes: primitives: %d\n", rtn->vertices.size());

		owlGeomSetBuffer(geom, "vertices", vertices);
		owlGeomSetBuffer(geom, "endPointRadii", endPointRadii);
		owlGeomSetBuffer(geom, "prevPrims", prevPrims);
		owlGeomSetBuffer(geom, "types", types);
		owlGeomSetBuffer(geom, "dendriteScale", dendriteScale);
		owlGeomSetBuffer(geom, "synapseScale", synapseScale);
		owlBuildPrograms(context);

		OWLGroup tubesGroup = owlUserGeomGroupCreate(context, 1, &geom);
		owlGroupBuildAccel(tubesGroup);
		return tubesGroup;
	}

	OWLGroup BasicNeurons::buildQuad(Tubes::SP rtn)
	{
		box3f neuron_bound = rtn->bounds;
		vec3f model_span = neuron_bound.span();
		float ground_height = neuron_bound.lower.y - model_span.y / 100.f;


		OWLVarDecl vars[] = {
			{ "p",      OWL_FLOAT3, OWL_OFFSETOF(QuadGeom,p)},
			{ "v1",     OWL_FLOAT3, OWL_OFFSETOF(QuadGeom,v1)},
			{ "v2",     OWL_FLOAT3, OWL_OFFSETOF(QuadGeom,v2)},
			{ /* sentinel to mark end of list */ }
		};
		OWLGeomType quadType = owlGeomTypeCreate(context, OWL_GEOMETRY_USER, sizeof(QuadGeom), vars, -1);
		owlGeomTypeSetBoundsProg(quadType, module, "Ground");
		owlGeomTypeSetIntersectProg(quadType, 0, module, "Ground");
		owlGeomTypeSetClosestHit(quadType, 0, module, "Ground");


		vec2f A(neuron_bound.lower.x, neuron_bound.lower.z), B(neuron_bound.lower.x, neuron_bound.upper.z), C(neuron_bound.upper.x, neuron_bound.upper.z);
		vec2f D = (A + B + C) * 0.333f;

		A = A + (A - D) * 1.5f;
		B = B + (B - D) * 1.5f;
		C = C + (C - D) * 1.5f;

		/* hide the ground*/
		A = B = C;

		OWLGeom geom = owlGeomCreate(context, quadType);
		owlGeomSetPrimCount(geom, 1);
		std::cout << neuron_bound << std::endl;
		owlGeomSet3f(geom, "p", { A.x , ground_height, A.y });
		owlGeomSet3f(geom, "v1", { B.x , ground_height, B.y });
		owlGeomSet3f(geom, "v2", { C.x , ground_height, C.y });
		owlBuildPrograms(context);

		OWLGroup quadGroup = owlUserGeomGroupCreate(context, 1, &geom);
		owlGroupBuildAccel(quadGroup);

		std::cout << "built quad" << std::endl;

		return quadGroup;
	}

	OWLGroup BasicNeurons::buildTriangles(Triangles::SP triangles)
	{
		if (triangles == 0) // ...
			return 0;

		OWLVarDecl trianglesGeomVars[] = {
			{ "index",  OWL_BUFPTR, OWL_OFFSETOF(TrianglesGeomData,index)},
			{ "vertex", OWL_BUFPTR, OWL_OFFSETOF(TrianglesGeomData,vertex)},
			{ "color",  OWL_BUFPTR, OWL_OFFSETOF(TrianglesGeomData,color)},
			{ /* sentinel to mark end of list */ }
		};

		OWLGeomType trianglesGeomType
			= owlGeomTypeCreate(context,
				OWL_TRIANGLES,
				sizeof(TrianglesGeomData),
				trianglesGeomVars, -1);

		owlGeomTypeSetClosestHit(trianglesGeomType, 0,
			module, "TriangleMesh");

		std::vector<vec3f> vertices;
		std::vector<vec3f> colors;
		std::vector<vec3i> indices;

		for (auto m : triangles->meshes) {
			int size = vertices.size();

			vertices.reserve(vertices.size() + m->vertex.size());
			colors.reserve(colors.size() + m->color.size());
			vertices.insert(vertices.end(), m->vertex.begin(), m->vertex.end());
			colors.insert(colors.end(), m->color.begin(), m->color.end());

			for (auto id : m->index)
				indices.push_back(vec3i(id) + size);
		}

		int numVertices = vertices.size();
		PRINT(numVertices);

		// ------------------------------------------------------------------
		// triangle mesh
		// ------------------------------------------------------------------
		OWLBuffer vertexBuffer
			= owlDeviceBufferCreate(context, OWL_FLOAT3, vertices.size(), vertices.data());
		OWLBuffer colorBuffer
			= owlDeviceBufferCreate(context, OWL_FLOAT3, colors.size(), colors.data());
		OWLBuffer indexBuffer
			= owlDeviceBufferCreate(context, OWL_INT3, indices.size(), indices.data());

		OWLGeom trianglesGeom
			= owlGeomCreate(context, trianglesGeomType);

		owlTrianglesSetVertices(trianglesGeom, vertexBuffer, vertices.size(), sizeof(vec3f), 0);
		owlTrianglesSetIndices(trianglesGeom, indexBuffer, indices.size(), sizeof(vec3i), 0);
		PRINT(indices.size());

		owlGeomSetBuffer(trianglesGeom, "vertex", vertexBuffer);
		owlGeomSetBuffer(trianglesGeom, "color", colorBuffer);
		owlGeomSetBuffer(trianglesGeom, "index", indexBuffer);

		// ------------------------------------------------------------------
		// the group/accel for that mesh
		// ------------------------------------------------------------------
		OWLGroup triGroup = owlTrianglesGeomGroupCreate(context, 1, &trianglesGeom);
		owlGroupBuildAccel(triGroup);
		PRINT(triGroup);

		std::cout << "built triangles" << std::endl;

		return triGroup;
	}

	OWLGroup BasicNeurons::buildCurves(Curves::SP curves)
	{

		coefficients = owlDeviceBufferCreate(context,
			OWL_USER_TYPE(curves->coefficients[0]),
			curves->coefficients.size(),
			curves->coefficients.data());

		radius = owlDeviceBufferCreate(context,
			OWL_USER_TYPE(curves->radius[0]),
			curves->radius.size(),
			curves->radius.data());

		curveColor = owlDeviceBufferCreate(context,
			OWL_USER_TYPE(curves->curveColor[0]),
			curves->curveColor.size(),
			curves->curveColor.data());

		length = owlDeviceBufferCreate(context,
			OWL_USER_TYPE(curves->length[0]),
			curves->length.size(),
			curves->length.data());

		OWLVarDecl curveVars[] = {
		{ "coefficients",      OWL_BUFPTR, OWL_OFFSETOF(CurveGeom, coefficients)},
		{ "radius",			       OWL_BUFPTR, OWL_OFFSETOF(CurveGeom, radius)},
		{ "curveColor",        OWL_BUFPTR, OWL_OFFSETOF(CurveGeom, curveColor)},
		{ "length",            OWL_BUFPTR, OWL_OFFSETOF(CurveGeom, length)},
		{ /* sentinel to mark end of list */ }
		};


		OWLGeomType curveType = owlGeomTypeCreate(context, OWL_GEOMETRY_USER, sizeof(CurveGeom), curveVars, -1);
		owlGeomTypeSetBoundsProg(curveType, module, "Curves");
		owlGeomTypeSetIntersectProg(curveType, 0, module, "Curves");
		owlGeomTypeSetClosestHit(curveType, 0, module, "Curves");


		OWLGeom geom = owlGeomCreate(context, curveType);
		owlGeomSetPrimCount(geom, curves->coefficients.size() / 4);

		printf("build curves: primitives: %d\n", curves->coefficients.size() / 4);

		owlGeomSetBuffer(geom, "coefficients", coefficients);
		owlGeomSetBuffer(geom, "radius", radius);
		owlGeomSetBuffer(geom, "curveColor", curveColor);
		owlGeomSetBuffer(geom, "length", length);
		owlBuildPrograms(context);

		OWLGroup curveGroup = owlUserGeomGroupCreate(context, 1, &geom);

		owlGroupBuildAccel(curveGroup);

		return curveGroup;
	}

	OWLGroup BasicNeurons::buildQuadLight(const QuadLightGeom& light)
	{
		OWLVarDecl vars[] = {
			{ "p",         OWL_FLOAT3, OWL_OFFSETOF(QuadLightGeom,p)},
			{ "v1",        OWL_FLOAT3, OWL_OFFSETOF(QuadLightGeom,v1)},
			{ "v2",        OWL_FLOAT3, OWL_OFFSETOF(QuadLightGeom,v2)},
			{ "color",     OWL_FLOAT3, OWL_OFFSETOF(QuadLightGeom,color)},
			{ "intensity", OWL_FLOAT, OWL_OFFSETOF(QuadLightGeom,intensity)},
			{ /* sentinel to mark end of list */ }
		};
		OWLGeomType quadType = owlGeomTypeCreate(context, OWL_GEOMETRY_USER, sizeof(QuadLightGeom), vars, -1);
		owlGeomTypeSetBoundsProg(quadType, module, "QuadLight");
		owlGeomTypeSetIntersectProg(quadType, 0, module, "QuadLight");
		owlGeomTypeSetClosestHit(quadType, 0, module, "QuadLight");

		OWLGeom geom = owlGeomCreate(context, quadType);
		owlGeomSetPrimCount(geom, 1);
#if 0
		owlGeomSet3f(geom, "p", { 3430.f,3580.f,2750.f });
		owlGeomSet3f(geom, "v1", { 3430.f,3580.f, -2750.f });
		owlGeomSet3f(geom, "v2", { 3430.f,-3850.f,2750.f });
		owlGeomSet3f(geom, "color", { 1.f,1.f,1.f });
		owlGeomSet1f(geom, "intensity", 1.3f);
#else
		owlGeomSet3f(geom, "p", { light.p.x, light.p.y, light.p.z });
		owlGeomSet3f(geom, "v1", { light.v1.x, light.v1.y, light.v1.z });
		owlGeomSet3f(geom, "v2", { light.v2.x, light.v2.y, light.v2.z });
		owlGeomSet3f(geom, "color", { light.color.x, light.color.y, light.color.z });
		owlGeomSet1f(geom, "intensity", light.intensity);
#endif

		owlBuildPrograms(context);

		OWLGroup quadGroup = owlUserGeomGroupCreate(context, 1, &geom);
		owlGroupBuildAccel(quadGroup);

		std::cout << "built quadlight" << std::endl;

		return quadGroup;
	}

}
