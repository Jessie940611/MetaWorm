#include "SimulationWindow.h"
#include "../interact/interact.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

#include "stb_image_write.h"

static int g_fogMode[] = {GL_LINEAR, GL_EXP, GL_EXP2};

SimulationWindow::SimulationWindow()
	: GLWindow(), mCurFrame(0), mTotalFrame(0), mElapsedTime(0.0), mFocusWormId(0), mPlay(false), mController(MetaController::MC_Default_Key),
	  mbDrawTets(false), mbDrawCreature(true), mbDrawMuscles(true), mbDrawArrows(false), mbDrawSamplingPoints(false), mbDrawLocalCoord(false),
	  mbDrawWorld(true), mbDraw2DGraph(true), mbDrawAxis(false), mbDrawPath(true), mbDrawReferenceTets(false), mDrawFood(true), mStartScreenShot(false)
{
	mDisplayTimeout = 33;
	mWorldFogColor = Eigen::Vector4d(.7, .8, .8, 1.0);
	mFogMode = GL_EXP;
	mFogParams = Eigen::Vector3d(0.05, 4.0, 8.0);

	initLights(mWorldFogColor, mFogMode, mFogParams);
	mWorldFloorColor = Eigen::Vector4d(.2, .4, .8, 1.0);

	mCamera->SetCamera(Eigen::Vector3d(0, 0.1, 0) /*lookat*/, Eigen::Vector3d(1.2, 0, 0) /*eye*/, Eigen::Vector3d(0, 1, 0) /*up*/);

	try
	{
		mm = p::import("__main__");
		mns = mm.attr("__dict__");
		sys_module = p::import("sys");

		//p::str module_dir = (std::string(SOFTCON_DIR) + "/learn").c_str();
		//sys_module.attr("path").attr("insert")(1, module_dir);

		p::str module_dir = (std::string(SOFTCON_DIR) + "/interact").c_str();
		sys_module.attr("path").attr("insert")(1, module_dir);
		p::str neural_model_dir = std::string(std::string(SOFTCON_DIR) + "/../eworm/ghost_in_mesh_sim").c_str();
		p::str eworm_dir = std::string(std::string(SOFTCON_DIR) + "/..").c_str();
		sys_module.attr("path").attr("insert")(1, neural_model_dir);
		sys_module.attr("path").attr("insert")(1, eworm_dir);

		std::cout << SOFTCON_DIR << std::endl;

		// p::exec("from run import Environment", mns);
		p::exec("from IPython import embed", mns);

		// env_module = p::eval("Environment()", mns);

		p::exec("from worm_in_env import WormInEnv", mns);
		// wormInEnv = p::eval("WormInEnv()", mns);
	}
	catch (const p::error_already_set &)
	{
		PyErr_Print();
	}
}

SimulationWindow::SimulationWindow(std::string network_name) : SimulationWindow()
{
	if (network_name == std::string("c302"))
	{
		mController = MetaController::MC_C302;
		Interact::InitEnvironment(this);

		try
		{
			wormInEnv = p::eval("WormInEnv()", mns);
		}
		catch (const p::error_already_set &)
		{
			PyErr_Print();
		}

		if (!mWorms.empty())
		{
			auto worm = mWorms[0];
			mActions.resize(worm->GetActions().size());
			mActions.setZero();
		}
	}
	else
	{
		AddWorm(Eigen::Vector3d(0, 0, 0), Eigen::Vector3d(0, 0, -1), Eigen::Vector4d(0.79, 0.79, 0.79, 0.4));
		auto worm = mWorms[0];
		mActions.resize(worm->GetActions().size());
		mActions.setZero();
		std::cout << "  Action Size: " << mActions.size() << std::endl;

		try
		{
			mController = MetaController::MC_ANN;

			p::object load = env_module.attr("loadNN");
			load(network_name);
			worm->Reset();
			worm->UpdateRandomTargetVelocity();
		}
		catch (const p::error_already_set &)
		{
			PyErr_Print();
		}
	}
}

