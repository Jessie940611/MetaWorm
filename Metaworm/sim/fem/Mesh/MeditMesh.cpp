#include "Mesh.h"
#include "MeditMesh.h"
#include <fstream>
#include <sstream>
#include <iostream>

using namespace FEM;

inline bool readMESH(FILE *mesh_file, std::vector<Eigen::Vector3d> &V, std::vector<Eigen::Vector4i> &T, std::vector<Eigen::Vector3i> &F)
{
using namespace std;
#ifndef LINE_MAX
#define LINE_MAX 2048
#endif

	char line[LINE_MAX];
	bool still_comments;
	V.clear();
	T.clear();
	F.clear();

	// eat comments at beginning of file
	still_comments = true;
	while (still_comments)
	{
		if (fgets(line, LINE_MAX, mesh_file) == NULL)
		{
			fprintf(stderr, "Error: couldn't find start of .mesh file");
			fclose(mesh_file);
			return false;
		}
		still_comments = (line[0] == '#' || line[0] == '\n');
	}
	char str[LINE_MAX];
	sscanf(line, " %s", str);
	// check that first word is MeshVersionFormatted
	if (0 != strcmp(str, "MeshVersionFormatted"))
	{
		fprintf(stderr,
				"Error: first word should be MeshVersionFormatted not %s\n", str);
		fclose(mesh_file);
		return false;
	}

	int one = -1;
	if (2 != sscanf(line, "%s %d", str, &one))
	{
		// 1 appears on next line?
		fscanf(mesh_file, " %d", &one);
	}
	if (one != 1)
	{
		fprintf(stderr, "Error: second word should be 1 not %d\n", one);
		fclose(mesh_file);
		return false;
	}

	// eat comments
	still_comments = true;
	while (still_comments)
	{
		fgets(line, LINE_MAX, mesh_file);
		still_comments = (line[0] == '#' || line[0] == '\n');
	}

	sscanf(line, " %s", str);
	// check that third word is Dimension
	if (0 != strcmp(str, "Dimension"))
	{
		fprintf(stderr, "Error: third word should be Dimension not %s\n", str);
		fclose(mesh_file);
		return false;
	}
	int three = -1;
	if (2 != sscanf(line, "%s %d", str, &three))
	{
		// 1 appears on next line?
		fscanf(mesh_file, " %d", &three);
	}
	if (three != 3)
	{
		fprintf(stderr, "Error: only Dimension 3 supported not %d\n", three);
		fclose(mesh_file);
		return false;
	}

	// eat comments
	still_comments = true;
	while (still_comments)
	{
		fgets(line, LINE_MAX, mesh_file);
		still_comments = (line[0] == '#' || line[0] == '\n');
	}

	sscanf(line, " %s", str);
	// check that fifth word is Vertices
	if (0 != strcmp(str, "Vertices"))
	{
		fprintf(stderr, "Error: fifth word should be Vertices not %s\n", str);
		fclose(mesh_file);
		return false;
	}

	//fgets(line,LINE_MAX,mesh_file);

	int number_of_vertices;
	if (1 != fscanf(mesh_file, " %d", &number_of_vertices) || number_of_vertices > 1000000000)
	{
		fprintf(stderr, "Error: expecting number of vertices less than 10^9...\n");
		fclose(mesh_file);
		return false;
	}
	// allocate space for vertices
	V.resize(number_of_vertices, Eigen::Vector3d(0, 0, 0));
	int extra;
	for (int i = 0; i < number_of_vertices; i++)
	{
		double x, y, z;
		if (4 != fscanf(mesh_file, " %lg %lg %lg %d", &x, &y, &z, &extra))
		{
			fprintf(stderr, "Error: expecting vertex position...\n");
			fclose(mesh_file);
			return false;
		}
		V[i][0] = x;
		V[i][1] = y;
		V[i][2] = z;
	}

	// eat comments
	still_comments = true;
	while (still_comments)
	{
		fgets(line, LINE_MAX, mesh_file);
		still_comments = (line[0] == '#' || line[0] == '\n');
	}

	sscanf(line, " %s", str);
	// check that sixth word is Triangles
	if (0 != strcmp(str, "Triangles"))
	{
		fprintf(stderr, "Error: sixth word should be Triangles not %s\n", str);
		fclose(mesh_file);
		return false;
	}
	int number_of_triangles;
	if (1 != fscanf(mesh_file, " %d", &number_of_triangles))
	{
		fprintf(stderr, "Error: expecting number of triangles...\n");
		fclose(mesh_file);
		return false;
	}
	// allocate space for triangles
	F.resize(number_of_triangles, Eigen::Vector3i(0, 0, 0));
	// triangle indices
	int tri[3];
	for (int i = 0; i < number_of_triangles; i++)
	{
		if (4 != fscanf(mesh_file, " %d %d %d %d", &tri[0], &tri[1], &tri[2], &extra))
		{
			printf("Error: expecting triangle indices...\n");
			return false;
		}
		for (int j = 0; j < 3; j++)
		{
			F[i][j] = tri[j] - 1;
		}
	}

	// eat comments
	still_comments = true;
	while (still_comments)
	{
		fgets(line, LINE_MAX, mesh_file);
		still_comments = (line[0] == '#' || line[0] == '\n');
	}

	sscanf(line, " %s", str);
	// check that sixth word is Triangles
	if (0 != strcmp(str, "Tetrahedra"))
	{
		fprintf(stderr, "Error: seventh word should be Tetrahedra not %s\n", str);
		fclose(mesh_file);
		return false;
	}
	int number_of_tetrahedra;
	if (1 != fscanf(mesh_file, " %d", &number_of_tetrahedra))
	{
		fprintf(stderr, "Error: expecting number of tetrahedra...\n");
		fclose(mesh_file);
		return false;
	}
	// allocate space for tetrahedra
	T.resize(number_of_tetrahedra, Eigen::Vector4i(0, 0, 0, 0));
	// tet indices
	int a, b, c, d;
	for (int i = 0; i < number_of_tetrahedra; i++)
	{
		if (5 != fscanf(mesh_file, " %d %d %d %d %d", &a, &b, &c, &d, &extra))
		{
			fprintf(stderr, "Error: expecting tetrahedra indices...\n");
			fclose(mesh_file);
			return false;
		}
		T[i][0] = a - 1;
		T[i][1] = b - 1;
		T[i][2] = c - 1;
		T[i][3] = d - 1;
	}
	fclose(mesh_file);
	return true;
}

