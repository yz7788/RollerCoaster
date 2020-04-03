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

#define MAX(a,b) (a>b ? a:b)

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// specify the time
typedef long clock_t;
clock_t t1 = clock();

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

// window size
int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework II";

// represents one control point along the spline 
struct Point
{
	float x;
	float y;
	float z;

	// default constructor
	Point() { x = y = z = 0.0f; }

	// customized constructor
	Point(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}

	Point& operator =(const Point& a)
	{
		x = a.x;
		y = a.y;
		z = a.z;
		return *this;
	}

	Point operator+(const Point& a) const
	{
		return Point(a.x + x, a.y + y, a.z + z);
	}

	Point operator-(const Point& a) const
	{
		return Point(x - a.x, y - a.y, z - a.z);
	}

	Point operator*(const Point& a) const
	{
		return Point(a.x * x, a.y * y, a.z * z);
	}

	Point operator*(const float& a) const
	{
		return Point(a * x, a * y, a * z);
	}
};

//spline data
vector<Point> splinePoints;
vector<Point> splineTangents;
vector<Point> splineNormals;
vector<Point> splineBinormals;
vector<Point> splinePointsCrosssection;
vector<Point> splinePointsOrderedCrosssection;
vector<Point> lightNormals;
vector<Point> railBinormals;
vector<Point> railNormals;
vector<float> parameterU;

//rail VAO, VBO
GLuint railVertexBuffer, raillightnormalVertexBuffer;
GLuint railVertexArray;

// scene data
vector<Point> backgroundPoints;
vector<glm::vec2> backgroundTexCoords;
vector<int> backgroundIndices;

//background VBO, VAO
GLuint backgroundVertexBuffer, backgroundTexCoordVertexBuffer, backgroundtexVertexBuffer;
GLuint backgroundVertexArray;
GLuint backgroundElementBuffer;

GLuint backgroundVertexArray_1;
GLuint backgroundtexVertexBuffer_1;

//matrix
OpenGLMatrix matrix;

//pipeline program
BasicPipelineProgram* pipelineProgram;
BasicPipelineProgram* textureProgram;

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
int numSegment;

// current index for camera when going through the rollercoaster
float currentCamera = 0;

// camera index moving velocity
float velocity;

// maximum and minimum height of roller coaster
float hMax = 0;

//set the gravitational acceleration
float g = 0.75f;

//specify which spline to choose from the file
int nameSpline;

// key board control
bool pause = FALSE;
bool screenshot = FALSE;
int screenshotName = 0;
char* screenshotFileName = new char[100];

int mode = 1;

//Math Helper Function-----------------------------------------------------------------------------------------------------------------
//return crossProduct of a & b
Point crossProduct(Point a, Point b) {
	Point p;
	p.x = a.y * b.z - a.z * b.y;
	p.y = a.z * b.x - a.x * b.z;
	p.z = a.x * b.y - a.y * b.x;
	return p;
}

//return the normalized vector a
Point normalize(Point a) {
	Point n;
	float length = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	n.x = a.x / length;
	n.y = a.y / length;
	n.z = a.z / length;
	return n;
}

//return the length of vector p
float length(Point p) {
	float l = sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
	return l;
}
//------------------------------------------------------------------------------------------------------------------------------------------------

