#include "Worm.h"
#include "util/json/json.h"
#include "util/JsonUtil.h"
#include "util/FileUtil.h"

#include <fstream>
#include <iomanip>

#define LOCAL 0
#define GLOBAL 1
using namespace FEM;

Worm::Worm(const double &muscle_stiffness, const double &youngs_modulus, const double &poisson_ratio)
	: mMesh(), mMuscleStiffness(muscle_stiffness), mYoungsModulus(youngs_modulus), mPoissonRatio(poisson_ratio)
{
	mTotalMuscleSegments = 0;
}

void Worm::ReadMetaData(const std::string &path)
{
	std::cout << "+ Meta Data Path: " << path << std::endl;
	std::ifstream meta_file(path);
	std::string read_line;
	std::stringstream ss;

	// scale
	double scale = 0.01;
	std::getline(meta_file, read_line);
	ss.str(read_line);
	ss >> scale;

	std::getline(meta_file, read_line); // mesh
	Eigen::Affine3d T = Eigen::Affine3d::Identity();
	T(0, 0) *= scale;
	T(1, 1) *= scale;
	T(2, 2) *= scale; // scale
	SetMesh(read_line, T);

	std::getline(meta_file, read_line);
	MakeMuscles(std::string(SOFTCON_DIR) + "/data/" + read_line, 1.0);

	std::getline(meta_file, read_line);
	std::ifstream ifs(std::string(SOFTCON_DIR) + "/data/" + read_line);
	std::string str;
	int index;
	str.clear();
	ss.clear();
	std::getline(ifs, str);
	ss.str(str);
	while (ss >> index)
	{
		mSamplingIndex.push_back(index);
	}
	ifs.close();

	// center of mass
	const auto &vertices = mMesh->GetVertices();
	Eigen::Vector3d com = Eigen::Vector3d::Zero();
	for (int i = 0; i < vertices.size(); i++)
	{
		com += vertices[i];
	}
	com /= vertices.size();
	double minD = std::numeric_limits<double>::max();
	double d = 0;
	for (int i = 0; i < vertices.size(); ++i)
	{
		d = (vertices[i] - com).squaredNorm();
		if (d < minD)
		{
			minD = d;
			mCOMIndex = i;
		}
	}
	std::cout << "  Computing center of mass ...  " << mCOMIndex << " Done" << std::endl; // center 332; head 102

	// center index
	minD = std::numeric_limits<double>::max();
	d = 0;
	for (int i = 0; i < vertices.size(); ++i)
	{
		d = vertices[i].squaredNorm();
		if (d < minD)
		{
			minD = d;
			mCenterIndex = i;
		}
	}
	std::cout << "  Computing center index ...  " << mCenterIndex << " Done" << std::endl; // center 312; head 102

	ss.clear();
	std::getline(meta_file, read_line);
	ss.str(read_line.c_str());
	mEndEffectorIndex.reserve(2);
	ss >> mEndEffectorIndex[0] >> mEndEffectorIndex[1];

	ss.clear();
	std::getline(meta_file, read_line);
	ss.str(read_line.c_str());
	ss >> mLocalContourIndex[0] >> mLocalContourIndex[1] >> mLocalContourIndex[2];
	meta_file.close();
}