Eigen::VectorXd SimulationWindow::GetActionFromNN(const Eigen::VectorXd &state)
{
	Eigen::VectorXd action = Eigen::VectorXd::Zero(mWorms[0]->GetActions().size());
	// std::cout << "  Debug: Size of Action: " << action.size() << std::endl;

	if (mController != MetaController::MC_ANN)
		return action;

	p::object get_action;

	get_action = env_module.attr("get_action");
	p::tuple shape = p::make_tuple(state.rows());
	np::dtype dtype = np::dtype::get_builtin<float>();
	np::ndarray state_np = np::empty(shape, dtype);
	float *dest = reinterpret_cast<float *>(state_np.get_data());
	for (int i = 0; i < state.rows(); i++)
		dest[i] = state[i];
	p::object temp = get_action(state_np);
	np::ndarray action_np = np::from_object(temp);

	float *srcs = reinterpret_cast<float *>(action_np.get_data());
	for (int i = 0; i < action.rows(); i++)
		action[i] = srcs[i];

	return action;
}

void SimulationWindow::initLights(Eigen::Vector4d fogColor, int fogMode, Eigen::Vector3d fogParams)
{
	static float ambient[] = {0.4, 0.4, 0.4, 1.0};
	static float diffuse[] = {0.4, 0.4, 0.4, 1.0};
	static float front_mat_shininess[] = {60.0};
	static float front_mat_specular[] = {0.2, 0.2, 0.2, 1.0};
	static float front_mat_diffuse[] = {0.2, 0.2, 0.2, 1.0};
	static float lmodel_ambient[] = {0.2, 0.2, 0.2, 1.0};
	static float lmodel_twoside[] = {GL_TRUE};

	GLfloat position[] = {0.0, 1.0, 1.0, 0.0};
	GLfloat position1[] = {0.0, 1.0, -1.0, 0.0};

	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, position);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, lmodel_twoside);

	glEnable(GL_LIGHT1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_POSITION, position1);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, front_mat_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, front_mat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, front_mat_diffuse);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_CULL_FACE);
	glEnable(GL_NORMALIZE);

	glEnable(GL_FOG);

	// GLfloat fogColor[] = {53.0 / 255.0, 214.0 / 255.0, 237.0 / 255.0, 1.0};
	GLfloat fogC[4] = {(float)fogColor[0], (float)fogColor[1], (float)fogColor[2], (float)fogColor[3]};

	glFogfv(GL_FOG_COLOR, fogC);
	glFogi(GL_FOG_MODE, fogMode);
	glFogf(GL_FOG_DENSITY, fogParams[0]);
	glFogf(GL_FOG_START, fogParams[1]);
	glFogf(GL_FOG_END, fogParams[2]);
}

void DrawFood(Eigen::Vector3d center, float radius, float scale, int count, Eigen::Vector3f color)
{
	glPushMatrix();
	glTranslated(center[0], center[1], center[2]);
	glColor3fv(color.data());
	for (int i = 0; i < count; ++i)
	{
		glPushMatrix();
		auto rotateAxis = Eigen::Vector3f::Random();
		float angle = std::rand() / (RAND_MAX + 1.f) * 360;
		glRotatef(angle, rotateAxis[0], rotateAxis[1], rotateAxis[2]);

		auto translate = Eigen::Vector3f::Random() * radius;
		glTranslatef(translate[0], translate[1], translate[2]);
		GUI::DrawCylinder(scale, scale * 2, 6, 6);
		glPopMatrix();
	}
	
	glPopMatrix();
}

