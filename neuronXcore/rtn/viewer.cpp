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

#include <GL/glew.h>

#include "Viewer.h"

// eventually to go into 'apps/'
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw_gl2.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#include "3rdParty/stb/stb_image_write.h"
#define STB_IMAGE_IMPLEMENTATION 1
#include "3rdParty/stb/stb_image.h"

// std
#include <sstream>
#include <iomanip>

#ifndef Log
#define Log  printf
#endif

namespace rtn 
{
	std::string screenShotFileName = /* add the 'method' later in screenshot() - it's not set yet */ "screenshot.png";

	TubesViewer::TubesViewer(OWLTubes &renderer, const std::string& title = "OWL Sample Viewer", const vec2i& initWindowSize = vec2i(1200, 800))
		: OWLViewer(title, initWindowSize), renderer(renderer)
	{
		frameState.frameID = 0;		
		glewInit();
		//glfwSwapInterval(0);
#ifdef WIN32
		typedef void (APIENTRY* PFNWGLEXTSWAPCONTROLPROC) (int);
		typedef int(*PFNWGLEXTGETSWAPINTERVALPROC) (void);
		PFNWGLEXTSWAPCONTROLPROC wglSwapIntervalEXT = NULL;
		PFNWGLEXTGETSWAPINTERVALPROC wglGetSwapIntervalEXT = NULL;
		char* extensions = (char*)glGetString(GL_EXTENSIONS);
		if (strstr(extensions, "WGL_EXT_swap_control"))
		{
			wglSwapIntervalEXT = (PFNWGLEXTSWAPCONTROLPROC)wglGetProcAddress("wglSwapIntervalEXT");stepInitFunc
			wglGetSwapIntervalEXT = (PFNWGLEXTGETSWAPINTERVALPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
			wglSwapIntervalEXT(0);
			std::cout << "vsync supported\n";
			//return true;
		}
#endif 
	}

	void TubesViewer::screenShot()
	{
		const uint32_t* fb = (const uint32_t*)fbPointer;
		const vec2i fbSize = renderer.fbSize;

		double elapsed = getCurrentTime() - render_start_time;
		char buf[512] = { 0 };
#ifdef WIN32
		sprintf(buf, "%s\\screenshot-%dx%d_%d-%.3lf.png", cmdline_args.screenshotPath.c_str(), fbSize.x, fbSize.y, frameState.accumID, elapsed);
#else
		sprintf(buf, "%s/screenshot-%dx%d_%05d.png", cmdline_args.screenshotPath.c_str(), fbSize.x, fbSize.y, screenshot_id);
#endif
		const std::string fileName(buf);

		screenshot_id += 1;

		// if (screenshot_id > 1800)
		// {
		// 	std::cout << "too many screenshots taken(>1800), stop taking more, exiting" << std::endl;
		// 	exit(0);
		// }

		//std::vector<uint32_t> pixels;
		// for (int y = 0; y < fbSize.y; y++)
		// {
		// 	const uint32_t *line = fb + (fbSize.y - 1 - y)*fbSize.x;
		// 	for (int x = 0; x < fbSize.x; x++)
		// 	{
		// 		pixels.push_back(line[x] | (0xff << 24));
		// 	}
		// }
		
		// unsigned char* pixelsTemp = new unsigned char[fbSize.x * fbSize.y * 3];
		// unsigned char* pixels = new unsigned char[fbSize.x * fbSize.y * 3];
		// glReadPixels(0, 0, fbSize.x, fbSize.y, GL_RGB, GL_UNSIGNED_BYTE, pixelsTemp);

		// for (int y = 0; y < fbSize.y; y++)
		// {
		// 	for (int x = 0; x < fbSize.x; x++)
		// 	{
		// 		unsigned char* pixelTemp = pixelsTemp + 3 * (fbSize.y - 1 - y)*fbSize.x + 3 * x;
		// 		unsigned char* pixel = pixels + 3 * y * fbSize.x + 3 * x;
		// 		*pixel = *pixelTemp;
		// 		*(pixel+1) = *(pixelTemp+1);
		// 		*(pixel+2) = *(pixelTemp+2);
		// 	}
		// }

		// stbi_write_png(fileName.c_str(), fbSize.x, fbSize.y, 3, pixels, fbSize.x * 3);	

		// delete pixels;
		// delete pixelsTemp;

		unsigned char* pixelsTemp = new unsigned char[fbSize.x * fbSize.y * 4];
		unsigned char* pixels = new unsigned char[fbSize.x * fbSize.y * 4];
		glReadPixels(0, 0, fbSize.x, fbSize.y, GL_RGBA, GL_UNSIGNED_BYTE, pixelsTemp);

		for (int y = 0; y < fbSize.y; y++)
		{
			for (int x = 0; x < fbSize.x; x++)
			{
				unsigned char* pixelTemp = pixelsTemp + 4 * (fbSize.y - 1 - y)*fbSize.x + 4 * x;
				unsigned char* pixel = pixels + 4 * y * fbSize.x + 4 * x;
				*pixel = *pixelTemp;
				*(pixel+1) = *(pixelTemp+1);
				*(pixel+2) = *(pixelTemp+2);
				*(pixel+3) = *(pixelTemp+3);
			}
		}

		stbi_write_png(fileName.c_str(), fbSize.x, fbSize.y, 4, pixels, fbSize.x * 4);	

		delete pixels;
		delete pixelsTemp;

		std::cout << "screenshot saved in '" << fileName << "'" << std::endl;
	}

	void TubesViewer::saveTofile(std::string fileName)
	{
		const uint32_t* fb = (const uint32_t*)fbPointer;
		const vec2i fbSize = renderer.fbSize;

		std::vector<uint32_t> pixels;
		for (uint32_t y = 0; y < fbSize.y; y++)
		{
			const uint32_t* line = fb + (fbSize.y - 1 - y) * fbSize.x;
			for (int x = 0; x < fbSize.x; x++)
			{
				pixels.push_back(line[x] | (0xff << 24));
			}
		}
		stbi_write_png(fileName.c_str(), fbSize.x, fbSize.y, 4, pixels.data(), fbSize.x * sizeof(uint32_t));
	}

	void TubesViewer::moveCameraAni()
	{
		if (moveCameraFlag)
		{
			static int ct = 0;
			static int numFrames = 0;
			if (numFrames > 20)
			{
				std::cout << "try to move" << std::endl;
				std::ostringstream  istr;
				istr << std::setw(4) << std::setfill('0') << ct;
#ifdef WIN32
				std::string fileName = cmdline_args.screenshotPath + "\\" + istr.str() + ".png";
#else
				std::string fileName = cmdline_args.screenshotPath + "/" + istr.str() + ".png";
#endif
				saveTofile(fileName);
				const vec2i a = vec2i(2, -0.4);
				mouseDragRight(a, { 2,2 });
				frameState.accumID = 0;
				//renderer.updateFrameState(frameState);
				ct++;
				if (ct > 360)
				{
					moveCameraFlag = false;
					ct = 0;
				}
				numFrames = 0;
			}
			numFrames++;
		}
	}

	void TubesViewer::rollCameraAni()
	{
		if (rollCameraFlag) {
			static int ct = 0;
			static int numFrames = 0;
			if (numFrames > 100) {
				std::cout << "try to move" << std::endl;
				std::ostringstream  istr;
				istr << std::setw(4) << std::setfill('0') << ct;
#ifdef WIN32
				std::string fileName = cmdline_args.screenshotPath + "\\" + istr.str() + ".png";
#else
				std::string fileName = cmdline_args.screenshotPath + "/" + istr.str() + ".png";
#endif
				saveTofile(fileName);
				const vec2i a = vec2i(0, 1);
				mouseDragLeft(a, { 2,2 });
				frameState.accumID = 0;
				//renderer.updateFrameState(frameState);
				ct++;
				if (ct > 360) {
					rollCameraFlag = false;
					ct = 0;
				}
				numFrames = 0;
			}
			numFrames++;
		}
	}

	void TubesViewer::saveVolChangeAni()
	{
		if (saveVolChangeFlag)
		{
			//  std::cout << "fuck\n";
			static int ct = 0;
			static int numFrames = 0;
			if (numFrames > 100)
			{ // 60
				std::cout << "try to vol" << std::endl;
				std::ostringstream  istr;
				istr << std::setw(4) << std::setfill('0') << ct;
#ifdef WIN32
				std::string fileName = cmdline_args.screenshotPath + "\\" + istr.str() + ".png";
#else
				std::string fileName = cmdline_args.screenshotPath + "/" + istr.str() + ".png";
#endif
				frameState.frameID++;

				int sz = rtn->voltages.size();
				if (frameState.frameID >= sz)
					frameState.frameID = 0;
				int id = clamp(frameState.frameID, 0, sz - 1);
				std::cout << frameState.frameID << " " << id << " " << rtn->voltages[id].size() << std::endl;

				if (ct % 1 == 0)
				{
					saveTofile(fileName);
					renderer.updatevoltagesColor(rtn->voltages[id].data());
					frameState.accumID = 0;
					renderer.updateFrameState(frameState);
					numFrames = 0;
				}

				ct++;
				if (ct > sz - 1)
				{
					saveVolChangeFlag = false;
					ct = 0;
				}

			}
			numFrames++;
		}
	}

	void TubesViewer::saveVolandRollChangeAni()
	{
		if (volAndrollFlag)
		{
			//  std::cout << "fuck\n";
			static int ct = 0;
			static int numFrames = 0;
			if (numFrames > 100)
			{
				std::cout << "try to vol" << std::endl;
				std::ostringstream  istr;
				istr << std::setw(4) << std::setfill('0') << ct;
#ifdef WIN32
				std::string fileName = cmdline_args.screenshotPath + "\\" + istr.str() + ".png";
#else
				std::string fileName = cmdline_args.screenshotPath + "/" + istr.str() + ".png";
#endif
				frameState.frameID++;

				int sz = rtn->voltages.size();
				if (frameState.frameID >= sz)
					frameState.frameID = 0;
				int id = clamp(frameState.frameID, 0, sz - 1);
				std::cout << frameState.frameID << " " << id << " " << rtn->voltages[id].size() << std::endl;

				if (ct % 1 == 0)
				{
					saveTofile(fileName);
					const vec2i a = vec2i(0, 1);

					renderer.updatevoltagesColor(rtn->voltages[id].data());
					//mouseDragLeft(a, true);
					numFrames = 0;
				}

				ct++;
				if (ct > sz - 1)
				{
					volAndrollFlag = false;
					ct = 0;
				}
			}
			numFrames++;
		}

	}

	float print_vector(const std::vector<float> a)
	{
		std::ostringstream  istr;
		float sum = 0.f;
		float average = 0.f;
		float variance = 0.f;
		float vmax = -1e9f;
		float vmin = 1e9f;
		float up_average = 0;
		float down_average = 0;
		for (int i = 0; i < a.size(); ++i)
		{
			//istr << std::setfill('0') << std::setprecision(6) << a[i] << " ";
			//if (i % 8 == 0) istr << "\n";
			sum += a[i];
			if (a[i] > vmax) vmax = a[i];
			if (a[i] < vmin) vmin = a[i];
		}
		average = sum / a.size();
		istr << "average=" << average << "\n";
		istr << "min=" << vmin << "\n";
		istr << "max=" << vmax << "\n";
		for (int i = 0; i < a.size(); ++i)
		{
			variance += (a[i] - average) * (a[i] - average);
			if (a[i] > average) up_average++;
			if (a[i] < average) down_average++;
		}

		istr << "variance=" << variance << "\n";
		istr << "up=" << up_average << "\n";
		istr << "down=" << down_average << "\n";

		std::cout << istr.str() << std::endl;
		return (average + 0.05 * (vmax - vmin) - vmin) / (vmax - vmin);
	}

	void TubesViewer::updateVoltageAndCamera()
	{
		if (bUpdateVoltageAndCamera)
		{
			static int image_id = 0;
			static int numFrames = 0;
			static char msg[4 * 1024] = { 0 };
			static std::vector<float> derivatives(rtn->voltages[0].size(), 0.f);
			const int sz = rtn->voltages.size();
			if (numFrames++ > anim_accum)
			{
				if (image_id > 0)
				{
					std::ostringstream  istr;
					istr << std::setw(4) << std::setfill('0') << image_id;
#ifdef WIN32
					std::string fileName = cmdline_args.screenshotPath + "\\" + istr.str() + ".png";
#else
					std::string fileName = cmdline_args.screenshotPath + "/" + istr.str() + ".png";
#endif

					saveTofile(fileName);
					printf("- save png:%s\n", fileName.c_str());
				}

				{
					if (voltage_index >= sz || voltage_index > voltage_index_end)
					{
						voltage_index = 0;
						numFrames = 0;
						bUpdateVoltageAndCamera = false;
						printf("============ combined animation done ============\n");
						return;
					}

					static const float threshold = -110.f + 100.f * frameState.voltage_threshold;

					const int id = voltage_index;
					const int prvid = id - 1;
					int has_high_voltage = 0;
					float* b = rtn->voltages[prvid].data();
					float* a = rtn->voltages[id].data();
					int n = rtn->voltages[id].size();
					for (int i = 0; i < n; ++i)
					{
						derivatives[i] = a[i] - b[i];
						if (a[i] > threshold)
							has_high_voltage++;
					}
					//print_vector(derivatives);
					//if (has_high_voltage > 1)
					{
						setCamera(voltage_index);
						renderer.updatevoltagesColor(rtn->voltages[id].data());
						renderer.updatevoltagesDeriv(derivatives.data());
						Log("- update voltage [%4d -> %4d] size=%d\n", prvid, id, rtn->voltages[id].size());
						voltage_index++;
						numFrames = 0;
						image_id++;
					}
					if (frameState.step++ > steps) frameState.step = 0;
				}

			}
		}
	}

	void TubesViewer::circleCamera(int vol_index, int step, int n = 100)
	{

		//float r = model_span.z * 0.85f;
		float r = model_span.z * this->cameraDistScale;
		float alpha = (float(vol_index) / float(n));
		float theta = alpha * (2.f * 3.1415926f);
		vec3f p;
		p.x = r * cos(theta);
		p.y = camera_target.y;
		p.z = r * sin(theta);
		p = p + camera_target;

		this->setCameraOrientation(
			/*origin   */ p,
			/*lookat   */ camera_target,
			/*up-vector*/ vec3f(0.f, 1.f, 0.f),
			/*fovy(deg)*/ camera_fov);

		Log("circle camera set position dist=%f alpha = %f (%d/%d), [%f,%f,%f] --> [%f,%f,%f]\n", r, alpha, vol_index, n, p.x, p.y, p.z, camera_target.x, camera_target.y, camera_target.z);

	}

	void TubesViewer::circleCameraAnimation(int nSamples = 100)
	{
		const static int voltage_end = nSamples;
		static int voltage_cur = 1;

		if (bTestCamera)
		{
			static int image_id = 0;
			static int numFrames = 0;
			static std::vector<float> derivatives(rtn->voltages[0].size(), 0.f);
			static const int sz = rtn->voltages.size();
			if (numFrames++ > anim_accum)
			{
				if (image_id > 0)
				{
					std::ostringstream  istr;
					istr << std::setw(4) << std::setfill('0') << image_id;
#ifdef WIN32
					std::string fileName = cmdline_args.screenshotPath + "\\" + istr.str() + ".png";
#else
					std::string fileName = cmdline_args.screenshotPath + "/" + istr.str() + ".png";
#endif
					saveTofile(fileName);
					Log("- save png:%s\n", fileName.c_str());
				}

				{
					if (voltage_cur >= sz || voltage_cur > voltage_end)
					{
						voltage_cur = 0;
						numFrames = 0;
						image_id = 0;
						bTestCamera = false;
						Log("============ circle animation done ============\n");
						return;
					}

					const float threshold = -110.f + 100.f * frameState.voltage_threshold;

					const int id = voltage_cur;
					const int prvid = id - 1;
					int has_high_voltage = 0;
					float* b = rtn->voltages[prvid].data();
					float* a = rtn->voltages[id].data();
					int n = rtn->voltages[id].size();
					for (int i = 0; i < n; ++i)
					{
						derivatives[i] = a[i] - b[i];
						if (a[i] > threshold)
							has_high_voltage++;
					}
					//print_vector(derivatives);
					//if (has_high_voltage > 1)
					{
						renderer.updatevoltagesColor(rtn->voltages[id].data());
						renderer.updatevoltagesDeriv(derivatives.data());
						Log("- circle camera update voltage [%4d -> %4d] size=%ld\n", prvid, id, rtn->voltages[id].size());
						circleCamera(voltage_cur - 1, 0, nSamples);
						voltage_cur++;
						numFrames = 0;
						image_id++;
						if (frameState.step++ > steps) frameState.step = 0;
					}
				}

			}
		}
	}

	void TubesViewer::aroundCenterCameraAnimation(int n = 100)
	{
		const static int voltage_end = n / steps;
		static int voltage_cur = 1;

		if (bTestCamera)
		{
			static int image_id = 0;
			static int numFrames = 0;
			static std::vector<float> derivatives(rtn->voltages[0].size(), 0.f);
			static const int sz = rtn->voltages.size();
			if (numFrames++ > anim_accum)
			{
				if (image_id > 0)
				{
					std::ostringstream  istr;
					istr << std::setw(4) << std::setfill('0') << image_id;
#ifdef WIN32
					std::string fileName = cmdline_args.screenshotPath + "\\" + istr.str() + ".png";
#else
					std::string fileName = cmdline_args.screenshotPath + "/" + istr.str() + ".png";
#endif                
					saveTofile(fileName);
					Log("- save png:%s\n", fileName.c_str());
				}

				if (frameState.step == 0)
				{
					if (voltage_cur >= sz || voltage_cur > voltage_end)
					{
						voltage_cur = 0;
						numFrames = 0;
						image_id = 0;
						bTestCamera = false;
						Log("============ circle animation done ============\n");
						return;
					}

					const float threshold = -110.f + 100.f * frameState.voltage_threshold;

					const int id = voltage_cur;
					const int prvid = id - 1;
					int has_high_voltage = 0;
					float* b = rtn->voltages[prvid].data();
					float* a = rtn->voltages[id].data();
					int n = rtn->voltages[id].size();
					for (int i = 0; i < n; ++i)
					{
						derivatives[i] = a[i] - b[i];
						if (a[i] > threshold)
							has_high_voltage++;
					}
					//print_vector(derivatives);
					//if (has_high_voltage > 1)
					{
						renderer.updatevoltagesColor(rtn->voltages[id].data());
						renderer.updatevoltagesDeriv(derivatives.data());
						Log("- circle camera update voltage [%4d -> %4d] size=%d\n", prvid, id, rtn->voltages[id].size());
						//circleCamera(voltage_cur - 1, frameState.step,n);
						//voltage_cur++;
						//frameState.accumID = 0;
						numFrames = anim_accum - 1;
						//image_id++;
						frameState.step++;
					}
				}
				else
				{
					if (frameState.step >= steps)
					{
						frameState.step = 0;
						if (voltage_cur > voltage_end)
						{
							voltage_cur = 0;
							numFrames = 0;
							image_id = 0;
							bTestCamera = false;
							Log("============ circle animation done ============\n");
							return;
						}
						else
						{
							voltage_cur++;
							numFrames = 0;
							image_id++;
							//Log("============ circle animation done ============\n");

						}
					}
					else
					{
						circleCamera(voltage_cur - 1, frameState.step, n);
						Log("- circle camera update step:%d \n", frameState.step);
						numFrames = 0;
						image_id++;
						frameState.step++;
					}
					//frameState.accumID = 0;
				}
			}
		}
	}

	void TubesViewer::autoUpdateVoltage()
	{
		if (bUpdateVoltage)
		{
			static int image_id = 0;
			static int numFrames = 0;
			static char msg[4 * 1024] = { 0 };

			static std::vector<float> voltages(rtn->voltages.size(), 0.0f);
			static std::vector<float> derivatives(rtn->voltages.size(), 0.0f);

			if (numFrames++ > anim_accum)
			{
				if (image_id > 0)
				{
					std::ostringstream  istr;
					istr << std::setw(4) << std::setfill('0') << image_id;
#ifdef WIN32
					std::string fileName = cmdline_args.screenshotPath + "\\" + istr.str() + ".png";
#else
					std::string fileName = cmdline_args.screenshotPath + "/" + istr.str() + ".png";
#endif
					saveTofile(fileName);
					image_id = 0;
				}

				//frameState.frameID++;
				voltage_index += frameState.step;

				const int sz = rtn->maxTime - 1;
				//if (frameState.frameID >= sz)
				if (voltage_index >= sz)
				{
					//frameState.frameID = 0;
					voltage_index = 0;
					numFrames = 0;
					bUpdateVoltage = false;
					std::cout << "============ update done ============\n";
					return;
				}
				else
				{
					static const float threshold = -110.f + 100.f * frameState.voltage_threshold;

					//const int id = clamp(frameState.frameID, 0, sz - 1);
					const int id = voltage_index; // clamp(frameState.frameID, 0, sz - 1);
					const int previd = id - frameState.step;
					int has_high_voltage = 0;

					float currentVoltage, previousVoltage;

					for (int i = 0; i < rtn->voltages.size(); i++)
					{
						if (!rtn->haveVoltage[i] || (id < rtn->voltageStart[i] || id >= rtn->voltageEnd[i]))
						{
							currentVoltage = frameState.resting;
						}
						else
						{
							currentVoltage = rtn->voltages[i][id];
						}

						if (!rtn->haveVoltage[previd] || (previd < rtn->voltageStart[i] || previd >= rtn->voltageEnd[i]))
						{
							previousVoltage = frameState.resting;
						}
						else
						{
							previousVoltage = rtn->voltages[i][previd];
						}

						voltages[i] = currentVoltage;
						derivatives[i] = currentVoltage - previousVoltage;

						if (currentVoltage > threshold) has_high_voltage++;
					}

					print_vector(voltages);
					print_vector(derivatives);

					if (has_high_voltage > 1)
					{
						renderer.updatevoltagesColor(voltages.data());
						renderer.updatevoltagesDeriv(derivatives.data());
						//frameState.accumID = 0;
						image_id = id;
						sprintf(msg, "-[%4d] update voltage [%4d  ->  %4d] size=%ld\n", voltage_index, previd, id, rtn->voltages[id].size());
						std::cout << msg;
						numFrames = 0;
					}
				}
			}
		}
	}


	// /*! this function gets called whenever the viewer widget changes camera settings */
	void TubesViewer::cameraChanged()
	{
		const owl::viewer::SimpleCamera scamera = getSimplifiedCamera();
		const vec3f screen_du = scamera.screen.horizontal / float(getWindowSize().x);
		const vec3f screen_dv = scamera.screen.vertical / float(getWindowSize().y);
		frameState.camera_screen_du = screen_du;
		frameState.camera_screen_dv = screen_dv;
		frameState.camera_screen_00 = scamera.screen.lower_left;
		frameState.camera_lens_center = scamera.lens.center;
		frameState.camera_lens_du = scamera.lens.du;
		frameState.camera_lens_dv = scamera.lens.dv;
		frameState.accumID = 0;
	}

	/*! window notifies us that we got resized */
	void TubesViewer::resize(const vec2i &newSize)
	{
		this->fbSize = newSize;

		// ... tell parent to resize (also resizes the pbo in the window)
		OWLViewer::resize(newSize);

		renderer.resizeFrameBuffer(newSize, fbPointer);

		// ... and finally: update the camera's aspect
		setAspect(newSize.x / float(newSize.y));

		// update camera as well, since resize changed both aspect and
		// u/v pixel delta vectors ...
		updateCamera();
		owlBuildSBT(renderer.context);
	}


	void TubesViewer::recordCameraTrack()
	{
		if (bRecordCamera)
		{
			auto& fc = this->camera;
			std::cout << "Record camera:" << "@" << voltage_index << std::endl;
			std::cout << "\t- from :" << fc.position << std::endl;
			std::cout << "\t- poi  :" << fc.getPOI() << std::endl;
			std::cout << "\t- upVec:" << fc.upVector << std::endl;
			std::cout << "\t- fov  :" << fc.getFovyInDegrees() << std::endl;
			std::cout << "\t- frame:" << fc.frame << std::endl;
			std::cout << "\n";
			camera_track[voltage_index] = { voltage_index, fc.position, fc.getPOI(),fc.upVector,fc.getFovyInDegrees() };

			interp_map.clear();
			int prv_sample = -1;
			for (auto const& entry : camera_track)
			{
				int cur_sample = entry.first;
				interp_map.insert(std::make_pair(cur_sample, std::make_pair(cur_sample, cur_sample)));
				for (int i = cur_sample - 1; prv_sample >= 0 && i > prv_sample; --i)
				{
					interp_map.insert(std::make_pair(i, std::make_pair(prv_sample, cur_sample)));
				}
				prv_sample = cur_sample;
			}
			for (auto const& entry : camera_track)
			{
				std::cout << " (" << entry.first << "," << entry.second.position << " )" << '\n';
			}
			for (auto const& entry : interp_map)
			{
				//std::cout << " (" << entry.first << "," << entry.second.first << "-->" << entry.second.second << " ) ";
			}
			std::cout << '\n';
			auto start = camera_track.begin()->first;
			auto end = (--camera_track.end())->first;
			printf("[%d,%d]\n", start, end);

			bRecordCamera = false;
		}
	}

	void TubesViewer::setCamera(int sample_index)
	{
		auto size = camera_track.size();
		if (size <= 1)
		{
			Log("No camera track recorded, stop!!\n");
			bUpdateVoltageAndCamera = false;
			return;
		}
		else
		{
			Log("set camera for: %d  \n", sample_index);
		}
		auto pairs = interp_map[sample_index];
		auto a = pairs.first;
		auto b = pairs.second;
		if (a == b)
		{
			auto& info = camera_track[sample_index];

			std::cout << "sample point [" << a << "," << b << "]" << info.position << '\n';

			this->setCameraOrientation(
				/*origin   */ info.position,
				/*lookat   */ camera_target,
				/*up-vector*/ vec3f(0.f, 1.f, 0.f),
				/*fovy(deg)*/ camera_fov);
		}
		else
		{
			auto alpha = float(sample_index - a) / float(b - a);
			const auto& pa = camera_track[a].position;
			const auto& pb = camera_track[b].position;
			const auto p = pa * (1.f - alpha) + alpha * pb;

			std::cout << "interpolate sample point [" << a << "," << b << "," << alpha << "]:  { " << p << " }\n";

			this->setCameraOrientation(
				/*origin   */ p,
				/*lookat   */ camera_target,
				/*up-vector*/ vec3f(0.f, 1.f, 0.f),
				/*fovy(deg)*/ camera_fov);
		}
	}


	/*! gets called whenever the viewer needs us to re-render out widget */
	void TubesViewer::render()
	{
		if (fbSize.x <= 0 || fbSize.y <= 0)
		{
			return;
		}
		moveCameraAni();
		rollCameraAni();
		autoUpdateVoltage();
		// updateVoltageAndCamera();
		// circleCameraAnimation(nCircleSamples);

		static double avg_t = 0.;
		//UI
		if (cameraManipulator != fpsModeManipulator && !bWormMainView && !bSingleMainView)
		{
			if (frameState.accumID == 0)
			{
				render_start_time = getCurrentTime();
			}

			double elapsed = getCurrentTime() - render_start_time;
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.9f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.0f);

			ImGui::SetNextWindowPos(ImVec2(2.0f, 2.0f));
			ImGui::Begin("stats", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize |
				ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);
			ImGui::Text("fps: %8.2f  %8.2f", fps, (1.f / avg_t));
			ImGui::Text("frame: %d", (frameState.accumID));
			ImGui::Text("time: %lf", elapsed);
			ImGui::End();

			ImGui::SetNextWindowPos(ImVec2(2.0f, 70.0f));
			ImGui::Begin("settings", 0, /*ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize*/ ImGuiWindowFlags_AlwaysAutoResize);
			if (ImGui::CollapsingHeader("Camera property"))
			{
				// if (ImGui::Checkbox("move camera", &moveCameraFlag)) {}
				// ImGui::SameLine();
				// if (ImGui::Checkbox("roll camera", &rollCameraFlag)) {}
				// ImGui::SameLine();
				// if (ImGui::Checkbox("record camera", &bRecordCamera)) { recordCameraTrack(); }

				// if (ImGui::Checkbox("clear camera track", &bClearCameraTrack))
				// {
				// 	if (bClearCameraTrack == true)
				// 	{
				// 		camera_track.clear();
				// 		bClearCameraTrack = false;
				// 	}
				// }


				// if (ImGui::Checkbox("test camera", &bTestCamera))
				// {
				// 	if (bTestCamera == true)
				// 	{
				// 		bUpdateVoltage = false;
				// 		moveCameraFlag = false;
				// 		rollCameraFlag = false;
				// 		bUpdateVoltageAndCamera = false;
				// 	}
				// }
				// ImGui::SameLine();
				static bool bResetCamera = false;
				if (ImGui::Checkbox("reset camera", &bResetCamera))
				{
					if (bResetCamera == true)
					{
						this->setCameraOrientation(
							/*origin   */ camera_pos,
							/*lookat   */ camera_target,
							/*up-vector*/ vec3f(0.f, 1.f, 0.f),
							/*fovy(deg)*/ camera_fov);
					}
				}
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Renderer Property"))
			{

				if (ImGui::SliderInt("path depth", &frameState.pathDepth, 1, 10)) { frameState.accumID = 0; }

				if (ImGui::SliderInt("spp", &frameState.samplesPerPixel, 1, 128)) { frameState.accumID = 0; }

				// if (ImGui::SliderInt("shader mode", (&frameState.shadeMode), 0, 2)) { frameState.accumID = 0; }

				// if (ImGui::SliderInt("max accum", &frameState.maxAccum, -1, 140)) { frameState.accumID = 0; }

			}

			ImGui::Separator();

			if (triangles && ImGui::CollapsingHeader("Triangle Mesh Property"))
			{

				if (ImGui::Checkbox("enable mesh transparency", &frameState.enable_mesh_transparency)) { frameState.accumID = 0; }

				if (ImGui::SliderFloat("mesh transparency", &frameState.mesh_transparency, 0.8f, 0.999f)) 
				{
					if (frameState.maxAccum <= 0)
					{
						frameState.accumID = 0;
					}
				}

				if (ImGui::SliderInt("max transparency depth", &frameState.max_transparency_depth, 1, 7)) 
				{
					if (frameState.maxAccum <= 0)
					{
						frameState.accumID = 0;
					}
				}					
			}
			if (triangles)
			{
				ImGui::Separator();
			}

			if (rtn && ImGui::CollapsingHeader("SDF Property"))
			{
				if (ImGui::Checkbox("activate SDF and ray marching", &frameState.useSDF)) { frameState.accumID = 0; }

				if (ImGui::SliderInt("max ray marching iteration", &frameState.maxMarchIteration, 0, 1000)) { frameState.accumID = 0; }
			}
			if (rtn)
			{
				ImGui::Separator();
			}
				
			if (rtn)
			{
				if (rtn->haveVoltages && ImGui::CollapsingHeader("Voltage Property"))
				{
					// if (ImGui::Checkbox("voltage animation", &bUpdateVoltage)) {}
					// ImGui::SameLine();
					// if (ImGui::Checkbox("derivative mode", &frameState.enable_voltage_deriv)) { frameState.accumID = 0; }

					/*if (ImGui::Checkbox("animate voltage & camera", &bUpdateVoltageAndCamera))
					{
						if (bUpdateVoltageAndCamera == true)
						{
							bUpdateVoltage = false;
							moveCameraFlag = false;
							rollCameraFlag = false;
							bTestCamera = false;

							if (camera_track.size() > 1)
							{
								voltage_index = camera_track.begin()->first;
								voltage_index_end = (--camera_track.end())->first;
							}
							else
							{
								Log("No enough Camera track recorded, may crash, stop animation!\n");
								bUpdateVoltageAndCamera = false;
							}
						}
					}*/

					// static std::vector<float> voltages(rtn->voltages.size(), 0.0f);
					// static std::vector<float> derivatives(rtn->voltages.size(), 0.0f);

					// if (ImGui::SliderInt("voltage sequence", &voltage_index, 0, rtn->maxTime - 1))
					// {
					// 	auto id = voltage_index;
					// 	auto previd = id - 1;
					// 	previd = previd < 0 ? 0 : previd;

					// 	float currentVoltage, previousVoltage;

					// 	for (int i = 0; i < rtn->voltages.size(); i++)
					// 	{
					// 		if (!rtn->haveVoltage[i] || (id < rtn->voltageStart[i] || id >= rtn->voltageEnd[i]))
					// 		{
					// 			currentVoltage = frameState.resting;
					// 		}
					// 		else
					// 		{
					// 			currentVoltage = rtn->voltages[i][id];
					// 		}
								
					// 		if (!rtn->haveVoltage[previd] || (previd < rtn->voltageStart[i] || previd >= rtn->voltageEnd[i]))
					// 		{
					// 			previousVoltage = frameState.resting;
					// 		}
					// 		else
					// 		{
					// 			previousVoltage = rtn->voltages[i][previd];
					// 		}

					// 		voltages[i] = currentVoltage;
					// 		derivatives[i] = currentVoltage - previousVoltage;
					// 	}	

					// 	renderer.updatevoltagesColor(voltages.data());
					// 	renderer.updatevoltagesDeriv(derivatives.data());
					// 	print_vector(voltages);
					// 	print_vector(derivatives);

					// 	if (frameState.maxAccum <= 0)
					// 	{
					// 		frameState.accumID = 0;
					// 	}
					// }

					// if (ImGui::SliderInt("sequence step", &frameState.step, 0, 40)) { frameState.accumID = 0; }

					// if (ImGui::SliderFloat("voltage threshold", &frameState.voltage_threshold, 0.001f, 0.999f)) { frameState.accumID = 0; }

					// if (ImGui::SliderInt("animation accum frame", (&anim_accum), 1, 140)) {}

					// ImGui::Separator();

					if (ImGui::Checkbox("enable emissive", &frameState.enable_emissive)) { frameState.accumID = 0; }

					if (ImGui::SliderFloat("emissive intensity", &frameState.emissive_intensity, 0.001f, 1.0f)) { frameState.accumID = 0; }

					if (!frameState.enable_voltage_deriv && ImGui::SliderFloat("inhibition", &frameState.inhibition, frameState.minVoltage, frameState.resting)) { frameState.accumID = 0; }

					if (!frameState.enable_voltage_deriv && ImGui::SliderFloat("resting", &frameState.resting, frameState.inhibition, frameState.excitation)) { frameState.accumID = 0; }

					if (!frameState.enable_voltage_deriv && ImGui::SliderFloat("excitation", &frameState.excitation, frameState.resting, frameState.firing)) { frameState.accumID = 0; }

					if (!frameState.enable_voltage_deriv && ImGui::SliderFloat("firing", &frameState.firing, frameState.excitation, frameState.maxVoltage)) { frameState.accumID = 0; }

					if (!frameState.enable_voltage_deriv && ImGui::SliderFloat("minVoltage", &frameState.minVoltage, -200, frameState.inhibition)) { frameState.accumID = 0; }

					if (!frameState.enable_voltage_deriv && ImGui::SliderFloat("maxVoltage", &frameState.maxVoltage, frameState.firing, 1200)) { frameState.accumID = 0; }

					if (frameState.enable_voltage_deriv && ImGui::SliderFloat("excitationDeriv", &frameState.excitationDeriv, 0.0f, frameState.firingDeriv)) { frameState.accumID = 0; }

					if (frameState.enable_voltage_deriv && ImGui::SliderFloat("firingDeriv", &frameState.firingDeriv, frameState.excitationDeriv, 1.0f)) { frameState.accumID = 0; }

					ImGui::Separator();

					if (ImGui::ColorEdit3("synapse type 0 color", &frameState.synapse_type0_color.x, ImGuiColorEditFlags_NoInputs)) { frameState.accumID = 0; }
					ImGui::SameLine();
					if (ImGui::ColorEdit3("synapse type 1 color", &frameState.synapse_type1_color.x, ImGuiColorEditFlags_NoInputs)) { frameState.accumID = 0; }
					ImGui::SameLine();
					if (ImGui::ColorEdit3("synapse type 2 color", &frameState.synapse_type2_color.x, ImGuiColorEditFlags_NoInputs)) { frameState.accumID = 0; }
				}
			}
			if (rtn)
			{
				if (rtn->haveVoltages)
				{
					ImGui::Separator();
				}					
			}					

			if (ImGui::CollapsingHeader("Light Property"))
			{
				//if (ImGui::Checkbox("enable point light", &frameState.enable_point_light)) { frameState.accumID = 0; }
				//ImGui::SameLine();
				//if (ImGui::Checkbox("enable quad  light", &frameState.enable_quad_light)) { frameState.accumID = 0; }

				if (ImGui::ColorEdit3("top color", &bg_top_color.x, ImGuiColorEditFlags_NoInputs)) { frameState.accumID = 0; }
				ImGui::SameLine();
				if (ImGui::ColorEdit3("bottom color", &bg_bottom_color.x, ImGuiColorEditFlags_NoInputs)) { frameState.accumID = 0; }
				ImGui::SameLine();
				if (ImGui::ColorEdit3("ground color", &frameState.ground_color.x, ImGuiColorEditFlags_NoInputs)) { frameState.accumID = 0; }

				ImGui::Separator();

				//if (ImGui::SliderFloat3("point light translate", &frameState.point_light_pos.x, -1000.f, 1000.f)) { frameState.accumID = 0; }

				//if (ImGui::SliderFloat("point light intensity", &frameState.point_light_intensity, .5f, 2.f)) { frameState.accumID = 0; }

				if (ImGui::SliderFloat("quad light intensity", &frameState.quad_light.intensity, .5f, 2.f)) { frameState.accumID = 0; }

				if (ImGui::ColorEdit3("quad light color", &frameState.quad_light.color.x, ImGuiColorEditFlags_NoInputs)) { frameState.accumID = 0; }

				ImGui::Separator();

				if (ImGui::Checkbox("enable light decay", &frameState.enable_light_decay)) { frameState.accumID = 0; }

				if (ImGui::SliderFloat("max decay distance", &frameState.max_decay_distance, 1e2f, 1e5f)) { frameState.accumID = 0; }

				if (ImGui::SliderFloat("decay factor", &frameState.light_decay_index, 1.f, 2.2f)) { frameState.accumID = 0; }

			}
			ImGui::End();

			ImGui::PopStyleVar(3);
		}
		if (frameState.accumID == 0)
		{
			render_start_time = getCurrentTime();
		}
		static double t_last = -1;
		renderer.updateBgColors(bg_top_color, bg_bottom_color);
		renderer.updateFrameState(frameState);
		renderer.render();

		double t_now = getCurrentTime();

		if (t_last >= 0)
			avg_t = 0.8*avg_t + 0.2*(t_now - t_last);

		static double measure_begin = t_now;
		static int numFrames = 0;
		if (displayFPS)
		{
			if (t_now - measure_begin > 3.f)
			{
				std::cout << "MEASURE_FPS " << (numFrames / (t_now - measure_begin)) << std::endl;
				measure_begin = t_now;
				numFrames = 0;
			}
		}
		t_last = t_now;
		if (cmdline_args.measure)
		{
			if (t_now - measure_begin > 60.f)
			{
				std::cout << "MEASURE_FPS " << (numFrames / (t_now - measure_begin)) << std::endl;
				screenShot();
				exit(0);
			}
		}

		numFrames++;
		frameState.accumID++;
	}

	


	// ------------------------------------------------------------------------------------
	// callbacks 

	/*! callback for a window resizing event */
	static void glfwindow_reshape_cb(GLFWwindow* window, int width, int height)
	{
		OWLViewer *gw = static_cast<OWLViewer*>(glfwGetWindowUserPointer(window));
		assert(gw);
		gw->resize(vec2i(width, height));
	}

	/*! callback for a key press */
	static void glfwindow_char_cb(GLFWwindow *window,
		unsigned int key)
	{
		OWLViewer *gw = static_cast<OWLViewer*>(glfwGetWindowUserPointer(window));
		assert(gw);
		gw->key(key, gw->getMousePos());
		ImGui_ImplGlfw_CharCallback(window, key);
	}

	/*! callback for a key press */
	static void glfwindow_key_cb(GLFWwindow *window,
		int key,
		int scancode,
		int action,
		int mods)
	{
		OWLViewer *gw = static_cast<OWLViewer*>(glfwGetWindowUserPointer(window));
		assert(gw);
		if (action == GLFW_PRESS)
		{
			gw->special(key, mods, gw->getMousePos());
		}

		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	}

	/*! callback for _moving_ the mouse to a new position */
	static void glfwindow_mouseMotion_cb(GLFWwindow *window, double x, double y)
	{
		OWLViewer *gw = static_cast<OWLViewer*>(glfwGetWindowUserPointer(window));
		assert(gw);
		gw->mouseMotion(vec2i((int)x, (int)y));
	}

	/*! callback for _moving_ the mouse to a new position */
	static void fps_mode_mouseMotion_cb(GLFWwindow *window, double x, double y)
	{
		TubesViewer *gw = static_cast<TubesViewer*>(glfwGetWindowUserPointer(window));
		assert(gw);
		gw->fpsMouseMotion(vec2i((int)x, (int)y));
	}

	/*! callback for pressing _or_ releasing a mouse button*/
	static void glfwindow_mouseButton_cb(GLFWwindow *window, int button, int action, int mods)
	{
		OWLViewer *gw = static_cast<OWLViewer*>(glfwGetWindowUserPointer(window));
		assert(gw);
		gw->mouseButton(button, action, mods);
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	}


	void TubesViewer::renderWorm()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, wormFb);
		glBindTexture(GL_TEXTURE_2D, 0);

