// ======================================================================== //
// Copyright 2018-2019 The Contributors                                            //
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

#define TINYOBJLOADER_IMPLEMENTATION
#include "external/tiny_obj_loader.h"

#include "samples/common/3rdParty/stb/stb_image.h"

#include "Triangles.h"
#include "profiler.h"
//std
#include <set>

namespace std {
	inline bool operator<(const tinyobj::index_t &a,
		const tinyobj::index_t &b)
	{
		if (a.vertex_index < b.vertex_index) return true;
		if (a.vertex_index > b.vertex_index) return false;

		if (a.normal_index < b.normal_index) return true;
		if (a.normal_index > b.normal_index) return false;

		if (a.texcoord_index < b.texcoord_index) return true;
		if (a.texcoord_index > b.texcoord_index) return false;

		return false;
	}
}

/*! \namespace triangles --- orginally from Optix Siggraph Course by Ingo */

namespace rtn {
	static std::string getExt(const std::string &fileName)
	{
		int pos = fileName.rfind('.');
		if (pos == fileName.npos)
			return "";
		return fileName.substr(pos);
	}

	/*! find vertex with given position, normal, texcoord, and return
		its vertex ID, or, if it doesn't exit, add it to the mesh, and
		its just-created index */
	int addVertex(TriangleMesh *mesh,
		tinyobj::attrib_t &attributes,
		const tinyobj::index_t &idx,
		std::map<tinyobj::index_t, int> &knownVertices,
		vec3f default_color)
	{
		if (knownVertices.find(idx) != knownVertices.end())
			return knownVertices[idx];

		const vec3f *vertex_array = (const vec3f*)attributes.vertices.data();
		const vec3f* color_array = (const vec3f*)attributes.colors.data();
		const vec3f *normal_array = (const vec3f*)attributes.normals.data();
		const vec2f *texcoord_array = (const vec2f*)attributes.texcoords.data();

		int newID = (int)mesh->vertex.size();
		knownVertices[idx] = newID;

		mesh->vertex.push_back(vertex_array[idx.vertex_index]);
		if (color_array != nullptr)
		{
			static bool first = false;
			if (!first)
			{
				first = true;
				std::cout << "color array exsits~" << std::endl;
				//std::cout << color_array[idx.vertex_index] << std::endl;         
			}
			// Lei : the tiny-obj loader will automatically create a color array with all [1,1,1]s.
			// Nate: this default vertex colors behavior can be disabled when calling LoadOBJ. 
			// if(color_array[idx.vertex_index] == vec3f(1.0)) 
			//   mesh->color.push_back(vec3f(58/255.0f, 39.0/255.0f, 36/255.0f));
			// else 
			mesh->color.push_back(color_array[idx.vertex_index]);
		}
		else
			mesh->color.push_back(default_color);

		if (idx.normal_index >= 0) {
			while (mesh->normal.size() < mesh->vertex.size())
				mesh->normal.push_back(normal_array[idx.normal_index]);
		}
		if (idx.texcoord_index >= 0) {
			while (mesh->texcoord.size() < mesh->vertex.size())
				mesh->texcoord.push_back(texcoord_array[idx.texcoord_index]);
		}

		return newID;
	}

	/*! load a texture (if not already loaded), and return its ID in the
		model's textures[] vector. Textures that could not get loaded
		return -1 */
	int loadTexture(Triangles *model,
		std::map<std::string, int> &knownTextures,
		const std::string &inFileName,
		const std::string &modelPath)
	{
		if (inFileName == "")
			return -1;

		if (knownTextures.find(inFileName) != knownTextures.end())
			return knownTextures[inFileName];

		std::string fileName = inFileName;

#ifdef WIN32  // Only a temp fix. should do the "get full path" here.

		if (modelPath != "")
			fileName = modelPath + "/" + fileName;
#else
		// first, fix backspaces:
		for (auto& c : fileName)
			if (c == '\\') c = '/';
		fileName = modelPath + "/" + fileName;
#endif // WIN32

		vec2i res;
		int   comp;
		unsigned char* image = stbi_load(fileName.c_str(),
			&res.x, &res.y, &comp, STBI_rgb_alpha);
		int textureID = -1;
		if (image) {
			textureID = (int)model->textures.size();
			Texture *texture = new Texture;
			texture->resolution = res;
			texture->pixel = (uint32_t*)image;

			/* iw - actually, it seems that stbi loads the pictures
				 mirrored along the y axis - mirror them here */
			for (int y = 0; y < res.y / 2; y++) {
				uint32_t *line_y = texture->pixel + y * res.x;
				uint32_t *mirrored_y = texture->pixel + (res.y - 1 - y) * res.x;
				int mirror_y = res.y - 1 - y;
				for (int x = 0; x < res.x; x++) {
					std::swap(line_y[x], mirrored_y[x]);
				}
			}

			model->textures.push_back(texture);
		}
		else {
			std::cout << OWL_TERMINAL_RED
				<< "Could not load texture from " << fileName << "!"
				<< OWL_TERMINAL_DEFAULT << std::endl;
		}

		knownTextures[inFileName] = textureID;
		return textureID;
	}

