#include "SimulationWindow.h"

#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

namespace p = boost::python;
namespace np = boost::python::numpy;

int main(int argc,char** argv)
{
	Py_Initialize();
	wchar_t** _argv = (wchar_t**)PyMem_Malloc(sizeof(wchar_t*)*argc);
	for (int i=0; i<argc; i++) {
		wchar_t* arg = Py_DecodeLocale(argv[i], NULL);
		_argv[i] = arg;
	}

	PySys_SetArgv(argc,_argv);
	np::initialize();

	if( argc < 2 ) 
	{
		SimulationWindow* simwindow = new SimulationWindow();
		glutInit(&argc, argv);
		simwindow->InitWindow(1920, 1080, "MetaWorm - Default");
		glutMainLoop();
	} 
	else 
	{
		std::string para = std::string(argv[1]);
		SimulationWindow* simwindow = new SimulationWindow(para);
		glutInit(&argc, argv);
		if(para==std::string("c302"))
			simwindow->InitWindow(1920, 1080, "MetaWorm -C302");
		else
			simwindow->InitWindow(1920, 1080, "MetaWorm -ANN");
		glutMainLoop();
	}
}
