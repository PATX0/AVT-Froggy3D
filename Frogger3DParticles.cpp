//
// AVT Multitexturing light 
// based on demos from GLSL Core Tutorial in Lighthouse3D.com   
//
// This demo was built for learning purposes only.
// Some code could be severely optimised, but I tried to
// keep as simple and clear as possible.
//
// The code comes with no warranties, use it at your own risk.
// You may use it, or parts of it, wherever you want.
//
//
//Author: Jo„o Madeiras Pereira

#include <math.h>
#include <iostream>
#include <sstream>

#include <string>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>

// Use Very Simple Libs
#include "VSShaderlib.h"
#include "AVTmathLib.h"
#include "VertexAttrDef.h"
#include "basic_geometry.h"
#include "TGA.h"

#define CAPTION "AVT MultiTexture Demo"
#define FPS 60

#define frand()			((float)rand()/RAND_MAX)
#define M_PI			3.14159265
#define MAX_PARTICULAS  1500

int WindowHandle = 0;
int WinX = 640, WinY = 480;

unsigned int FrameCount = 0;



VSShaderLib shader;

struct MyMesh mesh[12];
int objId = 0; //id of the object mesh - to be used as index of mesh: mesh[objID] means the current mesh


//External array storage defined in AVTmathLib.cpp

/// The storage for matrices
extern float mMatrix[COUNT_MATRICES][16];
extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];

/// The normal matrix
extern float mNormal3x3[9];

GLint pvm_uniformId;
GLint vm_uniformId;
GLint normal_uniformId;
GLint lPos_uniformId;
GLint tex_loc, tex_loc1, tex_loc2;
GLint texMode_uniformId;

GLuint TextureArray[3];
char camMode = 'O'; //keep track of what camera is being used

//Orthogonal Camera Position
float coX, coY, coZ;

//Fixed Perspective Camera Position
float cX, cY, cZ;

//Moving Perspective Camera Position
float camX, camY, camZ;


// Camera Spherical Coordinates for Orthogonal view
float alphaO = -90.0f, betaO = 90.0f;
float rO = 30.0f;

// Camera Spherical Coordinates for Perspective view
float alphaP = -90.0f, betaP = 55.0f;
float rP = 30.0f;

// Camera Spherical Coordinates for MOVING Perspective view
float alpha = -85.0f, beta = 45.0f;
float r = 10.0f;

// angle of rotation for the camera direction
float angle = 0.0;
// actual vector representing the camera's direction
float lx = 0.0f, lz = -1.0f;
// values to move
float deltaAngle = 0.0f;
float deltaMove = 0;

// Mouse Tracking Variables
int startX, startY, tracking = 0;

// Frame counting and FPS computation
long myTime, timebase = 0, frame = 0;
char s[32];
float lightPos[4] = { 4.0f, 6.0f, 2.0f, 1.0f };

float posi[3] = { 0.0f, 0.0f, 0.0f };
float posi2[3] = { 0.0f, 0.0f, 0.0f };
float posi3[3] = { 0.0f, 0.0f, 0.0f };
float positionMov[3] = { 0.0f, 0.0f, 0.0f };
float rotationMov[3] = { 0.0f, 0.0f, 0.0f };
float scaleMov[3] = { 1.0f, 1.0f, 1.0f };

//FIREWORKS
int fireworks = 0;

typedef struct {
	float	life;		// vida
	float	fade;		// fade
	float	r, g, b;    // color
	GLfloat x, y, z;    // posição
	GLfloat vx, vy, vz; // velocidade 
	GLfloat ax, ay, az; // aceleraçcao
} Particle;

Particle particula[MAX_PARTICULAS];
int dead_num_particles = 0;

void timer(int value)
{
	std::ostringstream oss;
	oss << CAPTION << ": " << FrameCount << " FPS @ (" << WinX << "x" << WinY << ")";
	std::string s = oss.str();
	glutSetWindow(WindowHandle);
	glutSetWindowTitle(s.c_str());
	FrameCount = 0;
	glutTimerFunc(1000, timer, 0);
}

