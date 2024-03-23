// ======================================================================== //
// Copyright 2018-2022 The Contributors                                            //
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

#include "owlViewer/OWLViewer.h"
#include "owlViewer/Camera.h"

namespace rtn {

	using namespace owl::common;
	using namespace owl::viewer;
	
	struct TubesViewer;

	/*! Haixin Ma: inherited from ingo's CameraFlyMode.
		changed key function for our TubesViewer */
	struct flyMode : public CameraManipulator
	{
		flyMode(TubesViewer *widget, float worldScale);

		void motionKey();
		virtual void key(char key, const vec2i &where) override;
		virtual void special(int key, const vec2i &where) override;

		float moveSpeed = 1.0f;
		float kbdRotateDegree = 1.0f;
		float mouseRotateDegree = 120.0f;

	private:
		void rotate(const float deg_x, const float deg_y);
		void move(const float step);
		void strafe(const vec2f delta);

		void kbd_up();
		void kbd_down();
		void kbd_left();
		void kbd_right();
		void kbd_forward();
		void kbd_back();

		virtual void mouseDragLeft(const vec2i &where, const vec2i &delta) override;
		virtual void mouseDragRight(const vec2i &where, const vec2i &delta) override;
		virtual void mouseDragCenter(const vec2i &where, const vec2i &delta) override;

		TubesViewer* tubesViewer;
	};

	/*! Haixin Ma: inherited from ingo's CameraInspectMode.
			changed key function for our TubesViewer */
	struct inspectMode : public CameraManipulator
	{
		inspectMode(TubesViewer *widget,
			box3f poiBounds,
			float minDist,
			float maxDist,
			float worldScale);

		void motionKey();
		virtual void key(char key, const vec2i &where) override;
		virtual void special(int key, const vec2i &where) override;

		box3f validPoiBounds;
		float moveSpeed = 1.0f;
		float kbdRotateDegree = 1.0f;
		float mouseRotateDegree = 120.0f;
		

	private:
		void rotate(const float deg_x, const float deg_y);
		void move(const float step);
		void movePOI(const vec2f delta);

		void kbd_up();
		void kbd_down();
		void kbd_left();
		void kbd_right();
		void kbd_forward();
		void kbd_back();

		virtual void mouseDragLeft(const vec2i &where, const vec2i &delta) override;
		virtual void mouseDragRight(const vec2i &where, const vec2i &delta) override;
		virtual void mouseDragCenter(const vec2i &where, const vec2i &delta) override;

		TubesViewer* tubesViewer;
		float minDistance;
		float maxDistance;
	};

	/*! Haixin Ma: camera rotates without pressing mouse button.
			up vector is always positive Y-axis */
	struct fpsMode : public CameraManipulator
	{
		fpsMode(TubesViewer *widget, float worldScale);
		
		void motionKey();
		virtual void key(char key, const vec2i &where) override;
		virtual void special(int key, const vec2i &where) override;

		void moveMouse(const vec2i& currentPosition, const vec2i& lastPosition);

		float rotateRate = 0.6f;
		float moveSpeed = 1.0f;
		/*bool fixedUpVector = true;
		vec3f upVector = vec3f(0.0f, 1.0f, 0.0f);*/

	private:
		void move(vec3f direction);

		void moveForward();
		void moveBackward();
		void moveLeft();
		void moveRight();
		void moveUp();
		void moveDown();

		TubesViewer* tubesViewer;
	};

	/*! Haixin Ma: camera rotates without pressing mouse button.
			up vector is always positive Y-axis */
	struct freeMode : public CameraManipulator
	{
		freeMode(TubesViewer *widget, float worldScale);

		void motionKey();
		virtual void key(char key, const vec2i &where) override;
		virtual void special(int key, const vec2i &where) override;

		float moveSpeed = 1.0f;
		float kbdRotateDegree = 0.6f;
		float mouseRotateDegree = 120.0f;

	private:
		void rotate(const int dir, const float rad);
		void move(const float step);
		void strafe(const vec2f delta);

		void moveForward();
		void moveBackward();

		void rotateLeft();
		void rotateRight();
		void rotateUp();
		void rotateDown();
		void rotateClockwise();
		void rotateCounterClockwise();

		virtual void mouseDragLeft(const vec2i &where, const vec2i &delta) override;
		virtual void mouseDragRight(const vec2i &where, const vec2i &delta) override;
		virtual void mouseDragCenter(const vec2i &where, const vec2i &delta) override;

		TubesViewer* tubesViewer;
	};

}