int loadSplines(char* argv) {
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
		while (fscanf(fileSpline, "%f %f %f",
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

int initTexture(const char* imageFilename, GLuint textureHandle) {
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
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
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
void saveScreenshot(const char* filename) {
	unsigned char* screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete[] screenshotData;
}

// specify the camera position and camera view at current u
void setCamera(int u, float h) {
	// consider the camera view in a new frame: Tangents, Normal, Binormal as 3 basis
	Point frame;
	if (landRotate[1] > 80) landRotate[1] = 80;
	if (landRotate[1] < -80) landRotate[1] = -80;
	if (landRotate[0] > 65) landRotate[0] = 65;
	if (landRotate[0] < -65) landRotate[0] = -65;

	frame.x = (float)cos(glm::radians(-landRotate[0])) * cos(glm::radians(landRotate[1])); 
	frame.y = (float)sin(glm::radians(-landRotate[0]));
	frame.z = (float)cos(glm::radians(-landRotate[0])) * sin(glm::radians(landRotate[1]));
	
	Point cameraFront = normalize(splineTangents[u]) * frame.x + splineNormals[u] * frame.y + splineBinormals[u] * frame.z;

	Point normal = splineNormals[u];
	Point eye = splinePoints[u] + splineNormals[u] * h;
	Point center = eye + cameraFront;
	matrix.LookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, normal.x, normal.y, normal.z);
}

void setModelViewMatrix() {
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.LoadIdentity();

	setCamera((int)currentCamera, 0.05);
	//get Modelview matrix
	float m[16];
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.GetMatrix(m);

	//get Normal matrix
	float n[16];
	matrix.GetNormalMatrix(n);

	//get Projection matrix
	float p[16];
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.GetMatrix(p);

	pipelineProgram->Bind();
	pipelineProgram->SetModelViewMatrix(m);
	pipelineProgram->SetProjectionMatrix(p);
	glUniformMatrix4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "normalMatrix"), 1, GL_FALSE, n);

	textureProgram->Bind();

	//specify the value of uniform variable for the textureProgram
	textureProgram->SetModelViewMatrix(m);
	textureProgram->SetProjectionMatrix(p);

	if (glGetError() != 0) cout << "set modelview matrix error" << endl;
}

void setPhongShading() {
	pipelineProgram->Bind();

	//set the phong shading parameters
	float La[4] = { 0.5f, 0.5f, 0.5f, 1 };
	float Ld[4] = { 0.5f, 0.5f, 0.5f, 1 };
	float Ls[4] = { 0.5f, 0.5f, 0.5f, 1 };
	float ka[4] = { 0.135, 0.2225, 0.1575, 1 };
	float kd[4] = { 0.54, 0.89, 0.63, 1 };
	float ks[4] = { 0.316228, 0.316228, 0.316228, 1 };
	float alpha = 0.1;
	float LightDirection[3] = { 0, 1, 1 };

	// Specify the value of uniform variable for the pipelineProgram
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "La"), 1, La);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "Ld"), 1, Ld);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "Ls"), 1, Ls);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ka"), 1, ka);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "kd"), 1, kd);
	glUniform4fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "ks"), 1, ks);
	glUniform3fv(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "LightDirection"), 1, LightDirection);
	glUniform1f(glGetUniformLocation(pipelineProgram->GetProgramHandle(), "alpha"), alpha);

	if (glGetError() != 0) cout << "set phong shading error" << endl;
}

void displayFunc() {
	// render some stuff...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// set modelview matrix for both pipelineProgram & textureProgram
	setModelViewMatrix();
	//set phong shading parameters for pipelineProgram
	setPhongShading();

	//Use BasicProgram--------------------------------------------------------------------------------------------------------
	pipelineProgram->Bind();

	//render double rail by drawing single rail twice by instancing

	// Use Instancing and offset to render single railway twice
	glBindVertexArray(railVertexArray);
	glDrawArrays(GL_TRIANGLES, 0, splinePointsOrderedCrosssection.size());

	if (glGetError() != 0) cout << "basic pipeline program rendering error" << endl;

	//Use Texture Program-------------------------------------------------------------------------------------------------------
	textureProgram->Bind();

	//render ground
	if (mode == 1) {
		glBindTexture(GL_TEXTURE_2D, backgroundtexVertexBuffer);
		glBindVertexArray(backgroundVertexArray);
	}
	else if (mode == 2) {
		glBindTexture(GL_TEXTURE_2D, backgroundtexVertexBuffer_1);
		glBindVertexArray(backgroundVertexArray_1);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundElementBuffer);
	glDrawElements(GL_TRIANGLE_STRIP, backgroundIndices.size(), GL_UNSIGNED_INT, 0);

	if (glGetError() != 0) cout << "texture pipeline program rendering error" << endl;
	
	glutSwapBuffers();
}