void SimulationWindow::Display()
{
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	initLights(mWorldFogColor, mFogMode, mFogParams);
	glEnable(GL_LIGHTING);

	Eigen::Vector3d focusPosition = Eigen::Vector3d(0, 0, 0);

	if (mFocusWormId < mWorms.size())
	{
		auto r = Eigen::AngleAxis<double>(mInitRotations[mFocusWormId][0] * gDegreesToRadians, mInitRotations[mFocusWormId].segment(1, 3));
		const Eigen::VectorXd &x = mWorms[mFocusWormId]->GetSoftWorld()->GetPositions();
		focusPosition = r * x.block<3, 1>(3 * mWorms[mFocusWormId]->GetCreature()->GetCenterIndex(), 0) + mInitPositions[mFocusWormId];

		// glPushMatrix();
		// glTranslated(mInitPositions[mFocusWormId].x(), mInitPositions[mFocusWormId].y(), mInitPositions[mFocusWormId].z());
		// glRotated(mInitRotations[mFocusWormId][0], mInitRotations[mFocusWormId][1], mInitRotations[mFocusWormId][2], mInitRotations[mFocusWormId][3]);

		// GUI::DrawArrow3D(Eigen::Vector3d(0, 0, 0), x.block<3, 1>(3 * mWorms[mFocusWormId]->GetCreature()->GetCenterIndex(), 0), 0.01);
		// glPopMatrix();
	}
	else
	{
		// focus at center of  worms' center
		// for (int i = 0; i < mWorms.size(); ++i)
		// {
		// 	auto r = Eigen::AngleAxis<double>(mInitRotations[i][0] * gDegreesToRadians, mInitRotations[i].segment(1, 4));
		// 	const Eigen::VectorXd &x = mWorms[i]->GetSoftWorld()->GetPositions();
		// 	focusPosition += r * x.block<3, 1>(3 * mWorms[i]->GetCreature()->GetCenterIndex(), 0) + mInitPositions[i];
		// }
		// focusPosition /= mWorms.size();
		focusPosition = mFoodPoses[0];
	}

	
	mCamera->SetFocusAt(focusPosition);
	mCamera->Apply(fbWidth, fbHeight);

	//-------------------World-------------------------
	if (mbDrawWorld)
		GUI::DrawWorld(mWorldFloorColor);
	//-------------------World Axis-------------------------
	if (mbDrawAxis)
	{
		GUI::DrawAxis(focusPosition);
	}

	if (mDrawFood)
	{
		glEnable(GL_LIGHTING);
		for (int i = 0; i < mFoodPoses.size(); ++i)
		{
			std::srand(i);
			DrawFood(mFoodPoses[i], 0.06, 0.002, 200, Eigen::Vector3f(0.1, 0.3, 0.6));
		}
		glDisable(GL_LIGHTING);
	}

	for(int i = 0; i < mWorms.size(); ++i)
	{
		// GUI::DrawArrow3D(mInitPositions[i], mInitForwards[i], 0.01);
		// GUI::DrawArrow3D(mInitPositions[i], mInitRotations[i].segment(1, 4), 0.01);

		glPushMatrix();

		glTranslated(mInitPositions[i].x(), mInitPositions[i].y(), mInitPositions[i].z());
		glRotated(mInitRotations[i][0], mInitRotations[i][1], mInitRotations[i][2], mInitRotations[i][3]);
		DisplayWorm(mWorms[i], mTrajectories[i], mRefCoordTrajectories[i], mWormColors[i], i);
		glPopMatrix();

		// auto r = Eigen::AngleAxisd(mInitRotations[i][0] * gDegreesToRadians, mInitRotations[i].segment(1, 3));
		// std::cout << mRefCoordTrajectories[i][0].transpose() << ' : ' << (r * mRefCoordTrajectories[i][0]).transpose() << std::endl;
	}

	//-------------------2D Graph-------------------------

	if (mFocusWormId < mWorms.size())
	{
		auto& muscles = mWorms[mFocusWormId]->GetCreature()->GetMuscles();
		double init_x = 0.8;
		double init_y = 1.0;
		double length = 0.2;
		double height = 0.04;
		std::vector<Eigen::Vector3d> colors = {
			Eigen::Vector3d(1, 0, 0),
			Eigen::Vector3d(0, 1, 0),
			Eigen::Vector3d(0, 0, 1),
			Eigen::Vector3d(1, 1, 0)};
		if (mbDraw2DGraph)
		{
			for (int i = 0; i < muscles.size(); i++)
			{
				GUI::DrawActivations(init_x, init_y - (i + 1) * 1.2 * height, length, height, colors[i], muscles[i]);
			}
		}
	}
	
	// glutSwapBuffers();

	if (mStartScreenShot)
	{
		char filePath[128];
		sprintf(filePath, "data/screenshots/screenshot%d.png", mFrameCount);
		ScreenShotAndSaveImage(filePath);
		mFrameCount++;
	}
}

