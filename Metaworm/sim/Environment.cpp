#include "Environment.h"
#include <chrono>
#include <random>
#include <fstream>
#include <ctime>
#include <iomanip>
#include "util/json/json.h"
#include "util/JsonUtil.h"
#include "util/FileUtil.h"

#define LOCAL   0
#define GLOBAL  1

unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
std::default_random_engine generator(seed);
std::uniform_real_distribution<double> l_distribution(0.1, 0.4);  // todo
std::uniform_real_distribution<double> angle_distribution(0.0, 0.5);
std::uniform_real_distribution<double> vx_distribution(-1.0, 1.0);
std::uniform_real_distribution<double> vy_distribution(-1.0, 1.0);
std::uniform_real_distribution<double> vz_distribution(-1.0, 1.0);

Environment::Environment() : mPhase(0), mHeadLocation(0, 0, 0), mLocalCoordPosition(0, 0, 0)
{
    mArgParser = std::shared_ptr<cArgParser>(new cArgParser());
    mArgParser->LoadFile("args/train_worm_swim_args.txt");
	
	mSimulationHz = 240;    // todo read by parser
    mControlHz = 30;
    mCurrIters = 0;
    mMaxIters = 2400;       
    mRecordLength = (mMaxIters/mSimulationHz)*mControlHz;      // todo: json
    mRecordCounter = 0;
    mSampleCount = 0;
    mbOutputFiles = false;

    mSoftWorld = new FEM::World(1.0/mSimulationHz, 50, 0.9999, false);

    mCreature = new Worm(5E5, 2E5, 0.4);
    mCreature->ReadMetaData(std::string(SOFTCON_DIR)+"/data/worm.meta");
    mCreature->Initialize(mSoftWorld);

    mKeyCreature = new KeyWorm();
    mKeyCreature->ParseArgs(mArgParser);
    mKeyCreature->Initialize();

    mSoftWorld->Initialize();

    mStateSize = mCreature->GetSamplingIndex().size()*6+6;      // todo

    mCreature->SetInitReferenceRotation(mSoftWorld->GetPositions());
    mCreature->SetInitReferenceSamplingStates(mSoftWorld->GetPositions());
    mCreature->SetInitReferenceVertices(mSoftWorld->GetPositions());

    // toyudong
    // 
    //mCreature->SetWormStates(std::string(SOFTCON_DIR)+"/data/mesh/test.json", mSoftWorld);
    
    InitializeActions();

    mTargetVelocity.setZero();
    mAverageVelocity.setZero();
    mAverageVelocityDeque.clear();
    mCurrentCoordVelocity.setZero();
    mAverageEvalCoordVelocity.setZero();
    mAverageEvalCoordPosDeque.clear();

    UpdateRandomTargetVelocity();
}

void Environment::ParseArgs(const std::vector<std::string>& args)
{
	mArgParser->LoadArgs(args);

	std::string arg_file = "";
	mArgParser->ParseString("arg_file", arg_file);
	if (arg_file != "")
	{
		// append the args from the file to the ones from the commandline
		// this allows the cmd args to overwrite the file args
		bool succ = mArgParser->LoadFile(arg_file);
		if (!succ)
		{
			printf("Failed to load args from: %s\n", arg_file.c_str());
			assert(false);
		}
	}

    int t = 99;
	mArgParser->ParseInt("num_update_substeps", t);
    std::cout << "Parse para: " << t << std::endl; 
}

void Environment::Step()
{
    Eigen::VectorXd external_force = mCreature->ComputeDragForces(mSoftWorld);
    mSoftWorld->SetExternalForce(external_force);
    mSoftWorld->TimeStepping();

    //UpdateAverageVelocity();
    ++mCurrIters;
    //std::cout << mCurrIters << " " ;
}

void Environment::SetPhase(const unsigned int& phase)
{
    mPhase = phase;
}

void Environment::Reset()
{
    mAverageVelocityDeque.clear();
    mAverageVelocity.setZero();
    mAverageEvalCoordPosDeque.clear();
    mCurrentCoordVelocity.setZero();
    mAverageEvalCoordVelocity.setZero();
    mAverageSamplingVelocity.setZero();

    for(int i=0; i<mCreature->GetMuscles().size(); i++)
    {
        mCreature->GetMuscles()[i]->Reset();
    }

    mSoftWorld->Reset();
    mKeyCreature->Reset();
    mPhase = 0;
    mCurrIters = 0;
    mHeadLocation.setZero();
    mLocalCoordPosition.setZero();
}


