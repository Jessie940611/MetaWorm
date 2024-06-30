#include "interact.h"
#include "../../neuronXcore/rtn/Viewer.h"

Eigen::VectorXd toEigenVector(const np::ndarray& array, int n)
{
	Eigen::VectorXd vec(n);

	float* srcs = reinterpret_cast<float*>(array.get_data());

	for(int i=0;i<n;i++)
	{
		vec[i] = srcs[i];
	}
	return vec;
}

//always return 1-dim array
np::ndarray toNumPyArray(const Eigen::VectorXd& vec)
{
	int n = vec.rows();
	p::tuple shape = p::make_tuple(n);
	np::dtype dtype = np::dtype::get_builtin<float>();
	np::ndarray array = np::empty(shape,dtype);

	float* dest = reinterpret_cast<float*>(array.get_data());
	for(int i =0;i<n;i++)
	{
		dest[i] = vec[i];
	}

	return array;
}

using namespace rtn;

namespace Interact
{
    static SimulationWindow* g_sim=nullptr;
    static TubesViewer* g_viewer = nullptr;

    void InitEnvironment(SimulationWindow *sim)
    {
        g_sim = sim;
    }

    void InitTubesViewer(TubesViewer *viewer)
    {
        g_viewer = viewer;
    }

    void SetActivationLevels(np::ndarray a, int wormId)
    {
        if (g_sim == nullptr)
        {
            std::cout << "Environtment not init!" << std::endl;
            return;
        }

        int length = a.shape(0);
        if (length != 96)
        {
            std::cout << "Activation levels size not match! "
                      << " expect 96"
                      << " get " << length << std::endl;
            return;
        }

        auto worm = g_sim->GetEnvs()->at(wormId)->GetCreature();
        worm->SetActivationLevelsDirectly(toEigenVector(a, length));
    }

    np::ndarray GetHeadLocation(int wormId)
    {
        if (g_sim == nullptr)
        {
            p::tuple shape = p::make_tuple(3);
            np::dtype dtype = np::dtype::get_builtin<float>();
            np::ndarray ret = np::empty(shape, dtype);

            std::cout << "Environtment not init!" << std::endl;
            return ret;
        }

        // return toNumPyArray(g_sim->GetEnvs()->at(wormId)->GetHeadLocation());

        // std::cout << "\nlocal coord speed\n" << localCoordSpeed
        //     << "head offset\n" << headLocalOffset
        //     << "position\n" << pos << std::endl;

        const Eigen::VectorXd &x = g_sim->GetmWorms()[wormId]->GetSoftWorld()->GetPositions();
        Eigen::Vector3i idx = g_sim->GetmWorms()[wormId]->GetCreature()->GetLocalContourIndex();
        return toNumPyArray(x.block<3, 1>(3 * idx[0], 0));
    }

