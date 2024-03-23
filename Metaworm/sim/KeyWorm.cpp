#include "KeyWorm.h"

#include <fstream>
#include <iomanip>

KeyWorm::Params::Params()
{

}

KeyWorm::KeyWorm() : mTotalFrames(0), mNumSamplings(0), mActionSize(0), mStateSize(0), mPhase(0), mCurrentFrame(0)
{
	
}

void KeyWorm::ParseArgs(const std::shared_ptr<cArgParser>& parser)
{
	//cScene::ParseArgs(parser);

	bool succ = true;
	parser->ParseString("state_file", mParams.mStateFile);
	parser->ParseString("action_file", mParams.mActionFile);

	// parser->ParseBool("enable_char_contact_fall", mEnableContactFall);
	// parser->ParseBool("enable_rand_char_placement", mEnableRandCharPlacement);
	
	// succ &= ParseCharTypes(parser, mCharTypes);
	// succ &= ParseCharParams(parser, mCharParams);
	// succ &= ParseCharCtrlParams(parser, mCtrlParams);
	// if (mCharParams.size() != mCtrlParams.size())
	// {
	// 	printf("Char and ctrl file mismatch, %zi vs %zi\n", mCharParams.size(), mCtrlParams.size());
	// 	assert(false);
	// }

	// parser->ParseInt("num_sim_substeps", mWorldParams.mNumSubsteps);
	// parser->ParseDouble("world_scale", mWorldParams.mScale);
	// parser->ParseVector("gravity", mWorldParams.mGravity);
	// parser->ParseInts("fall_contact_bodies", mFallContactBodies);

	//ParseGroundParams(parser, mGroundParams);
}

void KeyWorm::Initialize()
{
	std::cout << "  Init KeyWorm... ";
	bool succ = false;
	succ = LoadStates();
	if(succ)
		std::cout << "Done! " << mTotalFrames << " States of DOF=" << mStateSize << ". ";
	
	succ = LoadActions();
	if(succ)
		std::cout <<  mTotalFrames << " Actions of DOF=" << mActionSize << ". ";

	std::cout << std::endl;
}

void KeyWorm::LoadMuscleSignal(const std::string& ms_file)
{
	// Eigen::MatrixXd key;
	// std::ofstream muscle_file;
	// muscle_file.open("data/output/Muscle_Signal.csv", std::ofstream::out | std::ofstream::app);
	// if (!muscle_file.good())
	// 	std::cout << "  Open Muscle Signal File Error!" << std::endl;

	// muscle_file << std::setw(9) << std::fixed << std::setprecision(4);
	// for (int i = 0; i < mMuscles.size(); ++i)
	// {
	// 	Eigen::VectorXd acts = mMuscles[i]->GetActivationLevels();
	// 	for (int j = 0; j < mMuscles[i]->GetNumSampling(); ++j)
	// 	{
	// 		muscle_file << acts(j) << " ";
	// 	}
	// }

	// muscle_file << std::endl;

	// muscle_file.close();
	// std::cout << "  -- Output Muscle Signal... " << std::endl;
}

bool KeyWorm::LoadJsonActions(const Json::Value& root, Eigen::MatrixXd& out_action)
{
	bool succ = true;

	assert(root.isArray());
	int num_frames = root.size();

	if (num_frames > 0)
	{
		Json::Value frame_json = root.get(int(0), 0);
		mActionSize = frame_json.size();
		out_action.resize(num_frames, mActionSize);
	}

	for (int f = 0; f < num_frames; ++f)
	{
		Eigen::VectorXd curr_frame_data;
		succ &= ParseFrameJson(root.get(f, 0), curr_frame_data);
		if (succ)
		{
			out_action.row(f) = curr_frame_data;
		}
		else
		{
			out_action.resize(0, 0); 
			break;
		}
	}
	return succ;
}

bool KeyWorm::LoadActions()
{
	std::ifstream file(mParams.mActionFile);
	Json::Value root;
	Json::Reader reader;
	bool succ = reader.parse(file, root);
	file.close();

	if (succ)
	{
		if (!root["Loop"].isNull())
		{
			//std::string loop_str = root["Loop"].asString();
			//ParseLoop(loop_str, mLoop);
		}

		if (!root["NumSamplings"].isNull())
		{
			int ns = root["NumSamplings"].asInt();
			assert(ns == mNumSamplings);
		}

		if (!root["Frames"].isNull())
		{
			succ &= LoadJsonActions(root["Frames"], mAction);
		}

		if (succ)
		{

		}
		else
		{
			printf("Failed to load states from file %s\n", mParams.mActionFile.c_str());
			assert(false);
		}
	}
	else
	{
		printf("Failed to parse Json from %s\n", mParams.mActionFile.c_str());
		assert(false);
	}

	return succ;
}

