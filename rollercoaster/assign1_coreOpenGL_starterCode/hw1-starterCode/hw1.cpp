/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: Yangzhen Zhang
  USC ID: 7052227306
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"
#include "math.h"
#include <vector>
#include <cmath>
#include <time.h>

#include <iostream>
#include <cstring>

#if defined(WIN32) || defined(_WIN32)
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#if defined(WIN32) || defined(_WIN32)
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

#define MIN(a,b) (a<b ? a:b)
#define MAX(a,b) (a>b ? a:b)

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
typedef long clock_t;
CONTROL_STATE controlState = ROTATE;

clock_t t1 = clock();
int prev_t = 0;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

// window size
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";

int move_test = 0;

//rail VAO, VBO
GLuint railVertexBuffer, railColorVertexBuffer;
GLuint railVertexArray;
GLuint railnormalVertexBuffer;

//rail_1 VAO, VBO
GLuint railVertexBuffer_1, railColorVertexBuffer_1;
GLuint railVertexArray_1;
GLuint railnormalVertexBuffer_1;

//rail_2 VAO, VBO
GLuint railVertexBuffer_2, railColorVertexBuffer_2;
GLuint railVertexArray_2;
GLuint railnormalVertexBuffer_2;

//rail_3 VAO, VBO
GLuint railVertexBuffer_3, railColorVertexBuffer_3;
GLuint railVertexArray_3;
GLuint railnormalVertexBuffer_3;

//ground VBO, VAO
GLuint groundVertexBuffer, groundtexcoordVertexBuffer, groundtexVertexBuffer;
GLuint groundVertexArray;

//sky VAO, VBO, EBO
GLuint skyVertexBuffer, skytexcoordVertexBuffer, skytexVertexBuffer;
GLuint skyVertexArray;
GLuint skyelementBuffer;

std::vector<GLfloat> skyPoints;
std::vector<GLfloat> skyColor;
std::vector<GLfloat> skyNormal;
std::vector<GLfloat> skytexcoords;
std::vector<GLushort> skyindices;
std::vector<GLfloat> skyimage;

//mountain VAO, VBO, EBO
GLuint mountainVertexBuffer, mountaintexcoordVertexBuffer, mountaintexVertexBuffer;
GLuint mountainVertexArray;
GLuint mountainelementBuffer;

int sizeMountain;

OpenGLMatrix matrix;
BasicPipelineProgram* pipelineProgram;
BasicPipelineProgram* textureProgram;
BasicPipelineProgram* testProgram;

// represents one control point along the spline 
struct Point
{
	double x;
	double y;
	double z;
};

// spline struct 
// contains how many control points the spline has, and an array of control points 
struct Spline
{
	int numControlPoints;
	Point* points;
};
// the spline array 
Spline* splines;

// total number of splines 
int numSplines;

// segment number of the spline
int segment_num;

// interval number of each segment of the spline
int spline_interval_num = 100;

// current index for camera when going through the rollercoaster
float camera_index = 0;

// camera index moving velocity
float velocity;

int railcrosssectionSize;

float h_max = 0;
float h_min = 100;

//set the gravitational acceleration
float g = 1.633;

//the spline points
glm::vec3* splinePoints;
glm::vec3* splineTangents;
glm::vec3* splineNormals;
glm::vec3* splineBinormals;
glm::vec3* splinePoints_crosssection;
glm::vec3* splinePoints_ordered_crosssection;
glm::vec3* lightNormals;


//Math Helper Function
//return crossProduct of a & b
glm::vec3 crossProduct(glm::vec3 a, glm::vec3 b) {
	glm::vec3 p;
	p.x = a.y * b.z - a.z * b.y;
	p.y = a.z * b.x - a.x * b.z;
	p.z = a.x * b.y - a.y * b.x;
	return p;
}

//return the normalized vector a
glm::vec3 normalize(glm::vec3 a) {
	glm::vec3 n;
	float length = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	for (int i = 0; i < 3; i++) {
		n[i] = a[i] / length;
	}
	return n;
}

//return scalar a multiply vector p
glm::vec3 scaleMultiply(float a, glm::vec3 p) {
	glm::vec3 m;
	for (int i = 0; i < 3; i++) {
		m[i] = p[i] * a;
	}
	return m;
}

//return the length of vector p
float length(glm::vec3 p) {
	float l = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
	return l;
}

int loadSplines(char* argv)
{
	char* cName = (char*)malloc(128 * sizeof(char));
	FILE* fileList;
	FILE* fileSpline;
	int iType, i = 0, j, iLength;

	// load the track file 
	fileList = fopen(argv, "r");
	if (fileList == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}

	// stores the number of splines in a global variable 
	fscanf(fileList, "%d", &numSplines);

	splines = (Spline*)malloc(numSplines * sizeof(Spline));

	// reads through the spline files 
	for (j = 0; j < numSplines; j++)
	{
		i = 0;
		fscanf(fileList, "%s", cName);
		fileSpline = fopen(cName, "r");

		if (fileSpline == NULL)
		{
			printf("can't open file\n");
			exit(1);
		}

		// gets length for spline file
		fscanf(fileSpline, "%d %d", &iLength, &iType);

		// allocate memory for all the points
		splines[j].points = (Point*)malloc(iLength * sizeof(Point));
		splines[j].numControlPoints = iLength;

		// saves the data to the struct
		while (fscanf(fileSpline, "%lf %lf %lf",
			&splines[j].points[i].x,
			&splines[j].points[i].y,
			&splines[j].points[i].z) != EOF)
		{
			i++;
		}
	}

	free(cName);

	return 0;
}