// DeepRL
const Eigen::VectorXd& Environment::GetStates()
{
    const Eigen::VectorXd& x = mSoftWorld->GetPositions();
    const Eigen::VectorXd& v = mSoftWorld->GetVelocities();
    const Eigen::VectorXd& refVertices = mCreature->GetVerticesReference();
    const std::vector<Eigen::Vector3d>& sampling_ref_pos = mCreature->GetSamplingReference();
	mEvalLocalCoord = mCreature->EvalBodyTransform(refVertices, x);
		
    //--------------------------------------------------
    //     ref_coord    eval_local_coord
    // world -----> worm_ref -----> worm_local
    //--------------------------------------------------
    Eigen::Matrix3d R = mEvalLocalCoord.rotation();
    Eigen::Vector3d t = mEvalLocalCoord.translation();
    Eigen::AngleAxisd ref_coord = Eigen::AngleAxisd(M_PI/2, Eigen::Vector3d::UnitY());  // world->worm_ref
    Eigen::Matrix3d local_body_rot = R*ref_coord;

    Eigen::Vector3d local_target_velocity = local_body_rot.transpose()*mTargetVelocity;  // todo: ideal local target is (1, 0, 0)
    Eigen::Vector3d local_average_velocity = local_body_rot.transpose()*mAverageEvalCoordVelocity;    
    Eigen::Vector3d local_velocity = local_body_rot.transpose()*mCurrentCoordVelocity;  // todo

    const std::vector<int>& sampling_index = mCreature->GetSamplingIndex();
    Eigen::VectorXd local_sampling_pos(3*sampling_index.size());
    Eigen::VectorXd local_sampling_vel(3*sampling_index.size());
    mAverageSamplingVelocity.setZero();

    for(int i=0; i<sampling_index.size(); i++) 
    {
        local_sampling_pos.block<3,1>(3*i, 0) = local_body_rot.inverse()*(x.block<3,1>(3*sampling_index[i], 0)-t) - ref_coord.inverse()*sampling_ref_pos[i];
        local_sampling_vel.block<3,1>(3*i, 0) = local_body_rot.transpose()*v.block<3,1>(3*sampling_index[i], 0);   
        mAverageSamplingVelocity += v.block<3, 1>(3*sampling_index[i], 0);
    }
    mAverageSamplingVelocity /= sampling_index.size();

    mStateSize = local_target_velocity.size()    // target velocity
                + local_average_velocity.size()  //+ local_velocity.size() // time average velocity
                + local_sampling_pos.size()      // body sampling position
                + local_sampling_vel.size();     // body sampling velocity

    mStates.resize(mStateSize);
    mStates.setZero();

    mStates << local_target_velocity, local_average_velocity, local_sampling_pos, local_sampling_vel;

    return mStates;
}

int Environment::GetStateSize() const
{
	return mStateSize;
}

// const Eigen::VectorXd& Environment::GetPos() 
// {
//     int ns = mCreature->GetSamplingIndex().size();
//     return mStates.segment(6, ns*3);
// }

// const Eigen::VectorXd& Environment::GetVel() 
// {
//     int ns = mCreature->GetSamplingIndex().size();
//     return mStates.segment(6+ns*3, ns*3);
// }

// const Eigen::Vector3d& Environment::GetCoordVel() 
// {
//     return mStates.segment(3, 3);
// }

void Environment::InitializeActions()
{
    const auto& muscles = mCreature->GetMuscles();

    int num_action = 4/*signal, alpah, beta, period*/*muscles.size();  
    mActions.resize(num_action);

    Eigen::VectorXd real_lower_bound(num_action);
    Eigen::VectorXd real_upper_bound(num_action);

    int cnt =0;
    for(const auto& m : muscles) 
    {
        real_lower_bound.segment(cnt, 4) = m->GetActionLowerBound();
        real_upper_bound.segment(cnt, 4) = m->GetActionUpperBound();
        cnt+=4;
    }

    mNormLowerBound.resize(real_lower_bound.size());
    mNormUpperBound.resize(real_upper_bound.size());

    mNormLowerBound.setOnes();
    mNormLowerBound *= -5.0;
    mNormUpperBound.setOnes();
    mNormUpperBound *= 5.0;

    mNormalizer = new Normalizer(real_upper_bound, real_lower_bound, mNormUpperBound, mNormLowerBound);
}

