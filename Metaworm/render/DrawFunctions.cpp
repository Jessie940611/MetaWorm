#include "DrawFunctions.h"
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

void GUI::DrawWorld(Eigen::Vector4d floorColor)
{
	glEnable(GL_LIGHTING); 
	glPolygonMode(GL_FRONT_AND_BACK, /* GL_LINE */GL_FILL); 
	glBegin(GL_QUADS);

	double ground_y = -5.0;
	double width_x = 200.0;
	double width_z = 200.0;
	double height_y = 400.0;

	for(int x=-width_x;x<width_x;x+=1)
	{
		for(int z=-width_z; z<width_z; z+=1)
		{
			glColor3f(floorColor[0], floorColor[1], floorColor[2]);
			glNormal3d(0,1,0);
			glVertex3f(x, ground_y, z+1.0);
			glVertex3f(x+1.0, ground_y, z+1.0);
			glVertex3f(x+1.0, ground_y, z);
			glVertex3f(x, ground_y, z);
		}
	}
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d(0,0,1);
	glVertex3f(-width_x,ground_y,-width_z);
	glVertex3f(width_x,ground_y,-width_z);
	glVertex3f(width_x,height_y,-width_z);
	glVertex3f(-width_x,height_y,-width_z);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d(0,0,-1);
	glVertex3f(-width_x,ground_y,width_z);
	glVertex3f(-width_x,height_y,width_z);
	glVertex3f(width_x,height_y,width_z);
	glVertex3f(width_x,ground_y,width_z);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d(1,0,0);
	glVertex3f(-width_x,ground_y,-width_z);
	glVertex3f(-width_x,height_y,-width_z);
	glVertex3f(-width_x,height_y,width_z);
	glVertex3f(-width_x,ground_y,width_z);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d(-1,0,0);
	glVertex3f(width_x,ground_y,-width_z);
	glVertex3f(width_x,ground_y,width_z);
	glVertex3f(width_x,height_y,width_z);
	glVertex3f(width_x,height_y,-width_z);
	glEnd();

	glBegin(GL_QUADS);
	glNormal3d(0, -1, 0);
	glVertex3f(width_x, height_y, -width_z);
	glVertex3f(-width_x, height_y, -width_z);
	glVertex3f(-width_x, height_y, width_z);
	glVertex3f(width_x, height_y, width_z);
	glEnd();

    glDisable(GL_LIGHTING);

	glDisable(GL_LIGHTING);
	glDepthFunc(GL_ALWAYS);

	glEnable(GL_BLEND);

	GLint oldMode;
    glGetIntegerv(GL_MATRIX_MODE, &oldMode);
    glMatrixMode(GL_PROJECTION);

    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2f(0, 0);

	glBegin(GL_QUADS);
	glColor3f(0.2f, 0.2f, 0.2f);
	glVertex2f(0.f, 0.f);
	glColor3f(0.2f, 0.2f, 0.2f);
	glVertex2f(1.f, 0.f);
	glColor3f(0.2f, 0.8f, 0.9f);
	glVertex2f(1.f, .4f);
	glColor3f(0.2f, 0.8f, 0.9f);
	glVertex2f(0.f, .4f);
	glEnd();

	glBegin(GL_QUADS);
	glColor3f(0.2f, 0.8f, 0.9f);
	glVertex2f(0.f, .4f);
	glColor3f(0.2f, 0.8f, 0.9f);
	glVertex2f(1.f, 0.4f);
	glColor3f(0.9f, 0.9f, 0.9f);
	glVertex2f(1.f, 1.f);
	glColor3f(0.9f, 0.9f, 0.9f);
	glVertex2f(0.f, 1.f);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(oldMode);

	glDisable(GL_BLEND);

	glDepthFunc(GL_LEQUAL);

    //glEnable(GL_LIGHTING);  
}