int initTexture(const char* imageFilename, GLuint textureHandle)
{
	// read the texture image
	ImageIO img;
	ImageIO::fileFormatType imgFormat;
	ImageIO::errorType err = img.load(imageFilename, &imgFormat);

	if (err != ImageIO::OK)
	{
		printf("Loading texture from %s failed.\n", imageFilename);
		return -1;
	}

	// check that the number of bytes is a multiple of 4
	//if (img.getWidth() * img.getBytesPerPixel() % 4)
	//{
	//	printf("Error (%s): The width*numChannels in the loaded image must be a multiple of 4.\n", imageFilename);
	//	return -1;
	//}

	// allocate space for an array of pixels
	int width = img.getWidth();
	int height = img.getHeight();
	unsigned char* pixelsRGBA = new unsigned char[4 * width * height]; // we will use 4 bytes per pixel, i.e., RGBA

	// fill the pixelsRGBA array with the image pixels
	memset(pixelsRGBA, 0, 4 * width * height); // set all bytes to 0
	for (int h = 0; h < height; h++)
		for (int w = 0; w < width; w++)
		{
			// assign some default byte values (for the case where img.getBytesPerPixel() < 4)
			pixelsRGBA[4 * (h * width + w) + 0] = 0; // red
			pixelsRGBA[4 * (h * width + w) + 1] = 0; // green
			pixelsRGBA[4 * (h * width + w) + 2] = 0; // blue
			pixelsRGBA[4 * (h * width + w) + 3] = 255; // alpha channel; fully opaque

			// set the RGBA channels, based on the loaded image
			int numChannels = img.getBytesPerPixel();
			for (int c = 0; c < numChannels; c++) // only set as many channels as are available in the loaded image; the rest get the default value
				pixelsRGBA[4 * (h * width + w) + c] = img.getPixel(w, h, c);
		}

	// bind the texture
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	// initialize the texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);//pixelsRGBA is the texel array

	// generate the mipmaps for this texture
	glGenerateMipmap(GL_TEXTURE_2D);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// query support for anisotropic texture filtering
	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	printf("Max available anisotropic samples: %f\n", fLargest);
	// set anisotropic texture filtering
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.5f * fLargest);

	// query for any errors
	GLenum errCode = glGetError();
	if (errCode != 0)
	{
		printf("Texture initialization error. Error code: %d.\n", errCode);
		return -1;
	}

	// de-allocate the pixel array -- it is no longer needed
	delete[] pixelsRGBA;

	return 0;
}

// write a screenshot to the specified filename
void saveScreenshot(const char* filename)
{
	unsigned char* screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete[] screenshotData;
}

// specify the camera position at current index(u)
void setCamera(int index, float h) {
	glm::vec3 eye = splinePoints[index] + scaleMultiply(h, splineNormals[index]);
	glm::vec3 center = eye + splineTangents[index];
	glm::vec3 normal = splineNormals[index];
	matrix.LookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, normal.x, normal.y, normal.z);
}