void Worm::ParseArgs(const std::shared_ptr<cArgParser>& parser)
{
	//cScene::ParseArgs(parser);

	bool succ = true;

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

Eigen::Matrix3d Worm::GetReferenceRotation(bool type, const Eigen::VectorXd &x)
{
	int idx0 = mLocalContourIndex[0];
	int idx1 = mLocalContourIndex[1];
	int idx2 = mLocalContourIndex[2];

	Eigen::Vector3d p0 = x.block<3, 1>(3 * idx0, 0);
	Eigen::Vector3d p1 = x.block<3, 1>(3 * idx1, 0);
	Eigen::Vector3d p2 = x.block<3, 1>(3 * idx2, 0);

	Eigen::Vector3d v_x = (p1 - p0).normalized();
	Eigen::Vector3d v_y = (p1 - p0).cross((p2 - p0)).normalized();
	Eigen::Vector3d v_z = v_x.cross(v_y).normalized();

	Eigen::Matrix3d R;
	R(0, 0) = v_x[0];
	R(0, 1) = v_y[0];
	R(0, 2) = v_z[0];
	R(1, 0) = v_x[1];
	R(1, 1) = v_y[1];
	R(1, 2) = v_z[1];
	R(2, 0) = v_x[2];
	R(2, 1) = v_y[2];
	R(2, 2) = v_z[2];

	if (type == LOCAL)
		return R.transpose();
	else
		return R;
}

void Worm::SetInitReferenceVertices(const Eigen::VectorXd &x)
{
	mVerticesReference = x;
}

void Worm::SetInitReferenceSamplingStates(const Eigen::VectorXd &x)
{
	for (int i = 0; i < mSamplingIndex.size(); i++)
	{
		mSamplingReference.push_back(x.block<3, 1>(3 * mSamplingIndex[i], 0));
	}
}

void Worm::SetInitReferenceRotation(const Eigen::VectorXd &x)
{
	mInitReferenceMatrix = GetReferenceRotation(GLOBAL, x);
}

Eigen::Vector3d Worm::GetUpVector(const Eigen::VectorXd &x)
{
	Eigen::Matrix3d R = GetReferenceRotation(GLOBAL, x);
	auto v_up_simul = R * mInitReferenceMatrix.inverse().col(1);
	return v_up_simul;
}

Eigen::Vector3d Worm::GetForwardVector(const Eigen::VectorXd &x)
{
	Eigen::Matrix3d R = GetReferenceRotation(GLOBAL, x);
	return R.col(1);
}

void Worm::Initialize(FEM::World *world)
{
	std::cout << "  Init Worm...";

	const auto &vertices = mMesh->GetVertices();
	const auto &tetrahedras = mMesh->GetTetrahedrons();

	std::vector<Eigen::Vector3i> triangles;
	std::vector<std::pair<Eigen::Vector3i, int>> surfaces;

	double total_volume = 0;
	double volume = 0;
	for (const auto &tet : tetrahedras)
	{
		int i0, i1, i2, i3;
		Eigen::Vector3d p0, p1, p2, p3;

		i0 = tet[0];
		i1 = tet[1];
		i2 = tet[2];
		i3 = tet[3];
		p0 = vertices[i0];
		p1 = vertices[i1];
		p2 = vertices[i2];
		p3 = vertices[i3];

		Eigen::Matrix3d Dm;

		Dm.block<3, 1>(0, 0) = p1 - p0;
		Dm.block<3, 1>(0, 1) = p2 - p0;
		Dm.block<3, 1>(0, 2) = p3 - p0;
		if (Dm.determinant() < 0)
		{
			i0 = tet[0];
			i1 = tet[2];
			i2 = tet[1];
			i3 = tet[3];
			p0 = vertices[i0];
			p1 = vertices[i1];
			p2 = vertices[i2];
			p3 = vertices[i3];
			Dm.block<3, 1>(0, 0) = p1 - p0;
			Dm.block<3, 1>(0, 1) = p2 - p0;
			Dm.block<3, 1>(0, 2) = p3 - p0;
		}
		volume = 1.0 / 6.0 * (Dm.determinant());
		total_volume += volume;
		mConstraints.push_back(new CorotateFEMConstraint(mYoungsModulus, mPoissonRatio, i0, i1, i2, i3, volume, Dm.inverse()));
		int sorted_idx[4] = {i0, i1, i2, i3};
		std::sort(sorted_idx, sorted_idx + 4);
		triangles.push_back(Eigen::Vector3i(sorted_idx[0], sorted_idx[1], sorted_idx[2]));
		triangles.push_back(Eigen::Vector3i(sorted_idx[0], sorted_idx[1], sorted_idx[3]));
		triangles.push_back(Eigen::Vector3i(sorted_idx[0], sorted_idx[2], sorted_idx[3]));
		triangles.push_back(Eigen::Vector3i(sorted_idx[1], sorted_idx[2], sorted_idx[3]));
	}
	// find surface
	for (int i = 0; i < triangles.size(); i++)
	{
		Eigen::Vector3i t_i = triangles[i];
		bool unique = true;
		for (int j = 0; j < triangles.size(); j++)
		{
			if (i == j)
				continue;
			if (t_i.isApprox(triangles[j]))
				unique = false;
		}
		if (unique)
			surfaces.push_back(std::make_pair(t_i, i / 4 /*tet index*/));
	}

	double total_surface_area = 0;
	for (int i = 0; i < surfaces.size(); i++)
	{
		Eigen::Vector3i t_i = surfaces[i].first;
		int tet_index = surfaces[i].second;

		int i0 = tetrahedras[tet_index][0], i1 = tetrahedras[tet_index][1], i2 = tetrahedras[tet_index][2], i3 = tetrahedras[tet_index][3];
		Eigen::Vector3d p0 = vertices[i0], p1 = vertices[i1], p2 = vertices[i2], p3 = vertices[i3];
		Eigen::Vector3d center = 0.25 * (p0 + p1 + p2 + p3);

		Eigen::Vector3d q0 = vertices[t_i[0]], q1 = vertices[t_i[1]], q2 = vertices[t_i[2]];
		Eigen::Vector3d n = (q1 - q0).cross(q2 - q1);

		if ((center - q0).dot(n) > 0.0)
		{
			int j1 = t_i[0];
			t_i[0] = t_i[1];
			t_i[1] = j1;
		}

		mContours.push_back(t_i);
		total_surface_area += 0.5 * n.norm();
	}

	for (const auto &m : mMuscles)
		for (const auto &s : m->GetSegments())
			for (int i = 0; i < s->GetMuscleConstraints().size(); i++)
				mConstraints.push_back(s->GetMuscleConstraints()[i]);

	Eigen::VectorXd v(vertices.size() * 3);
	for (int i = 0; i < vertices.size(); i++)
		v.block<3, 1>(i * 3, 0) = vertices[i];

	double mass = 10;
	std::cout << " Vertices: " << vertices.size() << " Triangles: " << triangles.size() << " Tets: " << tetrahedras.size()
			  << " Countours: " << mContours.size() << " Constraints: " << mConstraints.size() << std::endl;
	std::cout << "               Volume: " << total_volume << " Surface Area: " << total_surface_area << " Mass: " << mass << std::endl;

	world->AddBody(v, mConstraints, mass /*todo*/);

	//SetKey(std::string(SOFTCON_DIR) + "/data/worm_givenkey_tn_fast.csv");
	//SetKey(std::string(SOFTCON_DIR) + "/data/worm_givenkey_t0_fast.csv");
	//SetKey(std::string(SOFTCON_DIR) + "/data/worm_empty_key.csv");
	SetKey(std::string(SOFTCON_DIR) + "/data/worm_givenkey_turn2.csv"); // todo
	// SetKey();

	ReadC302(std::string(SOFTCON_DIR) + "/data/mengdi/eworm.muscle-220428-152601.txt");
}

void Worm::SetKey()
{
	Eigen::MatrixXd key;
	key.resize(50, mMuscles.size());
	key.setZero();

	for (int i = 0; i < key.cols(); i++)
	{
		mMuscles[i]->SetKey(key.col(i));
	}

#if 0
	std::ofstream key_file;
	key_file.open("worm_empty_key.csv");
	for(int i=0; i<key.rows(); i++)
	{
		key_file << key(i, 0) << "\t" << key(i, 1) << "\t" << key(i, 2) << "\t" << key(i, 3) << std::endl;
	}
	key_file.close();
#endif
}

void Worm::SetKey(std::string filename)
{
	Eigen::MatrixXd mGivenKey, key;
	std::ifstream given_key_file;
	given_key_file.open(filename);
	if (given_key_file.good())
		std::cout << "  Reading Given Key File " << filename << "... ";
	else
		std::cout << "  Read Given Key File " << filename << " Error!" << std::endl;

	std::stringstream ss;
	double d1 = 0, d2 = 0, d3 = 0, d4 = 0;

	int rows = 0, cols = mMuscles.size();
	for (std::string one_line; std::getline(given_key_file, one_line);)
		++rows;
	// given_key_file.seekg(0, given_key_file.end);
	// rows = given_key_file.tellg();
	mGivenKey.resize(rows, cols);

	given_key_file.clear();
	given_key_file.seekg(0, given_key_file.beg);

	int i = 0;
	for (std::string one_line; std::getline(given_key_file, one_line);)
	{
		ss.clear();
		ss.str(one_line);
		ss >> d1 >> d2 >> d3 >> d4;
		// std::cout << d1 << " " << d2 << " " << d3 << " " << d4 << std::endl;
		mGivenKey.row(i) << d1, d2, d3, d4;
		++i;
	}

	given_key_file.close();

	// for (int i = 0; i < 20; i++)
	// {
	// 	mGivenKey.row(i) << 0.025, -0.0625, 0.025, -0.0625;	// 0.025, -0.0625, 0.025, -0.0625, 0.025, -0.0625, 0.025, -0.0625, 0.025, -0.0625, 0.025, -0.0625;
	// }
	// for (int i = 20; i < 30; i++)
	// {
	// 	mGivenKey.row(i) << -0.125, 0.03125, -0.125, 0.03125; // -0.125, 0.03125, -0.125, 0.03125, -0.125, 0.03125, -0.125, 0.03125, -0.125, 0.03125, -0.125, 0.03125;
	// }
	// for (int i = 30; i < 50; i++)
	// {
	// 	mGivenKey.row(i) << 0.0, -0.125, 0.0, -0.125; // 0.0, -0.125, 0.0, -0.125, 0.0, -0.125, 0.0, -0.125, 0.0, -0.125, 0.0, -0.125;
	// }

	key.resize(mGivenKey.rows(), mGivenKey.cols());
	key.setZero();

	for (int j = 0; j < key.cols(); j++) // 4
	{
		for (int i = 0; i < key.rows(); i++) // 50
		{
			if (i == 0)
			{
				key(i, j) += mGivenKey(i, j) * 20;
			}
			else
			{
				key(i, j) += /* key(i - 1, j) +  */ mGivenKey(i, j) * 20;
			}

			key(i, j) = std::min(std::max(key(i, j), 0.0), 1.0);
		}
	}

	for (int i = 0; i < key.cols(); i++)
	{
		mMuscles[i]->SetKey(key.col(i));
	}

	std::cout << "Done! Rows*Cols=" << key.rows() << "*" << key.cols() << std::endl;

#if 0
	std::ofstream key_file;
	key_file.open("worm_key.csv");
	for(int i=0; i<key.rows(); i++)
	{
		key_file << key(i, 0) << "\t" << key(i, 1) << "\t" << key(i, 2) << "\t" << key(i, 3) << std::endl;
	}
	key_file.close();
#endif
}

void Worm::ReadC302(std::string filename)
{
	Eigen::MatrixXd key;
	std::ifstream c302_file;
	c302_file.open(filename);
	if (c302_file.good())
		std::cout << "  Reading C302 File " << filename << "... ";
	else
		std::cout << "  Read C302 File " << filename << " Error!" << std::endl;

	std::stringstream ss;
	double d = 0;
	double dmax = -std::numeric_limits<double>::max();
	double dmin = std::numeric_limits<double>::max();

	int t = 0;
	for (std::string one_line; std::getline(c302_file, one_line);)
	{
		ss.clear();
		ss.str(one_line);
		for (int i = 0; i < mMuscles.size(); ++i)
		{
			std::vector<Eigen::VectorXd> &c302Act = mMuscles[i]->GetC302Activations();
			for (int j = 0; j < mMuscles[i]->GetNumSampling(); ++j)
			{
				ss >> d;
				if (d < dmin)
					dmin = d;
				if (d > dmax)
					dmax = d;
			}
		}
		++t;
	}

	c302_file.clear();
	c302_file.seekg(0, c302_file.beg);

	t = 0;
	const double scale = 1; // todo
	for (std::string one_line; std::getline(c302_file, one_line);)
	{
		ss.clear();
		ss.str(one_line);
		for (int i = 0; i < mMuscles.size(); ++i)
		{
			std::vector<Eigen::VectorXd> &c302Act = mMuscles[i]->GetC302Activations();
			Eigen::VectorXd muscleAct(mMuscles[i]->GetNumSampling());
			for (int j = 0; j < mMuscles[i]->GetNumSampling(); ++j)
			{
				ss >> d;
				muscleAct(j) = d;
				//muscleAct(j) = (1 - (dmax - d) / (dmax - dmin));
				// if(muscleAct(j)<0.4) muscleAct(j)=0;
				muscleAct(j) *= scale;
			}
			c302Act.push_back(muscleAct);
		}
		++t;
	}

	c302_file.close();
	std::cout << "Done! total time=" << t << " value of [" << dmin << ", " << dmax << "]" << std::endl;
}

void Worm::GetMuscleActivations(Eigen::VectorXd& activations)
{
	for (int i = 0; i < mMuscles.size(); ++i)
	{
		Eigen::VectorXd muscle_signals = mMuscles[i]->GetActivationLevels();
		activations << muscle_signals;
	}

	Eigen::VectorXd sig0 = mMuscles[0]->GetActivationLevels();
	Eigen::VectorXd sig1 = mMuscles[1]->GetActivationLevels();
	Eigen::VectorXd sig2 = mMuscles[2]->GetActivationLevels();
	Eigen::VectorXd sig3 = mMuscles[3]->GetActivationLevels();
	activations << sig0, sig1, sig2, sig3;
}

void Worm::WriteMuscleSignal()
{
	Eigen::MatrixXd key;
	std::ofstream muscle_file;
	muscle_file.open("data/output/Muscle_Signal.csv", std::ofstream::out | std::ofstream::app);
	if (!muscle_file.good())
		std::cout << "  Open Muscle Signal File Error!" << std::endl;

	muscle_file << std::setw(9) << std::fixed << std::setprecision(4);
	for (int i = 0; i < mMuscles.size(); ++i)
	{
		Eigen::VectorXd acts = mMuscles[i]->GetActivationLevels();
		for (int j = 0; j < mMuscles[i]->GetNumSampling(); ++j)
		{
			muscle_file << acts(j) << " ";
		}
	}

	muscle_file << std::endl;

	muscle_file.close();
	// std::cout << "  -- Output Muscle Signal... " << std::endl;
}

void Worm::WriteMesh(const std::string& path_file, FEM::World *world)
{
	Eigen::VectorXd velocities = world->GetVelocities();
	Eigen::VectorXd positions = world->GetPositions();
	unsigned int n = positions.size() / 3;

	Json::Value root;
	Json::Value pos(Json::arrayValue), vel(Json::arrayValue);
	std::ofstream json_file(path_file);
	if (!json_file.good())
		std::cout << "  Open Mesh File Error!" << std::endl;

	Eigen::Vector3d p, v;
	for (unsigned int i = 0; i < n; i++)
	{
		p = positions.segment<3>(i*3);
		pos.append(Json::Value(p(0)));
		pos.append(Json::Value(p(1)));
		pos.append(Json::Value(p(2)));
		v = velocities.segment<3>(i*3);
		vel.append(Json::Value(v(0)));
		vel.append(Json::Value(v(1)));
		vel.append(Json::Value(v(2)));
	}

	root["nVertices"] = n;
	root["Position"] = pos;
	root["Velocity"] = vel;
	json_file << root;
	json_file.close();

	// export obj file
	{
		std::ofstream obj_file;
		std::string obj_path_file = path_file;
		obj_path_file.replace(obj_path_file.rfind(".json"), 5, ".obj");
		obj_file.open(obj_path_file, std::ofstream::out);
		if (!obj_file.good())
			std::cout << "  Open Obj File Error!" << std::endl;

		obj_file << "# From MetaWorm WriteMesh() " << std::endl;
		obj_file << "o worm_mesh" << std::endl;
		for (unsigned int i = 0; i < n; i++)
		{
			// obj_file << "v " << positions.block<3, 1>(i*3, 0)(0) << " " 
			// 				 << positions.block<3, 1>(i*3, 0)(1) << " " 
			// 				 << positions.block<3, 1>(i*3, 0)(2) << std::endl;
			obj_file << "v " << positions.segment<3>(i*3)(0) << " " 
							 << positions.segment<3>(i*3)(1) << " " 
							 << positions.segment<3>(i*3)(2) << std::endl;
			positions.segment<3>(i*3);
		}

		// for (unsigned int i = 0; i < n; i++)
		// {
		// 	obj_file << "vn " << mVertexNormal.segment<3>(i*3)(0) << " " 
		// 					  << mVertexNormal.segment<3>(i*3)(1) << " " 
		// 				      << mVertexNormal.segment<3>(i*3)(2) << std::endl;
		// }

		for (unsigned int i = 0; i < mContours.size(); i++)
		{
			obj_file << "f " << mContours[i](0)+1 << " " 
							 << mContours[i](1)+1 << " " 
							 << mContours[i](2)+1 << std::endl;
		}

		// const std::vector<Eigen::Vector4i>& tets = GetTets();
		// Eigen::Vector3d p0, p1, p2, p3;
		// for(const auto& t : tets)
		// {
		// 	p0 = positions.block<3, 1>(3*t[0], 0);
		// 	p1 = positions.block<3, 1>(3*t[1], 0);
		// 	p2 = positions.block<3, 1>(3*t[2], 0);
		// 	p3 = positions.block<3, 1>(3*t[3], 0);
			
		// 	Eigen::Vector3d center = 0.25*(p0+p1+p2+p3);
		// 	Eigen::Vector3d normal = ((p1-p0).cross(p2-p0)).normalized();
		// 	if((center-Eigen::Vector3d((p0+p1+p2)/3.0)).dot(normal)>0.0)
		// 	{
		// 		obj_file << "f " << t[0]+1 << " "  << t[2]+1 << " "  << t[1]+1 << std::endl;
		// 	} else {
		// 		obj_file << "f " << t[0]+1 << " "  << t[1]+1 << " "  << t[2]+1 << std::endl;
		// 	}

		// 	normal = ((p1-p0).cross(p3-p0)).normalized();
		// 	if((center-Eigen::Vector3d((p0+p1+p3)/3.0)).dot(normal)>0.0)
		// 	{
		// 		obj_file << "f " << t[0]+1 << " "  << t[3]+1 << " "  << t[1]+1 << std::endl;
		// 	} else {
		// 		obj_file << "f " << t[0]+1 << " "  << t[1]+1 << " "  << t[3]+1 << std::endl;
		// 	}

		// 	normal = ((p2-p0).cross(p3-p0)).normalized();
		// 	if((center-Eigen::Vector3d((p0+p2+p3)/3.0)).dot(normal)>0.0)
		// 	{
		// 		obj_file << "f " << t[0]+1 << " "  << t[3]+1 << " "  << t[2]+1 << std::endl;
		// 	} else {
		// 		obj_file << "f " << t[0]+1 << " "  << t[2]+1 << " "  << t[3]+1 << std::endl;
		// 	}

		// 	normal = ((p2-p1).cross(p3-p1)).normalized();
		// 	if((center-Eigen::Vector3d((p1+p2+p3)/3.0)).dot(normal)>0.0)
		// 	{
		// 		obj_file << "f " << t[1]+1 << " "  << t[3]+1 << " "  << t[2]+1 << std::endl;
		// 	} else {
		// 		obj_file << "f " << t[1]+1 << " "  << t[2]+1 << " "  << t[3]+1 << std::endl;
		// 	}
		// }

		obj_file.close();
	}
}

void Worm::SetActivationLevels(const Eigen::VectorXd& actions, const unsigned int& phase)
{
	int idx = 0;
	for (int i = 0; i < mMuscles.size(); i++)
	{
		mMuscles[i]->SetActivationLevels(actions.segment(idx, 4), phase); // modulate with local CPG muscle excitation model
		idx += 4;
	}
}

void Worm::SetActivationLevels(const unsigned int &phase)
{
	for (int i = 0; i < mMuscles.size(); i++)
	{
		mMuscles[i]->SetActivationLevels(phase);
	}
}

void Worm::SetActivationLevelsDirectly(const Eigen::VectorXd &activations)
{
	int idx = 0;
	for (int i = 0; i < mMuscles.size(); i++)
	{
		int sampleCount = mMuscles[i]->GetNumSampling();
		mMuscles[i]->SetActivationLevelsDirectly(activations.segment(idx, sampleCount));
		idx += sampleCount;
	}
}

void Worm::GetMuscleActions(Eigen::VectorXd& muscle_actions)
{
	Eigen::VectorXd act0 = mMuscles[0]->GetActions();
	Eigen::VectorXd act1 = mMuscles[1]->GetActions();
	Eigen::VectorXd act2 = mMuscles[2]->GetActions();
	Eigen::VectorXd act3 = mMuscles[3]->GetActions();
	muscle_actions << act0, act1, act2, act3;
}

void Worm::SetMesh(const std::string &path, const Eigen::Affine3d &T)
{
	mScalingMatrix = T;
	mMesh = new MeditMesh(std::string(SOFTCON_DIR) + "/data/" + path, mScalingMatrix);
}

void Worm::SetWormStates(const std::string& path_file, FEM::World *world)
{
	//Json::Value pos(Json::arrayValue), vel(Json::arrayValue);
	std::ifstream json_file;
	json_file.open(path_file);
	if (!json_file.good())
	{
		std::cout << "  Open Mesh File Error! Can NOT set Worm States!" << std::endl;
		return;
	}

	Json::Reader reader;
	Json::Value root;
	bool succ = false;
	succ = reader.parse(json_file, root);
	json_file.close();

	if(succ)
	{
		int numVertices;
		if(!root["nVertices"].isNull())
		{
			Json::Value nv_val = root["nVertices"];
			numVertices = nv_val.asInt();
		}

		Eigen::VectorXd pos(numVertices*3), vel(numVertices*3);

		if(!root["Position"].isNull())
		{
			Json::Value p = root["Position"];
			int n = p.size();
			for(int j = 0; j<n/3; ++j)
			{
				Json::Value p1_val = p.get(j*3, 0);
				Json::Value p2_val = p.get(j*3+1, 0);
				Json::Value p3_val = p.get(j*3+2, 0);
				pos(j*3) = p1_val.asDouble();
				pos(j*3+1) = p2_val.asDouble();
				pos(j*3+2) = p3_val.asDouble();
			}
		}

		if(!root["Velocity"].isNull())
		{
			Json::Value v = root["Velocity"];
			int n = v.size();
			for(int j = 0; j<n/3; ++j)
			{
				Json::Value v1_val = v.get(j*3, 0);
				Json::Value v2_val = v.get(j*3+1, 0);
				Json::Value v3_val = v.get(j*3+2, 0);
				vel(j*3) = v1_val.asDouble();
				vel(j*3+1) = v2_val.asDouble();
				vel(j*3+2) = v3_val.asDouble();
			}
		}

		world->SetPositions(pos);
		world->SetVelocities(vel);
	}
}

void Worm::MakeMuscles(const std::string &path, const double &gamma)
{
	std::ifstream ifs(path);
	if (!ifs.is_open())
	{
		std::cout << "Muscle file doesn't exist." << std::endl;
		exit(0);
	}
	std::string str;
	std::string index;
	std::stringstream ss;

	int num_sampling = 24;

	int current_fiber_index = 0;
	std::string muscle_name;
	bool bNewFiber = false;
	Eigen::Vector3d point;
	std::vector<Eigen::Vector3d> point_list;

	while (!ifs.eof())
	{
		str.clear();
		index.clear();
		ss.clear();

		std::getline(ifs, str);
		ss.str(str);
		ss >> index;

		if (!index.compare("o"))
		{
			ss >> muscle_name;
			bNewFiber = true;
		}
		else if (!index.compare("v"))
		{
			ss >> point[0] >> point[1] >> point[2];
			point = mScalingMatrix * point;
			point_list.push_back(point);
		}
		else if (!index.compare("l"))
		{
			if (bNewFiber)
			{
				mMuscles.push_back(new Muscle(num_sampling, point_list));
				Muscle *muscle = mMuscles.back();
				muscle->Initialize(mMesh, mMuscleStiffness, muscle_name);
				std::cout << "  Reading Muscle " << muscle_name << "...  Successed!  "
						  << " length: " << point_list.size() << "  MaxMinAvg: " << muscle->mStats.maxD << " " << muscle->mStats.minD << " " << muscle->mStats.avgD << std::endl;

				point_list.clear();
				current_fiber_index++;
				bNewFiber = false;
			}
		}
	}

	mTotalMuscleSegments = mMuscles.size()*num_sampling;
}

double C_d(double theta)
{
	return -cos(theta * 2.0) + 1.05;
}

double C_t(double theta)
{
	return 0.25 * (exp(0.8 * theta) - 1);
}

double Cd(double aoa, double a, double c)
{
	return (a - c) * (0.5 * (-cos(2.0 * aoa) + 1.0)) + c;
}

double _interpolate(double t, double a, double b)
{
	return (1.0 - t) * a + t * b;
}

double Cl(double aoa, double cutoff, double x[5], double y[5])
{
	const double xa = x[0], ya = y[0];
	const double xb = x[1], yb = y[1];
	const double xc = x[2], yc = y[2];
	const double xd = x[3], yd = y[3];
	const double xe = x[4], ye = y[4];

	double theta = aoa * 180.0 / M_PI;

	if (fabs(theta) > cutoff)
		return 0.0;

	if (xa <= theta && theta < xb)
		return _interpolate((theta - xa) / (xb - xa), ya, yb);
	else if (xb <= theta && theta < xc)
		return _interpolate((theta - xb) / (xc - xb), yb, yc);
	else if (xc <= theta && theta <= xd)
		return _interpolate((theta - xc) / (xd - xc), yc, yd);
	else if (xd <= theta && theta <= xe)
		return _interpolate((theta - xd) / (xe - xd), yd, ye);
	else
	{
		std::cout << "Error: this should not be reached... " << std::endl;
		std::cout << "Theta: " << aoa << "(deg) " << theta << "(rad)" << std::endl;
		std::cout << "x: " << xa << " " << xb << " " << xc << " " << xd << std::endl;
		std::cout << "y: " << ya << " " << yb << " " << yc << " " << yd << std::endl;
		std::cin.get();
	}
	return 0.0;
}

Eigen::VectorXd Worm::ComputeDragForces(FEM::World *world)
{
	// const Eigen::VectorXd& x = world->GetPositions();
	// const Eigen::VectorXd& x_dot = world->GetVelocities();

	// int n = x.rows();
	// Eigen::VectorXd f = Eigen::VectorXd::Zero(n);
	// Eigen::Vector3d f_sum = Eigen::Vector3d::Zero();
	// Eigen::Vector3d avg_vel = Eigen::Vector3d::Zero();

	// for(const auto& con : mContours)
	// {
	// 	int i0 = con[0];
	// 	int i1 = con[1];
	// 	int i2 = con[2];

	// 	Eigen::Vector3d p0 = x.segment<3>(i0*3);
	// 	Eigen::Vector3d p1 = x.segment<3>(i1*3);
	// 	Eigen::Vector3d p2 = x.segment<3>(i2*3);
	// 	Eigen::Vector3d com = (p0+p1+p2)/3.0;

	// 	Eigen::Vector3d v0 = x_dot.segment<3>(i0*3);
	// 	Eigen::Vector3d v1 = x_dot.segment<3>(i1*3);
	// 	Eigen::Vector3d v2 = x_dot.segment<3>(i2*3);

	// 	Eigen::Vector3d v = -(v0+v1+v2)/3.0;
	// 	if(v.norm()<1E-6)
	// 		continue;
	// 	Eigen::Vector3d n = (p1-p0).cross(p2-p1);

	// 	double area = 0.5*n.norm();
	// 	n.normalize();
	// 	n = -n;

	// 	double theta = atan2(v.dot(n),(v-v.dot(n)*n).norm());
	// 	Eigen::Vector3d d = v.normalized();
	// 	Eigen::Vector3d t = -n;
	// 	Eigen::Vector3d fv = GetForwardVector(x);
	// 	double theta2 = atan2(t.dot(fv),(t-t.dot(fv)*fv).norm());
	// 	Eigen::Vector3d l = d.cross(n);

	// 	l.normalize();
	// 	l = l.cross(d);

	// 	double f_d = 0.5*1000.0*area*v.squaredNorm()*C_d(theta)*0.5;
	// 	double f_t = 0.5*1000.0*area*v.squaredNorm()*C_t(theta2)*std::abs(fv.dot(t))*2;
	// 	Eigen::Vector3d f_i = 0.333*(f_d*d+f_t*t);
	// 	f.segment<3>(i0*3) += f_i;
	// 	f.segment<3>(i1*3) += f_i;
	// 	f.segment<3>(i2*3) += f_i;
	// }
	// double clip_f = 1E3;
	// for(int i =0;i<n;i++)
	// {
	// 	f[i] = std::max(-clip_f,std::min(clip_f,f[i]));
	// }
	// return f;
	Eigen::VectorXd force_drag = Eigen::VectorXd::Zero(3 * world->GetNumVertices());
	Eigen::VectorXd force_lift = Eigen::VectorXd::Zero(3 * world->GetNumVertices());
	Eigen::VectorXd force_total = Eigen::VectorXd::Zero(3 * world->GetNumVertices());
	Eigen::VectorXd velocities = world->GetVelocities();
	Eigen::VectorXd positions = world->GetPositions();

	double max_force = 1.0e03;

	double cl_x[5] = {-90, -10, -5, 15, 90};
	double cl_y[5] = {-0.5, -1.26, 0.0, 1.8, 0.5};
	double cutoff = 85.0;

	for (int i = 0; i < mContours.size(); i++)
	{
		Eigen::Vector3d p1 = positions.segment<3>(mContours[i][0] * 3);
		Eigen::Vector3d p2 = positions.segment<3>(mContours[i][1] * 3);
		Eigen::Vector3d p3 = positions.segment<3>(mContours[i][2] * 3);
		Eigen::Vector3d v1 = velocities.segment<3>(mContours[i][0] * 3);
		Eigen::Vector3d v2 = velocities.segment<3>(mContours[i][1] * 3);
		Eigen::Vector3d v3 = velocities.segment<3>(mContours[i][2] * 3);

		Eigen::Vector3d v_water(0.0, 0.0, 0.0);
		Eigen::Vector3d v_rel = v_water - (v1 + v2 + v3) / 3.0;
		if (v_rel.norm() < 1e-6)
			continue;
		Eigen::Vector3d v_rel_norm = v_rel.normalized();

		Eigen::Vector3d n = ((p2 - p1).cross(p3 - p1)).normalized();
		Eigen::Vector3d d = -n;
		Eigen::Vector3d d_lift(0.0, 0.0, 0.0);

		Eigen::Vector3d force_drag_per_face(0.0, 0.0, 0.0);
		Eigen::Vector3d force_lift_per_face(0.0, 0.0, 0.0);

		double area = 0.5 * ((p2 - p1).cross(p3 - p1)).norm();

		if (area < 1.0e-10)
		{
			std::cerr << "Error: Area is too small, you should check the simulation" << std::endl;
			std::cin.get();
		}

		double d_dot_v_rel_norm = d.dot(v_rel_norm);
		double aoa = 0.5 * M_PI - acos(d_dot_v_rel_norm); // angle of attack
		double f = 1.0;

		// Ignore faces which have reverse direction
		if (d_dot_v_rel_norm > 0.0)
		{
			if (d_dot_v_rel_norm > 1.0 || d_dot_v_rel_norm < -1.0)
			{
				std::cerr << "Error: this should not be reached... " << std::endl;
				std::cerr << "d_dot_v_rel_norm: " << d_dot_v_rel_norm << std::endl;
				std::cerr << "Drag: " << d.transpose() << std::endl;
				std::cerr << "Vel: " << v_rel_norm.transpose() << std::endl;
				std::cin.get();
			}

			if (fabs(aoa) > 0.5 * M_PI)
			{
				std::cerr << "Error: this should not be reached... " << std::endl;
				std::cerr << "aoa: " << aoa << std::endl;
				std::cin.get();
			}

			force_drag_per_face = 0.5 * 1000.0 * (1.0 * area) * Cd(aoa, f, 0.0) * v_rel.squaredNorm() * v_rel_norm;
		}

		if (d_dot_v_rel_norm > 0.0)
		{
			d_lift = (v_rel_norm.cross(d)).normalized().cross(v_rel_norm);
			force_lift_per_face = 0.5 * 1000.0 * (1.0 * area) * Cl(aoa, cutoff, cl_x, cl_y) * v_rel.squaredNorm() * d_lift;
		}

		if (force_drag_per_face.norm() > max_force)
		{
			std::cerr << "Error: Drag force max_force reached..." << std::endl;
			force_drag_per_face = max_force * force_drag_per_face.normalized();
		}

		if (force_lift_per_face.norm() > max_force)
		{
			std::cerr << "Error: Thrust force max_force reached..." << std::endl;
			force_lift_per_face = max_force * force_lift_per_face.normalized();
		}

		force_total.segment<3>(mContours[i][0] * 3) += force_drag_per_face / 3.0 + force_lift_per_face / 3.0;
		force_total.segment<3>(mContours[i][1] * 3) += force_drag_per_face / 3.0 + force_lift_per_face / 3.0;
		force_total.segment<3>(mContours[i][2] * 3) += force_drag_per_face / 3.0 + force_lift_per_face / 3.0;
	}

	return force_total;
}

void Worm::SetVertexNormal()
{
	const auto &vertices = mMesh->GetVertices();
	const auto &tetrahedras = mMesh->GetTetrahedrons();

	mVertexNormal.resize(3 * vertices.size());
	mVertexNormal.setZero();
	Eigen::VectorXd cnt_list(vertices.size());
	cnt_list.setZero();

	for (int i = 0; i < mContours.size(); i++)
	{
		int i0 = mContours[i][0];
		int i1 = mContours[i][1];
		int i2 = mContours[i][2];

		Eigen::Vector3d p0 = vertices[i0];
		Eigen::Vector3d p1 = vertices[i1];
		Eigen::Vector3d p2 = vertices[i2];

		Eigen::Vector3d face_normal;
		face_normal = (p1 - p0).cross(p2 - p1);

		mVertexNormal.block<3, 1>(3 * i0, 0) = cnt_list[i0] / (cnt_list[i0] + 1) * mVertexNormal.block<3, 1>(3 * i0, 0) + 1 / (cnt_list[i0] + 1) * face_normal;
		mVertexNormal.block<3, 1>(3 * i1, 0) = cnt_list[i1] / (cnt_list[i1] + 1) * mVertexNormal.block<3, 1>(3 * i1, 0) + 1 / (cnt_list[i1] + 1) * face_normal;
		mVertexNormal.block<3, 1>(3 * i2, 0) = cnt_list[i2] / (cnt_list[i2] + 1) * mVertexNormal.block<3, 1>(3 * i2, 0) + 1 / (cnt_list[i2] + 1) * face_normal;

		cnt_list[i0] += 1;
		cnt_list[i1] += 1;
		cnt_list[i2] += 1;
	}
}

Eigen::Affine3d Worm::EvalBodyTransform(const Eigen::VectorXd &x, const Eigen::VectorXd &y)
{
	int num = x.rows()/3;	// x.cols();
	//Eigen::Vector3d X = Eigen::Map<Eigen::Vector3d>(test.data(), 3, dim);
	//std::vector<Eigen::Vector3d> x_(x.data(), x.data()+x.size());
	Eigen::MatrixXd X(3, num);
	Eigen::MatrixXd Y(3, num);

	for (int i = 0; i < num; i++)
	{
		X.col(i) = x.block<3, 1>(3*i, 0);
		Y.col(i) = y.block<3, 1>(3*i, 0);
	}
	
	/// De-mean
	int dim = X.rows();
	Eigen::Vector3d X_mean, Y_mean;
	for (int i = 0; i < dim; ++i)
	{
		X_mean(i) = X.row(i).array().sum();
		Y_mean(i) = Y.row(i).array().sum();
	}
	X_mean /= num; Y_mean /= num;
	//std::cout << "X_mean:" << X_mean << "Y_mean" << Y_mean << std::endl;
	X.colwise() -= X_mean;
	Y.colwise() -= Y_mean;

	/// Compute transformation
	Eigen::Affine3d transformation;
	Eigen::Matrix3d sigma = X * Y.transpose();
	Eigen::JacobiSVD<Eigen::Matrix3d> svd(sigma, Eigen::ComputeFullU | Eigen::ComputeFullV);
	if (svd.matrixU().determinant() * svd.matrixV().determinant() < 0.0)
	{
		Eigen::VectorXd S = Eigen::VectorXd::Ones(dim);
		S(dim - 1) = -1.0;
		transformation.linear() = svd.matrixV() * S.asDiagonal() * svd.matrixU().transpose();
		//assert(false);
	}
	else
	{
		transformation.linear() = svd.matrixV() * svd.matrixU().transpose();	// rotation
	}
	transformation.translation() = Y_mean - transformation.linear() * X_mean;
	//Eigen::Vector3d t = Y_mean - transformation.linear() * X_mean;
	//std::cout << "x: " << t(0) << " " << t(1) << " " << t(2) << std::endl;
	/// Re-apply mean
	//X.colwise() += X_mean;
	//Y.colwise() += Y_mean;

	return transformation;
}

bool Worm::IsEndEffector(int idx)
{
	if(idx==0 || idx==(mSamplingIndex.size()-1))
		return true;
	else
		return false;
}