int getTime() {
	return glutGet(GLUT_ELAPSED_TIME);
}

void showTime()
{
	glPushMatrix();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 'T');
	glPopMatrix();
}

void refresh(int value)
{
	glutPostRedisplay();
	glutTimerFunc(1000 / FPS, refresh, 0);
}

void iterate(int value)
{
	int i;
	float h;

	/* MÈtodo de Euler de integraÁ„o de eq. diferenciais ordin·rias
	h representa o step de tempo; dv/dt = a; dx/dt = v; e conhecem-se os valores iniciais de x e v */

	//h = 0.125f;
	h = 0.033;
	if (fireworks) {

		for (i = 0; i < MAX_PARTICULAS; i++)
		{
			particula[i].x += (h * particula[i].vx);
			particula[i].y += (h * particula[i].vy);
			particula[i].z += (h * particula[i].vz);
			particula[i].vx += (h * particula[i].ax);
			particula[i].vy += (h * particula[i].ay);
			particula[i].vz += (h * particula[i].az);
			particula[i].life -= particula[i].fade;
		}
		glutPostRedisplay();
		glutTimerFunc(33, iterate, 0);
	}
}

// ------------------------------------------------------------
//
// Reshape Callback Function
//

void changeSize(int w, int h) {

	float ratio;
	// Prevent a divide by zero, when window is too short
	if (h == 0)
		h = 1;
	// set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// set the projection matrix
	ratio = (1.0f * w) / h;

	loadIdentity(PROJECTION);

	if (camMode == 'O') {
		ortho(-25.00f, 25.00f, -10.0f, 40.0f, 0.1f, 1000.0f);
	}
	if (camMode == 'P') {
		perspective(65.00f, ratio, 0.1f, 1000.0f);
	}
	if (camMode == 'M') {
		perspective(53.13f, ratio, 0.1f, 1000.0f);
	}
}


// ------------------------------------------------------------
//
// Render stufff
//
void renderScene(void) {

	GLint loc;	
	//if (deltaMove) { computePos(deltaMove); }
	//if (deltaAngle) { computeDir(deltaAngle); }

	FrameCount++;

	float particle_color[4];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// load identity matrices
	loadIdentity(VIEW);
	loadIdentity(MODEL);
	// set the camera using a function similar to gluLookAt
	if (camMode == 'O') {
		lookAt(coX, coY, coZ, 0, 0, 0, 0, 1, 0);
	}
	if (camMode == 'P') {
		lookAt(cX, cY, cZ, 0, 0, 0, 0, 1, 0);
	}
	if (camMode == 'M') {
		lookAt(camX, camY, camZ, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f);
	}
	// use our shader
	glUseProgram(shader.getProgramIndex());

	//send the light position in eye coordinates

	glUniform4fv(lPos_uniformId, 1, lightPos); //efeito capacete do mineiro, ou seja lighPos foi definido em eye coord 
/*
		float res[4];
		multMatrixPoint(VIEW, lightPos,res);   //lightPos definido em World Coord so it is converted to eye space
		glUniform4fv(lPos_uniformId, 1, res);
*/
//Associar os Texture Units aos Objects Texture
//stone.tga loaded in TU0; checker.tga loaded in TU1;  lightwood.tga loaded in TU2

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, TextureArray[0]);

	//glActiveTexture(GL_TEXTURE1);
//	glBindTexture(GL_TEXTURE_2D, TextureArray[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, TextureArray[2]);

	//Indicar aos tres samplers do GLSL quais os Texture Units a serem usados