void displayFunc()
{
	// render some stuff...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.LoadIdentity();

	setCamera((int)camera_index, 0.05);

	/*
	//ModelView
	matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
	matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
	matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
	matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
	matrix.Scale(landScale[0], landScale[1], landScale[2]);
	*/
	
	//get Modelview matrix
	float m[16];
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.GetMatrix(m);

	//get normalmatrix
	float n[16];
	matrix.GetNormalMatrix(n);

	//get Projection matrix
	float p[16];
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.GetMatrix(p);

	if (glGetError() != 0) cout << "matrix error" << endl;

	//Use BasicProgram--------------------------------------------------------------------------------------------------------
	pipelineProgram->Bind();
	
	//set the phong shading parameters
	float La[4] = { 0.5f, 0.5f, 0.5f };
	float Ld[4] = { 0.5f, 0.5f, 0.5f };
	float Ls[4] = { 0.5f, 0.5f, 0.5f };
	float ka[4] = { 0.135, 0.2225, 0.1575 };
	float kd[4] = { 0.54, 0.89, 0.63 };
	float ks[4] = { 0.316228, 0.316228, 0.316228 };
	float alpha = 0.1;
	float LightDirection[3] = { 0, 1, 1 };

	// Specify the value of uniform variable for the pipelineProgram
	pipelineProgram->SetModelViewMatrix(m);
	pipelineProgram->SetProjectionMatrix(p);
	glUniformMatrix4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "normalMatrix"), 1, GL_FALSE, n);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "La"), 1, La);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "Ld"), 1, Ld);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "Ls"), 1, Ls);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ka"), 1, ka);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "kd"), 1, kd);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ks"), 1, ks);
	glUniform3fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "LightDirection"), 1, LightDirection);
	glUniform1f(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "alpha"), alpha);

	//render double T-shaped rail by combing four rectangle crosssection rail  
	glBindVertexArray(railVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, railcrosssectionSize);

	glBindVertexArray(railVertexArray_1);
	glDrawArrays(GL_TRIANGLES, 0, railcrosssectionSize);

	//glBindVertexArray(railVertexArray_2);
	//glDrawArrays(GL_TRIANGLES, 0, railcrosssectionSize);

	//glBindVertexArray(railVertexArray_3);
	//glDrawArrays(GL_TRIANGLES, 0, railcrosssectionSize);

	if (glGetError() != 0) cout << "basic pipeline program setting error" << endl;

	//Use Texture Program-------------------------------------------------------------------------------------------------------
	textureProgram->Bind();

	//specify the value of uniform variable for the textureProgram
	textureProgram->SetModelViewMatrix(m);
	textureProgram->SetProjectionMatrix(p);

	//render ground
   	glBindTexture(GL_TEXTURE_2D, groundtexVertexBuffer);
	glBindVertexArray(groundVertexArray);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	//render sky
	glBindTexture(GL_TEXTURE_2D, skytexVertexBuffer);
	glBindVertexArray(skyVertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyelementBuffer);
	glDrawElements(GL_TRIANGLE_STRIP, skyindices.size(), GL_UNSIGNED_SHORT, 0);

	//render mountain
	glBindTexture(GL_TEXTURE_2D, mountaintexVertexBuffer);
	glBindVertexArray(mountainVertexArray);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mountainelementBuffer);
	glDrawElements(GL_TRIANGLES, sizeMountain, GL_UNSIGNED_INT, 0);

	if (glGetError() != 0) cout << "texture pipeline program setting error" << endl;
	
	glutSwapBuffers();
}

void idleFunc()
{
	// for example, here, you can save the screenshots to disk (to make the animation)

	//set velocity------------------------------------------------------------------------------------------------------
	// record the current height
	float h = splinePoints[(int)camera_index].y;
    
	// calculate the time step
	float delta_t = (float)(clock() - t1) / CLOCKS_PER_SEC;
	t1 = clock();

	// calculate the velocity acoording to the equation: 
	// u_new - u_current = delta_t * sqrt(2 * g * (h_max - h))/||dp/du||
	// set a initial velocity as 0.01, otherwise the train will stop at the highest point (when h = h_max, velocity == 0)
	velocity = 0.01 + delta_t * sqrt(2 * g * (h_max - h)) / (length(splineTangents[(int)camera_index]) / spline_interval_num);
	camera_index = camera_index + velocity;

	//when moving to the end of the rollercoaster, start from the beginning again
	if (camera_index >= (segment_num * spline_interval_num+1)) camera_index = camera_index - 1.0f - float(segment_num * spline_interval_num);

	//-5 10  .2 
	//0  10  .3
	//.1  10  .4 

	//-7 5 .6 
	//-10 6 2

	// make the screen update 
	glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
	glViewport(0, 0, w, h);

	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.LoadIdentity();
	matrix.Perspective(54.0f, (float)w / (float)h, 0.01f, 100.0f);
}

void mouseMotionDragFunc(int x, int y)
{
	// mouse has moved and one of the mouse buttons is pressed (dragging)

	// the change in mouse position since the last invocation of this function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	switch (controlState)
	{
		// translate the landscape
	case TRANSLATE:
		if (leftMouseButton)
		{
			// control x,y translation via the left mouse button
			landTranslate[0] += mousePosDelta[0] * 0.01f;
			landTranslate[1] -= mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control z translation via the middle mouse button
			landTranslate[2] += mousePosDelta[1] * 0.01f;
		}
		break;

		// rotate the landscape
	case ROTATE:
		if (leftMouseButton)
		{
			// control x,y rotation via the left mouse button
			landRotate[0] += mousePosDelta[1];
			landRotate[1] += mousePosDelta[0];
		}
		if (middleMouseButton)
		{
			// control z rotation via the middle mouse button
			landRotate[2] += mousePosDelta[1];
		}
		break;

		// scale the landscape
	case SCALE:
		if (leftMouseButton)
		{
			// control x,y scaling via the left mouse button
			landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
			landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control z scaling via the middle mouse button
			landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
	// mouse has moved
	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
	// a mouse button has has been pressed or depressed

	// keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		leftMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_MIDDLE_BUTTON:
		middleMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_RIGHT_BUTTON:
		rightMouseButton = (state == GLUT_DOWN);
		break;
	}

	// keep track of whether CTRL and SHIFT keys are pressed
	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		controlState = TRANSLATE;
		break;

	case GLUT_ACTIVE_SHIFT:
		controlState = SCALE;
		break;

		// if CTRL and SHIFT are not pressed, we are in rotate mode
	default:
		controlState = ROTATE;
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // ESC key
		exit(0); // exit the program
		break;

	case ' ':
		cout << "You pressed the spacebar." << endl;
		break;

	case 'x':
		// take a screenshot
		saveScreenshot("screenshot.jpg");
		break;

	case 'w':
		camera_index = camera_index + 5;
		//cout << camera_index << endl;
		//cout << splinePoints[(int)camera_index].x << " " << splinePoints[(int)camera_index].y << " " << splinePoints[(int)camera_index].z << " " << camera_index << endl;
	    break;

	case 's':
		camera_index = MAX(0, camera_index - 1);
		break;

	case 'm':
		move_test++;
		break;

	case 'n':
		move_test--;
		break;
    }
}