void SimulationWindow::DisplayWorm(Environment *mEnvironment, std::deque<Eigen::Vector3d> &mTrajectory, std::deque<Eigen::Vector3d> &mRefCoordTrajectory, Eigen::Vector4d color, int wormIdx)
{
	const Eigen::VectorXd &x = mEnvironment->GetSoftWorld()->GetPositions();
	Eigen::Vector3d center_position = x.block<3, 1>(3 * mEnvironment->GetCreature()->GetCenterIndex(), 0);

	const Eigen::Vector3d &eye = mCamera->GetEye();
	mEnvironment->GetCreature()->SetVertexNormal();
	const std::vector<Muscle *> &muscles = mEnvironment->GetCreature()->GetMuscles();

	//-------------------Muscles-------------------------
	if (mbDrawMuscles)
		GUI::DrawMuscles(muscles, x);
	
	//-------------------Trajectory-------------------------
	if (mTrajectory.size() > 3000)
		mTrajectory.pop_front();
	mTrajectory.push_back(center_position);
	if (mbDrawPath)
	{
		GUI::DrawTrajectory(mTrajectory, Eigen::Vector4d(0.96, 0.51, 0.13, 0.5), 4);
	}

	//-------------------Local coord-------------------------
	//if (mbDrawLocalCoord || mbDrawReferenceTets || mbDrawSamplingPoints || mbDrawArrows)
	{
		// Eval Local Coord
		const Eigen::VectorXd& refVertices = mEnvironment->GetCreature()->GetVerticesReference();
		Eigen::Affine3d affine = mEnvironment->GetEvalLocalCoord();
		Eigen::Vector3d t = affine.translation();
		Eigen::Matrix3d r = affine.rotation();
		Eigen::Matrix3d body_local_rot = r*Eigen::AngleAxisd(M_PI/2, Eigen::Vector3d::UnitY())/*world->worm body local*/;

		Eigen::Vector3i idx = mEnvironment->GetCreature()->GetLocalContourIndex();
		Eigen::Vector3d ref_head_pos = refVertices.block<3, 1>(3*idx[0], 0);

		//-------------------Local Coord------------------
		if (mbDrawLocalCoord)
		{
			// Local Coord Head Triangle
			GUI::DrawLocalContour(mEnvironment->GetCreature(), x);
			const double scale = 0.04;
			const double thickness = 0.0015;
			// head coord
			Eigen::Vector3d head_forward = mEnvironment->GetCreature()->GetForwardVector(x);
			Eigen::Vector3d head_up = mEnvironment->GetCreature()->GetUpVector(x);
			GUI::DrawArrow3D(x.block<3, 1>(3 * idx[0], 0), head_forward * scale, thickness, Eigen::Vector3d(1.0, 0.3, 0.3));
			GUI::DrawArrow3D(x.block<3, 1>(3 * idx[0], 0), head_up * scale, thickness, Eigen::Vector3d(0.3, 1.0, 0.3));
			GUI::DrawArrow3D(x.block<3, 1>(3 * idx[0], 0), head_forward.cross(head_up) * scale, thickness, Eigen::Vector3d(0.3, 0.3, 1.0));
			// body coord
			Eigen::Vector3d body_forward = body_local_rot.col(0).normalized();
			Eigen::Vector3d body_up = body_local_rot.col(1).normalized();
			GUI::DrawArrow3D(t+r*ref_head_pos, body_forward * scale * 3, thickness*2, Eigen::Vector3d(1.0, 0.3, 0.3));
			GUI::DrawArrow3D(t+r*ref_head_pos, body_up * scale * 3, thickness*2, Eigen::Vector3d(0.3, 1.0, 0.3));
			GUI::DrawArrow3D(t+r*ref_head_pos, body_forward.cross(body_up) * scale * 3, thickness*2, Eigen::Vector3d(0.3, 0.3, 1.0));
			// world coord at ref head position. used for test.
			GUI::DrawAxis(r*ref_head_pos+t);
		}

		//-------------------Reference Coord Trajectory------------------
		if (mRefCoordTrajectory.size() > 3000)
			mRefCoordTrajectory.pop_front();
		mRefCoordTrajectory.push_back(t);
		if (mbDrawPath)
			GUI::DrawTrajectory(mRefCoordTrajectory, Eigen::Vector4d(0.3, 0.96, 0.13, 0.5), 3);

		//-------------------Sampling States-------------------------
		const Eigen::VectorXd& v = mEnvironment->GetSoftWorld()->GetVelocities();
		if (mbDrawSamplingPoints)
			GUI::DrawSamplingPoints(mEnvironment->GetCreature(), x, v, r, t);

		//-------------------Arrows-------------------------
		if (mbDrawArrows)
		{
			Eigen::Vector3d arrow_base = Eigen::Vector3d(0, 0.1, -0.1) /*displacement*/;
			Eigen::Vector3d target_velocity = mEnvironment->GetTargetVelocity();
			Eigen::Vector3d average_eval_velocity = mEnvironment->GetAverageEvalCoordVelocity();
			Eigen::Vector3d current_eval_velocity = mEnvironment->GetCurrentCoordVelocity();
			Eigen::Vector3d average_center_velocity = mEnvironment->GetAverageVelocity();
			Eigen::Vector3d average_sampling_velocity = mEnvironment->GetAverageSamplingVelocity();

			//GUI::DrawArrow3D(r*ref_head_pos+t, target_velocity, 0.002, Eigen::Vector3d(1, 1, 0.1));
			//GUI::DrawArrow3D(r*ref_head_pos+t, average_eval_velocity, 0.003, Eigen::Vector3d(0.5, 0.73, 0.05));
			GUI::DrawArrow3D(r*ref_head_pos+t, current_eval_velocity, 0.003, Eigen::Vector3d(1, 1.0, 0.1)/* Eigen::Vector3d(0.5, 0.73, 0.05) */);
			//GUI::DrawArrow3D(r*ref_head_pos+t, average_center_velocity, 0.003, Eigen::Vector3d(0.96, 0.47, 0.13));
			//GUI::DrawArrow3D(r*ref_head_pos+t, average_sampling_velocity, 0.003, Eigen::Vector3d(1, 0, 1));
		}

		//-------------------Reference Worm-------------------------
		if (mbDrawReferenceTets)
		{
			Eigen::VectorXd transRefVertices(refVertices.size());
			int n = refVertices.size()/3;
			for (int i = 0; i < n; i++)
			{
				transRefVertices.block<3, 1>(i*3, 0) = r*refVertices.block<3, 1>(i*3, 0) + t;
			}
			//GUI::DrawTets(mEnvironment->GetCreature(), transRefVertices, Eigen::Vector4d(0.8, 0.8, 0.8, 0.5));
			GUI::DrawCharacter(mEnvironment->GetCreature(), transRefVertices, WorldToWormLocal(wormIdx, eye), color);
		}
#if 0
		// test worm local coord
		{
			const Eigen::VectorXd& x = mEnvironment->GetSoftWorld()->GetPositions();
			const Eigen::VectorXd& v = mEnvironment->GetSoftWorld()->GetVelocities();
			const Eigen::VectorXd& refVertices = mEnvironment->GetCreature()->GetVerticesReference();
			const std::vector<Eigen::Vector3d>& sampling_ref_pos = mEnvironment->GetCreature()->GetSamplingReference();
			Eigen::Affine3d eval_local_coord = mEnvironment->GetEvalLocalCoord();
			Eigen::Vector3d target_velocity = mEnvironment->GetTargetVelocity();
			Eigen::Vector3d average_eval_velocity = mEnvironment->GetAverageEvalCoordVelocity();
				
			//--------------------------------------------------
			//     ref_coord    eval_local_coord
			// world -----> worm_ref -----> worm_local
			//--------------------------------------------------
			Eigen::Matrix3d R = eval_local_coord.rotation();
			Eigen::Vector3d t = eval_local_coord.translation();
			Eigen::AngleAxisd ref_coord = Eigen::AngleAxisd(M_PI/2, Eigen::Vector3d::UnitY());  // world->worm_ref
			Eigen::Matrix3d local_body_rot = R*ref_coord;

			Eigen::Vector3d local_target_velocity = local_body_rot.transpose()*target_velocity;
			Eigen::Vector3d local_average_velocity = local_body_rot.transpose()*average_eval_velocity;   
			GUI::DrawArrow3D(Eigen::Vector3d(0, 0, 0), local_target_velocity, 0.001, Eigen::Vector3d(1.0, 1.0, 0.0));
			GUI::DrawArrow3D(Eigen::Vector3d(0, 0, 0), local_average_velocity, 0.001, Eigen::Vector3d(0, 1, 1)); 

			const std::vector<int>& sampling_index = mEnvironment->GetCreature()->GetSamplingIndex();
			Eigen::VectorXd local_sampling_pos(3*sampling_index.size());
			Eigen::VectorXd local_sampling_vel(3*sampling_index.size());

			for(int i=0; i<sampling_index.size(); i++) 
			{
				local_sampling_pos.block<3,1>(3*i, 0) = /* local_body_rot.inverse()*(x.block<3,1>(3*sampling_index[i], 0)-t) */  ref_coord.inverse()*sampling_ref_pos[i];
				local_sampling_vel.block<3,1>(3*i, 0) = local_body_rot.transpose()*v.block<3,1>(3*sampling_index[i], 0);   
			}

			for(int i=0; i< sampling_index.size(); ++i)
			{
				GUI::DrawPoint(local_sampling_pos.block<3,1>(3*i, 0), 8.0, Eigen::Vector4d(1, 0, 1, 1));
				GUI::DrawPoint(ref_coord.inverse()*sampling_ref_pos[i], 8.0, Eigen::Vector4d(1, 0, 1, 1));
				GUI::DrawLine(ref_coord.inverse()*sampling_ref_pos[i], local_sampling_pos.block<3,1>(3*i, 0), Eigen::Vector4d(1, 0, 1, 1), 1.0);
				GUI::DrawArrow3D(local_sampling_pos.block<3,1>(3*i, 0), local_sampling_vel.block<3,1>(3*i, 0)*0.1, 0.001, Eigen::Vector3d(0, 0, 1));
			}

			// test local worm
			int nv = mEnvironment->GetSoftWorld()->GetNumVertices();
			Eigen::VectorXd worm_local(nv*3);
			for (int i = 0; i < nv; i++)
			{
				worm_local.block<3,1>(3*i, 0) = local_body_rot.inverse()*(x.block<3,1>(3*i, 0)-t);
			}
			
			//GUI::DrawCharacter(mEnvironment->GetCreature(), worm_local, eye, Eigen::Vector4d(0.79, 0.79, 0.2, 0.4));
		}
#endif

#if 0
		// test worm registration
		{
			const Eigen::VectorXd& x = mEnvironment->GetSoftWorld()->GetPositions();
			const Eigen::VectorXd& v = mEnvironment->GetSoftWorld()->GetVelocities();
			const Eigen::VectorXd& refVertices = mEnvironment->GetCreature()->GetVerticesReference();
			const std::vector<Eigen::Vector3d>& sampling_ref_pos = mEnvironment->GetCreature()->GetSamplingReference();
			Eigen::Affine3d eval_local_coord = mEnvironment->GetEvalLocalCoord();
			Eigen::Vector3d target_velocity = mEnvironment->GetTargetVelocity();
			Eigen::Vector3d average_eval_velocity = mEnvironment->GetAverageEvalCoordVelocity();
				
			//--------------------------------------------------
			//     ref_coord    eval_local_coord
			// world -----> worm_ref -----> worm_local
			//--------------------------------------------------
			Eigen::Matrix3d R = eval_local_coord.rotation();
			Eigen::Vector3d t = eval_local_coord.translation();
			Eigen::AngleAxisd ref_coord = Eigen::AngleAxisd(M_PI/2, Eigen::Vector3d::UnitY());  // world->worm_ref
			Eigen::Matrix3d local_body_rot = R*ref_coord;

			Eigen::Vector3d local_target_velocity = local_body_rot.transpose()*target_velocity;
			Eigen::Vector3d local_average_velocity = local_body_rot.transpose()*average_eval_velocity;    

			const std::vector<int>& sampling_index = mEnvironment->GetCreature()->GetSamplingIndex();
			Eigen::VectorXd sampling_pos1(3*sampling_index.size());
			Eigen::VectorXd sampling_pos2(3*sampling_index.size());

			for(int i=0; i<sampling_index.size(); i++) 
			{
				sampling_pos1.block<3,1>(3*i, 0) = sampling_ref_pos[i];
				sampling_pos2.block<3,1>(3*i, 0) = x.block<3,1>(3*sampling_index[i], 0);   
			}

			for(int i=0; i< sampling_index.size(); ++i)
			{
				GUI::DrawPoint(sampling_pos1.block<3,1>(3*i, 0), 8.0, Eigen::Vector4d(1, 0, 1, 1));
				GUI::DrawPoint(sampling_pos2.block<3,1>(3*i, 0), 8.0, Eigen::Vector4d(1, 0, 1, 1));
				GUI::DrawLine(sampling_pos1.block<3,1>(3*i, 0), sampling_pos2.block<3,1>(3*i, 0), Eigen::Vector4d(1, 0, 1, 1), 1.0);
			}

			GUI::DrawCharacter(mEnvironment->GetCreature(), refVertices, eye, Eigen::Vector4d(0.79, 0.79, 0.2, 0.4));
		}
#endif
	}

	//-------------------Worm-------------------------
	if(mbDrawCreature)
		GUI::DrawCharacter(mEnvironment->GetCreature(), x, WorldToWormLocal(wormIdx, eye), color);

	//-------------------Tets-------------------------
	if (mbDrawTets)
		GUI::DrawTets(mEnvironment->GetCreature(), x, Eigen::Vector4d(0.2, 0.2, 0.2, 1));
}