    // 
    np::ndarray GetTrackingPoint(int wormId)
    {
        if (g_sim == nullptr)
        {
            p::tuple shape = p::make_tuple(3);
            np::dtype dtype = np::dtype::get_builtin<float>();
            np::ndarray ret = np::empty(shape, dtype);

            std::cout << "Environtment not init!" << std::endl;
            return ret;
        }

        const Eigen::VectorXd& x = g_sim->GetmWorms()[wormId]->GetSoftWorld()->GetPositions();  // XYZ of target worm
        const Eigen::VectorXd& v = g_sim->GetmWorms()[wormId]->GetSoftWorld()->GetVelocities(); // Vel of target worm
        //const Eigen::VectorXd& refVertices = g_sim->GetmWorms()[wormId]->GetCreature()->GetVerticesReference();     // XYZ of standard worm,t0
        const std::vector<Eigen::Vector3d>& sampling_ref_pos = g_sim->GetmWorms()[wormId]->GetCreature()->GetSamplingReference();
        Eigen::Affine3d eval_local_coord = g_sim->GetmWorms()[wormId]->GetEvalLocalCoord();
        Eigen::Vector3d currCoordVelocity = g_sim->GetmWorms()[wormId]->GetCurrentCoordVelocity();
            
        //--------------------------------------------------
        //     ref_coord    eval_local_coord
        // world -----> worm_ref -----> worm_local
        //--------------------------------------------------
        Eigen::Matrix3d R = eval_local_coord.rotation();    // rotation of TWRCS
        Eigen::Vector3d t = eval_local_coord.translation(); // translation of TWRCS
        Eigen::AngleAxisd ref_coord = Eigen::AngleAxisd(M_PI/2, Eigen::Vector3d::UnitY());  // world->worm_ref
        Eigen::Matrix3d local_body_rot = R*ref_coord;

        const std::vector<int>& sampling_index = g_sim->GetmWorms()[wormId]->GetCreature()->GetSamplingIndex();
		Eigen::VectorXd local_sampling_pos(3*sampling_index.size());
		Eigen::VectorXd local_sampling_vel(3*sampling_index.size());

        Eigen::Vector3d local_coord_velocity = local_body_rot.transpose()*currCoordVelocity/*mengdi compute vel*/; 
        for(int i=0; i<sampling_index.size(); i++) 
        {
            local_sampling_pos.block<3,1>(3*i, 0) = local_body_rot.inverse()*(x.block<3,1>(3*sampling_index[i], 0)-t) - ref_coord.inverse()*sampling_ref_pos[i];
            local_sampling_vel.block<3,1>(3*i, 0) = local_body_rot.transpose()*v.block<3,1>(3*sampling_index[i], 0)-local_coord_velocity;  
        }

        Eigen::VectorXd data_joined(local_sampling_pos.size()+local_sampling_vel.size());
        data_joined << local_sampling_pos, local_sampling_vel;
        return toNumPyArray(data_joined);
    }

    np::ndarray GetGlobalTrajectories(int wormId)
    {
        if (g_sim == nullptr)
        {
            p::tuple shape = p::make_tuple(3);
            np::dtype dtype = np::dtype::get_builtin<float>();
            np::ndarray ret = np::empty(shape, dtype);

            std::cout << "Environtment not init!" << std::endl;
            return ret;
        }

        const Eigen::VectorXd& x = g_sim->GetmWorms()[wormId]->GetSoftWorld()->GetPositions();
        const std::vector<int>& sampling_indices = g_sim->GetmWorms()[wormId]->GetCreature()->GetSamplingIndex();
        Eigen::VectorXd global_sampling_pos(3*sampling_indices.size());

        for(int i=0; i<sampling_indices.size(); i++) 
        {
            Eigen::Vector3d sample_position = x.block<3, 1>(3 * sampling_indices[i], 0);
            Eigen::Vector3d sample_pos = g_sim->WormLocalToWorld(wormId, sample_position);
            global_sampling_pos.block<3,1>(3*i, 0) = sample_pos;
        }

        return toNumPyArray(global_sampling_pos);
    }

    np::ndarray GetCenterPointGlobalTrajectory(int wormId)
    {
        if (g_sim == nullptr)
        {
            p::tuple shape = p::make_tuple(3);
            np::dtype dtype = np::dtype::get_builtin<float>();
            np::ndarray ret = np::empty(shape, dtype);

            std::cout << "Environtment not init!" << std::endl;
            return ret;
        }

        const Eigen::VectorXd& x = g_sim->GetmWorms()[wormId]->GetSoftWorld()->GetPositions();
        Eigen::Vector3d center_position = x.block<3, 1>(3 * g_sim->GetmWorms()[wormId]->GetCreature()->GetCenterIndex(), 0);
        Eigen::Vector3d center_pos = g_sim->WormLocalToWorld(wormId, center_position);
        return toNumPyArray(center_pos); 
    }