void idleFunc() {
	// save the screenshots to disk

	if (screenshot == TRUE) {
		sprintf(screenshotFileName, "%03d.jpg", screenshotName);
		saveScreenshot(screenshotFileName);

		screenshotName = screenshotName + 1;
	}

	//set camera moving velocity------------------------------------------------------------------------------------------------------
	// calculate the time step
	float delta_t = (float)(clock() - t1) / CLOCKS_PER_SEC;
	t1 = clock();
	
	if (pause == FALSE) {
		// specify the current height
	    float h = splinePoints[(int)currentCamera].y;

		/*calculate the velocity acoording to the equation: 
		u_new - u_current = delta_t * sqrt(2 * g * (hMax - h))/||dp/du||*/

		// set the maximum height as hMax + 0.1, otherwise the train will stop at the highest point (when h = hMax, velocity = 0)
		float delta_u;
		delta_u = (parameterU[(int)currentCamera + 1] - parameterU[(int)currentCamera]);
	
		velocity = delta_t * sqrt(2 * g * (hMax + 0.1 - h)) / (length(splineTangents[(int)currentCamera]) * delta_u);
		currentCamera = currentCamera + velocity;
	
		//when moving to the end of the rollercoaster, start from the beginning again
		while (currentCamera >= (splinePoints.size()-1)) currentCamera = currentCamera - (splinePoints.size() - 1);
	}

	// make the screen update 
	glutPostRedisplay();
}

void reshapeFunc(int w, int h) {
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
		// pause / continue
		if (pause == TRUE) pause = FALSE;
		else pause = TRUE;
		break;

	case 'x':
		// take a screenshot
		//saveScreenshot("screenshot.jpg");
		screenshot = TRUE;
		break;

	case 'w':
		// move forward
		currentCamera = currentCamera + 5;
		if (currentCamera >= splinePoints.size() - 1) currentCamera = currentCamera - splinePoints.size();
		break;

	case 's':
		// move backward
		currentCamera = MAX(0, currentCamera - 5);
		break;

	case '1':
		mode = 1;
		break;
	
	case '2':
		mode = 2;
    }
}

//given 4 controll points p1, p2, p3, p4 & parameter u, return the corresponding point value on catmull-roll
Point catmullRollFunc(Point p1, Point p2, Point p3, Point p4, float u) {
	Point point;
	point.x = (u * u * u * (-0.5 * p1.x + 1.5 * p2.x - 1.5 * p3.x + 0.5 * p4.x)
		+ u * u * (1 * p1.x - 2.5 * p2.x + 2 * p3.x - 0.5 * p4.x)
		+ u * (-0.5 * p1.x + 0.5 * p3.x)
		+ (1 * p2.x));
	point.y = (u * u * u * (-0.5 * p1.y + 1.5 * p2.y - 1.5 * p3.y + 0.5 * p4.y)
		+ u * u * (1 * p1.y - 2.5 * p2.y + 2 * p3.y - 0.5 * p4.y)
		+ u * (-0.5 * p1.y + 0.5 * p3.y)
		+ (1 * p2.y));
	point.z = (u * u * u * (-0.5 * p1.z + 1.5 * p2.z - 1.5 * p3.z + 0.5 * p4.z)
		+ u * u * (1 * p1.z - 2.5 * p2.z + 2 * p3.z - 0.5 * p4.z)
		+ u * (-0.5 * p1.z + 0.5 * p3.z)
		+ (1 * p2.z));
	return point;
}

// given 4 controll points p1, p2, p3, p4 & parameter u, return the tangent vector at u
Point catmullRollTang(Point p1, Point p2, Point p3, Point p4, float u) {
	Point tang;
	tang.x = (3 * u * u * (-0.5 * p1.x + 1.5 * p2.x - 1.5 * p3.x + 0.5 * p4.x)
		+ (2 * u * (1 * p1.x - 2.5 * p2.x + 2 * p3.x - 0.5 * p4.x))
		+ (-0.5 * p1.x + 0.5 * p3.x));
	tang.y = (3 * u * u * (-0.5 * p1.y + 1.5 * p2.y - 1.5 * p3.y + 0.5 * p4.y)
		+ (2 * u * (1 * p1.y - 2.5 * p2.y + 2 * p3.y - 0.5 * p4.y))
		+ (-0.5 * p1.y + 0.5 * p3.y));
	tang.z = (3 * u * u * (-0.5 * p1.z + 1.5 * p2.z - 1.5 * p3.z + 0.5 * p4.z)
		+ (2 * u * (1 * p1.z - 2.5 * p2.z + 2 * p3.z - 0.5 * p4.z))
		+ (-0.5 * p1.z + 0.5 * p3.z));
	return tang;
}