//given 4 controll points p1, p2, p3, p4 & parameter u, return the corresponding point value on catmull-roll
glm::vec3 catmullRoll_point(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, float u) {
	glm::vec3 point;
	for (int i = 0; i < 3; i++) {
		point[i] = (u * u * u * (-0.5 * p1[i] + 1.5 * p2[i] - 1.5 * p3[i] + 0.5 * p4[i])
			+ u * u * (1 * p1[i] - 2.5 * p2[i] + 2 * p3[i] - 0.5 * p4[i])
			+ u * (-0.5 * p1[i] + 0.5 * p3[i])
			+ (1 * p2[i]));

		//another representation of catmullroll spline
		/*point[i] = p1[i] * (-0.5 * u * u * u + u * u - 0.5 * u) +
			p2[i] * (1.5 * u * u * u - 2.5 * u * u + 1.0) +
			p3[i] * (-1.5 * u * u * u + 2.0 * u * u + 0.5 * u) +
			p4[i] * (0.5 * u * u * u - 0.5 * u * u);*/
	}
	return point;
}

// given 4 controll points p1, p2, p3, p4 & parameter u, return the tangent vector at u
glm::vec3 catmullRoll_tangent(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, float u) {
	glm::vec3 tang;
	for (int i = 0; i < 3; i++) {
		tang[i] = (3 * u * u * (-0.5 * p1[i] + 1.5 * p2[i] - 1.5 * p3[i] + 0.5 * p4[i])
			+ (2 * u * (1 * p1[i] - 2.5 * p2[i] + 2 * p3[i] - 0.5 * p4[i]))
			+ (-0.5 * p1[i] + 0.5 * p3[i]));
	}
	return tang;
}

//genrate a spline, including points, tangents, normals, and binormals from splines[spline_index] to splinePoints, and also rail cross section points
void generateSpline(int spline_index) {

	segment_num = splines[spline_index].numControlPoints - 3;
	int segment_num_3 = splines[spline_index].numControlPoints - 3;

	splinePoints = new glm::vec3[spline_interval_num * segment_num];
	splineTangents = new glm::vec3[spline_interval_num * segment_num];
	splineNormals = new glm::vec3[spline_interval_num * segment_num];
	splineBinormals = new glm::vec3[spline_interval_num * segment_num];

	splinePoints_crosssection = new glm::vec3[4 * spline_interval_num * segment_num];

	//record the index for spline arrays (splinePoints[], splineTangents[], splineNormals[], splineBiNormals[])
	int index = 0;

	//for each n, draw a segment of curve (control points from spline->points[n] to spline->points[n+3])
	for (int n = 0; n < segment_num; n++) {
		//record the four control points
		glm::vec3* controlpoint = new glm::vec3[4];

		if (n < segment_num_3) {
			for (int i = 0; i < 4; i++) {
				controlpoint[i] = glm::vec3(splines[spline_index].points[n + i].x, splines[spline_index].points[n + i].y, splines[spline_index].points[n + i].z);
			}
		}
		else {
			for (int i = 0; i < segment_num - n; i++) {
				controlpoint[i] = glm::vec3(splines[spline_index].points[n + i].x, splines[spline_index].points[n + i].y, splines[spline_index].points[n + i].z);
			}
			for (int i = segment_num - n; i < 4; i++) {
				controlpoint[i] = glm::vec3(splines[spline_index].points[i - (segment_num - n)].x, splines[spline_index].points[i - (segment_num - n)].y, splines[spline_index].points[i - (segment_num - n)].z);
			}
		}

		//brute force method for vary the parameter u
		for (int j = 0; j < spline_interval_num; j++) {
			float u = float(j) / spline_interval_num;
			splinePoints[index] = catmullRoll_point(controlpoint[0], controlpoint[1], controlpoint[2], controlpoint[3], u);
			splineTangents[index] = catmullRoll_tangent(controlpoint[0], controlpoint[1], controlpoint[2], controlpoint[3], u);
			//cout << index << " " << splinePoints[index].x << " " << splinePoints[index].y << " " << splinePoints[index].z << endl;

			h_max = MAX(h_max, splinePoints[index].y);
			h_min = MIN(h_min, splinePoints[index].y);

			//calculate splineNormals and spline Binormals
			if (index == 0) {
				splineNormals[index] = normalize(crossProduct(splineTangents[index], glm::vec3(0, 0, -1)));
				splineBinormals[index] = normalize(crossProduct(splineTangents[index], splineNormals[index]));
			}
			else {
				splineNormals[index] = normalize(crossProduct(splineBinormals[index - 1], splineTangents[index]));
				splineBinormals[index] = normalize(crossProduct(splineTangents[index], splineNormals[index]));
			}
			index++;
		}
	}
}