//	glUniform1i(tex_loc, 0);
	//glUniform1i(tex_loc1, 1);
	glUniform1i(tex_loc2, 2);

	objId = 0;

	for (int i = 0; i < 12; ++i) {

		// send the material
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.ambient");
		glUniform4fv(loc, 1, mesh[objId].mat.ambient);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
		glUniform4fv(loc, 1, mesh[objId].mat.diffuse);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.specular");
		glUniform4fv(loc, 1, mesh[objId].mat.specular);
		loc = glGetUniformLocation(shader.getProgramIndex(), "mat.shininess");
		glUniform1f(loc, mesh[objId].mat.shininess);
		pushMatrix(MODEL);
		if (i == 0) {
			translate(MODEL, 2.5f, 0.0f, -25.0f);
			scale(MODEL, 10.0f, 0.1f, 50.0f);
		}
		else if (i == 1) {
			translate(MODEL, -2.5f, 0.0f, -25.0f);
			scale(MODEL, 5.0f, 0.1f, 50.0f);
		}
		else if (i == 2) {
			translate(MODEL, 12.5, 0.0f, -25.0f);
			scale(MODEL, 15.0f, 0.1f, 50.0f);
		}
		else if (i == 3) {
			translate(MODEL, 27.5, 0.0f, -25.0f);
			scale(MODEL, 5.0f, 0.1f, 50.0f);
		}
		else if (i == 4) {
			scale(MODEL, 1.5f, 1.1f, 1.0f);
			translate(MODEL, positionMov[0], positionMov[1], positionMov[2]);
		}

		else if (i == 5) {
			translate(MODEL, 0.8f, 0.7f, 0.0f);
			translate(MODEL, positionMov[0] * 1.5, positionMov[1], positionMov[2]);
		}
		else if (i == 6) {
			posi[2] += 0.01f; // to translate in the z-axis
			translate(MODEL, 4.0f, 0.1f, -25.0f);
			scale(MODEL, 3.0f, 2.0f, 5.0f);
			translate(MODEL, posi[0], posi[1], posi[2]);
		}
		else if (i == 7 || i == 8 || i == 9 || i == 10) {
			if (i == 7) {
				posi2[2] += 0.0125f; // to translate in the z-axis
				translate(MODEL, 4.0f, 0.5f, -24.0f);
				translate(MODEL, posi2[0], posi2[1], posi2[2]);
			}
			if (i == 8) {
				posi2[2] += 0.0125f; // to translate in the z-axis
				translate(MODEL, 7.0f, 0.5f, -24.0f);
				translate(MODEL, posi2[0], posi2[1], posi2[2]);
			}
			if (i == 9) {
				posi2[2] += 0.0125f; // to translate in the z-axis
				translate(MODEL, 4.0f, 0.5f, -21.0f);
				translate(MODEL, posi2[0], posi2[1], posi2[2]);
			}
			if (i == 10) {
				posi2[2] += 0.0125f; // to translate in the z-axis
				translate(MODEL, 7.0f, 0.5f, -21.0f);
				translate(MODEL, posi2[0], posi2[1], posi2[2]);
			}
		}
		else if (i == 11) {
			posi3[2] += 0.04f;
			angle += 1.f;
			translate(MODEL, 25.0f, 0.1f, -20.0f);
			translate(MODEL, posi3[0], posi3[1], posi3[2]);
			rotate(MODEL, 90.0f, 1.0f, 0.0f, 0.0f);
			rotate(MODEL, angle, posi3[0], posi3[1], posi3[2]);
		}
		// send matrices to OGL
		computeDerivedMatrix(PROJ_VIEW_MODEL);
		glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
		glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
		computeNormalMatrix3x3();
		glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

		// Render mesh

		if (objId == 0) glUniform1i(texMode_uniformId, 2); // modulate Phong color with texel color
		else if (objId >= 1) glUniform1i(texMode_uniformId, 0); // sÛ componente especular
		//else glUniform1i(texMode_uniformId, 2); // multitexturing

		glBindVertexArray(mesh[objId].vao);
		glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		popMatrix(MODEL);
		objId = (objId + 1) % 12;
	}
	//PARTICLES
	if (fireworks) {
		// draw fireworks particles
		objId = 6;  //quad for particle

		glBindTexture(GL_TEXTURE_2D, TextureArray[2]); //particle.tga associated to TU1 

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glDepthMask(GL_FALSE);  //Depth Buffer Read Only

		glUniform1i(texMode_uniformId, 2); // draw modulated textured particles 

		for (int i = 0; i < MAX_PARTICULAS; i++)
		{
			if (particula[i].life > 0.0f) /* sÛ desenha as que ainda est„o vivas */
			{

				/* A vida da partÌcula representa o canal alpha da cor. Como o blend est· activo a cor final È a soma da cor rgb do fragmento multiplicada pelo
				alpha com a cor do pixel destino */

				particle_color[0] = particula[i].r;
				particle_color[1] = particula[i].g;
				particle_color[2] = particula[i].b;
				particle_color[3] = particula[i].life;

				// send the material - diffuse color modulated with texture
				loc = glGetUniformLocation(shader.getProgramIndex(), "mat.diffuse");
				glUniform4fv(loc, 1, particle_color);

				pushMatrix(MODEL);
				translate(MODEL, particula[i].x, particula[i].y, particula[i].z);

				// send matrices to OGL
				computeDerivedMatrix(PROJ_VIEW_MODEL);
				glUniformMatrix4fv(vm_uniformId, 1, GL_FALSE, mCompMatrix[VIEW_MODEL]);
				glUniformMatrix4fv(pvm_uniformId, 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);
				computeNormalMatrix3x3();
				glUniformMatrix3fv(normal_uniformId, 1, GL_FALSE, mNormal3x3);

				glBindVertexArray(mesh[objId].vao);
				glDrawElements(mesh[objId].type, mesh[objId].numIndexes, GL_UNSIGNED_INT, 0);
				popMatrix(MODEL);
			}
			else dead_num_particles++;
		}

		glDepthMask(GL_TRUE); //make depth buffer again writeable

		if (dead_num_particles == MAX_PARTICULAS) {
			fireworks = 0;
			dead_num_particles = 0;
			printf("All particles dead\n");
		}

	}
	//glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
	glutSwapBuffers();
}