// use recursive subdivide method to calculate the points on segment n
void Subdivide(float u0, float u1, float maxlinelength, unsigned int n) {
	float umid = (u0 + u1)/2;
   
	Point x0 = catmullRollFunc(splines[nameSpline].points[n], splines[nameSpline].points[n + 1], splines[nameSpline].points[n + 2], splines[nameSpline].points[n + 3], u0);
	Point x1 = catmullRollFunc(splines[nameSpline].points[n], splines[nameSpline].points[n + 1], splines[nameSpline].points[n + 2], splines[nameSpline].points[n + 3], u1);
	if (length(x0 - x1) > maxlinelength) {
		Subdivide(u0, umid, maxlinelength, n);
		Subdivide(umid, u1, maxlinelength, n);
	}
	else {
		//keep track of parameter u for each spline points to calculate camera moving velocity 
		parameterU.push_back(u0 + n);

		//calculate spline Points & spline Tangents & spline Normals & spline BiNormals
		splinePoints.push_back(x0);
		splineTangents.push_back(catmullRollTang(splines[nameSpline].points[n], splines[nameSpline].points[n + 1], splines[nameSpline].points[n + 2], splines[nameSpline].points[n + 3], u0));

		if (splinePoints.size() - 1 == 0) {
			splineNormals.push_back(normalize(crossProduct(splineTangents[splinePoints.size() - 1], Point(0, 0, -1))));
			splineBinormals.push_back(normalize(crossProduct(splineTangents[splinePoints.size() - 1], splineNormals[splinePoints.size() - 1])));
		}
		else {
			splineNormals.push_back(normalize(crossProduct(splineBinormals[splinePoints.size() - 1 - 1], splineTangents[splinePoints.size() - 1])));
			splineBinormals.push_back(normalize(crossProduct(splineTangents[splinePoints.size() - 1], splineNormals[splinePoints.size() - 1])));
		}

		hMax = MAX(hMax, x0.y);
	}
}