void Environment::SetActions(const Eigen::VectorXd& actions)
{
    Eigen::VectorXd real_actions = mNormalizer->NormToReal(actions);
    mCreature->SetActivationLevels(real_actions, mPhase);

    if(mbOutputFiles)
    {
        if (mRecordCounter>0)
        {
            //mCreature->WriteMesh();
            Eigen::VectorXd muscle_actions;
            muscle_actions.setZero(real_actions.size());
            mCreature->GetMuscleActions(muscle_actions);
            mJsonActions.row(mRecordLength-mRecordCounter) = muscle_actions;

            Eigen::VectorXd muscle_activations;
            muscle_activations.setZero(GetMuscleActivationSize());
            mCreature->GetMuscleActivations(muscle_activations);
            mJsonMuscleActivations.row(mRecordLength-mRecordCounter) = muscle_activations;

            mJsonStates.row(mRecordLength-mRecordCounter) = mStates;
            --mRecordCounter;
        }
        else
        {
            auto t = std::time(nullptr);
            auto lt = *std::localtime(&t);
            std::ostringstream oss;
            oss << std::put_time(&lt, "%y%m%d-%H%M%S");
            auto time = oss.str();
            std::string tail_name = std::to_string(mRecordLength) + std::string("_") + time + std::string(".json");
            WriteStates(std::string(SOFTCON_DIR)+std::string("data/state/worm_states_")+tail_name, mJsonStates);
            WriteActions(std::string(SOFTCON_DIR)+std::string("data/action/worm_actions_")+tail_name, mJsonActions);
            WriteMuscleSignals(std::string(SOFTCON_DIR)+std::string("data/muscle/worm_muscles_")+tail_name, mJsonMuscleActivations);
		    std::cout << "  -- Record Signal: Finished! Total Length: " << mRecordLength << std::endl;
            mJsonActions.resize(0, 0);
            mJsonStates.resize(0, 0);
            mJsonMuscleActivations.resize(0, 0);
            mbOutputFiles = false;
        }
        
    }

    mPhase += 1;
}

void Environment::SetActions()
{
    mCreature->SetActivationLevels(mPhase);
    mPhase += 1;
}

void Environment::RecordSimWormSignal()
{
    if(!mbOutputFiles)
    {
        std::cout << "  -- Record Signal: Begin... " << std::endl;
        mRecordCounter = mRecordLength;
        mJsonActions.resize(mRecordLength, GetActionSize());
        mJsonStates.resize(mRecordLength, GetStateSize());
        mJsonMuscleActivations.resize(mRecordLength, GetMuscleActivationSize());
        mbOutputFiles = true;
    }
}

void Environment::WriteActions(const std::string& path_file, const Eigen::MatrixXd &acts)
{
    FILE* file = cFileUtil::OpenFile(path_file.c_str(), "w");
    if (!file)
    {
	 	std::cout << "  Create Action File Error!" << std::endl;
        return;
    }

	fprintf(file, "{\n");
	fprintf(file, "\"%s\": ", "Loop");
	fprintf(file, "\"%s\",\n", "none");

	fprintf(file, "\n");
	fprintf(file, "\"Frames\":\n[\n");

	for (int f = 0; f < mRecordLength; ++f)
	{
		if (f != 0)
		{
			fprintf(file, ",\n");
		}

		Eigen::VectorXd curr_frame = acts.row(f);
		std::string frame_json = cJsonUtil::BuildVectorJson(curr_frame);
		fprintf(file, "%s", frame_json.c_str());
	}

	fprintf(file, "\n]");
	fprintf(file, "\n}");
	cFileUtil::CloseFile(file);


	// Json::Value root;
	// Json::FastWriter writer;
	// std::ofstream act_file;
	// act_file.open("data/output/tmp/act_sig.json", std::ofstream::out/*  | std::ofstream::app */);
	// if (!act_file.good())
	// 	std::cout << "  Create Action File Error!" << std::endl;

    // root["Loop"] = "none";
	// root["FrameTime"] = 0.033;
	// for (int f = 0; f < mRecordLength; ++f)
	// {
	// 	if(f!=0) act_file << "," << std::endl;
    //     Eigen::VectorXd curr_frame = acts.row(f);
    //     std::string json_frame = cJsonUtil::BuildVectorJson(curr_frame);
    //     act_file << json_frame;
	// }
	
	// act_file << std::endl;

    // act_file << root;
	// act_file.close();
}

