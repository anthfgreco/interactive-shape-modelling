#include <stdio.h>
#include <windows.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <math.h>
#include <string.h>
#include "surfaceModeller.h"
#include "subdivcurve.h"

GLdouble worldLeft = -12;
GLdouble worldRight = 12;
GLdouble worldBottom = -9;
GLdouble worldTop = 9;
GLdouble worldCenterX = 0.0;
GLdouble worldCenterY = 0.0;
GLdouble wvLeft = -12;
GLdouble wvRight = 12;
GLdouble wvBottom = -9;
GLdouble wvTop = 9;

GLint glutWindowWidth = 800;
GLint glutWindowHeight = 600;
GLint viewportWidth = glutWindowWidth;
GLint viewportHeight = glutWindowHeight;
int window2D, window3D;
int window3DSizeX = 800, window3DSizeY = 600;
GLdouble aspect = (GLdouble)window3DSizeX / window3DSizeY;

int lastMouseX;
int lastMouseY;

GLdouble eyeX = 0.0, eyeY = 6.0, eyeZ = 22.0;
GLdouble zNear = 0.1, zFar = 100.0;
GLdouble fov = 60.0;


// Ground Mesh material
GLfloat groundMat_ambient[] = { 0.4, 0.4, 0.4, 1.0 };
GLfloat groundMat_specular[] = { 0.01, 0.01, 0.01, 1.0 };
GLfloat groundMat_diffuse[] = { 0.4, 0.4, 0.7, 1.0 };
GLfloat groundMat_shininess[] = { 1.0 };

// Two Lights
GLfloat light_position0[] = { 4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse0[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_position1[] = { -4.0, 8.0, 8.0, 1.0 };
GLfloat light_diffuse1[] = { 1.0, 1.0, 1.0, 1.0 };

GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat model_ambient[] = { 0.5, 0.5, 0.5, 1.0 };


// The profile curve is a subdivision curve

int numCirclePoints = 30;
double circleRadius = 0.2;
int hoveredCircle = -1;

int currentCurvePoint = 0;
int angle = 0;
int animate = 0;
int delay = 15; // milliseconds


SubdivisionCurve subcurve;
// Use circles to draw subdivision curve control points
Circle circles[MAXCONTROLPOINTS];

void keyboard(unsigned char key, int x, int y);
void functionKeys(int key, int x, int y);
boolean wireFrame = false;

int main(int argc, char* argv[])
{
	glutInit(&argc, (char **)argv);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(glutWindowWidth, glutWindowHeight);
	glutInitWindowPosition(50, 100);

	// The 2D Window
	window2D = glutCreateWindow("Profile Curve");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	// Initialize the 2D profile curve system
	init2DCurveWindow();
	// A few input handlers
	glutMouseFunc(mouseButtonHandler);
	glutMotionFunc(mouseMotionHandler);
	glutPassiveMotionFunc(mouseHoverHandler);
	glutMouseWheelFunc(mouseScrollWheelHandler);
	glutKeyboardFunc(keyboardHandler);
	glutSpecialFunc(specialKeyHandler);


	// The 3D Window
	window3D = glutCreateWindow("Surface of Revolution");
	glutPositionWindow(900, 100);
	glutDisplayFunc(display3D);
	glutReshapeFunc(reshape3D);
	glutMouseFunc(mouseButtonHandler3D);
	glutMouseWheelFunc(mouseScrollWheelHandler3D);
	glutMotionFunc(mouseMotionHandler3D);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(functionKeys);
	// Initialize the 3D system
	init3DSurfaceWindow();

	// Annnd... ACTION!!
	glutMainLoop();

	return 0;
}

void init2DCurveWindow()
{
	glLineWidth(3.0);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);
	initSubdivisionCurve();
	initControlPoints();
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	draw2DScene();
	glutSwapBuffers();
}


void draw2DScene()
{
	drawAxes();
	drawSubdivisionCurve();
	drawControlPoints();
}

void drawAxes()
{
	glPushMatrix();
	glColor3f(1.0, 0.0, 0);
	glBegin(GL_LINE_STRIP);
	glVertex3f(0, 8.0, 0);
	glVertex3f(0, -8.0, 0);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(-8, 0.0, 0);
	glVertex3f(8, 0.0, 0);
	glEnd();
	glPopMatrix();
}

void drawSubdivisionCurve() {

	// Subdivide the given curve based on control points
	computeSubdivisionCurve(&subcurve);

	// Draw it
	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_LINE_STRIP);
	for (int i = 0; i < subcurve.numCurvePoints; i++)
	{
		glVertex3f(subcurve.curvePoints[i].x, subcurve.curvePoints[i].y, 0.0);
	}
	glEnd();
	glPopMatrix();
}