void KeyWorm::LoadMesh(const std::string& mesh_file)
{
	// Eigen::VectorXd velocities = world->GetVelocities();
	// Eigen::VectorXd positions = world->GetPositions();
	// unsigned int n = positions.size() / 3;

	// Json::Value root;
	// Json::Value pos(Json::arrayValue), vel(Json::arrayValue);
	// std::ofstream mesh_file;
	// mesh_file.open("data/output/mesh_pv.json", std::ofstream::out | std::ofstream::app);
	// if (!mesh_file.good())
	// 	std::cout << "  Open Mesh File Error!" << std::endl;

	// Eigen::Vector3d p, v;
	// for (unsigned int i = 0; i < n; i++)
	// {
	// 	p = positions.segment<3>(i);
	// 	pos.append(Json::Value(p(0)));
	// 	pos.append(Json::Value(p(1)));
	// 	pos.append(Json::Value(p(2)));
	// 	v = velocities.segment<3>(i);
	// 	vel.append(Json::Value(v(0)));
	// 	vel.append(Json::Value(v(1)));
	// 	vel.append(Json::Value(v(2)));
	// }

	// root["nVertices"] = n;
	// root["Position"] = pos;
	// root["Velocity"] = vel;
	// mesh_file << root;
	// mesh_file.close();
}

bool KeyWorm::LoadJsonStates(const Json::Value& root, Eigen::MatrixXd& out_target_vel, Eigen::MatrixXd& out_coord_vel,
							 Eigen::MatrixXd& out_pos, Eigen::MatrixXd& out_vel)
{
	bool succ = true;

	assert(root.isArray());
	int num_frames = root.size();

	if (num_frames > 0)
	{
		Json::Value frame_json = root.get(int(0), 0);
		out_pos.resize(num_frames, mNumSamplings*3);
		out_vel.resize(num_frames, mNumSamplings*3);
		out_target_vel.resize(num_frames, 3);
		out_coord_vel.resize(num_frames, 3);
		mTotalFrames = num_frames;
		mStateSize = frame_json.size();
	}

	for (int f = 0; f < num_frames; ++f)
	{
		Eigen::VectorXd curr_frame_data;
		succ &= ParseFrameJson(root.get(f, 0), curr_frame_data);
		if (succ)
		{
			assert((6*mNumSamplings+6) == curr_frame_data.size());
			out_target_vel.row(f) = curr_frame_data.segment(0, 3);
			out_coord_vel.row(f) = curr_frame_data.segment(3, 3);
			out_pos.row(f) = curr_frame_data.segment(6, mNumSamplings*3);
			out_vel.row(f) = curr_frame_data.segment(6+mNumSamplings*3, mNumSamplings*3);
		}
		else
		{
			out_pos.resize(0, 0); out_vel.resize(0, 0);
			out_target_vel.resize(0, 0); out_coord_vel.resize(0, 0);
			break;
		}
	}
	return succ;
}

bool KeyWorm::ParseFrameJson(const Json::Value& root, Eigen::VectorXd& out_frame)
{
	bool succ = false;
	if (root.isArray())
	{
		int data_size = root.size();
		out_frame.resize(data_size);
		for (int i = 0; i < data_size; ++i)
		{
			Json::Value json_elem = root.get(i, 0);
			out_frame[i] = json_elem.asDouble();
		}

		succ = true;
	}
	return succ;
}

bool KeyWorm::LoadStates()
{
	// std::string json = BuildStateJson(pose, vel);
	// FILE* f = cFileUtil::OpenFile(file, "w");
	// if (f != nullptr)
	// {
	// 	fprintf(f, "%s", json.c_str());
	// 	cFileUtil::CloseFile(f);
	// 	return true;
	// }
	std::ifstream file(mParams.mStateFile);
	Json::Value root;
	Json::Reader reader;
	bool succ = reader.parse(file, root);
	file.close();

	if (succ)
	{
		if (!root["Loop"].isNull())
		{
			//std::string loop_str = root["Loop"].asString();
			//ParseLoop(loop_str, mLoop);
		}

		if (!root["NumSamplings"].isNull())
		{
			mNumSamplings = root["NumSamplings"].asInt();
		}

		if (!root["Frames"].isNull())
		{
			succ &= LoadJsonStates(root["Frames"], mTargetVel, mCoordVel, mStatePos, mStateVel);
		}

		if (succ)
		{

		}
		else
		{
			printf("Failed to load states from file %s\n", mParams.mStateFile.c_str());
			assert(false);
		}
	}
	else
	{
		printf("Failed to parse Json from %s\n", mParams.mStateFile.c_str());
		assert(false);
	}

	return succ;
}

void KeyWorm::Step()
{
	++mCurrentFrame;
	if(mCurrentFrame>=mTotalFrames) 
		mCurrentFrame = mCurrentFrame%mTotalFrames;
}

void KeyWorm::Reset()
{
	mCurrentFrame = 0;
	mPhase = 0;
}