//INIT PARTICLES

void iniParticulas()
{
	GLfloat v, theta, phi;
	int i;

	for (i = 0; i < MAX_PARTICULAS; i++)
	{
		v = 0.8 * frand() + 0.2;
		phi = frand() * M_PI;
		theta = 2.0 * frand() * M_PI;

		particula[i].x = 30.0f;
		particula[i].y = 10.0f;
		particula[i].z = 0.0f;
		particula[i].vx = v * cos(theta) * sin(phi);
		particula[i].vy = v * cos(phi);
		particula[i].vz = v * sin(theta) * sin(phi);
		particula[i].ax = 0.1f; /* simular um pouco de vento */
		particula[i].ay = -0.15f; /* simular a aceleraÁ„o da gravidade */
		particula[i].az = 0.0f;

		/* tom amarelado que vai ser multiplicado pela textura que varia entre branco e preto */
		particula[i].r = 0.882f;
		particula[i].g = 0.552f;
		particula[i].b = 0.211f;

		particula[i].life = 1.0f;		/* vida inicial */
		particula[i].fade = 0.005f;	    /* step de decrÈscimo da vida para cada iteraÁ„o */
	}
}
// ------------------------------------------------------------
//
// Events from the Keyboard
//

void processKeys(unsigned char key, int xx, int yy)
{
	switch (key) {

	case 'e':
		fireworks = 1;
		iniParticulas();
		glutTimerFunc(0, iterate, 0);  //timer for particle system
		break;
	case 27:
		glutLeaveMainLoop();
		break;

	case 'c':
		printf("Camera Spherical Coordinates (%f, %f, %f)\n", alpha, beta, r);
		break;
	case 'm': glEnable(GL_MULTISAMPLE); break;
	case 'n': glDisable(GL_MULTISAMPLE); break;

		//frog position
	case 'w': positionMov[0] += 0.1f; break;
	case 's': positionMov[0] -= 0.1f; break;
	case 'a': positionMov[2] -= 0.1f; break;
	case 'd': positionMov[2] += 0.1f; break;
		//cameras
	case '1': camMode = 'O'; break;
	case '2': camMode = 'P'; break;
	case '3': camMode = 'M'; break;

	}
}