void Environment::WriteMuscleSignals(const std::string& path_file, const Eigen::MatrixXd& signals)
{
    FILE* file = cFileUtil::OpenFile(path_file.c_str(), "w");
    if (!file)
    {
	 	std::cout << "  Create Muscle Signal File Error!" << std::endl;
        return;
    }

	fprintf(file, "{\n");
	fprintf(file, "\"%s\": ", "Loop");
	fprintf(file, "\"%s\",\n", "none");

	fprintf(file, "\n");
	fprintf(file, "\"Frames\":\n[\n");

	for (int f = 0; f < mRecordLength; ++f)
	{
		if (f != 0)
		{
			fprintf(file, ",\n");
		}

		Eigen::VectorXd curr_frame = signals.row(f);
		std::string frame_json = cJsonUtil::BuildVectorJson(curr_frame);
		fprintf(file, "%s", frame_json.c_str());
	}

	fprintf(file, "\n]");
	fprintf(file, "\n}");
	cFileUtil::CloseFile(file);
}

void Environment::WriteStates(const std::string& path_file, const Eigen::MatrixXd& states)
{
    FILE* file = cFileUtil::OpenFile(path_file.c_str(), "w");
    if (!file)
    {
	 	std::cout << "  Create States File Error!" << std::endl;
        return;
    }

	fprintf(file, "{\n");
	fprintf(file, "\"%s\": ", "Loop");
	fprintf(file, "\"%s\",\n", "none");
    fprintf(file, "\"%s\": ", "NumSamplings");
    int ns = mCreature->GetSamplingIndex().size();
	fprintf(file, "%d,\n", ns);

	fprintf(file, "\n");
	fprintf(file, "\"Frames\":\n[\n");

	for (int f = 0; f < mRecordLength; ++f)
	{
		if (f != 0)
		{
			fprintf(file, ",\n");
		}

		Eigen::VectorXd curr_frame = states.row(f);
		std::string frame_json = cJsonUtil::BuildVectorJson(curr_frame);
		fprintf(file, "%s", frame_json.c_str());
	}

	fprintf(file, "\n]");
	fprintf(file, "\n}");
	cFileUtil::CloseFile(file);
}

