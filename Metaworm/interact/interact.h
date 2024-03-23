#pragma once

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <Eigen/Core>
#include "../render/SimulationWindow.h"

namespace p = boost::python;
namespace np = boost::python::numpy;

class Environment;
namespace rtn
{
    struct TubesViewer;
}

namespace Interact
{
    void InitEnvironment(SimulationWindow *env);
    void InitTubesViewer(rtn::TubesViewer* viewer);

    // void SetActivationLevels(np::ndarray a);
    // np::ndarray GetHeadLocation();

    void SetTubesVoltages(np::ndarray v);

    void SetTubesVoltagesAndScreenShot(np::ndarray v);
};