void SimulationWindow::Keyboard(unsigned char key, int x, int y)
{
	auto mEnvironment = mWorms[0];

	switch (key)
	{
	case 27:
		exit(0);
		break;
	case ' ':
	{
		mPlay = !mPlay;
		if (mPlay)
			std::cout << "Play." << std::endl;
		else
			std::cout << "Pause." << std::endl;
		break;
	}
	case '!':
	{
		autoScreenShot = !autoScreenShot;
		if (autoScreenShot)
			std::cout << "autoScreenShot." << std::endl;
		else
			std::cout << "stop autoScreenShot." << std::endl;
		break;
	}
	case 'r':
	{
		for (auto worm : mWorms)
			worm->Reset();
		break;
	}
	case 'k':
	{
		mPlay = true;
		const auto &muscles = mEnvironment->GetCreature()->GetMuscles();
		mActions.resize(mEnvironment->GetActions().size());
		mActions.setZero();

		int cnt = 0;
		//for (const auto &m : muscles)

		{
			mActions[cnt + 0] = 0.0; // signal0
			mActions[cnt + 1] = 0.7; // alpha
			mActions[cnt + 2] = 0.3; // beta = 1.0-alpha
			mActions[cnt + 3] = 3.0; // prop speed
			cnt += 4;
		}

		mActions = mEnvironment->GetNormalizer()->RealToNorm(mActions);
		break;
	}
	case 't':
	{
		mbDrawTets = !mbDrawTets;
		break;
	}
	case 'w':
	{
		mbDrawCreature = !mbDrawCreature;
		break;
	}
	case 'l':
	{
		mbDrawReferenceTets = !mbDrawReferenceTets;
		break;
	}
	case 'a':
	{
		mbDrawArrows = !mbDrawArrows;
		break;
	}
	case 'm':
	{
		mbDrawMuscles = !mbDrawMuscles;
		break;
	}
	case 's':
	{
		mbDrawSamplingPoints = !mbDrawSamplingPoints;
		break;
	}
	case 'c':
	{
		mbDrawLocalCoord = !mbDrawLocalCoord;
		break;
	}
	case 'b':
	{
		mbDrawWorld = !mbDrawWorld;
		break;
	}
	case 'g':
	{
		mbDraw2DGraph = !mbDraw2DGraph;
		break;
	}
	case 'x':
	{
		mbDrawAxis = !mbDrawAxis;
		break;
	}
	case 'p':
	{
		mbDrawPath = !mbDrawPath;
		break;
	}
	case 'v':
	{
		mEnvironment->UpdateRandomTargetVelocity();
		break;
	}
	case 'o':
	{
		mEnvironment->RecordSimWormSignal();
		break;
	}
	case 'f':
	{
		mFocusWormId = (mFocusWormId + 1) % (mWorms.size() + 1);
		break;
	}
	}
	// glutPostRedisplay();
}