//Create a vertex buffer object
void createVBO(GLuint& vboname, GLsizeiptr size, const void* data) {
	glGenBuffers(1, &vboname);
	glBindBuffer(GL_ARRAY_BUFFER, vboname);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

//Create a vertex array object
void createVAO(GLuint& arrayname) {
	glGenVertexArrays(1, &arrayname);
	glBindVertexArray(arrayname);
}

//Create an element buffer object
void createEBO(GLuint& eboname, GLsizeiptr size, const void* data) {
	glGenBuffers(1, &eboname);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboname);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
}

//store vbo into vertex attrib array in a pipelineprogram
void SetVertexAttrib(GLuint& vboname, BasicPipelineProgram* pipelineprogram, const GLchar* nameinShader, GLuint size) {
	glBindBuffer(GL_ARRAY_BUFFER, vboname);
	GLuint loc =
		glGetAttribLocation(pipelineprogram->GetProgramHandle(), nameinShader);
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, size, GL_FLOAT, GL_FALSE, 0, (const void*)0);
}

//Generate a rail whose crosssection is a rectangle, store data in railVertexBuffer & railnormalVertexBuffer
//The rectangle has a width scale of w, a height scale of h.
//Center of the rectangle is away from the spline point by a * UnitBinorml in horizon, and b * UnitNormal in vertical
void generateSingleRail(float w, float h, float a, float b, GLuint& VertexBuffer, GLuint& normalVertexBuffer) {

	//calculate the four points for the rectangle crosssection
	for (int i = 0; i < segment_num * spline_interval_num; i++) {
		splinePoints_crosssection[i * 4] = splinePoints[i] + scaleMultiply(h, splineNormals[i]) + scaleMultiply(w, splineBinormals[i]) + scaleMultiply(a, splineBinormals[i]) + scaleMultiply(b, splineNormals[i]);
		splinePoints_crosssection[i * 4 + 1] = splinePoints[i] + scaleMultiply(-h, splineNormals[i]) + scaleMultiply(w, splineBinormals[i]) + scaleMultiply(a, splineBinormals[i]) + scaleMultiply(b, splineNormals[i]);
		splinePoints_crosssection[i * 4 + 2] = splinePoints[i] + scaleMultiply(h, splineNormals[i]) + scaleMultiply(-w, splineBinormals[i]) + scaleMultiply(a, splineBinormals[i]) + scaleMultiply(b, splineNormals[i]);
		splinePoints_crosssection[i * 4 + 3] = splinePoints[i] + scaleMultiply(-h, splineNormals[i]) + scaleMultiply(-w, splineBinormals[i]) + scaleMultiply(a, splineBinormals[i]) + scaleMultiply(b, splineNormals[i]);
	}

	//reorganize the crosssection points to prepare for draw by OpenGL using GL_TRIANGLES
	railcrosssectionSize = (spline_interval_num * segment_num - 1) * 24;
	splinePoints_ordered_crosssection = new glm::vec3[railcrosssectionSize];

	for (int i = 0; i < spline_interval_num * segment_num - 1; i++) {
		//if (i != spline_interval_num * segment_num - 1) {
			splinePoints_ordered_crosssection[i * 24] = splinePoints_crosssection[i * 4 + 2];
			splinePoints_ordered_crosssection[i * 24 + 1] = splinePoints_crosssection[(i + 1) * 4 + 2];
			splinePoints_ordered_crosssection[i * 24 + 2] = splinePoints_crosssection[i * 4 + 3];

			splinePoints_ordered_crosssection[i * 24 + 3] = splinePoints_crosssection[(i + 1) * 4 + 2];
			splinePoints_ordered_crosssection[i * 24 + 4] = splinePoints_crosssection[i * 4 + 3];
			splinePoints_ordered_crosssection[i * 24 + 5] = splinePoints_crosssection[(i + 1) * 4 + 3];

			splinePoints_ordered_crosssection[i * 24 + 6] = splinePoints_crosssection[i * 4 + 3];
			splinePoints_ordered_crosssection[i * 24 + 7] = splinePoints_crosssection[(i + 1) * 4 + 3];
			splinePoints_ordered_crosssection[i * 24 + 8] = splinePoints_crosssection[i * 4 + 1];

			splinePoints_ordered_crosssection[i * 24 + 9] = splinePoints_crosssection[(i + 1) * 4 + 3];
			splinePoints_ordered_crosssection[i * 24 + 10] = splinePoints_crosssection[i * 4 + 1];
			splinePoints_ordered_crosssection[i * 24 + 11] = splinePoints_crosssection[(i + 1) * 4 + 1];

			splinePoints_ordered_crosssection[i * 24 + 12] = splinePoints_crosssection[i * 4 + 1];
			splinePoints_ordered_crosssection[i * 24 + 13] = splinePoints_crosssection[(i + 1) * 4 + 1];
			splinePoints_ordered_crosssection[i * 24 + 14] = splinePoints_crosssection[i * 4];

			splinePoints_ordered_crosssection[i * 24 + 15] = splinePoints_crosssection[(i + 1) * 4 + 1];
			splinePoints_ordered_crosssection[i * 24 + 16] = splinePoints_crosssection[i * 4];
			splinePoints_ordered_crosssection[i * 24 + 17] = splinePoints_crosssection[(i + 1) * 4];

			splinePoints_ordered_crosssection[i * 24 + 18] = splinePoints_crosssection[i * 4];
			splinePoints_ordered_crosssection[i * 24 + 19] = splinePoints_crosssection[(i + 1) * 4];
			splinePoints_ordered_crosssection[i * 24 + 20] = splinePoints_crosssection[i * 4 + 2];

			splinePoints_ordered_crosssection[i * 24 + 21] = splinePoints_crosssection[(i + 1) * 4];
			splinePoints_ordered_crosssection[i * 24 + 22] = splinePoints_crosssection[i * 4 + 2];
			splinePoints_ordered_crosssection[i * 24 + 23] = splinePoints_crosssection[(i + 1) * 4 + 2];
		//}
        /*
		else {
			splinePoints_ordered_crosssection[i * 24] = splinePoints_crosssection[i * 4 + 2];
			splinePoints_ordered_crosssection[i * 24 + 1] = splinePoints_crosssection[2];
			splinePoints_ordered_crosssection[i * 24 + 2] = splinePoints_crosssection[i * 4 + 3];

			splinePoints_ordered_crosssection[i * 24 + 3] = splinePoints_crosssection[2];
			splinePoints_ordered_crosssection[i * 24 + 4] = splinePoints_crosssection[i * 4 + 3];
			splinePoints_ordered_crosssection[i * 24 + 5] = splinePoints_crosssection[3];

			splinePoints_ordered_crosssection[i * 24 + 6] = splinePoints_crosssection[i * 4 + 3];
			splinePoints_ordered_crosssection[i * 24 + 7] = splinePoints_crosssection[3];
			splinePoints_ordered_crosssection[i * 24 + 8] = splinePoints_crosssection[i * 4 + 1];

			splinePoints_ordered_crosssection[i * 24 + 9] = splinePoints_crosssection[3];
			splinePoints_ordered_crosssection[i * 24 + 10] = splinePoints_crosssection[i * 4 + 1];
			splinePoints_ordered_crosssection[i * 24 + 11] = splinePoints_crosssection[1];

			splinePoints_ordered_crosssection[i * 24 + 12] = splinePoints_crosssection[i * 4 + 1];
			splinePoints_ordered_crosssection[i * 24 + 13] = splinePoints_crosssection[1];
			splinePoints_ordered_crosssection[i * 24 + 14] = splinePoints_crosssection[i * 4];

			splinePoints_ordered_crosssection[i * 24 + 15] = splinePoints_crosssection[1];
			splinePoints_ordered_crosssection[i * 24 + 16] = splinePoints_crosssection[i * 4];
			splinePoints_ordered_crosssection[i * 24 + 17] = splinePoints_crosssection[4];

			splinePoints_ordered_crosssection[i * 24 + 18] = splinePoints_crosssection[i * 4];
			splinePoints_ordered_crosssection[i * 24 + 19] = splinePoints_crosssection[4];
			splinePoints_ordered_crosssection[i * 24 + 20] = splinePoints_crosssection[i * 4 + 2];

			splinePoints_ordered_crosssection[i * 24 + 21] = splinePoints_crosssection[4];
			splinePoints_ordered_crosssection[i * 24 + 22] = splinePoints_crosssection[i * 4 + 2];
			splinePoints_ordered_crosssection[i * 24 + 23] = splinePoints_crosssection[2];
		}
		*/
	}

	//calculate normal for each rail cross section point
	lightNormals = new glm::vec3[railcrosssectionSize];
	for (int i = 0; i < spline_interval_num * segment_num - 1; i++) {
		for (int j = 0; j < 6; j++) {
			lightNormals[i * 24 + j] = scaleMultiply(-1, splineBinormals[i]);
			lightNormals[i * 24 + j + 6] = scaleMultiply(-1, splineNormals[i]);
			lightNormals[i * 24 + j + 12] = scaleMultiply(1, splineBinormals[i]);
			lightNormals[i * 24 + j + 18] = scaleMultiply(1, splineNormals[i]);
		}
	}

	//store the data in corresponding VBOs
	createVBO(VertexBuffer, sizeof(glm::vec3) * railcrosssectionSize, splinePoints_ordered_crosssection);
	createVBO(normalVertexBuffer, sizeof(glm::vec3) * railcrosssectionSize, lightNormals);
}