// ------------------------------------------------------------
//
// Mouse Events for the MOVING Camera
//

void processMouseButtons(int button, int state, int xx, int yy)
{
	// start tracking the mouse
	if (state == GLUT_DOWN) {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
	}

	//stop tracking the mouse
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha -= (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			r += (yy - startY) * 0.01f;
			if (r < 0.1f)
				r = 0.1f;
		}
		tracking = 0;
	}
}

// Track mouse motion while buttons are pressed

void processMouseMotion(int xx, int yy)
{

	int deltaX, deltaY;
	float alphaAux, betaAux;
	float rAux;

	deltaX = -xx + startX;
	deltaY = yy - startY;

	// left mouse button: move camera
	if (tracking == 1) {


		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0f)
			betaAux = 85.0f;
		else if (betaAux < -85.0f)
			betaAux = -85.0f;
		rAux = r;
	}
	// right mouse button: zoom
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r + (deltaY * 0.01f);
		if (rAux < 0.1f)
			rAux = 0.1f;
	}

	camX = rAux * sin(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camZ = rAux * cos(alphaAux * 3.14f / 180.0f) * cos(betaAux * 3.14f / 180.0f);
	camY = rAux * sin(betaAux * 3.14f / 180.0f);

	//  uncomment this if not using an idle func
	//	glutPostRedisplay();
}


void mouseWheel(int wheel, int direction, int x, int y) {

	r += direction * 0.1f;
	if (r < 0.1f)
		r = 0.1f;

	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);

	//  uncomment this if not using an idle func
	//	glutPostRedisplay();
}


// --------------------------------------------------------
//
// Shader Stuff
//


GLuint setupShaders() {

	// Shader for models
	shader.init();
	shader.loadShader(VSShaderLib::VERTEX_SHADER, "shaders/texture_demo.vert");
	shader.loadShader(VSShaderLib::FRAGMENT_SHADER, "shaders/texture_demo.frag");

	// set semantics for the shader variables
	glBindFragDataLocation(shader.getProgramIndex(), 0, "colorOut");
	glBindAttribLocation(shader.getProgramIndex(), VERTEX_COORD_ATTRIB, "position");
	glBindAttribLocation(shader.getProgramIndex(), NORMAL_ATTRIB, "normal");
	glBindAttribLocation(shader.getProgramIndex(), TEXTURE_COORD_ATTRIB, "texCoord");

	glLinkProgram(shader.getProgramIndex());

	texMode_uniformId = glGetUniformLocation(shader.getProgramIndex(), "texMode"); // different modes of texturing
	pvm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_pvm");
	vm_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_viewModel");
	normal_uniformId = glGetUniformLocation(shader.getProgramIndex(), "m_normal");
	lPos_uniformId = glGetUniformLocation(shader.getProgramIndex(), "l_pos");
	tex_loc = glGetUniformLocation(shader.getProgramIndex(), "texmap");
	tex_loc1 = glGetUniformLocation(shader.getProgramIndex(), "texmap1");
	tex_loc2 = glGetUniformLocation(shader.getProgramIndex(), "texmap2");

	printf("InfoLog for Hello World Shader\n%s\n\n", shader.getAllInfoLogs().c_str());

	return(shader.isProgramLinked());
}

// ------------------------------------------------------------
//
// Model loading and OpenGL setup
//