void GUI::DrawCharacter(Worm* pCreature, const Eigen::VectorXd& x, const Eigen::Vector3d& eye, const Eigen::Vector4d& color)
{
	glEnable(GL_LIGHTING); 

	const std::vector<Eigen::Vector3i>& contours = pCreature->GetContours();
	std::vector<std::pair<int,double>> sorted_contours;
	const Eigen::VectorXd& v_n = pCreature->GetVertexNormal();

	for(int i=0; i<contours.size(); i++)
	{
		const Eigen::Vector3i& c = contours[i];
		int idx0 = (c)[0];
		int idx1 = (c)[1];
		int idx2 = (c)[2];

		Eigen::Vector3d p0 = x.block<3,1>(3*idx0,0);
		Eigen::Vector3d p1 = x.block<3,1>(3*idx1,0);
		Eigen::Vector3d p2 = x.block<3,1>(3*idx2,0);

		Eigen::Vector3d center = 0.3333*(p0+p1+p2);

		sorted_contours.push_back(std::make_pair(i, (center-eye).norm()));
	}
	std::sort(sorted_contours.begin(), sorted_contours.end(), [](const std::pair<int, double>& A, const std::pair<int, double>& B){
		return A.second > B.second;
	});
	for(int i=0; i<sorted_contours.size(); i++)
	{
		const auto& c = contours[sorted_contours[i].first];
		int idx0 = (c)[0];
		int idx1 = (c)[1];
		int idx2 = (c)[2];

		Eigen::Vector3d p0 = x.block<3,1>(3*idx0,0);
		Eigen::Vector3d p1 = x.block<3,1>(3*idx1,0);
		Eigen::Vector3d p2 = x.block<3,1>(3*idx2,0);

		Eigen::Vector3d n0 = v_n.block<3,1>(3*idx0,0);
		Eigen::Vector3d n1 = v_n.block<3,1>(3*idx1,0);
		Eigen::Vector3d n2 = v_n.block<3,1>(3*idx2,0);

		GUI::DrawTriangle(p0,p1,p2,n0,n1,n2, color);
	}
	glDisable(GL_LIGHTING);
}

void GUI::DrawMuscles(const std::vector<Muscle*>& muscles, const Eigen::VectorXd& x)
{
	glEnable(GL_LIGHTING);

	std::vector<Eigen::Vector3d> colors =
	{
		//Eigen::Vector3d(30/256.0, 158/256.0, 185/256.0),
		//Eigen::Vector3d(254/256.0, 114/256.0, 121/256.0),
		Eigen::Vector3d(1, 0, 0),
		Eigen::Vector3d(0, 1, 0),
		Eigen::Vector3d(0, 0, 1),
		Eigen::Vector3d(1, 1, 0)
	};
	const int nc = 4;
	int count = 0; 

	for(const auto& m : muscles)
	{
		//int count = 0;
		for(const auto& seg : m->GetSegments())
		{
			Eigen::Vector4d start_bary = seg->GetStartBarycentric();
			Eigen::Vector4d end_bary = seg->GetEndBarycentric();
			Eigen::Vector4i start_idx = seg->GetStartIdx();
			Eigen::Vector4i end_idx = seg->GetEndIdx();

			double a = seg->GetActivationLevel();

			Eigen::Vector3d p_start = start_bary[0]*x.block<3,1>(3*start_idx[0],0) + start_bary[1]*x.block<3,1>(3*start_idx[1],0) 
					+ start_bary[2]*x.block<3,1>(3*start_idx[2],0) + start_bary[3]*x.block<3,1>(3*start_idx[3],0);
			Eigen::Vector3d p_end = end_bary[0]*x.block<3,1>(3*end_idx[0],0) + end_bary[1]*x.block<3,1>(3*end_idx[1],0) 
					+ end_bary[2]*x.block<3,1>(3*end_idx[2],0) + end_bary[3]*x.block<3,1>(3*end_idx[3],0);

			Eigen::Vector4d color;
			color[0] = colors[count%nc][0];
			color[1] = colors[count%nc][1];
			color[2] = colors[count%nc][2];
			color[3] = 5*a;//0.6

			GUI::DrawCylinder(p_start, p_end, 0.001, color);	
			//GUI::DrawCylinder(start_bary.head<3>(), end_bary.head<3>(), 0.009, color);
			// std::cout << count << " [" << p_start[0] << " " << p_start[1] << " " << p_start[2] << "] " 
			//           << " [" << p_end[0] << " " << p_end[1] << " " << p_end[2] << "]"  << std::endl;
			//count+=1;
		}
		count+=1;
	}

	glDisable(GL_LIGHTING);
}