//genrate a spline, including points, tangents, normals, and binormals from splines[nameSpline] to splinePoints, and also rail cross section points
void generateSpline() {

	numSegment = splines[nameSpline].numControlPoints - 3;

	//unsigned int index = 0;
	//for each n, traversal a segment of curve (control points from spline->points[n] to spline->points[n+3])
	for (int n = 0; n < numSegment; n++) {
		Subdivide(0, 1, 0.015, n);    
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

// add one point
void addPoint(int index, Point lightnormal) {
	splinePointsOrderedCrosssection.push_back(splinePointsCrosssection[index]);
	lightNormals.push_back(lightnormal);
}

// add one triangle 
void addTriangle(int index1, int index2, int index3, Point lightnormal) {
	addPoint(index1, lightnormal);
	addPoint(index2, lightnormal);
	addPoint(index3, lightnormal);
}

// add spline segment at parameter u as triangles
void addTriangles(int u) {
	addTriangle(u * 4 + 2, (u + 1) * 4 + 2, u * 4 + 3, splineBinormals[u] * -1.0f);
	addTriangle((u + 1) * 4 + 2, u * 4 + 3, (u + 1) * 4 + 3, splineBinormals[u] * -1.0f);
	addTriangle(u * 4 + 3, (u + 1) * 4 + 3, u * 4 + 1, splineNormals[u] * -1.0f);
	addTriangle((u + 1) * 4 + 3, u * 4 + 1, (u + 1) * 4 + 1, splineNormals[u] * -1.0f);
	addTriangle(u * 4 + 1, (u + 1) * 4 + 1, u * 4, splineBinormals[u]);
	addTriangle((u + 1) * 4 + 1, u * 4, (u + 1) * 4, splineBinormals[u]);
	addTriangle(u * 4, (u + 1) * 4, u * 4 + 2, splineNormals[u]);
	addTriangle((u + 1) * 4, u * 4 + 2, (u + 1) * 4 + 2, splineNormals[u]);
}

//Generate a rail whose crosssection is a rectangle, store data in splinePointsOrderedCrossection & LightNormals
//The rectangle crosssection has a width scale of w, a height scale of h.
//Center of the rectangle is away from the spline point b * UnitNormal in vertical,  a * UnitBinormal in horizon
void generateCuboidRail(float w, float h, float a, float b) {
	splinePointsCrosssection.clear();
	// calculate the four points for each rectangle crosssection
	for (int i = 0; i < splinePoints.size(); i++) {
		splinePointsCrosssection.push_back(splinePoints[i] + splineNormals[i] * h + splineBinormals[i] * w + splineNormals[i] * b + splineBinormals[i] * a);
		splinePointsCrosssection.push_back(splinePoints[i] + splineNormals[i] * -h + splineBinormals[i] * w + splineNormals[i] * b + splineBinormals[i] * a);
		splinePointsCrosssection.push_back(splinePoints[i] + splineNormals[i] * h + splineBinormals[i] * -w + splineNormals[i] * b + splineBinormals[i] * a);
		splinePointsCrosssection.push_back(splinePoints[i] + splineNormals[i] * -h + splineBinormals[i] * -w + splineNormals[i] * b + splineBinormals[i] * a);
	}

	//reorganize the crosssection points to prepare for draw by OpenGL using GL_TRIANGLES
	for (int i = 0; i < splinePoints.size() - 1; i++) {
		addTriangles(i);
	}
}

// store ground points, texture uvs and indices into corresponding background vectors
void generateGround() {
	unsigned int prevPointSize = backgroundPoints.size();

	backgroundPoints.push_back(Point(200, 0, -200));
	backgroundPoints.push_back(Point(200, 0, 200));
	backgroundPoints.push_back(Point(-200, 0, -200));
	backgroundPoints.push_back(Point(-200, 0, 200));

	backgroundTexCoords.push_back(glm::vec2(0, 2));
	backgroundTexCoords.push_back(glm::vec2(0, 0));
	backgroundTexCoords.push_back(glm::vec2(0.333, 2));
	backgroundTexCoords.push_back(glm::vec2(0.333, 0));

	backgroundIndices.push_back(prevPointSize);

	for (int i = 0; i < 4; i++) {
		backgroundIndices.push_back(i);
	}

	backgroundIndices.push_back(prevPointSize + 3);
}

 // store sky points, texture uvs and indices into corresponding background vectors
void generateSky(Point center, float radius, unsigned int rings, unsigned int sectors) {
	unsigned int prevPointSize = backgroundPoints.size();
	float R = 1. / (float)(rings - 1);
	float S = 1. / (float)(sectors - 1);

	sectors = sectors / 2;

	// set points and texture uvs for mountain
	for (int r = 0; r < rings; r++) {
		for (int s = 0; s < sectors; s++) {
			float x = sin(3.1415926 * r * R) * cos(2 * 3.1415926 * s * S);
			float y = sin(3.1415926 * r * R) * sin(2 * 3.1415926 * s * S);
			float z = cos(3.1415926 * r * R);

			backgroundTexCoords.push_back(glm::vec2(s * S * 0.333 + 0.333, r * R));
			backgroundPoints.push_back(Point(x * radius + center.x, y * radius + center.y, z * radius + center.z));
		}
	}

	//set the indices of sky vertices
	backgroundIndices.push_back(prevPointSize);
	for (int r = 0; r < rings; r++) {
		for (int s = 0; s < sectors; s++) {
			backgroundIndices.push_back(prevPointSize + r * sectors + s);
			backgroundIndices.push_back(prevPointSize + (r + 1) * sectors + s);
		}
	}
	backgroundIndices.push_back(prevPointSize + rings * sectors - 1);
}


// store mountain points, texture uvs and indices into corresponding background vectors
void generateMountain(const char* filename) {
	unsigned int prevPointSize = backgroundPoints.size();

	//read heightmap.jpg from file
	ImageIO* heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(filename) != ImageIO::OK)
	{
		cout << "Error reading heightmap image." << endl;
		exit(EXIT_FAILURE);
	}
	int map_height = heightmapImage->getHeight();
	int map_width = heightmapImage->getWidth();

	// set points and texture uvs for mountain
	for (int i = 0; i < map_height; i++) {
		for (int j = 0; j < map_width; j++) {
			backgroundPoints.push_back(Point((float)(i - map_height / 2) / map_height * 500, (float)(int)(heightmapImage->getPixels()[i * map_width + j]) / 2 - 40, (float)(j - map_width / 2) / map_width * 500));
			backgroundTexCoords.push_back(glm::vec2((float)i / (map_height - 1) * 0.333 + 0.666, (float)j / (map_width - 1)));
		}
	}

	// set indices for mountain vertices
	backgroundIndices.push_back(prevPointSize);
	for (int i = 0; i < map_height - 1; i++) {
		for (int j = 0; j < map_width; j++) {
			backgroundIndices.push_back(prevPointSize + i * map_width + j);
			backgroundIndices.push_back(prevPointSize + (i + 1) * map_width + j);
		}
		if (i < map_height - 2) {
			backgroundIndices.push_back(prevPointSize + (i + 1) * map_width + map_width - 1);
			backgroundIndices.push_back(prevPointSize + (i + 1) * map_width);
		}
	}
	backgroundIndices.push_back(prevPointSize + map_height * map_width - 1);
}

void initScene(int argc, char* argv[])
{
	// load the image from a jpeg disk file to main memory
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	//basic shading program (render the railway)--------------------------------------------------------------------------------------------------------------------
	pipelineProgram = new BasicPipelineProgram;
	int ret = pipelineProgram->Init(shaderBasePath, "basic.vertexShader.glsl", "basic.fragmentShader.glsl");
	if (ret != 0) abort();
	
	// calculate spline points, tangents, normals, binormals
	generateSpline();

	//To generate a double T-shaped double rail, I combine 4 Cuboid Rail Track.
	generateCuboidRail(0.01, 0.01, -0.1,0);
	generateCuboidRail(0.014, 0.002, -0.1, 0.01);
	generateCuboidRail(0.01, 0.01, 0.1, 0);
	generateCuboidRail(0.014, 0.002, 0.1, 0.01);

	createVBO(railVertexBuffer, sizeof(Point) * splinePointsOrderedCrosssection.size(), splinePointsOrderedCrosssection.data());
	createVBO(raillightnormalVertexBuffer, sizeof(Point) * lightNormals.size(), lightNormals.data());
	createVAO(railVertexArray);
	SetVertexAttrib(railVertexBuffer, pipelineProgram, "position", 3);
	SetVertexAttrib(raillightnormalVertexBuffer, pipelineProgram, "lightnormal", 3);

	// enable Depth Test
	glEnable(GL_DEPTH_TEST);

	//texture shading program------------------------------------------------------------------------------------------------------------------
   	textureProgram = new BasicPipelineProgram;
	ret = textureProgram->Init(shaderBasePath, "texture.vertexShader.glsl", "texture.fragmentShader.glsl");
	if (ret != 0) abort();

	// mode 1 (ocean background when press key '1')
	generateGround();
	generateSky(Point(0, -15, 0), 80, 80, 80);
	generateMountain("heightmap/GrandTeton-128.jpg");

	createVBO(backgroundVertexBuffer, sizeof(Point) * backgroundPoints.size(), backgroundPoints.data());
	createVBO(backgroundTexCoordVertexBuffer, sizeof(glm::vec2) * backgroundTexCoords.size(), backgroundTexCoords.data());
	createVAO(backgroundVertexArray);
	createEBO(backgroundElementBuffer, sizeof(int) * backgroundIndices.size(), backgroundIndices.data());
	SetVertexAttrib(backgroundVertexBuffer, textureProgram, "position", 3);
	SetVertexAttrib(backgroundTexCoordVertexBuffer, textureProgram, "texCoords", 2);

	glGenBuffers(1, &backgroundtexVertexBuffer);
	initTexture("mode_1.jpg", backgroundtexVertexBuffer);

    // mode 2 (grass background when press key '2')
	glBindBuffer(GL_ARRAY_BUFFER, backgroundVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, backgroundTexCoordVertexBuffer);

    createVAO(backgroundVertexArray_1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundElementBuffer);

	SetVertexAttrib(backgroundVertexBuffer, textureProgram, "position", 3);
	SetVertexAttrib(backgroundTexCoordVertexBuffer, textureProgram, "texCoords", 2);

	glGenBuffers(1, &backgroundtexVertexBuffer_1);
	initTexture("mode_2.jpg", backgroundtexVertexBuffer_1);

	std::cout << "GL error: " << glGetError() << std::endl;
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

	//randomly choose a spline from file, generate data for the spline (including splinePoints[], splineTangents[], splineNormals[], splineBinormals[])
	nameSpline = rand() % numSplines;
	cout << "Using spline No. " << nameSpline << endl;

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
}