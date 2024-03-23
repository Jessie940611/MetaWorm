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

#include "Viewer.h"

namespace rtn {

	// -------------------------------------------------------------------------
	/*! fly mode ajustment */
	flyMode::flyMode(TubesViewer *widget,
					 float worldScale)
		: CameraManipulator(widget),
		tubesViewer(widget)
	{
		moveSpeed = worldScale / 100.f;
	}

	void flyMode::move(const float step)
	{
		Camera &fc = viewer->camera;
		// negative z axis: 'subtract' step
		fc.position = fc.position - step * moveSpeed * fc.frame.vz;
		viewer->updateCamera();
	}

	void flyMode::rotate(const float deg_u,
		const float deg_v)
	{
		float rad_u = -(float)M_PI / 180.f*deg_u;
		float rad_v = -(float)M_PI / 180.f*deg_v;

		assert(viewer);
		Camera &fc = viewer->camera;

		fc.frame
			= linear3f::rotate(fc.frame.vy, rad_u)
			* linear3f::rotate(fc.frame.vx, rad_v)
			* fc.frame;

		if (fc.forceUp) fc.forceUpFrame();

		viewer->updateCamera();
	}

	void flyMode::strafe(const vec2f step)
	{
		Camera &fc = viewer->camera;
		fc.position = fc.position
			- step.x*fc.motionSpeed * fc.frame.vx * moveSpeed
			+ step.y*fc.motionSpeed * fc.frame.vy * moveSpeed;
		viewer->updateCamera();
	}

	void flyMode::mouseDragLeft(const vec2i &where, const vec2i &delta)
	{
		const vec2f fraction = vec2f(delta) / vec2f(viewer->getWindowSize());
		rotate(fraction.x * mouseRotateDegree, fraction.y * mouseRotateDegree);
	}

	void flyMode::mouseDragCenter(const vec2i &where, const vec2i &delta)
	{
		const vec2f fraction = vec2f(delta) / vec2f(viewer->getWindowSize());
		strafe(fraction);
	}

	void flyMode::mouseDragRight(const vec2i &where, const vec2i &delta)
	{
		const vec2f fraction = vec2f(delta);
		move(fraction.y);
	}

	void flyMode::kbd_up()
	{
		rotate(0, +kbdRotateDegree);
	}

	void flyMode::kbd_down()
	{
		rotate(0, -kbdRotateDegree);
	}

	void flyMode::kbd_right()
	{
		rotate(-kbdRotateDegree, 0);
	}

	void flyMode::kbd_left()
	{
		rotate(+kbdRotateDegree, 0);
	}

	void flyMode::kbd_forward()
	{
		move(+1.f);
	}

	void flyMode::kbd_back()
	{
		move(-1.f);
	}