void GUI::DrawActivations(const double& x, const double& y, const double& length, const double& height, const Eigen::Vector3d& color, Muscle* muscle)
{
	glDisable(GL_LIGHTING);
	glDepthFunc(GL_ALWAYS);

	GLint oldMode;
    glGetIntegerv(GL_MATRIX_MODE, &oldMode);
    glMatrixMode(GL_PROJECTION);

    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0.0, 1.0, 0.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRasterPos2f(x, y);

    auto act = muscle->GetActivationLevels();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_POLYGON);
    glColor4f(0.85, 0.85, 0.85, 0.7);
	glVertex2d(x, y);
	glVertex2d(x+length/act.size()*(act.size()-1), y);
	glVertex2d(x+length/act.size()*(act.size()-1), y+height);
	glVertex2d(x, y+height);
	glVertex2d(x, y);
	//glDisable(GL_BLEND);
    glEnd();
	glDisable(GL_BLEND);

    double base = y+0.5*height;
	// glColor3f(0.5, 0, 0);
	// glBegin(GL_LINES);
	// glVertex2d(x, base);
	// glVertex2d(x+length/act.size()*(act.size()-1), base);
	// glEnd();

	for(int i=0; i<act.size()-1; i++) 
	{
		float x1,y1;
		float x2,y2;

		x1=x+length/act.size()*i;
		x2=x+length/act.size()*(i+1);

		y1=base+act[i]*height; 
		y2=base+act[i+1]*height;

		if(y1<y)
			y1 = y;
		if(y2<y)
			y2 = y;

		glColor4f(color[0], color[1], color[2], 0.7);
		//glColor3f(0.22, 0.71, 0.63);
		glBegin(GL_POLYGON);
	    glVertex2d(x1,base);
	    glVertex2d(x1,y1);
	    glVertex2d(x2,y2);
	    glVertex2d(x2,base);
	    glVertex2d(x1,base);
	    glEnd();
	}	

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(oldMode);

	glDepthFunc(GL_LEQUAL);
    glEnable(GL_LIGHTING);  
}

// void GUI::DrawActivations(const double& x, const double& y, const double& length, const double& height, Muscle* muscle1, Muscle* muscle2)
// {
// 	glDisable(GL_LIGHTING);

// 	GLint oldMode;
//     glGetIntegerv(GL_MATRIX_MODE, &oldMode);
//     glMatrixMode(GL_PROJECTION);

//     glPushMatrix();
//     glLoadIdentity();
//     gluOrtho2D(0.0, 1.0, 0.0, 1.0);

// 	glMatrixMode(GL_MODELVIEW);
//     glPushMatrix();
//     glLoadIdentity();
//     glRasterPos2f(x, y);

//     auto act1 = muscle1->GetActivationLevels();
//     auto act2 = muscle2->GetActivationLevels();

//     glEnable(GL_BLEND);
//     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//     glBegin(GL_POLYGON);
//     glColor4f(0.8, 0.8, 0.8, 0.75);
// 	glVertex2d(x, y);
// 	glVertex2d(x+length/act1.size()*(act1.size()-1), y);
// 	glVertex2d(x+length/act1.size()*(act1.size()-1), y+height);
// 	glVertex2d(x, y+height);
// 	glVertex2d(x, y);
// 	glDisable(GL_BLEND);
//     glEnd();

//     double base = y+0.5*height;

//     // down
// 	for(int i=0; i<act1.size()-1; i++) {
// 		float x1,y1;
// 		float x2,y2;

// 		x1=x+length/act1.size()*i;
// 		x2=x+length/act1.size()*(i+1);

// 		y1=y+0.5*height-2.0*act1[i]*0.5*height; 
// 		y2=y+0.5*height-2.0*act1[i+1]*0.5*height;

// 		if(y1<y)
// 			y1 = y;
// 		if(y2<y)
// 			y2 = y;

// 		glColor3f(30/256.0, 158/256.0, 185/256.0);
// 		glBegin(GL_POLYGON);
// 	    glVertex2d(x1,base);
// 	    glVertex2d(x1,y1);
// 	    glVertex2d(x2,y2);
// 	    glVertex2d(x2,base);
// 	    glVertex2d(x1,base);
// 	    glEnd();
// 	}	
// 	// up
// 	for(int i=0; i<act2.size()-1; i++) {
// 		float x1,y1;
// 		float x2,y2;

