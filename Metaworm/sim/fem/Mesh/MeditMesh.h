#ifndef __MEDIT_MESH_H__
#define __MEDIT_MESH_H__
#include <Eigen/Core>
#include <Eigen/Geometry>

namespace FEM
{
class Mesh;
class MeditMesh : public Mesh
{
public:
	MeditMesh(const std::string& obj_file, const Eigen::Affine3d& T = Eigen::Affine3d::Identity());	
};
};
#endif