    np::ndarray GetInitPositionWorld(int wormId)
    {
        if (g_sim == nullptr)
        {
            p::tuple shape = p::make_tuple(3);
            np::dtype dtype = np::dtype::get_builtin<float>();
            np::ndarray ret = np::empty(shape, dtype);

            std::cout << "Environtment not init!" << std::endl;
            return ret;
        }

        // return toNumPyArray(g_sim->GetEnvs()->at(wormId)->GetHeadLocation());

        // std::cout << "\nlocal coord speed\n" << localCoordSpeed
        //     << "head offset\n" << headLocalOffset
        //     << "position\n" << pos << std::endl;
        return toNumPyArray(g_sim->GetInitPostions()[wormId]);
    }

    np::ndarray GetInitRotationWorld(int wormId)
    {
        if (g_sim == nullptr)
        {
            p::tuple shape = p::make_tuple(3);
            np::dtype dtype = np::dtype::get_builtin<float>();
            np::ndarray ret = np::empty(shape, dtype);

            std::cout << "Environtment not init!" << std::endl;
            return ret;
        }

        // return toNumPyArray(g_sim->GetEnvs()->at(wormId)->GetHeadLocation());

        // std::cout << "\nlocal coord speed\n" << localCoordSpeed
        //     << "head offset\n" << headLocalOffset
        //     << "position\n" << pos << std::endl;
        return toNumPyArray(g_sim->GetInitRotation()[wormId]);
    }

    np::ndarray GetForwardDirection(int wormId)
    {
        if (g_sim == nullptr)
        {
            p::tuple shape = p::make_tuple(3);
            np::dtype dtype = np::dtype::get_builtin<float>();
            np::ndarray ret = np::empty(shape, dtype);

            std::cout << "Environtment not init!" << std::endl;
            return ret;
        }

        // return toNumPyArray(g_sim->GetEnvs()->at(wormId)->GetHeadLocation());

        // std::cout << "\nlocal coord speed\n" << localCoordSpeed
        //     << "head offset\n" << headLocalOffset
        //     << "position\n" << pos << std::endl;

        const Eigen::VectorXd &x = g_sim->GetEnvs()->at(wormId)->GetSoftWorld()->GetPositions();
        Eigen::Vector3d head_forward = g_sim->GetEnvs()->at(wormId)->GetCreature()->GetForwardVector(x);
        return toNumPyArray(head_forward);
    }

    np::ndarray GetDorsalDirection(int wormId)
    {
        if (g_sim == nullptr)
        {
            p::tuple shape = p::make_tuple(3);
            np::dtype dtype = np::dtype::get_builtin<float>();
            np::ndarray ret = np::empty(shape, dtype);

            std::cout << "Environtment not init!" << std::endl;
            return ret;
        }

        // return toNumPyArray(g_sim->GetEnvs()->at(wormId)->GetHeadLocation());

        // std::cout << "\nlocal coord speed\n" << localCoordSpeed
        //     << "head offset\n" << headLocalOffset
        //     << "position\n" << pos << std::endl;

        const Eigen::VectorXd &x = g_sim->GetEnvs()->at(wormId)->GetSoftWorld()->GetPositions();
        Eigen::Vector3d head_forward = g_sim->GetEnvs()->at(wormId)->GetCreature()->GetForwardVector(x);
        Eigen::Vector3d head_up = g_sim->GetEnvs()->at(wormId)->GetCreature()->GetUpVector(x);
        return toNumPyArray(head_forward.cross(head_up));
    }

    int AddWorm(np::ndarray initPos, np::ndarray dir, np::ndarray color)
    {
        if (g_sim == nullptr)
        {
            std::cout << "Environtment not init!" << std::endl;
            return -1;
        }

        if (initPos.shape(0) != 3 || dir.shape(0) != 3)
        {
            std::cout << "Init position or direction not correct!" << std::endl;
            return -1;
        }

        int id = g_sim->GetEnvs()->size();

        Eigen::Vector3d wormPos = toEigenVector(initPos, 3);
        Eigen::Vector3d direction = toEigenVector(dir, 3);
        Eigen::Vector4d c = toEigenVector(color, 4);

        g_sim->AddWorm(wormPos, direction, c);
        return id;
    }