//generate a half of a sphere to act as the sky
void generateSphere(glm::vec3 center, float radius, unsigned int rings, unsigned int sectors)
{
	float R = 1. / (float)(rings - 1);
	float S = 1. / (float)(sectors - 1);

	sectors = sectors / 2;

	skyPoints.resize(rings * sectors * 3);
	skytexcoords.resize(rings * sectors * 2);
	skyColor.resize(rings * sectors * 4);
	skyNormal.resize(rings * sectors * 3);

	std::vector<GLfloat>::iterator v = skyPoints.begin();
	std::vector<GLfloat>::iterator t = skytexcoords.begin();
	std::vector<GLfloat>::iterator c = skyColor.begin();
	std::vector<GLfloat>::iterator n = skyNormal.begin();

	for (int r = 0; r < rings; r++) {
		for (int s = 0; s < sectors; s++) {
			float x = sin(3.1415926 * r * R) * cos(2 * 3.1415926 * s * S);
			float y = sin(3.1415926 * r * R) * sin(2 * 3.1415926 * s * S);
			float z = cos(3.1415926 * r * R); 

			*t++ = s * S * 2;
			*t++ = r * R;

			*v++ = x * radius + center.x;
			*v++ = y * radius + center.y;
			*v++ = z * radius + center.z;

			*c++ = 0;
			*c++ = 0;
			*c++ = 1;
			*c++ = 1;

			*n++ = x;
			*n++ = y;
			*n++ = z;
		}
	}

	//set the indices of sky vertices
	skyindices.resize(rings * sectors * 2);
	std::vector<GLushort>::iterator i = skyindices.begin();
	for (int r = 0; r < rings; r++) {
		for (int s = 0; s < sectors; s++) {
			*i++ = r * sectors + s;
			*i++ = (r + 1) * sectors + s;
		}
	}
}