void drawControlPoints() {
	int i, j;
	for (i = 0; i < subcurve.numControlPoints; i++) {
		glPushMatrix();
		glColor3f(1.0f, 0.0f, 0.0f);
		glTranslatef(circles[i].circleCenter.x, circles[i].circleCenter.y, 0);
		// for the hoveredCircle, draw an outline and change its colour
		if (i == hoveredCircle) {
			// outline
			glColor3f(0.0, 1.0, 0.0);
			glBegin(GL_LINE_LOOP);
			for (j = 0; j < numCirclePoints; j++) {
				glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
			}
			glEnd();
			// colour change
			glColor3f(0.5, 0.0, 1.0);
		}
		glBegin(GL_LINE_LOOP);
		for (j = 0; j < numCirclePoints; j++) {
			glVertex3f(circles[i].circlePoints[j].x, circles[i].circlePoints[j].y, 0);
		}
		glEnd();
		glPopMatrix();
	}
}

void initSubdivisionCurve() {
	// Initialize 3 control points of the subdivision curve
	subcurve.controlPoints[0].x = 1;
	subcurve.controlPoints[1].x = 2;
	subcurve.controlPoints[2].x = 1;

	subcurve.controlPoints[0].y = 6;
	subcurve.controlPoints[1].y = 1;
	subcurve.controlPoints[2].y = -4;

	subcurve.numControlPoints = 3;
	subcurve.subdivisionSteps = 3;
}

void initControlPoints() {
	int i;
	int num = subcurve.numControlPoints;
	for (i = 0; i < num; i++) {
		constructCircle(circleRadius, numCirclePoints, circles[i].circlePoints);
		circles[i].circleCenter = subcurve.controlPoints[i];
	}
}

void screenToWorldCoordinates(int xScreen, int yScreen, GLdouble *xw, GLdouble *yw)
{
	GLdouble xView, yView;
	screenToCameraCoordinates(xScreen, yScreen, &xView, &yView);
	cameraToWorldCoordinates(xView, yView, xw, yw);
}

void screenToCameraCoordinates(int xScreen, int yScreen, GLdouble *xCamera, GLdouble *yCamera)
{
	*xCamera = ((wvRight - wvLeft) / glutWindowWidth)  * xScreen;
	*yCamera = ((wvTop - wvBottom) / glutWindowHeight) * (glutWindowHeight - yScreen);
}

void cameraToWorldCoordinates(GLdouble xcam, GLdouble ycam, GLdouble *xw, GLdouble *yw)
{
	*xw = xcam + wvLeft;
	*yw = ycam + wvBottom;
}

void worldToCameraCoordinates(GLdouble xWorld, GLdouble yWorld, GLdouble *xcam, GLdouble *ycam)
{
	double wvCenterX = wvLeft + (wvRight - wvLeft) / 2.0;
	double wvCenterY = wvBottom + (wvTop - wvBottom) / 2.0;
	*xcam = worldCenterX - wvCenterX + xWorld;
	*ycam = worldCenterY - wvCenterY + yWorld;
}

int currentButton;