    void AddFood(np::ndarray foodPos)
    {
        if (g_sim == nullptr)
        {
            std::cout << "Environtment not init!" << std::endl;
            return;
        }

        if (foodPos.shape(0) != 3)
        {
            std::cout << "food position not correct!" << std::endl;
            return;
        }

        g_sim->AddFood(toEigenVector(foodPos, 3));
    }

    void SaveWormStates(int idx, p::str path)
    {
        if (g_sim == nullptr)
        {
            std::cout << "Environtment not init!" << std::endl;
            return;
        }

        if (idx >= g_sim->GetEnvs()->size())
        {
            std::cout << "index larger than worm count!" << std::endl;
            return;
        }

        g_sim->SaveWormStates(idx, std::string(p::extract<const char*>(path)));
    }

    void RestoreWormStates(int idx, p::str path)
    {
        if (g_sim == nullptr)
        {
            std::cout << "Environtment not init!" << std::endl;
            return;
        }

        if (idx >= g_sim->GetEnvs()->size())
        {
            std::cout << "index larger than worm count!" << std::endl;
            return;
        }

        g_sim->RestoreWormStates(idx, std::string(p::extract<const char *>(path)));
    }

    np::ndarray WormLocalToWorld(int wormId, np::ndarray pos)
    {
        return toNumPyArray(g_sim->WormLocalToWorld(wormId, toEigenVector(pos, 3)));
    }

    np::ndarray WorldToWormLocal(int wormId, np::ndarray pos)
    {
        return toNumPyArray(g_sim->WorldToWormLocal(wormId, toEigenVector(pos, 3)));
    }

    void SetWorldFloorColor(np::ndarray c)
    {
        g_sim->SetWorldFloorColor(toEigenVector(c, 4));
    }

    void SetTubesVoltages(np::ndarray v)
    {
        g_viewer->renderer.updatevoltagesColor(reinterpret_cast<float*>(v.get_data()));
        g_viewer->frameState.accumID = 0;
    }

    void SetTubesVoltagesAndScreenShot(np::ndarray v)
    {
        g_viewer->screenShot();
        g_viewer->renderer.updatevoltagesColor(reinterpret_cast<float*>(v.get_data()));
        g_viewer->frameState.accumID = 0;
    }

    void SetWorldFogColor(np::ndarray c)
    {
        g_sim->SetWorldFogColor(toEigenVector(c, 4));
    }

    void SetFogParams(int mode, np::ndarray params)
    {
        g_sim->SetFogParams(mode, toEigenVector(params, 3));
    }
}

using namespace boost::python;
BOOST_PYTHON_MODULE(interact)
{
    Py_Initialize();
    np::initialize();

    def("set_muscle_signals", &Interact::SetActivationLevels);
    def("get_head_location", &Interact::GetHeadLocation);
    def("get_tracking_location", &Interact::GetTrackingPoint);
    def("get_global_tracking_location", &Interact::GetGlobalTrajectories);
    def("get_center_tracking_location", &Interact::GetCenterPointGlobalTrajectory);
    def("set_neuron_voltages", &Interact::SetTubesVoltages);
    def("set_neuron_voltages_and_screen_shot", &Interact::SetTubesVoltagesAndScreenShot);
    def("add_worm", &Interact::AddWorm);
    def("add_food", &Interact::AddFood);
    def("save_worm", &Interact::SaveWormStates);
    def("restore_worm", &Interact::RestoreWormStates);
    def("worm_local_to_world", &Interact::WormLocalToWorld);
    def("world_to_worm_local", &Interact::WorldToWormLocal);
    def("set_fog_color", &Interact::SetWorldFogColor);
    def("set_floor_color", &Interact::SetWorldFloorColor);
    def("set_fog_params", &Interact::SetFogParams);
    def("get_dorsal_direction", &Interact::GetDorsalDirection);
    def("get_forward_direction", &Interact::GetForwardDirection);
    def("get_init_position", &Interact::GetInitPositionWorld);
    def("get_init_rotation", &Interact::GetInitRotationWorld);   
}
