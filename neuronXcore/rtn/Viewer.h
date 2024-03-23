// ======================================================================== //
// Copyright 2018-2022 The Contributors                                     //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "CameraMode.h"
#include "CmdlineParser.h"
#include "OptixTubes.h"
#include "TubesUtils.h"

#include "owlViewer/OWLViewer.h"
#include "SimulationWindow.h"

#include <map>

namespace rtn
{

	struct TubesViewer : public owl::viewer::OWLViewer
	{
		TubesViewer(OWLTubes &renderer, const std::string& title, const vec2i& initWindowSize);

		void screenShot();

		void saveTofile(std::string fileName);

		void moveCameraAni();

		void rollCameraAni();

		void saveVolChangeAni();

		void saveVolandRollChangeAni();

		void updateVoltageAndCamera();

		void circleCamera(int vol_index, int step, int n);

		void circleCameraAnimation(int nSamples);

		void aroundCenterCameraAnimation(int n);

		void autoUpdateVoltage();

		virtual void cameraChanged() override;

		virtual void resize(const vec2i &newSize) override;

		void recordCameraTrack();

		void setCamera(int sample_index);

		virtual void render() override;

		void renderWorm();

		virtual void draw() override;

		void showAndRun();

		virtual void motionKey();

		virtual void key(char key, const vec2i &where);

		void moveMouseCallback();

		virtual void moveMouse(const vec2i& currentPosition, const vec2i& lastPosition);

		virtual void mouseMotion(const vec2i& newMousePosition) override;

		void fpsMouseMotion(const vec2i& newMousePosition);

		void enableFlyMode();

		void enableInspectMode(const box3f &validPoiRange = box3f(),
			float minPoiDist = 1e-3f,
			float maxPoiDist = std::numeric_limits<float>::infinity());

		void enableFPSMode();

		void enableFreeMode();

		GLFWwindow* getWindow();

		CmdlineArgs cmdline_args;
		OWLTubes &renderer;	
		SimulationWindow* simulationWindow;

		std::shared_ptr<flyMode> flyModeManipulator;
		std::shared_ptr<inspectMode> inspectModeManipulator;
		std::shared_ptr<fpsMode> fpsModeManipulator;
		std::shared_ptr<freeMode> freeModeManipulator;

		bool enteringFpsMode = true;

		struct CameraInfo
		{
			int           index;
			vec3f         position;
			vec3f         poi;
			vec3f         up;
			float         fov;
		};

		std::map<int, std::pair<int, int>> interp_map;
		std::map<int, CameraInfo> camera_track;

		bool bWormMainView = true;
		bool bSingleMainView = false;

		bool moveCameraFlag = false;
		bool rollCameraFlag = false;
		bool saveVolChangeFlag = false;
		bool volAndrollFlag = false;

		bool bUpdateVoltage = false;
		bool bUpdateVoltageAndCamera = false;
		bool bRecordCamera = false;
		bool bClearCameraTrack = false;
		int anim_accum = 10;

		int steps = 10;

		bool bTestCamera = false;

		int nCircleSamples = 200;
		float cameraDistScale = 0.8f;
		
		float worldScale;

		vec2i fbSize{ -1, -1 };
		//GLuint fbTexture { 0 } ;
		GLuint wormTexture { 1 };
		GLuint wormDepthBuffer{ 2 };
		GLuint wormFb { 3 };
		uint32_t* wormPointer {nullptr} ;
		device::FrameState frameState;
		Tubes::SP rtn;
		Triangles::SP triangles;
		bool displayFPS = false;
		vec3f bg_top_color{ 0.511f, 0.628f, 0.1106f };
		vec3f bg_bottom_color{ 0.1231f, 0.1126f, 0.1216f };
		double render_start_time = -1.f;
		int voltage_index = 0;
		int voltage_index_end = 1;
		int screenshot_id = 0;

		vec3f camera_pos;
		vec3f camera_target;
		vec3f model_span;
		float camera_fov;

		box3f sceneBounds;

		struct PrimitiveRange
		{
			int a = 100000000;
			int b = -1;
		};

		//std::vector<PrimitiveRange> primitive_range;
		std::vector<vec2i> primitive_range;
		int max_primitive = 0;
		int min_primitive = 10000;
	};

}