void SimulationWindow::Mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		mIsDrag = true;
		mMouseType = button;
		mPrevX = x;
		mPrevY = y;
	}
	else
	{
		mIsDrag = false;
		mMouseType = 0;
	}

	// glutPostRedisplay();
}

void SimulationWindow::Motion(int x, int y)
{
	if (!mIsDrag)
		return;

	// int mod = glutGetModifiers();
	if (mMouseType == GLUT_LEFT_BUTTON)
	{
		mCamera->Translate(x, y, mPrevX, mPrevY);
	}
	else if (mMouseType == GLUT_MIDDLE_BUTTON)
	{
		mCamera->Pan(x, y, mPrevX, mPrevY);
	}
	else if (mMouseType == GLUT_RIGHT_BUTTON)
	{
		mCamera->Rotate(x, y, mPrevX, mPrevY, fbWidth, fbHeight);
	}
	mPrevX = x;
	mPrevY = y;
	// glutPostRedisplay();
}

void SimulationWindow::Reshape(int w, int h)
{
	//glViewport(0, 0, w, h);
	fbWidth = w;
	fbHeight = h;
	mCamera->Apply(w, h);
	// glutPostRedisplay();
}

void SimulationWindow::Timer(int value)
{
	auto worm = mWorms[0];
	int ratio = worm->GetSimulationHz() / worm->GetControlHz();

	if (mPlay)
	{
		switch (mController)
		{
		case MetaController::MC_ANN:
		{
			std::cout << mController << std::endl;
			unsigned int cur_phase = worm->GetPhase();
			Eigen::VectorXd action = GetActionFromNN(worm->GetStates());
			worm->SetActions(action);
			for (int i = 0; i < ratio; i++)
			{
				// if(mEnvironment->isCollision()) {
				// 	mEnvironment->Step(mEnvironment->GetObstacles());
				// } else {
				worm->Step();	// dynamics simualtion
				// }
			}
			worm->StepKeyWorm();
			worm->UpdateAverageVelocity();
			worm->GetRewards();
			worm->SetPhase(cur_phase + 1);
			worm->isEndOfEpisode();
		}
		break;
		case MetaController::MC_Default_Key:
		{
			unsigned int cur_phase = worm->GetPhase();
			Eigen::VectorXd tmpState = worm->GetStates();
			worm->SetActions(mActions);
			for (int i = 0; i < ratio; i++)
				worm->Step();

			worm->UpdateAverageVelocity();
			worm->SetPhase(cur_phase + 1);
		}
		break;
		case MetaController::MC_C302:
		{
			// neuron step, will set muscle activation in it
			try
			{
				if (!autoScreenShot)
				{
					// std::cout << "stepping" << std::endl;
					auto stepFunc = wormInEnv.attr("step");
					auto stepInitFunc = wormInEnv.attr("step_init");

					if (mElapsedTime < 0)  // change initial time 600000
					{
						stepInitFunc();
					}
					else
					{
						stepFunc();
					}
					// std::cout << " stepped " << std::endl;
				}
				else
				{
					auto stepFunc = wormInEnv.attr("step_with_auto_screen_shot");
					auto stepInitFunc = wormInEnv.attr("step_init_with_auto_screen_shot");

					if (mElapsedTime < 0)
					{
						stepInitFunc();
					}
					else
					{
						stepFunc();
					}
				}
			}
			catch (const p::error_already_set &)
			{
				PyErr_Print();
				exit(1);
			}

			for(auto worm : mWorms)
			{
				unsigned int cur_phase = worm->GetPhase();
				Eigen::VectorXd tmpState = worm->GetStates();

#if 1
				for (int i = 0; i < ratio; i++)
					worm->Step();

				// mEnvironment->Step();
				worm->UpdateAverageVelocity();
				worm->UpdateHeadLocation();
#else
				mEnvironment->SetActions(); // c302
											// for(int i=0; i<ratio; i++)
				mEnvironment->Step();
#endif

				worm->SetPhase(cur_phase + 1);
			}	
		}
		break;

		default:
			break;
		}

		mElapsedTime += mDisplayTimeout;
	}

	// glutTimerFunc(mDisplayTimeout, TimerEvent, 1);
	// glutPostRedisplay();
}

