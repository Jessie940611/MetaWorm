#ifndef __DRAW_FUNCTIONS_H__
#define __DRAW_FUNCTIONS_H__

#include "../sim/Worm.h"
#include "../sim/Muscle.h"
#include "DrawPrimitives.h"

namespace GUI
{
	void DrawWorld(Eigen::Vector4d floorColor);
	void DrawCharacter(Worm* pCreature, const Eigen::VectorXd& x, const Eigen::Vector3d& eye, const Eigen::Vector4d& color);
	void DrawMuscles(const std::vector<Muscle*>& muscles, const Eigen::VectorXd& x);
	void DrawActivations(const double& x, const double& y, const double& length, const double& height, const Eigen::Vector3d& color, Muscle* muscle);
	//void DrawActivations(const double& x, const double& y, const double& length, const double& height, Muscle* muscle1, Muscle* muscle2);
	void DrawOBJ();
	void DrawTets(Worm* pCreature, const Eigen::VectorXd& x, const Eigen::Vector4d& color);
	void DrawSamplingPoints(Worm* pCreature, const Eigen::VectorXd& x, const Eigen::VectorXd& v, const Eigen::Matrix3d& r, const Eigen::Vector3d& t);
	void DrawLocalContour(Worm* pCreature, const Eigen::VectorXd& x);
	void DrawTrajectory(const std::deque<Eigen::Vector3d>& traj, const Eigen::Vector4d& color, const double& width);
};
#endif