void mouseButtonHandler(int button, int state, int xMouse, int yMouse)
{
	int i;

	currentButton = button;
	if (button == GLUT_LEFT_BUTTON)
	{
		switch (state) {
		case GLUT_DOWN:
			if (hoveredCircle > -1) {
				screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCircle].circleCenter.x, &circles[hoveredCircle].circleCenter.y);
				screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCircle].x, &subcurve.controlPoints[hoveredCircle].y);
			}
			break;
		case GLUT_UP:
			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}
	else if (button == GLUT_MIDDLE_BUTTON)
	{
		switch (state) {
		case GLUT_DOWN:
			break;
		case GLUT_UP:
			if (hoveredCircle == -1 && subcurve.numControlPoints < MAXCONTROLPOINTS) {
				GLdouble newPointX;
				GLdouble newPointY;
				screenToWorldCoordinates(xMouse, yMouse, &newPointX, &newPointY);
				subcurve.controlPoints[subcurve.numControlPoints].x = newPointX;
				subcurve.controlPoints[subcurve.numControlPoints].y = newPointY;
				constructCircle(circleRadius, numCirclePoints, circles[subcurve.numControlPoints].circlePoints);
				circles[subcurve.numControlPoints].circleCenter = subcurve.controlPoints[subcurve.numControlPoints];
				subcurve.numControlPoints++;
			}
			else if (hoveredCircle > -1 && subcurve.numControlPoints > MINCONTROLPOINTS) {
				subcurve.numControlPoints--;
				for (i = hoveredCircle; i < subcurve.numControlPoints; i++) {
					subcurve.controlPoints[i].x = subcurve.controlPoints[i + 1].x;
					subcurve.controlPoints[i].y = subcurve.controlPoints[i + 1].y;
					circles[i].circleCenter = circles[i + 1].circleCenter;
				}
			}

			glutSetWindow(window3D);
			glutPostRedisplay();
			break;
		}
	}

	glutSetWindow(window2D);
	glutPostRedisplay();
}

void mouseMotionHandler(int xMouse, int yMouse)
{
	if (currentButton == GLUT_LEFT_BUTTON) {
		if (hoveredCircle > -1) {
			screenToWorldCoordinates(xMouse, yMouse, &circles[hoveredCircle].circleCenter.x, &circles[hoveredCircle].circleCenter.y);
			screenToWorldCoordinates(xMouse, yMouse, &subcurve.controlPoints[hoveredCircle].x, &subcurve.controlPoints[hoveredCircle].y);
		}
	}
	else if (currentButton == GLUT_MIDDLE_BUTTON) {
	}
	glutPostRedisplay();
}

void mouseHoverHandler(int xMouse, int yMouse)
{
	hoveredCircle = -1;
	GLdouble worldMouseX, worldMouseY;
	screenToWorldCoordinates(xMouse, yMouse, &worldMouseX, &worldMouseY);
	int i;
	// see if we're hovering over a control point
	for (i = 0; i < subcurve.numControlPoints; i++) {
		GLdouble distToX = worldMouseX - circles[i].circleCenter.x;
		GLdouble distToY = worldMouseY - circles[i].circleCenter.y;
		GLdouble euclideanDist = sqrt(distToX*distToX + distToY * distToY);
		//printf("Dist from point %d is %.2f\n", i, euclideanDist);
		if (euclideanDist < 2.0) {
			hoveredCircle = i;
		}
	}

	glutPostRedisplay();
}

void mouseScrollWheelHandler(int button, int dir, int xMouse, int yMouse)
{
	GLdouble worldViewableWidth;
	GLdouble worldViewableHeight;
	GLdouble cameraOnCenterX;
	GLdouble cameraOnCenterY;
	GLdouble anchorPointX, anchorPointY;
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft + (wvRight - wvLeft) / 2.0;
	double wvCenterY = wvBottom + (wvTop - wvBottom) / 2.0;
	double wvWidth = wvRight - wvLeft;
	double wvHeight = wvTop - wvBottom;

	worldToCameraCoordinates(worldCenterX, worldCenterY, &cameraOnCenterX, &cameraOnCenterY);
	if (wvWidth >= (worldRight - worldLeft)*1.2) {

		anchorPointX = cameraOnCenterX;
		anchorPointY = cameraOnCenterY;
	}
	else {
		// else, anchor the zoom to the mouse
		screenToWorldCoordinates(xMouse, yMouse, &anchorPointX, &anchorPointY);
	}
	GLdouble anchorToCenterX = anchorPointX - wvCenterX;
	GLdouble anchorToCenterY = anchorPointY - wvCenterY;

	// set up maximum shift
	GLdouble maxPosShift = 50;
	GLdouble maxNegShift = -50;

	anchorToCenterX = (anchorToCenterX > maxPosShift) ? maxPosShift : anchorToCenterX;
	anchorToCenterX = (anchorToCenterX < maxNegShift) ? maxNegShift : anchorToCenterX;
	anchorToCenterY = (anchorToCenterY > maxPosShift) ? maxPosShift : anchorToCenterY;
	anchorToCenterY = (anchorToCenterY < maxNegShift) ? maxNegShift : anchorToCenterY;

	// move the world centre closer to this point.
	wvCenterX += anchorToCenterX / 4;
	wvCenterY += anchorToCenterY / 4;


	if (dir > 0) {
		// Zoom in to mouse point
		clipWindowWidth = wvWidth * 0.8;
		clipWindowHeight = wvHeight * 0.8;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;

	}
	else {
		// Zoom out
		clipWindowWidth = wvWidth * 1.25;
		clipWindowHeight = wvHeight * 1.25;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
	}

	glutPostRedisplay();

}