	// Only works with the quad obj for now. 
	Triangles::SP Triangles::load(std::vector<std::string> objFiles, vec3f default_color)
	{
		utility::CPUProfiler profiler("Load Triangle");
		std::cout << OWL_TERMINAL_GREEN
			<< "#try to load a triangle mesh : "
			<< objFiles[0]
			<< OWL_TERMINAL_DEFAULT
			<< std::endl;

		Triangles::SP triModel = std::make_shared<Triangles>();
		Triangles* model = triModel.get();

		uint32_t idx_offset = 0;
		for (uint32_t oid = 0; oid < objFiles.size(); ++oid) {
			if (getExt(objFiles[oid]) == ".obj") {
				const std::string modelDir
					= objFiles[oid].substr(0, objFiles[oid].rfind('/') + 1);

				tinyobj::attrib_t attributes;
				std::vector<tinyobj::shape_t> shapes;
				std::vector<tinyobj::material_t> materials;
				std::string err = "";

				bool readOK
					= tinyobj::LoadObj(&attributes,
						&shapes,
						&materials,
						&err,
						&err,
						objFiles[oid].c_str(),
						modelDir.c_str(),
						/* triangulate */true,
						/* default vertex colors fallback*/ false);

				if (!readOK) {
					throw std::runtime_error("Could not read OBJ model from " + objFiles[oid] + " : " + err);
				}
				if (materials.empty()) {
					std::cout << "could not parse materials ..." << std::endl;
				}
				std::cout << "Done loading obj file - found " << shapes.size() << " shapes with " << materials.size() << " materials" << std::endl;

				for (int shapeID = 0; shapeID < (int)shapes.size(); shapeID++) {
					tinyobj::shape_t& shape = shapes[shapeID];
					std::set<int> materialIDs;
					for (auto faceMatID : shape.mesh.material_ids)
						materialIDs.insert(faceMatID);
					std::map<tinyobj::index_t, int> knownVertices;
					std::map<std::string, int>      knownTextures;
					for (int materialID : materialIDs) {
						TriangleMesh* mesh = new TriangleMesh;
						for (int faceID = 0; faceID < shape.mesh.material_ids.size(); faceID++) {
							if (shape.mesh.material_ids[faceID] != materialID) continue;
							if (shape.mesh.num_face_vertices[faceID] != 3)
								throw std::runtime_error("not properly tessellated");
							tinyobj::index_t idx0 = shape.mesh.indices[3 * faceID + 0];
							tinyobj::index_t idx1 = shape.mesh.indices[3 * faceID + 1];
							tinyobj::index_t idx2 = shape.mesh.indices[3 * faceID + 2];

							vec3i idx(addVertex(mesh, attributes, idx0, knownVertices, default_color),
								addVertex(mesh, attributes, idx1, knownVertices, default_color),
								addVertex(mesh, attributes, idx2, knownVertices, default_color));
							mesh->index.push_back(idx);
							if (materialID < 0) {
								mesh->diffuse = vec3f(1, 0, 0);
								mesh->diffuseTextureID = -1;
							}
							else {
								mesh->diffuse = (const vec3f&)materials[materialID].diffuse;
								mesh->diffuseTextureID = loadTexture(model,
									knownTextures,
									materials[materialID].diffuse_texname,
									modelDir);
							}
						}

						if (mesh->vertex.empty())
							delete mesh;
						else {
							// just for sanity's sake:
							if (mesh->texcoord.size() > 0)
								mesh->texcoord.resize(mesh->vertex.size());
							// just for sanity's sake:
							if (mesh->normal.size() > 0)
								mesh->normal.resize(mesh->vertex.size());

							for (auto idx : mesh->index) {
								if (idx.x < 0 || idx.x >= mesh->vertex.size() ||
									idx.y < 0 || idx.y >= mesh->vertex.size() ||
									idx.z < 0 || idx.z >= mesh->vertex.size())
								{
									PING; PRINT(idx); PRINT(mesh->vertex.size());
								}
							}
							model->meshes.push_back(mesh);
						}
					}
				}

				auto v_num = 0;
				auto f_num = 0;
				for (auto mesh : model->meshes) {
					v_num += mesh->vertex.size();
					f_num += mesh->index.size();
					for (auto vtx : mesh->vertex)
						model->bounds.extend(vtx);
				}

				std::cout << OWL_TERMINAL_GREEN
					<< "#triangle mesh : "
					<< v_num << " vertices,"
					<< f_num << " faces"
					<< OWL_TERMINAL_DEFAULT
					<< std::endl;

				// idx_offset += mesh->vertex.size();
			}
			else if (getExt(objFiles[oid]) == ".tris" || getExt(objFiles[oid]) == ".fro") {
				std::ifstream in(objFiles[oid]);
				if (!in.good() || in.eof())
					continue;

				TriangleMesh* mesh = new TriangleMesh;

				int numVerts;
				in.read((char *)&numVerts, sizeof(numVerts));
				if (!in.good() || in.eof())
					continue;
				mesh->vertex.resize(numVerts);
				in.read((char *)mesh->vertex.data(), numVerts * sizeof(vec3f));

				int numTris;
				in.read((char *)&numTris, sizeof(numTris));
				mesh->index.resize(numTris);
				in.read((char *)mesh->index.data(), numTris * sizeof(vec3i));

#if 1
				// Landing gear?
				//remap_from 0 0 0 131071 131071 65535
				//remap_to -16 -16 -.1     16 16 16

				for (auto& v : mesh->vertex) {
					v += vec3f(16.f, 16.f, .1f);
					v /= vec3f(32.f, 32.f, 16.1f);
					v *= vec3f(131071.f, 131071.f, 65535.f);
				}
#endif

				mesh->diffuse = vec3f(.8f);
				mesh->diffuseTextureID = -1;

				box3f bounds;
				for (auto idx : mesh->index) {
					if (idx.x < 0 || idx.x >= mesh->vertex.size())
						throw std::runtime_error("broken triangle model");
					bounds.extend(mesh->vertex[idx.x]);
					if (idx.y < 0 || idx.y >= mesh->vertex.size())
						throw std::runtime_error("broken triangle model");
					bounds.extend(mesh->vertex[idx.y]);
					if (idx.z < 0 || idx.z >= mesh->vertex.size())
						throw std::runtime_error("broken triangle model");
					bounds.extend(mesh->vertex[idx.z]);
				}
				model->bounds.extend(bounds); std::cout << "bounds? " << bounds << '\n';
				model->meshes.push_back(mesh);
			}
		}

		return triModel;
	}