	void flyMode::motionKey()
	{
		GLFWwindow *handle = tubesViewer->getWindow();

		if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS) kbd_up();
		if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS) kbd_down();
		if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS) kbd_left();
		if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS) kbd_right();
		if (glfwGetKey(handle, GLFW_KEY_E) == GLFW_PRESS) kbd_forward();
		if (glfwGetKey(handle, GLFW_KEY_C) == GLFW_PRESS) kbd_back();
	}

	void flyMode::key(char key, const vec2i& where)
	{
		Camera &fc = viewer->camera;

		switch (key) {
		case '+':
		case '=':
			moveSpeed *= 1.1f;
			break;
		case '-':
		case '_':
			moveSpeed /= 1.1f;
			break;
		case '>':
		case '.':
			kbdRotateDegree *= 1.1f;
			mouseRotateDegree *= 1.1f;
			break;
		case '<':
		case ',':
			kbdRotateDegree /= 1.1f;
			mouseRotateDegree /= 1.1f;
			break;
		}
	}

	void flyMode::special(int key, const vec2i &where)
	{
		/*switch (key) {
		case GLFW_KEY_UP:
			kbd_up();
			break;
		case GLFW_KEY_DOWN:
			kbd_down();
			break;
		case GLFW_KEY_RIGHT:
			kbd_right();
			break;
		case GLFW_KEY_LEFT:
			kbd_left();
			break;
		case GLFW_KEY_PAGE_UP:
			kbd_forward();
			break;
		case GLFW_KEY_PAGE_DOWN:
			kbd_back();
			break;
		}*/
	}


	// ------------------------------------------------------------------------------------------
	/*! inspect mode ajustment */
	inspectMode::inspectMode(TubesViewer *widget,
		box3f poiBounds,
		float minDist,
		float maxDist,
		float worldScale)
	: CameraManipulator(widget),
		validPoiBounds(poiBounds),
		minDistance(minDist),
		maxDistance(maxDist),
		tubesViewer(widget)
	{
		moveSpeed = worldScale / 100.f;
	}

	void inspectMode::move(const float step)
	{
		Camera &fc = viewer->camera;

		const vec3f poi = fc.getPOI();
		fc.poiDistance = min(maxDistance, max(minDistance, fc.poiDistance - step * moveSpeed));
		//      fc.focalDistance = fc.poiDistance;
		fc.position = poi + fc.poiDistance * fc.frame.vz;
		viewer->updateCamera();
	}

	void inspectMode::movePOI(const vec2f step)
	{
		Camera &fc = viewer->camera;

		const vec3f old_poi = fc.getPOI();
		vec3f new_poi = old_poi - step.x * fc.motionSpeed * fc.frame.vx * moveSpeed + step.y * fc.motionSpeed * fc.frame.vy * moveSpeed;
		if (!validPoiBounds.empty())
			new_poi = max(min(new_poi, validPoiBounds.upper), validPoiBounds.lower);
		fc.position += (new_poi - old_poi);
		viewer->updateCamera();
	}

	void inspectMode::rotate(const float deg_u,
		const float deg_v)
	{
		float rad_u = -(float)M_PI / 180.f*deg_u;
		float rad_v = -(float)M_PI / 180.f*deg_v;

		assert(viewer);
		Camera &fc = viewer->camera;

		const vec3f poi = fc.getPOI();
		fc.frame
			= linear3f::rotate(fc.frame.vy, rad_u)
			* linear3f::rotate(fc.frame.vx, rad_v)
			* fc.frame;

		if (fc.forceUp) fc.forceUpFrame();

		fc.position = poi + fc.poiDistance * fc.frame.vz;

		viewer->updateCamera();
	}

	void inspectMode::kbd_up()
	{
		rotate(0, +kbdRotateDegree);
	}

	void inspectMode::kbd_down()
	{
		rotate(0, -kbdRotateDegree);
	}

	void inspectMode::kbd_right()
	{
		rotate(-kbdRotateDegree, 0);
	}

	void inspectMode::kbd_left()
	{
		rotate(+kbdRotateDegree, 0);
	}

	void inspectMode::kbd_forward()
	{
		move(+1.f);
	}

	void inspectMode::kbd_back()
	{
		move(-1.f);
	}

	void inspectMode::mouseDragLeft(const vec2i &where, const vec2i &delta)
	{
		const vec2f fraction = vec2f(delta) / vec2f(viewer->getWindowSize());
		rotate(fraction.x * mouseRotateDegree, fraction.y * mouseRotateDegree);

	}

	void inspectMode::mouseDragCenter(const vec2i &where, const vec2i &delta)
	{
		const vec2f fraction = vec2f(delta) / vec2f(viewer->getWindowSize());
		movePOI(fraction);
	}

	void inspectMode::mouseDragRight(const vec2i &where, const vec2i &delta)
	{
		move(delta.y);
	}

	void inspectMode::motionKey()
	{
		GLFWwindow *handle = tubesViewer->getWindow();

		if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS) kbd_up();
		if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS) kbd_down();
		if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS) kbd_left();
		if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS) kbd_right();
		if (glfwGetKey(handle, GLFW_KEY_E) == GLFW_PRESS) kbd_forward();
		if (glfwGetKey(handle, GLFW_KEY_C) == GLFW_PRESS) kbd_back();
	}

	void inspectMode::key(char key, const vec2i &where)
	{
		Camera &fc = viewer->camera;

		switch (key) {
		case '+':
		case '=':
			moveSpeed *= 1.1f;
			break;
		case '-':
		case '_':
			moveSpeed /= 1.1f;
			break;
		case '>':
		case '.':
			kbdRotateDegree *= 1.1f;
			mouseRotateDegree *= 1.1f;
			break;
		case '<':
		case ',':
			kbdRotateDegree /= 1.1f;
			mouseRotateDegree /= 1.1f;
			break;
		}
	}

	void inspectMode::special(int key, const vec2i &where)
	{
		/*switch (key) {
		case GLFW_KEY_UP:
			kbd_up();
			break;
		case GLFW_KEY_DOWN:
			kbd_down();
			break;
		case GLFW_KEY_RIGHT:
			kbd_right();
			break;
		case GLFW_KEY_LEFT:
			kbd_left();
			break;
		case GLFW_KEY_PAGE_UP:
			kbd_forward();
			break;
		case GLFW_KEY_PAGE_DOWN:
			kbd_back();
			break;
		}*/
	}


	// -------------------------------------------------------------------------
	/*! fps mode */
	fpsMode::fpsMode(TubesViewer * widget,
					 float worldScale)
		: CameraManipulator(widget),
		tubesViewer(widget)
	{
		moveSpeed = worldScale / 100.f;
	}

	void fpsMode::move(vec3f direction)
	{
		Camera &fc = viewer->camera;

		fc.position = fc.position + moveSpeed * direction;
		viewer->updateCamera();
	}

	void fpsMode::moveForward()
	{
		Camera &fc = viewer->camera;

		move(-fc.frame.vz);
	}

	void fpsMode::moveBackward()
	{
		Camera &fc = viewer->camera;

		move(fc.frame.vz);
	}

	void fpsMode::moveLeft()
	{
		Camera &fc = viewer->camera;

		move(-fc.frame.vx);
	}

	void fpsMode::moveRight()
	{
		Camera &fc = viewer->camera;

		move(fc.frame.vx);
	}

	void fpsMode::moveUp()
	{
		Camera &fc = viewer->camera;

		/*if (fixedUpVector) move(upVector);
		else move(fc.frame.vy);*/
		move(fc.frame.vy);
	}

	void fpsMode::moveDown()
	{
		Camera &fc = viewer->camera;

		/*if (fixedUpVector) move(-upVector);
		else move(-fc.frame.vy);*/
		move(-fc.frame.vy);
	}

	void fpsMode::moveMouse(const vec2i& currentPosition, const vec2i& lastPosition)
	{
		Camera &fc = viewer->camera;
		vec2i screenSize = tubesViewer->cmdline_args.windowSize;

		vec2i delta = currentPosition - lastPosition;

		float radY = 0.5f * -fc.fovyInDegrees * (float)delta.y / (float)screenSize.y;
		float radX = 0.5f * -fc.fovyInDegrees * (float)delta.x / (float)screenSize.y;

		fc.frame
			= linear3f::rotate(fc.frame.vy, radX * rotateRate)
			* linear3f::rotate(fc.frame.vx, radY * rotateRate)
			* fc.frame;

		viewer->updateCamera();
	}

	void fpsMode::motionKey()
	{
		GLFWwindow *handle = tubesViewer->getWindow();

		if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS) moveForward();
		if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS) moveBackward();
		if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS) moveLeft();
		if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS) moveRight();
		if (glfwGetKey(handle, GLFW_KEY_E) == GLFW_PRESS) moveUp();
		if (glfwGetKey(handle, GLFW_KEY_C) == GLFW_PRESS) moveDown();
	}

	void fpsMode::key(char key, const vec2i& where)
	{
		switch (key) {
		case '+':
		case '=':
			moveSpeed *= 1.1f;
			break;
		case '-':
		case '_':
			moveSpeed /= 1.1f;
			break;
		case '>':
		case '.':
			rotateRate *= 1.1f;
			break;
		case '<':
		case ',':
			rotateRate /= 1.1f;
			break;
		}
	}

	void fpsMode::special(int key, const vec2i &where)
	{
		/*switch (key) {
		case GLFW_KEY_UP:
			moveForward();
			break;
		case GLFW_KEY_DOWN:
			moveBackward();
			break;
		case GLFW_KEY_RIGHT:
			moveRight();
			break;
		case GLFW_KEY_LEFT:
			moveLeft();
			break;
		case GLFW_KEY_PAGE_UP:
			moveUp();
			break;
		case GLFW_KEY_PAGE_DOWN:
			moveDown();
			break;
		}*/
	}


	// -------------------------------------------------------------------------
	/*! free mode ajustment */
	freeMode::freeMode(TubesViewer *widget,
					   float worldScale)
		: CameraManipulator(widget),
		tubesViewer(widget)
	{
		moveSpeed = worldScale / 100.f;
	}

	void freeMode::move(const float step)
	{
		Camera &fc = viewer->camera;
		
		fc.position = fc.position - step * moveSpeed * fc.frame.vz;
		viewer->updateCamera();
	}

	void freeMode::rotate(const int dir, const float deg)
	{
		const float rad = M_PI * deg / 180.0f;

		assert(viewer);
		Camera &fc = viewer->camera;

		switch (dir) {
		case 1:
			fc.frame
				= linear3f::rotate(fc.frame.vx, rad)
				* fc.frame;
			break;
		case 2:
			fc.frame
				= linear3f::rotate(fc.frame.vy, rad)
				* fc.frame;
			break;
		case 3:
			fc.frame
				= linear3f::rotate(fc.frame.vz, rad)
				* fc.frame;
			break;
		}	

		viewer->updateCamera();
	}

	void freeMode::strafe(const vec2f delta)
	{
		Camera &fc = viewer->camera;
		fc.position = fc.position
			- delta.x*fc.motionSpeed * fc.frame.vx * moveSpeed
			+ delta.y*fc.motionSpeed * fc.frame.vy * moveSpeed;
		viewer->updateCamera();
	}

	void freeMode::mouseDragLeft(const vec2i &where, const vec2i &delta)
	{
		const vec2f fraction = vec2f(delta) / vec2f(viewer->getWindowSize());
		rotate(1, fraction.y * mouseRotateDegree);
		rotate(2, fraction.x * mouseRotateDegree);
	}

	void freeMode::mouseDragCenter(const vec2i &where, const vec2i &delta)
	{
		const vec2f fraction = vec2f(delta) / vec2f(viewer->getWindowSize());
		strafe(fraction);
	}

	void freeMode::mouseDragRight(const vec2i &where, const vec2i &delta)
	{
		move(delta.y);
	}

	void freeMode::moveForward()
	{
		move(+1.f);
	}

	void freeMode::moveBackward()
	{
		move(-1.f);
	}

	void freeMode::rotateUp()
	{
		rotate(1, kbdRotateDegree);
	}

	void freeMode::rotateDown()
	{
		rotate(1, -kbdRotateDegree);
	}

	void freeMode::rotateRight()
	{
		rotate(2, -kbdRotateDegree);
	}

	void freeMode::rotateLeft()
	{
		rotate(2, kbdRotateDegree);
	}

	void freeMode::rotateClockwise()
	{
		rotate(3, -kbdRotateDegree);
	}

	void freeMode::rotateCounterClockwise()
	{
		rotate(3, kbdRotateDegree);
	}

	void freeMode::motionKey()
	{
		GLFWwindow *handle = tubesViewer->getWindow();

		if (glfwGetKey(handle, GLFW_KEY_W) == GLFW_PRESS) rotateUp();
		if (glfwGetKey(handle, GLFW_KEY_S) == GLFW_PRESS) rotateDown();
		if (glfwGetKey(handle, GLFW_KEY_A) == GLFW_PRESS) rotateLeft();
		if (glfwGetKey(handle, GLFW_KEY_D) == GLFW_PRESS) rotateRight();
		if (glfwGetKey(handle, GLFW_KEY_Z) == GLFW_PRESS) rotateCounterClockwise();
		if (glfwGetKey(handle, GLFW_KEY_X) == GLFW_PRESS) rotateClockwise();
		if (glfwGetKey(handle, GLFW_KEY_E) == GLFW_PRESS) moveForward();
		if (glfwGetKey(handle, GLFW_KEY_C) == GLFW_PRESS) moveBackward();
	}

	void freeMode::key(char key, const vec2i& where)
	{
		Camera &fc = viewer->camera;

		switch (key) {
		case '+':
		case '=':
			moveSpeed *= 1.1f;
			break;
		case '-':
		case '_':
			moveSpeed /= 1.1f;
			break;
		case '>':
		case '.':
			kbdRotateDegree *= 1.1f;
			mouseRotateDegree *= 1.1f;
			break;
		case '<':
		case ',':
			kbdRotateDegree /= 1.1f;
			mouseRotateDegree /= 1.1f;
			break;
		}
	}

	void freeMode::special(int key, const vec2i &where)
	{
		/*switch (key) {
		case GLFW_KEY_UP:
			kbd_up();
			break;
		case GLFW_KEY_DOWN:
			kbd_down();
			break;
		case GLFW_KEY_RIGHT:
			kbd_right();
			break;
		case GLFW_KEY_LEFT:
			kbd_left();
			break;
		case GLFW_KEY_PAGE_UP:
			kbd_forward();
			break;
		case GLFW_KEY_PAGE_DOWN:
			kbd_back();
			break;
		}*/
	}
}