MeditMesh::MeditMesh(const std::string &path, const Eigen::Affine3d &transform) 
: Mesh()
{
	std::cout << "  Reading MeditMesh " << path << " ...  ";
	FILE *file = nullptr;
	file = fopen(path.c_str(), "r");
	if (nullptr==file)
	{
		std::cout << "Can't read file " << path << std::endl;
		return;
	}
	
	readMESH(file, mVertices, mTetrahedrons, mTriangles);

	std::cout <<"Successed! ";

	for (auto &v : mVertices)
		v = transform * v;

	// compute AABB
 	for (auto &v : mVertices)
	{
		if(v[0]>mBBMax[0]) mBBMax[0] = v[0];
		if(v[1]>mBBMax[1]) mBBMax[1] = v[1];
		if(v[2]>mBBMax[2]) mBBMax[2] = v[2];

		if(v[0]<mBBMin[0]) mBBMin[0] = v[0];
		if(v[1]<mBBMin[1]) mBBMin[1] = v[1];
		if(v[2]<mBBMin[2]) mBBMin[2] = v[2];
	} 

	std::cout << "AABB: [" << mBBMin[0] << " " << mBBMin[1] << " " << mBBMin[2] << "] [" 
	                       << mBBMax[0] << " " << mBBMax[1] << " " << mBBMax[2] << "]" 
				 " Size: [" << mBBMax[0]-mBBMin[0] << " " << mBBMax[1]-mBBMin[1] << " " << mBBMax[2]-mBBMin[2] << "]" << std::endl;
}