	Triangles::SP Triangles::loadQuadObj(const std::string& objFile)
	{
	std::cout << OWL_TERMINAL_GREEN
				<< "#try to load a triangle mesh : "
				<< objFile
				<< OWL_TERMINAL_DEFAULT
				<< std::endl;

	Triangles::SP triModel = std::make_shared<Triangles>();
	Triangles* model = triModel.get();

	const std::string modelDir
		= objFile.substr(0, objFile.rfind('/') + 1);

	std::ifstream in(objFile);

	std::string line;
	char Ty;
	int MAX_NUM_TRIANGLES = 500000000;

	std::vector<vec3f> vertices;
	std::vector<vec3f> colors;
	std::vector<vec3i> indices;

	while (std::getline(in, line))
		{
		if (line.find_first_of("#") != std::string::npos)
			continue;
		if (line.find_first_of("O") != std::string::npos)
			continue;
		std::istringstream iss(line);
		if (line.find_first_of("v") != std::string::npos)
			{
			vec3f pt;
			iss >> Ty >> pt.x >> pt.y >> pt.z;
			vertices.push_back(pt);
			vec3f col(1.0,0.0,1.0);
			if (!iss.eof()) {                  
				iss >> col.x >> col.y >> col.z;                  
			}
			colors.push_back(col);
			}
		else if (line.find_first_of("f") != std::string::npos)
			{
			if (indices.size() >= MAX_NUM_TRIANGLES) {
				static bool warned = false;
				if (!warned) {
				std::cout << "WARNING: skipping more than " << MAX_NUM_TRIANGLES << " triangles!!!" << std::endl;
				warned = true;
				}
				continue;
			}
			int a, b, c, d;//quad mesh.
			iss >> Ty >> a >> b >> c >> d;
			indices.push_back(vec3i(a - 1, b - 1, c - 1)); // start with 1
			indices.push_back(vec3i(a - 1, c - 1, d - 1));
			}
		}

	TriangleMesh* mesh = new TriangleMesh;      
	mesh->vertex = vertices;
	mesh->index = indices;
	mesh->color = colors;
	model->meshes.push_back(mesh);

	for (auto mesh : model->meshes)
		for (auto vtx : mesh->vertex)
		model->bounds.extend(vtx);

	std::cout << OWL_TERMINAL_GREEN
				<< "#triangle mesh : "
				<< mesh->vertex.size() << " vertices,"
				<< mesh->index.size() << " faces"
				<< OWL_TERMINAL_DEFAULT
				<< std::endl;

	PRINT(model->bounds);
	return triModel;
	}

}