std::map<std::string, double> Environment::CalcRewardImitate() const
{
	double pose_w = 0.5;
	double vel_w = 0.2;
	double end_eff_w = 0.1;
	double coord_w = 0.2;

	double total_w = pose_w + vel_w + end_eff_w + coord_w;
	pose_w /= total_w;
	vel_w /= total_w;
	end_eff_w /= total_w;
	coord_w /= total_w;

    Worm& sim_char = *mCreature;
    KeyWorm& kin_char = *mKeyCreature;

	int num_samplings = sim_char.GetSamplingIndex().size();
	assert(num_samplings == kin_char.GetNumSamplings());
	
	const double pose_scale = 3.0;
	const double vel_scale = 0.5;
	const double end_eff_scale = 10;
	const double coord_scale = 2;
	const double err_scale = 1;

	double reward = 0;

	const Eigen::VectorXd& kin_pos = kin_char.GetPos();
	const Eigen::VectorXd& kin_vel = kin_char.GetVel();

	double pose_err = 0;
	double vel_err = 0;
	double end_eff_err = 0;
	double coord_vel_err = 0;
	double heading_err = 0;

    double w = 1.0/num_samplings;
	for (int j = 0; j < num_samplings; ++j)
	{
        Eigen::Vector3d p0 = mStates.segment(6+3*j, 3);
        Eigen::Vector3d p1 = kin_pos.segment(3*j, 3);
        Eigen::Vector3d v0 = mStates.segment(6+num_samplings*3+3*j, 3);
        Eigen::Vector3d v1 = kin_pos.segment(3*j, 3);
        //Eigen::Quaternion rot_diff = Eigen::Quaternion::FromTwoVectors(p0, p1);
        //double rot_diff = p0.normalized().dot(p1.normalized());
		double curr_pose_err = (p0-p1).squaredNorm()/*  + rot_diff */;
		double curr_vel_err = (v0-v1).squaredNorm();//CalcVelErr(j, mStates.segment(6+3*num_samplings, 3*num_samplings), vel1);
		pose_err += w * curr_pose_err;
		vel_err += w * curr_vel_err;

		bool is_end_eff = sim_char.IsEndEffector(j);
		if (is_end_eff)
		{
            double curr_end_err = (p0 - p1).squaredNorm();
			end_eff_err += curr_end_err;
		}
	}

    const Eigen::Vector3d coord_vel1 = kin_char.GetCoordVel();
	coord_vel_err = (mStates.segment(3, 3) - coord_vel1).squaredNorm();

	double pose_reward = exp(-err_scale * pose_scale * pose_err);
	double vel_reward = exp(-err_scale * vel_scale * vel_err);
	double end_eff_reward = exp(-err_scale * end_eff_scale * end_eff_err);
	double coord_reward = exp(-err_scale * coord_scale * coord_vel_err);

	reward = pose_w * pose_reward + vel_w * vel_reward  + end_eff_w * end_eff_reward + coord_w * coord_reward;

    std::map<std::string, double> reward_map;
    reward_map["pos"] = pose_w * pose_reward;
    reward_map["vel"] = vel_w * vel_reward;   
    reward_map["endeff"] = end_eff_w * end_eff_reward;
    reward_map["coord"] = coord_w * coord_reward; 
    reward_map["total"] = reward_map["pos"] + reward_map["vel"] + reward_map["endeff"] + reward_map["coord"];

	return reward_map; 
}

std::map<std::string, double> Environment::GetRewards()
{
#if 1
    return CalcRewardImitate();
#else
    //double d = (mAverageVelocity-mTargetVelocity).norm();   
    //double reward_target = exp(-d*d/0.05);

    //Eigen::Vector3d v_face_dir = mCreature->GetForwardVector(mSoftWorld->GetPositions());
    //Eigen::Vector3d v_center_dir = mAverageVelocity.normalized();
    Eigen::Vector3d v_eval_coord_dir = mAverageEvalCoordVelocity.normalized();
    Eigen::Vector3d v_tar_dir = mTargetVelocity.normalized();

    //double v_diff = mAverageVelocity.dot(v_tar_dir)-mTargetVelocity.norm();
    //double reward_center = exp(-v_diff*v_diff/0.005);
    //if(reward_target<0.2) reward_target=0;

    double v_diff = mAverageEvalCoordVelocity.dot(v_tar_dir)-mTargetVelocity.norm();
    double reward_target = exp(-v_diff*v_diff/0.005);

    auto diff_direction = (v_eval_coord_dir - v_tar_dir).norm()/*L2*/;  // (v_face_dir - v_tar_dir).norm();
    double reward_direction = exp(-fabs(diff_direction)/0.5);

    const double w_target = 1.0;
    const double w_direction = 2.0;

    std::map<std::string, double> reward_map;
    //reward_map["center"] = w_center*reward_center;
    reward_map["target"] = w_target*reward_target;
    reward_map["direction"] = w_direction*reward_direction;   
    reward_map["total"] = reward_map["target"] + reward_map["direction"];

    return reward_map;
#endif
}

bool Environment::isEndOfEpisode()
{
    bool eoe = (mCurrIters>=mMaxIters) ? true : false; 

    return eoe;
}

bool Environment::CheckValidEpisode()  // todo
{
    //if(mCreature->Exploded()) return false;
    Eigen::VectorXd pos = mSoftWorld->GetPositions();
    int n = mSoftWorld->GetNumVertices();
    Eigen::Vector3d max, min; 
    max << -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max();
    min << std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max();
    for (int i = 0; i < n; ++i)
    {
        if(pos(i*3) < min(0)) min(0) = pos(i*3);
        if(pos(i*3) > max(0)) max(0) = pos(i*3);
        if(pos(i*3+1) < min(1)) min(1) = pos(i*3+1);
        if(pos(i*3+1) > max(1)) max(1) = pos(i*3+1);
        if(pos(i*3+2) < min(2)) min(2) = pos(i*3+2);
        if(pos(i*3+2) > max(2)) max(2) = pos(i*3+2);
    }
    mWormAABB = Eigen::AlignedBox3d(min, max);
    //std::cout << mWormAABB.volume() << " ";
    if (mWormAABB.volume() > 0.5/*todo*/)
    {
        std::cout << "  Warning: Invalid Episode!";
        return false;
    }
    
    return true;
}

