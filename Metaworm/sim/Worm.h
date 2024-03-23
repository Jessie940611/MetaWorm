#ifndef __WORM_H__
#define __WORM_H__
#include "fem/World.h"
#include "fem/Mesh/MeshHeaders.h"
#include "Muscle.h"
#include "util/ArgParser.h"

class Worm
{
public:
	Worm(const double& muscle_stiffness,const double& youngs_modulus,const double& poisson_ratio);

	void Initialize(FEM::World* world);
	void ReadMetaData(const std::string& path);
	void ParseArgs(const std::shared_ptr<cArgParser>& parser);
	void SetMesh(const std::string& path, const Eigen::Affine3d& T);
	void MakeMuscles(const std::string& path, const double& gamma);

	void SetActivationLevels(const Eigen::VectorXd& actions, const unsigned int& phase);
	void SetActivationLevels(const unsigned int& phase);
	void SetActivationLevelsDirectly(const Eigen::VectorXd &activations);
	void GetMuscleActions(Eigen::VectorXd& muscle_actions);
	void SetKey(std::string filename);
	void SetKey();
	void ReadC302(std::string filename);

	void GetMuscleActivations(Eigen::VectorXd& activations);
	void WriteMuscleSignal();							
	void WriteMesh(const std::string& path_file, FEM::World *world);				

	Eigen::Matrix3d GetReferenceRotation(bool type, const Eigen::VectorXd& x);
	Eigen::Matrix3d GetInitReferenceRotation() { return mInitReferenceMatrix; }
	void SetInitReferenceRotation(const Eigen::VectorXd& x);
	void SetInitReferenceSamplingStates(const Eigen::VectorXd &x);
	void SetInitReferenceVertices(const Eigen::VectorXd &x);

	void SetWormStates(const std::string& path_file, FEM::World *world);

	const std::vector<Eigen::Vector3i>& GetContours(){return mContours;};
	const std::vector<Eigen::Vector4i>& GetTets() { return mMesh->GetTetrahedrons(); }

	void SetVertexNormal();
	const Eigen::VectorXd& GetVertexNormal(){return mVertexNormal;};

	const std::vector<Muscle*>& GetMuscles() {return mMuscles;};
	int GetNumMuscleSegments() { return mTotalMuscleSegments; };

	Eigen::VectorXd ComputeDragForces(FEM::World* world);
	Eigen::Affine3d EvalBodyTransform(const Eigen::VectorXd &x, const Eigen::VectorXd &y);

	Eigen::Vector3d GetUpVector(const Eigen::VectorXd& x);
	Eigen::Vector3d GetForwardVector(const Eigen::VectorXd& x);	

	const int& GetCenterIndex() {return mCenterIndex;};
	const int& GetCOMIndex() { return mCOMIndex; };
	const std::vector<int>& GetEndEffectorIndex() {return mEndEffectorIndex;};
	const std::vector<int>& GetSamplingIndex() {return mSamplingIndex;};
	const std::vector<Eigen::Vector3d>& GetSamplingReference() { return mSamplingReference; }
	const Eigen::VectorXd& GetVerticesReference() { return mVerticesReference; }
	const Eigen::Vector3i GetLocalContourIndex() { return mLocalContourIndex; }

	bool IsEndEffector(int idx);

private:
	double 									mMuscleStiffness;
	double 									mYoungsModulus;
	double									mPoissonRatio;

	FEM::Mesh*								mMesh;
	Eigen::Affine3d							mScalingMatrix;

	std::vector<FEM::Constraint*>			mConstraints;

	std::vector<Muscle*>					mMuscles;

	std::vector<Eigen::Vector3i> 			mContours;

	int                                     mTotalMuscleSegments;
	int 									mCenterIndex;
	int										mCOMIndex;
	std::vector<int> 						mEndEffectorIndex;
	std::vector<int> 						mSamplingIndex;
	std::vector<Eigen::Vector3d>			mSamplingReference;
	Eigen::VectorXd							mVerticesReference;
	Eigen::Vector3i							mLocalContourIndex;

	Eigen::VectorXd							mVertexNormal;

	Eigen::Matrix3d 						mInitReferenceMatrix;
};
#endif