void initScene(int argc, char* argv[])
{
	// load the image from a jpeg disk file to main memory
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	//basic shading program----------------------------------------------------------------------------------------------------------------
	pipelineProgram = new BasicPipelineProgram;
	int ret = pipelineProgram->Init(shaderBasePath, "basic.vertexShader.glsl", "basic.fragmentShader.glsl");
	if (ret != 0) abort();

	//generate data for splines[0] (including splinePoints[], splineTangents[], splineNormals[], splineBinormals[])
	generateSpline(rand() % numSplines);

	//To render a T-shaped double rail, I render 4 rails of rectangle crosssection.
	generateSingleRail(0.01, 0.01, 0.05, 0, railVertexBuffer, railnormalVertexBuffer);
	createVAO(railVertexArray);
	SetVertexAttrib(railVertexBuffer, pipelineProgram, "position", 3);
	SetVertexAttrib(railnormalVertexBuffer, pipelineProgram, "normal", 3);

	generateSingleRail(0.01, 0.01, -0.05, 0, railVertexBuffer_1, railnormalVertexBuffer_1); 
	createVAO(railVertexArray_1);
	SetVertexAttrib(railVertexBuffer_1, pipelineProgram, "position", 3);
	SetVertexAttrib(railnormalVertexBuffer_1, pipelineProgram, "normal", 3);

	generateSingleRail(0.014, 0.001, -0.05, 0.01, railVertexBuffer_2, railnormalVertexBuffer_2);
	createVAO(railVertexArray_2);
	SetVertexAttrib(railVertexBuffer_2, pipelineProgram, "position", 3);
	SetVertexAttrib(railnormalVertexBuffer_2, pipelineProgram, "normal", 3);

	generateSingleRail(0.014, 0.001, 0.05, 0.01, railVertexBuffer_3, railnormalVertexBuffer_3);
	createVAO(railVertexArray_3);
	SetVertexAttrib(railVertexBuffer_3, pipelineProgram, "position", 3);
	SetVertexAttrib(railnormalVertexBuffer_3, pipelineProgram, "normal", 3);

	glEnable(GL_DEPTH_TEST);

	//texture shading program------------------------------------------------------------------------------------------------------------------
	textureProgram = new BasicPipelineProgram;
	ret = textureProgram->Init(shaderBasePath, "texture.vertexShader.glsl", "texture.fragmentShader.glsl");
	if (ret != 0) abort();

	//ground------------------------------------
	glm::vec3 ground[4] = {
		glm::vec3(200, 0, -200),
		glm::vec3(200, 0, 200),
		glm::vec3(-200, 0, -200),
		glm::vec3(-200, 0, 200),
	};

	glm::vec2 groundtexCoords[4] = {
		glm::vec2(0, 1),
		glm::vec2(0, 0),
		glm::vec2(1, 1),
		glm::vec2(1, 0)
	};

	createVBO(groundVertexBuffer, sizeof(glm::vec3) * 4, ground);
	createVBO(groundtexcoordVertexBuffer, sizeof(glm::vec2) * 4, groundtexCoords);

	createVAO(groundVertexArray);

	SetVertexAttrib(groundVertexBuffer, textureProgram, "position", 3);
	SetVertexAttrib(groundtexcoordVertexBuffer, textureProgram, "texCoords", 2);

	glGenBuffers(1, &groundtexVertexBuffer);
	initTexture("Water_1.jpg", groundtexVertexBuffer);

	std::cout << "GL error: " << glGetError() << std::endl;

	//sky----------------------------------------
	//generate a half sphere whose center at (0, -15, 0), radius of 80, 80 rings & 80 sectors per ring
	generateSphere(glm::vec3(0, -15, 0), 80, 80, 80);

	createVBO(skyVertexBuffer, sizeof(float) * skyPoints.size(), skyPoints.data());
	createVBO(skytexcoordVertexBuffer, sizeof(float) * skytexcoords.size(), skytexcoords.data());

	createVAO(skyVertexArray);

	createEBO(skyelementBuffer, sizeof(GLushort) * skyindices.size(), skyindices.data());

	SetVertexAttrib(skyVertexBuffer, textureProgram, "position", 3);
	SetVertexAttrib(skytexcoordVertexBuffer, textureProgram, "texCoords", 2);

	glGenBuffers(1, &skytexVertexBuffer);
	initTexture("colorful.jpg", skytexVertexBuffer);

	//mountain---------------------------------------
	//read heightmap.jpg from file
	ImageIO* heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG("heightmap/GrandTeton-128.jpg") != ImageIO::OK)
	{
		cout << "Error reading heightmap image." << endl;
		exit(EXIT_FAILURE);
	}
	int map_height = heightmapImage->getHeight();
	int map_width = heightmapImage->getWidth();

  	int sizeHeightmap = map_height * map_width;
	sizeMountain = (map_width - 1) * (map_height - 1) * 6;

	glm::vec3* points = new glm::vec3[map_height * map_width];
	glm::vec2* mountaintexCoords = new glm::vec2[map_height * map_width];

	for (int i = 0; i < map_height; i++) {
		for (int j = 0; j < map_width; j++) {
			points[i * map_width + j] = glm::vec3((float)(i - map_height / 2) / map_height * 450, (float)(int)(heightmapImage->getPixels()[i * map_width + j])/2 - 60, (float)(j - map_width / 2) / map_width * 450);
			mountaintexCoords[i * map_width + j] = glm::vec2((float)i/(map_height-1) * 15, (float)j/(map_width-1) * 15);
		}
	}

	int* tri_indices = new int[sizeMountain];
	for (int i = 0; i < map_height - 1; i++) {
		for (int j = 0; j < map_width - 1; j++) {
			tri_indices[6 * i * (map_width - 1) + 6 * j] = tri_indices[6 * i * (map_width - 1) + 6 * j + 3] = i * map_width + j;
			tri_indices[6 * i * (map_width - 1) + 6 * j + 1] = i * map_width + j + 1;
			tri_indices[6 * i * (map_width - 1) + 6 * j + 2] = tri_indices[6 * i * (map_width - 1) + 6 * j + 4] = (i + 1) * map_width + (j + 1);
			tri_indices[6 * i * (map_width - 1) + 6 * j + 5] = (i + 1) * map_width + j;
		}
	}

	createVBO(mountainVertexBuffer, sizeof(glm::vec3) * sizeHeightmap, points);
	createVBO(mountaintexcoordVertexBuffer, sizeof(glm::vec2) * sizeHeightmap, mountaintexCoords);

	createVAO(mountainVertexArray);
	
	createEBO(mountainelementBuffer, sizeof(int) * sizeMountain, tri_indices);

	SetVertexAttrib(mountainVertexBuffer, textureProgram, "position", 3);
	SetVertexAttrib(mountaintexcoordVertexBuffer, textureProgram, "texCoords", 2);

	glGenBuffers(1, &mountaintexVertexBuffer);
	initTexture("mountain_texture.jpg", mountaintexVertexBuffer);
}