void keyboardHandler(unsigned char key, int x, int y)
{
	int i;

	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft + (wvRight - wvLeft) / 2.0;
	double wvCenterY = wvBottom + (wvTop - wvBottom) / 2.0;
	double wvWidth = wvRight - wvLeft;
	double wvHeight = wvTop - wvBottom;

	switch (key) {
	case 'q':
	case 'Q':
	case 27:
		// Esc, q, or Q key = Quit 
		exit(0);
		break;
	case 107:
	case '+':
		clipWindowWidth = wvWidth * 0.8;
		clipWindowHeight = wvHeight * 0.8;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
		break;
	case 109:
	case '-':
		clipWindowWidth = wvWidth * 1.25;
		clipWindowHeight = wvHeight * 1.25;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void specialKeyHandler(int key, int x, int y)
{
	double clipWindowWidth;
	double clipWindowHeight;
	double wvCenterX = wvLeft + (wvRight - wvLeft) / 2.0;
	double wvCenterY = wvBottom + (wvTop - wvBottom) / 2.0;
	double wvWidth = wvRight - wvLeft;
	double wvHeight = wvTop - wvBottom;

	switch (key) {
	case GLUT_KEY_LEFT:
		wvLeft -= 5.0;
		wvRight -= 5.0;
		break;
	case GLUT_KEY_RIGHT:
		wvLeft += 5.0;
		wvRight += 5.0;
		break;
	case GLUT_KEY_UP:
		wvTop += 5.0;
		wvBottom += 5.0;
		break;
	case GLUT_KEY_DOWN:
		wvTop -= 5.0;
		wvBottom -= 5.0;
		break;
		// Want to zoom in/out and keep  aspect ratio = 2.0
	case GLUT_KEY_F1:
		clipWindowWidth = wvWidth * 0.8;
		clipWindowHeight = wvHeight * 0.8;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
		break;
	case GLUT_KEY_F2:
		clipWindowWidth = wvWidth * 1.25;
		clipWindowHeight = wvHeight * 1.25;
		wvRight = wvCenterX + clipWindowWidth / 2.0;
		wvTop = wvCenterY + clipWindowHeight / 2.0;
		wvLeft = wvCenterX - clipWindowWidth / 2.0;
		wvBottom = wvCenterY - clipWindowHeight / 2.0;
		break;
	}
	glutPostRedisplay();
}


void reshape(int w, int h)
{
	glutWindowWidth = (GLsizei)w;
	glutWindowHeight = (GLsizei)h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(wvLeft, wvRight, wvBottom, wvTop);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}



/************************************************************************************
 *
 *
 * 3D Window and Surface of Revolution Code
 *
 * Fill in the code in the empty functions
 * Feel free to use your own functions or rename mine. Mine are just a guide.
 * Add whatever variables you think are necessary
 ************************************************************************************/

 //
 // Surface of Revolution consists of vertices and quads
 //
 // Set up lighting/shading and material properties for surface of revolution
GLfloat quadMat_ambient[] = { 0.1234f, 0.0115f, 0.0115f, 0.35f };
GLfloat quadMat_specular[] = { 0.31424f, 0.07568f, 0.07568f, 0.35f };
GLfloat quadMat_diffuse[] = { 0.327811f, 0.333f, 0.333f, 0.35f };
GLfloat quadMat_shininess[] = { 6.8f };

typedef struct Vertex
{
	GLdouble x, y, z;
	Vector3D normal; // vertex normal vector
} Vertex;

// Each quad shares vertices with other neighbor quads
typedef struct Quad
{
	Vertex *vertex[4]; // 4 pointers to vertices in the vertex array
	Vector3D normal; // quad normal vector
} Quad;

Vertex vertexArray[1000][1000];
Quad quadArray[1000];

double pi = 3.14159265358979323846;
int verticesPerLayer = 40; // How many vertices each x layer will have

// Variables for rotating and zooming in/out on 3D object
double xAngle = 0;
double yAngle = 0;
double scaleFactor = 1;

void init3DSurfaceWindow()
{
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, model_ambient);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);    // Renormalize normal vectors 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LINE_SMOOTH);
	glClearColor(0.4F, 0.4F, 0.4F, 0.0F);  // Color and depth for glClear

	glViewport(0, 0, (GLsizei)window3DSizeX, (GLsizei)window3DSizeY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void reshape3D(int w, int h)
{
	glutWindowWidth = (GLsizei)w;
	glutWindowHeight = (GLsizei)h;
	glViewport(0, 0, glutWindowWidth, glutWindowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, zNear, zFar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 6.0, 22.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

}

void buildVertexArray()
{
	for (int i = 0; i < subcurve.numCurvePoints; i++) {
		int j = 0;
		for (double theta = 0.0; theta < 2.0 * pi; theta += pi / (verticesPerLayer / 2)) {
			double xtheta = cos(theta);
			double ztheta = sin(theta);
			Vertex v;
			v.x = (subcurve.curvePoints[i].x)*xtheta;
			v.y = subcurve.curvePoints[i].y;
			v.z = (subcurve.curvePoints[i].x)*ztheta;
			v.normal.x = 0;
			v.normal.y = 0;
			v.normal.z = 0;
			vertexArray[i][j] = v;
			j = j + 1;
		}
	}
}


void buildQuadArray()
{
	int currentQuad = 0;

	for (int i = 0; i < subcurve.numCurvePoints; i++) {
		int j = 0;
		for (double theta = 0.0; theta < 2.0 * pi; theta += pi / (verticesPerLayer / 2)) {
			Quad q;
			if (j == verticesPerLayer - 1 && i == subcurve.numCurvePoints - 1) {
				q.vertex[0] = &vertexArray[i][j];
				q.vertex[1] = &vertexArray[i][0];
				q.vertex[2] = &vertexArray[i][0];
				q.vertex[3] = &vertexArray[i][j];
			}
			else if (j == verticesPerLayer - 1) {
				q.vertex[0] = &vertexArray[i][j];
				q.vertex[1] = &vertexArray[i][0];
				q.vertex[2] = &vertexArray[i + 1][0];
				q.vertex[3] = &vertexArray[i + 1][j];
			}
			else if (i == subcurve.numCurvePoints - 1) {
				q.vertex[0] = &vertexArray[i][j];
				q.vertex[1] = &vertexArray[i][j];
				q.vertex[2] = &vertexArray[i-1][j];
				q.vertex[3] = &vertexArray[i-1][j];
			}
			else {
				q.vertex[0] = &vertexArray[i][j];
				q.vertex[1] = &vertexArray[i][j + 1];
				q.vertex[2] = &vertexArray[i + 1][j + 1];
				q.vertex[3] = &vertexArray[i + 1][j];
			}
			quadArray[currentQuad] = q;
			currentQuad = currentQuad + 1;
			j = j + 1;
		}
	}
}

void computeQuadNormals()
{
	// compute one normal per quad polygon and store in the quad structure
	// Newell's Method - see http://www.dillonbhuff.com/?p=284
	
	int currentQuad = 0;

	for (int i = 0; i < subcurve.numCurvePoints; i++) {
		int j = 0;
		for (double theta = 0.0; theta < 2.0 * pi; theta += pi / (verticesPerLayer / 2)) {
			double normal_x = 0.0;
			double normal_y = 0.0;
			double normal_z = 0.0;

			Quad q = quadArray[currentQuad];

			int k, l;
			for (k = 0, l = 1; k < 4; k++, l++) {
				if (l == 4) {
					l = 0;
				}
				Vertex pk = *q.vertex[k];
				Vertex pl = *q.vertex[l];

				normal_x += (((pk.z) + (pl.z)) * ((pl.y) - (pk.y)));
				normal_y += (((pk.x) + (pl.x)) * ((pl.z) - (pk.z)));
				normal_z += (((pk.y) + (pl.y)) * ((pl.x) - (pk.x)));

			}
			Vector3D temp;
			temp.x = normal_x;
			temp.y = normal_y;
			temp.z = normal_z;
			
			quadArray[currentQuad].normal = temp;

			currentQuad = currentQuad + 1;
			j = j + 1;
		}
	}

}

void computeVertexNormals()
{
	// compute a normal for each vertex of the surface by averaging the 4 normals of 
	// the 4 quads that share the vertex
	// go through all quads and *add* the quad normal to each 
	// of its 4 vertices. Once this is done, go through all vertices and normalize the normal vector
	int currentQuad = 0;

	for (int i = 0; i < subcurve.numCurvePoints; i++) {
		int j = 0;
		for (double theta = 0.0; theta < 2.0 * pi; theta += pi / (verticesPerLayer / 2)) {
			Quad q;
			q = quadArray[currentQuad];
			vertexArray[i][j].normal.x += q.normal.x;
			vertexArray[i][j].normal.y += q.normal.y;
			vertexArray[i][j].normal.z += q.normal.z;
			vertexArray[i][j + 1].normal.x += q.normal.x;
			vertexArray[i][j + 1].normal.y += q.normal.y;
			vertexArray[i][j + 1].normal.z += q.normal.z;
			vertexArray[i + 1][j + 1].normal.x += q.normal.x;
			vertexArray[i + 1][j + 1].normal.y += q.normal.y;
			vertexArray[i + 1][j + 1].normal.z += q.normal.z;
			vertexArray[i + 1][j].normal.x += q.normal.x;
			vertexArray[i + 1][j].normal.y += q.normal.y;
			vertexArray[i + 1][j].normal.z += q.normal.z;
			
			currentQuad = currentQuad + 1;
			j = j + 1;
		}
	}
}

void normalizeNormals()
{
	for (int i = 0; i < subcurve.numCurvePoints; i++) {
		int j = 0;
		for (double theta = 0.0; theta < 2.0 * pi; theta += pi / (verticesPerLayer / 2)) {
			vertexArray[i][j].normal = normalize(vertexArray[i][j].normal);
			j = j + 1;
		}
	}
}

void flipNormals()
{
	for (int i = 0; i < subcurve.numCurvePoints; i++) {
		int j = 0;
		for (double theta = 0.0; theta < 2.0 * pi; theta += pi / (verticesPerLayer / 2)) {
			vertexArray[i][j].normal.x = -vertexArray[i][j].normal.x;
			vertexArray[i][j].normal.y = -vertexArray[i][j].normal.y;
			vertexArray[i][j].normal.z = -vertexArray[i][j].normal.z;
			j = j + 1;
		}
	}
}

// Not used, only for debugging
void drawVertices()
{
	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_POINTS);
	for (int i = 0; i < subcurve.numCurvePoints; i++) {
		int j = 0;
		for (double theta = 0.0; theta < 2.0 * pi; theta += pi / (verticesPerLayer / 2)) {
			Vertex v = vertexArray[i][j];
			glVertex3f(v.x, v.y, v.z);
			j = j + 1;
		}
	}
	glEnd();
	glPopMatrix();
}


void drawQuads()
{
	int currentQuad = 0;
	
	for (int i = 0; i < subcurve.numCurvePoints; i++) {
		int j = 0;
		for (double theta = 0.0; theta < 2.0 * pi; theta += pi / (verticesPerLayer / 2)) {
			if (wireFrame == true) {
				glBegin(GL_LINE_LOOP);
			}
			else {
				glBegin(GL_QUADS);
			}
			Quad q = quadArray[currentQuad];
			Vertex v0 = *q.vertex[0];
			Vertex v1 = *q.vertex[1];
			Vertex v2 = *q.vertex[2];
			Vertex v3 = *q.vertex[3];

			glNormal3f(v0.normal.x, v0.normal.y, v0.normal.z);
			glVertex3f(v0.x, v0.y, v0.z);

			glNormal3f(v1.normal.x, v1.normal.y, v1.normal.z);
			glVertex3f(v1.x, v1.y, v1.z);

			glNormal3f(v2.normal.x, v2.normal.y, v2.normal.z);
			glVertex3f(v2.x, v2.y, v2.z);

			glNormal3f(v3.normal.x, v3.normal.y, v3.normal.z);
			glVertex3f(v3.x, v3.y, v3.z);

			glEnd();
			currentQuad = currentQuad + 1;
			j = j + 1;
		}
	}
}

// Not used, only for debugging
void draw3DSubdivisionCurve() {

	// Subdivide the given curve based on control points
	//computeSubdivisionCurve(&subcurve);

	// Draw it
	glColor3f(0.0, 1.0, 0.0);
	glPushMatrix();
	glBegin(GL_POINTS);
	for (int i = 0; i < subcurve.numCurvePoints; i++) {
		for (double theta = 0.0; theta < 2.0 * pi; theta += pi / 3.0) {
			double xtheta = cos(theta);
			double ztheta = sin(theta);
			glVertex3f((subcurve.curvePoints[i].x)*xtheta, subcurve.curvePoints[i].y, ztheta);
		}
	}
	glEnd();
	glPopMatrix();
}

void display3D()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glScalef(scaleFactor, scaleFactor, scaleFactor);
	drawGround();

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, quadMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, quadMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, quadMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, quadMat_shininess);

	glPushMatrix();
	glRotatef(xAngle, 0.0, 1.0, 0.0);
	glRotatef(yAngle, 1.0, 0.0, 0.0);
	
	buildVertexArray();
	buildQuadArray();
	computeQuadNormals();
	computeVertexNormals();
	normalizeNormals();
	flipNormals();
	drawQuads();

	glPopMatrix();
	glutSwapBuffers();
}