		simulationWindow->Reshape(fbSize.x, fbSize.y);
		
		glViewport(0, 0, fbSize.x, fbSize.y);

		simulationWindow->Display();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	/*! main loop for window */
	void TubesViewer::showAndRun()
	{
		int width, height;
		glfwGetFramebufferSize(handle, &width, &height);
		resize(vec2i(width, height));

		ImGui::CreateContext();
		ImGui_ImplGlfwGL2_Init(handle, /*install callbacks*/ true);

		glfwSetFramebufferSizeCallback(handle, glfwindow_reshape_cb);

		glfwSetMouseButtonCallback(handle, glfwindow_mouseButton_cb);
		glfwSetKeyCallback(handle, glfwindow_key_cb);
		glfwSetCharCallback(handle, glfwindow_char_cb);
		glfwSetCursorPosCallback(handle, fps_mode_mouseMotion_cb);

		glGenFramebuffers(1, &wormFb);
		glBindFramebuffer(GL_FRAMEBUFFER, wormFb);

		glGenTextures(1, &wormTexture);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, wormTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbSize.x, fbSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, wormTexture, 0);
		
		// GLenum drawbuffers[1] = {GL_COLOR_ATTACHMENT0};
		// glDrawBuffers(1, drawbuffers);

		if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "off screen framebuffer for worm fails to work";
			exit(0);
		}

		while (!glfwWindowShouldClose(handle))
		{
			glfwPollEvents();

			if (cameraManipulator != fpsModeManipulator)
			{
				ImGui_ImplGlfwGL2_NewFrame();
				ImGuiIO& io = ImGui::GetIO();
				if (!io.WantCaptureMouse)
				{
					double x, y;
					glfwGetCursorPos(handle, &x, &y);
					glfwindow_mouseMotion_cb(handle, x, y);
				}
			}

			motionKey();
			simulationWindow->Timer(0);

			static double lastCameraUpdate = -1.f;
			if (camera.lastModified != lastCameraUpdate)
			{
				cameraChanged();
				lastCameraUpdate = camera.lastModified;
			}

			{
				frameCount++;
				double t_start = getCurrentTime(); // in second
				
				render();

				renderWorm();

				draw();

				if (cameraManipulator != fpsModeManipulator)
				{
					ImGui::Render();
					ImGui_ImplGlfwGL2_RenderDrawData(ImGui::GetDrawData());
				}

				double t_end = getCurrentTime();
				frameTime = t_end - t_start;
				fps = 1. / frameTime;

#if 0
				totalFrameTime += frameTime;
				if (frameCount >= 60)
				{
					fps = frameCount / totalFrameTime;
					frameCount = 0;
					totalFrameTime = 0;
				}
				else
				{
					fps = frameCount <= 1 ? 1 : 1. / frameTime;
				}
#endif
			}

			glfwSwapBuffers(handle);
			}
		glfwDestroyWindow(handle);
		glfwTerminate();
		ImGui_ImplGlfwGL2_Shutdown();
	}

	/*! re-draw the current frame. This function itself isn't
      virtual, but it calls the framebuffer's render(), which
      is */
    void TubesViewer::draw()
    {
      	if (shareGraphicResource)
      	{
      		cudaError_t rc = cudaSuccess;
      		cudaGraphicsMapResources(1, &cuDisplayTexture);
      		rc = cudaGetLastError();
      		if (rc != cudaSuccess)
        		fprintf(stderr, "error (%s: line %d): %s\n", __FILE__, __LINE__, cudaGetErrorString(rc));

      		cudaArray_t array;
      		cudaGraphicsSubResourceGetMappedArray(&array, cuDisplayTexture, 0, 0);
      		rc = cudaGetLastError();
      		if (rc != cudaSuccess)
        		fprintf(stderr, "error (%s: line %d): %s\n", __FILE__, __LINE__, cudaGetErrorString(rc));
      		
        	// sample.copyGPUPixels(cuDisplayTexture);
        	cudaMemcpy2DToArray(array,
            		            0,
                		        0,
                    	        reinterpret_cast<const void *>(fbPointer),
                    		    fbSize.x * sizeof(uint32_t),
                        		fbSize.x * sizeof(uint32_t),
    	                        fbSize.y,
            	                cudaMemcpyDeviceToDevice);
      		rc = cudaGetLastError();
			
      		if (rc != cudaSuccess)
        		fprintf(stderr, "error (%s: line %d): %s\n", __FILE__, __LINE__, cudaGetErrorString(rc));

      		cudaGraphicsUnmapResources(1, &cuDisplayTexture);
      	}
      	else
      	{
			glEnable(GL_TEXTURE_2D);
        	glBindTexture(GL_TEXTURE_2D, fbTexture);     	
        	glTexSubImage2D(GL_TEXTURE_2D,0, 0,0, fbSize.x, fbSize.y, GL_RGBA, GL_UNSIGNED_BYTE, fbPointer);
      	}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

      	//glDisable(GL_LIGHTING);
      	glColor3f(1.f, 1.f, 1.f);

      	glMatrixMode(GL_MODELVIEW);
      	glLoadIdentity();
      
      	//glDisable(GL_DEPTH_TEST);
		//glEnable(GL_DEPTH_TEST);

      	glViewport(0, 0, fbSize.x, fbSize.y);

      	glMatrixMode(GL_PROJECTION);
      	glLoadIdentity();
      	glOrtho(0.f, (float)fbSize.x, 0.f, (float)fbSize.y, -1.f, 1.f);

		glEnable(GL_TEXTURE_2D);

		if (bWormMainView) 
		{	
			
			glBindTexture(GL_TEXTURE_2D, wormTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.f, 0.f);
				glVertex3f(0.f, 0.f, 0.f);
		
				glTexCoord2f(0.f, 1.f);
				glVertex3f(0.f, (float)fbSize.y, 0.f);
		
				glTexCoord2f(1.f, 1.f);
				glVertex3f((float)fbSize.x, (float)fbSize.y, 0.f);
		
				glTexCoord2f(1.f, 0.f);
				glVertex3f((float)fbSize.x, 0.f, 0.f);
			}
			glEnd();
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			if (!bSingleMainView)
			{
				glBindTexture(GL_TEXTURE_2D, fbTexture);
				//glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbSize.x, fbSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fbPointer);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				glBegin(GL_QUADS);
				{
					glTexCoord2f(0.f, 0.f);
					glVertex3f((float)-fbSize.x * 0.0f, 0.f, 0.f);
			
					glTexCoord2f(1.f, 0.f);
					glVertex3f((float)-fbSize.x * 0.0f, (float)fbSize.y, 0.f);
			
					glTexCoord2f(1.f, 1.f);
					glVertex3f((float)fbSize.x * 0.316f, (float)fbSize.y, 0.f);
			
					glTexCoord2f(0.f, 1.f);
					glVertex3f((float)fbSize.x * 0.316f, 0.f, 0.f);
				}
				glEnd();

				// glBegin(GL_QUADS);
				// {
				// 	glTexCoord2f(0.f, 1.f);
				// 	glVertex3f(-fbSize.x * 0.1f, 0.f, 0.f);
			
				// 	glTexCoord2f(1.f, 1.f);
				// 	glVertex3f(-fbSize.x * 0.1f, (float)fbSize.y, 0.f);
			
				// 	glTexCoord2f(1.f, 0.f);
				// 	glVertex3f((float)fbSize.x * 0.3f, (float)fbSize.y, 0.f);
			
				// 	glTexCoord2f(0.f, 0.f);
				// 	glVertex3f((float)fbSize.x * 0.3f, 0.f, 0.f);
				// }
				// glEnd();	
			}
			glDisable(GL_BLEND);
		}
		else 
		{
			glBindTexture(GL_TEXTURE_2D, fbTexture);
			//glTexSubImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fbSize.x, fbSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, fbPointer);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glBegin(GL_QUADS);
			{
				glTexCoord2f(0.f, 0.f);
				glVertex3f(0.f, 0.f, 0.f);
		
				glTexCoord2f(0.f, 1.f);
				glVertex3f(0.f, (float)fbSize.y, 0.f);
		
				glTexCoord2f(1.f, 1.f);
				glVertex3f((float)fbSize.x, (float)fbSize.y, 0.f);
		
				glTexCoord2f(1.f, 0.f);
				glVertex3f((float)fbSize.x, 0.f, 0.f);
			}
			glEnd();

			if(!bSingleMainView)
			{
				glBindTexture(GL_TEXTURE_2D, wormTexture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glBegin(GL_QUADS);
				{
					glTexCoord2f(0.f, 0.f);
					glVertex3f((float)fbSize.x * 0.7f, (float)fbSize.y * 0.7f, 0.f);
			
					glTexCoord2f(0.f, 1.f);
					glVertex3f((float)fbSize.x * 0.7f, (float)fbSize.y, 0.f);
			
					glTexCoord2f(1.f, 1.f);
					glVertex3f((float)fbSize.x, (float)fbSize.y, 0.f);
			
					glTexCoord2f(1.f, 0.f);
					glVertex3f((float)fbSize.x, (float)fbSize.y * 0.7f, 0.f);
				}
				glEnd();	
			}

					
		}

		
	}

	void TubesViewer::motionKey()
	{
		if (!bWormMainView)
		{
			if (cameraManipulator == flyModeManipulator) flyModeManipulator->motionKey();
			if (cameraManipulator == inspectModeManipulator) inspectModeManipulator->motionKey();
			if (cameraManipulator == fpsModeManipulator) fpsModeManipulator->motionKey();
			if (cameraManipulator == freeModeManipulator) freeModeManipulator->motionKey();
		}	
	}

	/*! this gets called when the user presses a key on the keyboard ... */
	void TubesViewer::key(char key, const vec2i &where)
	{
		switch(key)
		{
			case '2':
			case '@':
				bSingleMainView = !bSingleMainView;
				break;
			case 'v':
			case 'V':
				if (bWormMainView) bWormMainView = false;
				else bWormMainView = true;
				return;
				break;
			case 'q':
			case 'Q':
				// Not sure if we need to std::exit here, it seems to return
				// from this call but quit out ok
				exit(0);
				break;
		}
		if (bWormMainView) 
		{
			this->simulationWindow->Keyboard(key, where.x, where.y);
		}
		else
		{
			/* Haixin Ma: w a s d e c z x were occupied in motionKey(), 
				 + - < > were occupied in cameraManipulator->key() */

			if (cameraManipulator) cameraManipulator->key(key, where);

			static float focal_distance = cmdline_args.camera.focal_distance;
			static float lens_radius = cmdline_args.camera.lens_radius;
			static float fov = cmdline_args.camera.fov;
			static int cameraMode = cmdline_args.camera.mode;
			switch (key) {
			// case 'n':
			// case 'N':
			// 	cameraMode--;
			// 	if (cameraMode < 0)
			// 	{
			// 		cameraMode = 3;
			// 		this->enableFreeMode();
			// 		this->camera.forceUp = false;
			// 	}
			// 	if (cameraMode == 0)
			// 	{
			// 		this->enableFlyMode();
			// 	}
			// 	if (cameraMode == 1)
			// 	{
			// 		this->enableInspectMode(sceneBounds, 1e-3f, 1e6f);
			// 		this->camera.forceUp = true;
			// 	}
			// 	if (cameraMode == 2)
			// 	{
			// 		this->enableFPSMode();
			// 	}
			// 	break;
			// case 'm':
			// case 'M':
			// 	cameraMode++;
			// 	if (cameraMode > 3)
			// 	{
			// 		cameraMode = 0;
			// 		this->enableFlyMode();
			// 		this->camera.forceUp = true;
			// 	}
			// 	if (cameraMode == 1)
			// 	{
			// 		this->enableInspectMode(sceneBounds, 1e-3f, 1e6f);
			// 	}
			// 	if (cameraMode == 2)
			// 	{
			// 		this->enableFPSMode();
			// 		this->camera.forceUp = false;
			// 	}
			// 	if (cameraMode == 3)
			// 	{
			// 		this->enableFreeMode();
			// 	}
			// 	break;
			case '[':
			case '{':
				fov--;
				if (fov <= 0.0f) fov = 1.0f;
				setCameraOptions(fov, focal_distance);
				break;
			case ']':
			case '}':
				fov++;
				if (fov >= 180.0f) fov = 179.0f;
				setCameraOptions(fov, focal_distance);
				break;
			case '\'':
			case '\"':
				focal_distance += .1;
				setCameraOptions(fov, focal_distance);
				break;
			case ';':
			case ':':
				focal_distance -= .1;
				if (focal_distance < 0.0) focal_distance = 0.0;
				setCameraOptions(fov, focal_distance);
				break;
			case '0':
			case ')':
				lens_radius += .1;
				frameState.camera_lens_radius = lens_radius;
				frameState.accumID = 0;
				renderer.updateFrameState(frameState);
				break;
			case '9':
			case '(':
				lens_radius -= .1;
				frameState.camera_lens_radius = lens_radius;
				frameState.accumID = 0;
				renderer.updateFrameState(frameState);
				break;
			case '1':
			case '!':
				screenShot();
				break;
			case '3':
			case '#':
				bUpdateVoltage = !bUpdateVoltage;
				break;
			case 'f':
			case 'F':
				displayFPS = !displayFPS;
				break;
			case 'i':
			case 'I':
			{
				auto &fc = this->camera;
				std::cout << "(C)urrent camera:" << std::endl;
				std::cout << "- from :" << fc.position << std::endl;
				std::cout << "- poi  :" << fc.getPOI() << std::endl;
				std::cout << "- upVec:" << fc.upVector << std::endl;
				std::cout << "- frame:" << fc.frame << std::endl;
				std::cout << "- fov:" << fov << std::endl;
				std::cout.precision(10);
				std::cout << "cmdline_args: --camera "
					<< fc.position.x << " "
					<< fc.position.y << " "
					<< fc.position.z << " "
					<< fc.getPOI().x << " "
					<< fc.getPOI().y << " "
					<< fc.getPOI().z << " "
					// << fc.upVector.x << " "
					// << fc.upVector.y << " "
					// << fc.upVector.z << std::endl;
					<< fc.frame.vx.x << " "
					<< fc.frame.vx.y << " "
					<< fc.frame.vx.z << std::endl;
			} break;			

			/*case 'w':
				moveCameraFlag = true;
				break;
			case 'r':
				rollCameraFlag = true;
				break;
			case 'z':
			case 'Z':
				saveVolChangeFlag = true;
				break;
			case 'a':
			case 'A':
				volAndrollFlag = true;
				break;*/

			default:
				;
			}

		}
		
	}

	void TubesViewer::moveMouseCallback()
	{
		//glfwSetCursorPosCallback(this->handle, cursorPosition.x, cursorPosition.y);
	}

	void TubesViewer::moveMouse(const vec2i & currentPosition, const vec2i & lastPosition)
	{
		if (cameraManipulator == fpsModeManipulator) fpsModeManipulator->moveMouse(currentPosition, lastPosition);
	}

	void TubesViewer::fpsMouseMotion(const vec2i& newMousePosition)
	{
		if (cameraManipulator == fpsModeManipulator && !bWormMainView)
		{
			if (enteringFpsMode) {
				//moveMouse(newMousePosition, vec2i(0, 0));
				enteringFpsMode = false;
			}
			else
			{
				moveMouse(newMousePosition, lastMousePosition);
			}
			lastMousePosition = newMousePosition;
		}		
	}

	void TubesViewer::mouseMotion(const vec2i &newMousePosition)
	{
		if (lastMousePosition != vec2i(-1)) 
		{
			if (bWormMainView) 
			{
				int x = newMousePosition.x;
				int y = newMousePosition.y;
				int mPrevX = lastMousePosition.x;
				int mPrevY = lastMousePosition.y;

				if (leftButton.isPressed)
				{
					this->simulationWindow->mCamera->Translate(x,y,mPrevX,mPrevY);
				}
				if (centerButton.isPressed)
				{
					this->simulationWindow->mCamera->Pan(x, y, mPrevX, mPrevY);
				}
				if (rightButton.isPressed)
				{
					this->simulationWindow->mCamera->Rotate(x, y, mPrevX, mPrevY, fbSize.x, fbSize.y);
				}
			}
			else
			{				
				if (leftButton.isPressed)
				{
					mouseDragLeft(newMousePosition, newMousePosition - lastMousePosition);
				}					
				if (centerButton.isPressed)
				{
					mouseDragCenter(newMousePosition, newMousePosition - lastMousePosition);
				}
				if (rightButton.isPressed)
				{
					mouseDragRight(newMousePosition, newMousePosition - lastMousePosition);
				}
			}
		}
				
		lastMousePosition = newMousePosition;
	}

	void TubesViewer::enableFlyMode()
	{
		flyModeManipulator
			= std::make_shared<flyMode>(this, worldScale);

		cameraManipulator = flyModeManipulator;

		glfwSetInputMode(this->handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		
	}

	void TubesViewer::enableInspectMode(const box3f &validPoiRange,
		float minPoiDist,
		float maxPoiDist)
	{
		inspectModeManipulator
			= std::make_shared<inspectMode>(this, validPoiRange, minPoiDist, maxPoiDist, worldScale);

		cameraManipulator = inspectModeManipulator;

		glfwSetInputMode(this->handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		
	}

	void TubesViewer::enableFPSMode()
	{
		fpsModeManipulator
			= std::make_shared<fpsMode>(this, worldScale);

		cameraManipulator = fpsModeManipulator;

		glfwSetInputMode(this->handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		enteringFpsMode = true;

	}

	void TubesViewer::enableFreeMode()
	{
		freeModeManipulator
			= std::make_shared<freeMode>(this, worldScale);

		cameraManipulator = freeModeManipulator;

		glfwSetInputMode(this->handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	}

	GLFWwindow* TubesViewer::getWindow()
	{
		return handle;
	}

}