// 		x1=x+length/act2.size()*i;
// 		x2=x+length/act2.size()*(i+1);

// 		y1=y+2.0*act2[i]*0.5*height + 0.5*height;
// 		y2=y+2.0*act2[i+1]*0.5*height + 0.5*height;

// 		if(y1<y)
// 			y1 = y;
// 		if(y2<y)
// 			y2 = y;

// 		glColor3f(254/256.0, 114/256.0, 121/256.0);
// 		glBegin(GL_POLYGON);
// 	    glVertex2d(x1,base);
// 	    glVertex2d(x2,base);
// 	    glVertex2d(x2,y2);
// 	    glVertex2d(x1,y1);
// 	    glVertex2d(x1,base);
// 	    glEnd();
// 	}	

// 	glPopMatrix();
// 	glMatrixMode(GL_PROJECTION);
//     glPopMatrix();
//     glMatrixMode(oldMode);

//     glEnable(GL_LIGHTING);    
// }

void GUI::DrawTets(Worm* pCreature, const Eigen::VectorXd& x, const Eigen::Vector4d& color)
{ 
	const std::vector<Eigen::Vector4i>& tets = pCreature->GetTets();
	Eigen::Vector3d p0, p1, p2, p3;
	for(const auto& t : tets)
	{
		p0 = x.block<3, 1>(3*t[0], 0);
		p1 = x.block<3, 1>(3*t[1], 0);
		p2 = x.block<3, 1>(3*t[2], 0);
		p3 = x.block<3, 1>(3*t[3], 0);
		GUI::DrawTetrahedron(p0, p1, p2, p3, color);
	}
}

void GUI::DrawSamplingPoints(Worm* pCreature, const Eigen::VectorXd& x, const Eigen::VectorXd& v, const Eigen::Matrix3d& r, const Eigen::Vector3d& t)
{
	const std::vector<int>& pts = pCreature->GetSamplingIndex();
	const std::vector<Eigen::Vector3d>& refs = pCreature->GetSamplingReference();
	const Eigen::Vector4d color = Eigen::Vector4d(1, 0, 1, 1);
	const Eigen::Vector3d color_vel = Eigen::Vector3d(0, 0, 1);
	for(int i=0; i< pts.size(); ++i)
	{
		GUI::DrawPoint(x.block<3, 1>(3*pts[i], 0), 8.0, color);
		GUI::DrawPoint(r*refs[i]+t, 8.0, color);
		GUI::DrawLine(r*refs[i]+t, x.block<3, 1>(3*pts[i], 0), color, 1.0);
		GUI::DrawArrow3D(x.block<3, 1>(3*pts[i], 0), v.block<3, 1>(3*pts[i], 0)*0.1, 0.001, color_vel);
	}

	// centers
	// const int& center_index = pCreature->GetCenterIndex();
	// const int& com_index = pCreature->GetCOMIndex();
	// GUI::DrawPoint(x.block<3, 1>(3*center_index, 0), 8.0, Eigen::Vector4d(0, 0, 1, 1));
	// GUI::DrawPoint(x.block<3, 1>(3*com_index, 0), 8.0, Eigen::Vector4d(0, 1, 0, 1));
}

void GUI::DrawLocalContour(Worm* pCreature, const Eigen::VectorXd& x)
{
	const Eigen::Vector3i idx = pCreature->GetLocalContourIndex();
	Eigen::Vector4d color = Eigen::Vector4d(1, 0.5, 0.2, 0.7);
	const double width = 3.0;
	Eigen::Vector3d p0 = x.block<3, 1>(3*idx[0], 0);
	Eigen::Vector3d p1 = x.block<3, 1>(3*idx[1], 0);
	Eigen::Vector3d p2 = x.block<3, 1>(3*idx[2], 0);
	GUI::DrawLine(p0, p1, color, width);
	GUI::DrawLine(p1, p2, color, width);
	GUI::DrawLine(p2, p0, color, width);
}

void GUI::DrawTrajectory(const std::deque<Eigen::Vector3d>& traj, const Eigen::Vector4d& color, const double& width)
{
	for (size_t i = 0; i < traj.size()-1; ++i)
	{
		GUI::DrawLine(traj[i], traj[i+1], color, width);
	}	
}