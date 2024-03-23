#ifndef __MUSCLE_H__
#define __MUSCLE_H__

#include "MuscleSegment.h"
#include "fem/Mesh/MeshHeaders.h"

class Muscle
{
public:
	Muscle(int num_sampling, std::vector<Eigen::Vector3d> point_list);

	void Initialize(FEM::Mesh* mesh, double muscle_stiffness, std::string name = "");
	void Reset();

	const std::vector<MuscleSegment*>& GetSegments() {return mSegments;};
	const unsigned int GetNumSampling() { return mNumSampling; };

	const std::vector<Eigen::Vector3d>& GetStarts() {return mStarts;};	
	const std::vector<Eigen::Vector3d>& GetEnds() {return mEnds;};	

	const Eigen::VectorXd& GetActions() {return mActions;};
	Eigen::VectorXd GetActionUpperBound() {return mActionUpperBound;};
	Eigen::VectorXd GetActionLowerBound() {return mActionLowerBound;};

	void SetKey(const Eigen::VectorXd& key);
	void SetActivationLevels(const Eigen::VectorXd& action, const int& phase);
	void SetActivationLevels(const int& phase);
	void SetActivationLevelsDirectly(const Eigen::VectorXd & activationLevels);

	Eigen::VectorXd GetActivationLevels(){ return mActivationLevels; };
	std::vector<Eigen::VectorXd>& GetC302Activations() { return mC302Activations; }

public:
	struct Statistics 
	{
		double minD, maxD, avgD;
	} mStats;

private:	
	unsigned int 					mNumSampling;
	std::vector<MuscleSegment*> 	mSegments;

	Eigen::VectorXd 				mActivationLevels;
	std::vector<Eigen::VectorXd>    mC302Activations;

	std::vector<Eigen::Vector3d> 	mStarts;
	std::vector<Eigen::Vector3d> 	mEnds;

	Eigen::VectorXd					mActions;
	Eigen::VectorXd 				mActionUpperBound;	
	Eigen::VectorXd 				mActionLowerBound;

	Eigen::VectorXd					mKey;

	std::string 					mName;
};

bool isInTetra(const Eigen::Vector3d& p0,const Eigen::Vector3d& p1,const Eigen::Vector3d& p2,const Eigen::Vector3d& p3,const Eigen::Vector3d& start,const Eigen::Vector3d& end);
Eigen::Matrix3d GetDm(const Eigen::Vector3d& p0,const Eigen::Vector3d& p1,const Eigen::Vector3d& p2,const Eigen::Vector3d& p3);
bool isLineTriangleIntersect(const Eigen::Vector3d& p0,const Eigen::Vector3d& p1,const Eigen::Vector3d& p2,const Eigen::Vector3d& start,const Eigen::Vector3d& end);

#endif