void drawGround() {
	glPushMatrix();
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, groundMat_ambient);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, groundMat_specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, groundMat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, groundMat_shininess);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glVertex3f(-22.0f, -10.0f, -22.0f);
	glVertex3f(-22.0f, -10.0f, 22.0f);
	glVertex3f(22.0f, -10.0f, 22.0f);
	glVertex3f(22.0f, -10.0f, -22.0f);
	glEnd();
	glPopMatrix();
}

void mouseButtonHandler3D(int button, int state, int x, int y)
{
	currentButton = button;
	lastMouseX = x;
	lastMouseY = y;

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{

		}

		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{

		}

		break;
	case GLUT_MIDDLE_BUTTON:
		if (state == GLUT_DOWN)
		{

		}

		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void mouseScrollWheelHandler3D(int button, int dir, int xMouse, int yMouse) {
	if (dir > 0) {
		scaleFactor = scaleFactor * 1.1;
		glutPostRedisplay();
	}
	else {
		scaleFactor = scaleFactor * 0.9;
		glutPostRedisplay();
	}
	
}

void mouseMotionHandler3D(int x, int y)
{
	int dx = x - lastMouseX;
	int dy = y - lastMouseY;

	if (currentButton == GLUT_LEFT_BUTTON) {
		
		xAngle += dx/1.3;
		yAngle += dy/1.3;

	}
	if (currentButton == GLUT_RIGHT_BUTTON)
	{
		
	}
	else if (currentButton == GLUT_MIDDLE_BUTTON)
	{
	}

	lastMouseX = x;
	lastMouseY = y;

	glutPostRedisplay();
}

// Callback, handles input from the keyboard, function and arrow keys
void functionKeys(int key, int x, int y)
{
	// Help key
	if (key == GLUT_KEY_F1)
	{
		printf("CONTROLS\n");
		printf("========================================\n");
		printf("Use w to toggle between wireframe and quad drawing\n");
		printf("Drag the left mouse to rotate around the x-axis and y-axis\n");
		printf("Scroll the middle mouse button to zoom in and out\n");
		printf("Use r to reset to the initial camera view\n");
		printf("\n");
	}
	
	glutPostRedisplay();   // Trigger a window redisplay
}

// Callback, handles input from the keyboard, non-arrow keys
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		wireFrame = !wireFrame;
		break;
	case 'r':
		scaleFactor = 1;
		xAngle = 0;
		yAngle = 0;
		break;
	}

	glutPostRedisplay();   // Trigger a window redisplay
}

double dotProduct(Vector3D a, Vector3D b) {
	double dotProduct;

	dotProduct = a.x*b.x + a.y*b.y * a.z*b.z;

	return dotProduct;
}

Vector3D crossProduct(Vector3D a, Vector3D b) {
	Vector3D cross;

	cross.x = a.y * b.z - b.y * a.z;
	cross.y = a.x * b.z - b.x * a.z;
	cross.z = a.x * b.y - b.x * a.y;

	return cross;
}

Vector3D fourVectorAverage(Vector3D a, Vector3D b, Vector3D c, Vector3D d) {
	Vector3D average;
	average.x = (a.x + b.x + c.x + d.x) / 4.0;
	average.y = (a.y + b.y + c.y + d.y) / 4.0;
	average.z = (a.z + b.z + c.z + d.z) / 4.0;
	return average;
}

Vector3D normalize(Vector3D a) {
	GLdouble norm = sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
	Vector3D normalized;
	normalized.x = a.x / norm;
	normalized.y = a.y / norm;
	normalized.z = a.z / norm;
	return normalized;
}