void init()
{
	//Orthogonal Camera spherical coordinates
	coX = rO * sin(alphaO * 3.14f / 180.0f) * cos(betaO * 3.14f / 180.0f);
	coZ = rO * cos(alphaO * 3.14f / 180.0f) * cos(betaO * 3.14f / 180.0f);
	coY = rO * sin(betaO * 3.14f / 180.0f);

	//Fixed Perspective Camera spherical coordinates
	cX = rP * sin(alphaP * 3.14f / 180.0f) * cos(betaP * 3.14f / 180.0f);
	cZ = rP * cos(alphaP * 3.14f / 180.0f) * cos(betaP * 3.14f / 180.0f);
	cY = rP * sin(betaP * 3.14f / 180.0f);

	//Moving Perspective Camera spherical coordinates
	camX = r * sin(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camZ = r * cos(alpha * 3.14f / 180.0f) * cos(beta * 3.14f / 180.0f);
	camY = r * sin(beta * 3.14f / 180.0f);

	//Texture Object definition

	glGenTextures(3, TextureArray);
	TGA_Texture(TextureArray, (char*)"street2Lanes.tga", 0);
	TGA_Texture(TextureArray, (char*)"checker.tga", 1);
	TGA_Texture(TextureArray, (char*)"particle.tga", 2);


	float amb[] = { 0.1f, 0.15f, 0.1f, 1.0f };
	float diff[] = { 0.1f, 0.50f, 0.1f, 1.0f };
	float spec[] = { 0.1f, 0.1f, 0.1f, 1.0f };

	float amb1[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float diff1[] = { 0.0f, 1.0f, 0.0f, 1.0f };
	float spec1[] = { 0.9f, 0.9f, 0.9f, 1.0f };

	float amb2[] = { 0.0f, 0.0f, 0.1f, 1.0f };
	float diff2[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	float spec2[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	float amb3[] = { 0.0f, 0.0f, 0.1f, 1.0f };
	float diff3[] = { 1.0f, 1.0f, 0.0f, 1.0f };
	float spec3[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	float amb4[] = { 0.2f, 0.15f, 0.1f, 1.0f };
	float diff4[] = { 0.8f, 0.6f, 0.4f, 1.0f };
	float spec4[] = { 0.8f, 0.8f, 0.8f, 1.0f };

	float emissive[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float shininess = 100.0f;
	int texcount = 0;

	// create geometry and VAO of the pawn
	objId = 0; //estrada
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	// create geometry and VAO of the sphere
	objId = 1;
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	objId = 2;
	memcpy(mesh[objId].mat.ambient, amb2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec2, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	objId = 3;
	memcpy(mesh[objId].mat.ambient, amb1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec1, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	objId = 4; //corpo frog
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	objId = 5; //cabeca frog
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	objId = 6; //corpo carro
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCube();

	objId = 7; //roda carro
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createSphere(0.5f, 5);

	objId = 8; //roda carro
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createSphere(0.5f, 5);

	objId = 9; //roda carro
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createSphere(0.5f, 5);

	objId = 10; //corpo carro
	memcpy(mesh[objId].mat.ambient, amb, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec3, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createSphere(0.5f, 5);

	objId = 11; //tronco 1  
	memcpy(mesh[objId].mat.ambient, amb4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.diffuse, diff4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.specular, spec4, 4 * sizeof(float));
	memcpy(mesh[objId].mat.emissive, emissive, 4 * sizeof(float));
	mesh[objId].mat.shininess = shininess;
	mesh[objId].mat.texCount = texcount;
	createCylinder(10.0f, 0.5f, 20);

	// some GL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

}

// ------------------------------------------------------------
//
// Main function
//


int main(int argc, char** argv) {

	//  GLUT initialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA | GLUT_MULTISAMPLE);

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE | GLUT_DEBUG);

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WinX, WinY);
	WindowHandle = glutCreateWindow(CAPTION);


	//  Callback Registration
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	//glutIdleFunc(renderScene);

//	Mouse and Keyboard Callbacks
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	glutMouseWheelFunc(mouseWheel);
	glutTimerFunc(0, timer, 0);
	glutTimerFunc(0, refresh, 0);


	//	return from main loop
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	//	Init GLEW
	glewExperimental = GL_TRUE;
	glewInit();

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	if (!setupShaders())
		return(1);

	init();
	//  GLUT main loop
	glutMainLoop();

	return(0);

}