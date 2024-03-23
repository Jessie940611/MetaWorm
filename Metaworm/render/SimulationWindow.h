#ifndef __SIMULATION_WINDOW_H__
#define __SIMULATION_WINDOW_H__

#include "GLWindow.h"
#include "DrawFunctions.h"
#include "DrawPrimitives.h"
#include "Environment.h"
#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

namespace p = boost::python;
namespace np = boost::python::numpy;

enum MetaController
{
	MC_Default_Key = 0,
	MC_ANN 		   = 1,
	MC_C302        = 2,
};

class SimulationWindow : public GLWindow
{
public:
	SimulationWindow();
	SimulationWindow(std::string network_name);

	const std::vector<Eigen::Vector3d>& GetInitPostions() const {return mInitPositions;}
	const std::vector<Eigen::Vector4d>& GetInitRotation() const {return mInitRotations;}
	const std::vector<Environment*>& GetmWorms() const {return mWorms;}

	int fbWidth, fbHeight;
	bool autoScreenShot = false;
	std::vector<Environment *>* GetEnvs() { return &mWorms; }
	void AddWorm(Eigen::Vector3d pos, Eigen::Vector3d dir, Eigen::Vector4d color);
	void AddFood(Eigen::Vector3d foodPos) { mFoodPoses.push_back(foodPos); }
	void SaveWormStates(int idx, const std::string& path) { mWorms[idx]->SaveWormStates(path); }
	void RestoreWormStates(int idx, const std::string &path) { mWorms[idx]->RestoreWormStates(path); }

	Eigen::Vector3d WormLocalToWorld(int idx, Eigen::Vector3d position);
	Eigen::Vector3d WorldToWormLocal(int idx, Eigen::Vector3d position);
	
	void SetWorldFloorColor(Eigen::Vector4d c) { mWorldFloorColor = c; }
	void SetWorldFogColor(Eigen::Vector4d c) { mWorldFogColor = c; }
	void SetFogParams(int mode, Eigen::Vector3d params);

//protected:
	void Display() override;
	void DisplayWorm(Environment *mEnvironment, std::deque<Eigen::Vector3d> &mTrajectory, std::deque<Eigen::Vector3d> &mRefCoordTrajectory, Eigen::Vector4d color, int wormIdx);

	void Keyboard(unsigned char key,int x,int y) override;
	void Mouse(int button, int state, int x, int y) override;
	void Motion(int x, int y) override;
	void Reshape(int w, int h) override;
	void Timer(int value) override;
	void initLights(Eigen::Vector4d fogColor, int fogMode, Eigen::Vector3d fogParams);

	Eigen::VectorXd GetActionFromNN(const Eigen::VectorXd& state);
	void ScreenShotAndSaveImage(const std::string &path);

protected:
	bool 				mPlay;
	int 				mCurFrame;
	int 				mTotalFrame;
	int 				mFocusWormId;
	double 				mElapsedTime;

	// Environment* 		mEnvironment;
	std::vector<Environment*> 		mWorms;
	std::vector<Eigen::Vector3d>	mInitPositions;
	std::vector<Eigen::Vector3d>	mInitForwards;
	std::vector<Eigen::Vector3d>	mFoodPoses;
	std::vector<Eigen::Vector4d> 	mInitRotations;
	std::vector<Eigen::Vector4d>	mWormColors;
	Eigen::Vector4d					mWorldFloorColor;
	Eigen::Vector4d					mWorldFogColor;

	int mFogMode;
	Eigen::Vector3d mFogParams;

	Eigen::VectorXd		mActions;

	p::object mm, mns, sys_module, env_module, wormInEnv;
	bool mbDrawWorld;
	bool mbDrawCreature;
	bool mbDrawTets;
	bool mbDrawReferenceTets;
	bool mbDrawMuscles;
	bool mbDrawArrows;
	bool mbDrawSamplingPoints;
	bool mbDrawLocalCoord;
	bool mbDraw2DGraph;
	bool mbDrawAxis;
	bool mbDrawPath;
	bool mDrawFood;
	bool mStartScreenShot;

	std::vector<std::deque<Eigen::Vector3d>> mTrajectories;
	std::vector<std::deque<Eigen::Vector3d>> mRefCoordTrajectories;
	MetaController mController;

	Eigen::VectorXd transRefT2;
	int mFrameCount = 0;
};

#endif