void SimulationWindow::AddWorm(Eigen::Vector3d pos, Eigen::Vector3d forward, Eigen::Vector4d color)
{
	auto worm = new Environment();
	worm->Reset();
	mWorms.push_back(worm);

	mInitPositions.push_back(pos);
	mInitForwards.push_back(forward);

	tQuaternion q;
	const Eigen::Vector3d defaultForward = Eigen::Vector3d(0, 0, -1);
	q.setFromTwoVectors(defaultForward, forward);
	Eigen::AngleAxisd ax;
	ax = q;

	mInitRotations.push_back(Eigen::Vector4d(ax.angle() * gRadiansToDegrees, ax.axis()[0], ax.axis()[1], ax.axis()[2]));
	mTrajectories.push_back(std::move(std::deque<Eigen::Vector3d>()));
	mRefCoordTrajectories.push_back(std::move(std::deque<Eigen::Vector3d>()));
	mWormColors.push_back(color);
}

Eigen::Vector3d SimulationWindow::WormLocalToWorld(int idx, Eigen::Vector3d position)
{
	auto ax = Eigen::AngleAxisd(mInitRotations[idx][0] * gDegreesToRadians, mInitRotations[idx].segment(1, 3));
	return ax * position + mInitPositions[idx];
}

Eigen::Vector3d SimulationWindow::WorldToWormLocal(int idx, Eigen::Vector3d position)
{
	auto ax = Eigen::AngleAxisd(mInitRotations[idx][0] * gDegreesToRadians, mInitRotations[idx].segment(1, 3));
	return ax.inverse() * (position - mInitPositions[idx]);
}

void SimulationWindow::ScreenShotAndSaveImage(const std::string &path)
{
	// GLint w = glutGet(GLUT_WINDOW_WIDTH);
	// GLint h = glutGet(GLUT_WINDOW_HEIGHT);

	// glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, mScreenshotTemp.data());
	// stbi_flip_vertically_on_write(true);
	// stbi_write_png(path.c_str(), w, h, 4, mScreenshotTemp.data(), w*4);
}

void SimulationWindow::SetFogParams(int mode, Eigen::Vector3d params)
{
	mFogMode = g_fogMode[mode%3];
	mFogParams = params;
}
