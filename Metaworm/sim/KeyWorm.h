#ifndef __KEY_WORM_H__
#define __KEY_WORM_H__

#include "fem/World.h"
#include "fem/Mesh/MeshHeaders.h"
#include "Muscle.h"
#include "util/ArgParser.h"
#include "util/json/json.h"
#include "util/JsonUtil.h"

class KeyWorm
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	struct Params
	{
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW

		Params();

		int mID;
		std::string mCharFile;
		std::string mStateFile;
		std::string mActionFile;
		Eigen::Vector4d mOrigin;
		bool mLoadDrawShapes;
	};

	KeyWorm();

	void Initialize();
	void ParseArgs(const std::shared_ptr<cArgParser>& parser);

	bool LoadActions();		
	void LoadMuscleSignal(const std::string& ms_file);						
	bool LoadStates();	
	void LoadMesh(const std::string& mesh_file);				

	const Eigen::VectorXd GetPos() {return mStatePos.row(mCurrentFrame);};
	const Eigen::VectorXd GetVel() {return mStateVel.row(mCurrentFrame);};
	const Eigen::Vector3d GetCoordVel() {return mCoordVel.row(mCurrentFrame);};
	bool IsMotionOver() const;
	int GetMotionDuration() const;
	int GetPhase() const {return mCurrentFrame;};
	int GetCycle() const;

	const int& GetCenterIndex() {return mCenterIndex;};
	const int& GetCOMIndex() { return mCOMIndex; };
	const std::vector<int>& GetEndEffectorIndex() {return mEndEffectorIndex;};

	int GetNumSamplings() {return mNumSamplings;};
	int GetActionSize() {return mActionSize;};
	int GetStateSize() {return mStateSize;};
	int GetTotalFrames() {return mTotalFrames;};

	void Step();
	void Reset();

private:
	bool LoadJsonActions(const Json::Value& root, Eigen::MatrixXd& out_action);
	bool LoadJsonStates(const Json::Value& root, Eigen::MatrixXd& out_target_vel, Eigen::MatrixXd& out_coord_vel,
						Eigen::MatrixXd& out_pos, Eigen::MatrixXd& out_vel);
	bool ParseFrameJson(const Json::Value& root, Eigen::VectorXd& out_frame);

private:
	Eigen::MatrixXd 						mStatePos;
	Eigen::MatrixXd							mStateVel;
	Eigen::MatrixXd							mTargetVel;
	Eigen::MatrixXd							mCoordVel;
	Eigen::MatrixXd							mAction;

	std::vector<int> 						mEndEffectorIndex;
	int 									mCenterIndex;
	int										mCOMIndex;

	int 									mNumSamplings;
	int 									mTotalFrames;
	int      								mActionSize;
	int 									mStateSize;
	Params 									mParams;
	int                                     mPhase;
	int                                     mCurrentFrame;
};

#endif