void cleanUp() {
	glDeleteVertexArrays(1, &railVertexArray);
	glDeleteBuffers(1, &railVertexBuffer);
	glDeleteBuffers(1, &railColorVertexBuffer);
}

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("usage: %s <trackfile>\n", argv[0]);
		exit(0);
	}

	// load the splines from the provided filename
	loadSplines(argv[1]);

	printf("Loaded %d spline(s).\n", numSplines);
	for (int i = 0; i < numSplines; i++)
		printf("Num control points in spline %d: %d.\n", i, splines[i].numControlPoints);

	// Initialize random number generator
	srand((int)time(0));

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc, argv);

	cout << "Initializing OpenGL..." << endl;

#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

#ifdef __APPLE__
	// This is needed on recent Mac OS X versions to correctly display the window.
	glutReshapeWindow(windowWidth - 1, windowHeight - 1);
#endif

	// tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);

	// init glew
#ifdef __APPLE__
  // nothing is needed on Apple
#else
  // Windows, Linux
	GLint result = glewInit();
	if (result != GLEW_OK)
	{
		cout << "error: " << glewGetErrorString(result) << endl;
		exit(EXIT_FAILURE);
	}
#endif

	// do initialization
	initScene(argc, argv);

	// sink forever into the glut loop
	glutMainLoop();

	cleanUp();

	system("pause");
}