void Environment::UpdateRandomTargetVelocity() 
{
    Eigen::Vector3d v,axis;

    double length = l_distribution(generator);

    v = length*mCreature->GetForwardVector(mSoftWorld->GetPositions()).normalized();
    //v = length*mAverageEvalCoordVelocity.normalized();

    double angle = angle_distribution(generator);
    axis[0] = vx_distribution(generator);
    axis[1] = vy_distribution(generator);
    axis[2] = vz_distribution(generator);

    Eigen::Matrix3d R;
    R = Eigen::AngleAxisd(angle, axis.normalized());

    mTargetVelocity = R*v; 

    //mTargetVelocity = Eigen::Vector3d(0, 0, -0.2);

    std::cout << "Avg Coord Velocity: [" << mAverageEvalCoordVelocity[0] << ", " << mAverageEvalCoordVelocity[1] << ", " << mAverageEvalCoordVelocity[2] << "]  "  
              << mAverageEvalCoordVelocity.norm()
              << "  Target Velocity: [" << mTargetVelocity[0] << ", " << mTargetVelocity[1] << ", " << mTargetVelocity[2] << "]  " 
              << mTargetVelocity.norm() << std::endl; 
}

void Environment::UpdateAverageVelocity()
{
    Eigen::Vector3d v_queue_front = Eigen::Vector3d(0, 0, 0);
    int deque_len = 2;  //mSimulationHz;
    if(mAverageVelocityDeque.size() > deque_len) 
    {
        v_queue_front = mAverageVelocityDeque.front();
        mAverageVelocityDeque.pop_front();
    }
    Eigen::Vector3d v_center = mSoftWorld->GetVelocities().block<3,1>(3*mCreature->GetCenterIndex(), 0);
    mAverageVelocityDeque.push_back(v_center);
    //mAverageVelocity = mAverageVelocity - (v_queue_front)/deque_len + v_center/deque_len;   // 迭代多了累积误差？
    mAverageVelocity = Eigen::Vector3d(0, 0, 0);
    std::for_each(mAverageVelocityDeque.begin(), mAverageVelocityDeque.end(), [&](Eigen::Vector3d v){ mAverageVelocity+=v;});
    mAverageVelocity /= mAverageVelocityDeque.size();

    // Average Velocity of Evaluated Coord
    double ratio = mSimulationHz/mControlHz;
    int deque_len2 = 6;
    if(mAverageEvalCoordPosDeque.size() > deque_len2)
        mAverageEvalCoordPosDeque.pop_front();

    Eigen::Vector3d eval_pos = mEvalLocalCoord.translation();  
    mAverageEvalCoordPosDeque.push_back(eval_pos);

    mAverageEvalCoordVelocity = Eigen::Vector3d(0, 0, 0);
    double timestep = mSoftWorld->GetTimeStep()*ratio/*important!*/;
    if (mAverageEvalCoordPosDeque.size() > 1)
    {
        for (int i = 1; i < mAverageEvalCoordPosDeque.size(); ++i)
            mAverageEvalCoordVelocity += (mAverageEvalCoordPosDeque[i] - mAverageEvalCoordPosDeque[i - 1])/timestep;
        mAverageEvalCoordVelocity /= (mAverageEvalCoordPosDeque.size() - 1);

        mCurrentCoordVelocity = (mAverageEvalCoordPosDeque[mAverageEvalCoordPosDeque.size()-1] - mAverageEvalCoordPosDeque[mAverageEvalCoordPosDeque.size()-2])/timestep;
    }
}

Eigen::Vector3d Environment::UpdateHeadLocation()
{
    auto localCoordSpeed = mStates.segment(3, 3);
    auto headLocalOffset = mStates.segment(6, 3);

    mLocalCoordPosition += localCoordSpeed / mControlHz;
    mHeadLocation = mLocalCoordPosition + headLocalOffset;
    return mHeadLocation;
}