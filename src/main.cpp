/*
musicvisualizer base code
by Christian Eckhardt 2018
with some code snippets from Ian Thomas Dunn and Zoe Wood, based on their CPE CSC 471 base code
On Windows, it whould capture "what you here" automatically, as long as you have the Stereo Mix turned on!! (Recording devices -> activate)
*/

#include <iostream>
#include <fstream>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Shape.h"
#include "line.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <thread>
using namespace std;
using namespace glm;
shared_ptr<Shape> shape;
shared_ptr<Shape> plane;
shared_ptr<Shape> tieFighter;
#define MESHSIZE 1000


ofstream file1;
ofstream file2;
ofstream file3;

int renderstate = 1;
int realspeed = 0;
//********************
#include <math.h>
#include <algorithm>    


double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d, q, e, z, c, space;

	


	camera()
	{
		w = a = s = d = q = e = z = c = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;

		float fwdspeed = 10;
		if (realspeed)
			fwdspeed = 10;

		if (w == 1)
		{
			speed = fwdspeed*ftime;
		}
		else if (s == 1)
		{
			speed = -fwdspeed*ftime;
		}
		float yangle=0;
		if (a == 1)
			yangle = -0.6*ftime;
		else if(d==1)
			yangle = 0.6*ftime;
		rot.y += yangle;
		float zangle = 0;
		if (q == 1)
			zangle = -0.6 * ftime;
		else if (e == 1)
			zangle = 0.6 * ftime;
		rot.z += zangle;
		float xangle = 0;
		if (z == 1)
			xangle = -0.6 * ftime;
		else if (c == 1)
			xangle = 0.6 * ftime;
		rot.x += xangle;

		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		R = Rz * Rx * R;
		dir = dir * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R * T;

	}
	void get_dirpos(vec3 &up,vec3 &dir,vec3 &position)
		{		
		position = pos;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir4 = glm::vec4(0, 0, 1, 0);
		R = Rx * Rz * R;
		dir4 = dir4*R;
		dir = vec3(dir4);
		glm::vec4 up4 = glm::vec4(0, 1, 0, 0);
		up4 = R*vec4(0, 1, 0, 0);
		up4 = vec4(0, 1, 0, 0)*R;
		up = vec3(up4);
		}
	
};

camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> prog, heightshader, skyprog, linesshader, pplane;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint MeshPosID, MeshTexID, IndexBufferIDBox;

	//texture data
	GLuint Texture,AudioTex, AudioTexBuf;
	GLuint Texture2,HeightTex;

	//line
	Line linerender1, linerender2, linerender3;
	Line smoothrender1, smoothrender2, smoothrender3;
	vector<vec3> line1, dir1, up1, line2, dir2, up2, line3, dir3, up3;

	vector<vec3> cardinal1, cardinal2, cardinal3;

	vec3 milleniumFalconPos;
	vec3 tieFighterPos;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		{
			mycam.q = 1;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE)
		{
			mycam.q = 0;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			mycam.e = 1;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
		{
			mycam.e = 0;
		}
		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		{
			mycam.z = 1;
		}
		if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
		{
			mycam.z = 0;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		{
			mycam.c = 1;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
		{
			mycam.c = 0;
		}

		//if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE)
		//{
		//	if (renderstate == 1)
		//		renderstate = 2;
		//	else
		//		renderstate = 1;
		//}
		if (key == GLFW_KEY_ENTER && action == GLFW_PRESS)
			{
			vec3 dir,pos,up;
			mycam.get_dirpos(up, dir, pos);
			cout << "point position:" << pos.x << "," << pos.y<< "," << pos.z << endl;
			cout << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
			cout << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
			}
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
			{
			mycam.space = 1;
			}
		if (key == GLFW_KEY_LEFT_ALT && action == GLFW_RELEASE)
			{
			realspeed = !realspeed;
			}
		
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;

			//change this to be the points converted to WORLD
			//THIS IS BROKEN< YOU GET TO FIX IT - yay!
			newPt[0] = 0;
			newPt[1] = 0;

			std::cout << "converted:" << newPt[0] << " " << newPt[1] << std::endl;
			glBindBuffer(GL_ARRAY_BUFFER, MeshPosID);
			//update the vertex array with the updated points
			glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*6, sizeof(float)*2, newPt);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}

	void init_mesh()
	{

		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &MeshPosID);
		glBindBuffer(GL_ARRAY_BUFFER, MeshPosID);
		glm::vec3 *vertices = new glm::vec3[MESHSIZE * MESHSIZE * 6];
		for (int x = 0; x < MESHSIZE; x++)
		{
			for (int z = 0; z < MESHSIZE; z++)
			{
				vertices[x * 6 + z*MESHSIZE * 6 + 0] = vec3(0.0, 0.0, 0.0) + vec3(x, 0, z);//LD
				vertices[x * 6 + z*MESHSIZE * 6 + 1] = vec3(1.0, 0.0, 0.0) + vec3(x, 0, z);//RD
				vertices[x * 6 + z*MESHSIZE * 6 + 2] = vec3(1.0, 0.0, 1.0) + vec3(x, 0, z);//RU
				vertices[x * 6 + z*MESHSIZE * 6 + 3] = vec3(0.0, 0.0, 0.0) + vec3(x, 0, z);//LD
				vertices[x * 6 + z*MESHSIZE * 6 + 4] = vec3(1.0, 0.0, 1.0) + vec3(x, 0, z);//RU
				vertices[x * 6 + z*MESHSIZE * 6 + 5] = vec3(0.0, 0.0, 1.0) + vec3(x, 0, z);//LU

			}
	
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MESHSIZE * MESHSIZE * 6, vertices, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		delete[] vertices;
		//tex coords
		float t = 1. / MESHSIZE;
		vec2 *tex = new vec2[MESHSIZE * MESHSIZE * 6];
		for (int x = 0; x<MESHSIZE; x++)
			for (int y = 0; y < MESHSIZE; y++)
			{
				tex[x * 6 + y*MESHSIZE * 6 + 0] = vec2(0.0, 0.0)+ vec2(x, y)*t;	//LD
				tex[x * 6 + y*MESHSIZE * 6 + 1] = vec2(t, 0.0)+ vec2(x, y)*t;	//RD
				tex[x * 6 + y*MESHSIZE * 6 + 2] = vec2(t, t)+ vec2(x, y)*t;		//RU
				tex[x * 6 + y*MESHSIZE * 6 + 3] = vec2(0.0, 0.0) + vec2(x, y)*t;	//LD
				tex[x * 6 + y*MESHSIZE * 6 + 4] = vec2(t, t) + vec2(x, y)*t;		//RU
				tex[x * 6 + y*MESHSIZE * 6 + 5] = vec2(0.0, t)+ vec2(x, y)*t;	//LU
			}
		glGenBuffers(1, &MeshTexID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, MeshTexID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * MESHSIZE * MESHSIZE * 6, tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		delete[] tex;
		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLuint *elements = new GLuint[MESHSIZE * MESHSIZE * 8];
	//	GLuint ele[10000];
		int ind = 0,i=0;
		for (i = 0; i<(MESHSIZE * MESHSIZE * 8); i+=8, ind+=6)
			{
			elements[i + 0] = ind + 0;
			elements[i + 1] = ind + 1;
			elements[i + 2] = ind + 1;
			elements[i + 3] = ind + 2;
			elements[i + 4] = ind + 2;
			elements[i + 5] = ind + 5;
			elements[i + 6] = ind + 5;
			elements[i + 7] = ind + 0;
			}			
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*MESHSIZE * MESHSIZE * 8, elements, GL_STATIC_DRAW);
		delete[] elements;
		glBindVertexArray(0);
	}
	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom()
	{
		//initialize the net mesh
		init_mesh();

		string resourceDirectory = "../resources" ;
		// Initialize mesh.
		shape = make_shared<Shape>();
		//shape->loadMesh(resourceDirectory + "/t800.obj");
		shape->loadMesh(resourceDirectory + "/sphere.obj");
		shape->resize();
		shape->init();

		plane = make_shared<Shape>();
		plane->loadMesh(resourceDirectory + "/Millennium_Falcon.obj");
		plane->resize();
		plane->init();

		tieFighter = make_shared<Shape>();
		tieFighter->loadMesh(resourceDirectory + "/TIE-fighter.obj");
		tieFighter->resize();
		tieFighter->init();

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/sky.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		//texture 2
		str = resourceDirectory + "/sky1.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 3
		str = resourceDirectory + "/height.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &HeightTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, HeightTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		//[TWOTEXTURES]
		//set the 2 textures to the correct samplers in the fragment shader:
		GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(prog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(heightshader->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(heightshader->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(heightshader->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		Tex1Location = glGetUniformLocation(skyprog->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(skyprog->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(skyprog->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);
		
		Tex1Location = glGetUniformLocation(linesshader->pid, "tex");//tex, tex2... sampler in the fragment shader
		Tex2Location = glGetUniformLocation(linesshader->pid, "tex2");
		// Then bind the uniform samplers to texture units:
		glUseProgram(linesshader->pid);
		glUniform1i(Tex1Location, 0);
		glUniform1i(Tex2Location, 1);

		smoothrender1.init();
		smoothrender2.init();
		smoothrender3.init();
		
		line1.push_back(vec3(-0, -0, -2.28902));
		dir1.push_back(vec3(0, 0, 1));
		up1.push_back(vec3(0, 1, 0));
		line1.push_back(vec3(-0, -0.340523, -12.3407));
		dir1.push_back(vec3(-0.00477349, 0.104115, 0.994554));
		up1.push_back(vec3(-0.0455506, 0.99351, -0.104225));
		line1.push_back(vec3(-0, -1.39042, -22.3593));
		dir1.push_back(vec3(-0.0241183, 0.101396, 0.994554));
		up1.push_back(vec3(-0.230147, 0.967559, -0.104225));
		line1.push_back(vec3(-0.579117, -2.02516, -32.3701));
		dir1.push_back(vec3(0.106862, 0.0409384, 0.993431));
		up1.push_back(vec3(-0.234532, 0.971995, -0.0148268));
		line1.push_back(vec3(-2.37974, -1.53229, -42.2524));
		dir1.push_back(vec3(0.225344, -0.121174, 0.966714));
		up1.push_back(vec3(-0.200287, 0.965281, 0.167682));
		line1.push_back(vec3(-4.47776, -0.199377, -52.0102));
		dir1.push_back(vec3(0.297725, -0.167783, 0.939792));
		up1.push_back(vec3(-0.276544, 0.927067, 0.25312));
		line1.push_back(vec3(-7.52306, 1.58152, -61.3563));
		dir1.push_back(vec3(0.393819, -0.167783, 0.903745));
		up1.push_back(vec3(-0.248729, 0.927067, 0.2805));
		line1.push_back(vec3(-11.9761, 3.37414, -70.1476));
		dir1.push_back(vec3(0.54193, -0.178088, 0.821338));
		up1.push_back(vec3(0.101288, 0.984007, 0.146528));
		line1.push_back(vec3(-17.472, 4.39982, -78.4647));
		dir1.push_back(vec3(0.549589, 0.0917252, 0.830384));
		up1.push_back(vec3(-0.0439695, 0.995753, -0.0808908));
		line1.push_back(vec3(-23.0017, 3.47588, -86.8329));
		dir1.push_back(vec3(0.59059, 0.0769854, 0.803291));
		up1.push_back(vec3(0.40113, 0.83574, -0.375012));
		line1.push_back(vec3(-28.4402, 3.29607, -95.2089));
		dir1.push_back(vec3(0.465762, -0.00967095, 0.884857));
		up1.push_back(vec3(0.553346, 0.78351, -0.282702));
		line1.push_back(vec3(-32.5507, 4.24701, -104.239));
		dir1.push_back(vec3(0.188931, -0.149388, 0.970561));
		up1.push_back(vec3(0.638983, 0.769197, -0.00599122));
		line1.push_back(vec3(-35.3369, 7.21693, -113.416));
		dir1.push_back(vec3(0.193718, -0.389872, 0.900263));
		up1.push_back(vec3(0.44767, 0.851667, 0.272498));
		line1.push_back(vec3(-38.1489, 11.405, -122.113));
		dir1.push_back(vec3(0.113589, -0.399215, 0.909794));
		up1.push_back(vec3(0.383012, 0.862535, 0.330658));
		line1.push_back(vec3(-40.0243, 16.4322, -130.543));
		dir1.push_back(vec3(0.0971481, -0.553477, 0.827179));
		up1.push_back(vec3(0.212059, 0.823537, 0.526134));
		line1.push_back(vec3(-42.0696, 22.1989, -138.524));
		dir1.push_back(vec3(0.20534, -0.631439, 0.747743));
		up1.push_back(vec3(0.171951, 0.775416, 0.607588));
		line1.push_back(vec3(-44.0824, 28.4296, -146.167));
		dir1.push_back(vec3(0.216751, -0.579919, 0.785311));
		up1.push_back(vec3(0.0595381, 0.81079, 0.582301));
		line1.push_back(vec3(-45.5559, 34.2494, -154.167));
		dir1.push_back(vec3(0.177242, -0.579919, 0.79516));
		up1.push_back(vec3(0.0303694, 0.81079, 0.584548));
		line1.push_back(vec3(-47.0361, 40.4103, -161.977));
		dir1.push_back(vec3(0.218256, -0.660936, 0.718002));
		up1.push_back(vec3(0.0898394, 0.746224, 0.659606));
		line1.push_back(vec3(-48.6056, 47.5068, -168.84));
		dir1.push_back(vec3(0.24898, -0.7329, 0.63314));
		up1.push_back(vec3(0.0749536, 0.666348, 0.741864));
		line1.push_back(vec3(-50.0006, 55.3708, -174.939));
		dir1.push_back(vec3(0.188777, -0.851117, 0.489861));
		up1.push_back(vec3(0.146136, 0.517623, 0.843036));
		line1.push_back(vec3(-50.7779, 64.692, -178.338));
		dir1.push_back(vec3(0.100979, -0.993642, 0.0497828));
		up1.push_back(vec3(0.216324, 0.0707703, 0.973753));
		line1.push_back(vec3(-50.2674, 74.3356, -176.106));
		dir1.push_back(vec3(-0.0397053, -0.858118, -0.511915));
		up1.push_back(vec3(0.235407, -0.505932, 0.829829));
		line1.push_back(vec3(-48.6392, 80.9472, -168.987));
		dir1.push_back(vec3(-0.165696, -0.422535, -0.891072));
		up1.push_back(vec3(0.171865, -0.902107, 0.395809));
		line1.push_back(vec3(-46.4481, 82.2693, -159.406));
		dir1.push_back(vec3(-0.233995, 0.164695, -0.958187));
		up1.push_back(vec3(0.0473206, -0.98245, -0.180422));
		line1.push_back(vec3(-44.4768, 77.7718, -150.787));
		dir1.push_back(vec3(-0.219438, 0.693769, -0.685953));
		up1.push_back(vec3(-0.0940207, -0.714855, -0.692923));
		line1.push_back(vec3(-43.4248, 69.0526, -146.187));
		dir1.push_back(vec3(-0.127219, 0.977011, -0.171069));
		up1.push_back(vec3(-0.20201, -0.194377, -0.959901));
		line1.push_back(vec3(-43.663, 59.1947, -147.229));
		dir1.push_back(vec3(0.0100395, 0.914511, 0.404437));
		up1.push_back(vec3(-0.23852, 0.394973, -0.887189));
		line1.push_back(vec3(-45.2627, 51.7243, -153.44));
		dir1.push_back(vec3(0.21138, 0.521769, 0.826484));
		up1.push_back(vec3(-0.365091, 0.826535, -0.428426));
		line1.push_back(vec3(-48.9903, 46.9231, -161.344));
		dir1.push_back(vec3(0.380491, 0.510598, 0.771049));
		up1.push_back(vec3(-0.374537, 0.847403, -0.376337));
		line1.push_back(vec3(-53.4386, 41.7607, -168.667));
		dir1.push_back(vec3(0.638175, 0.463805, 0.614506));
		up1.push_back(vec3(0.0531222, 0.769743, -0.636139));
		line1.push_back(vec3(-56.9022, 37.426, -176.953));
		dir1.push_back(vec3(0.50205, 0.115868, 0.857042));
		up1.push_back(vec3(0.728876, 0.476703, -0.491419));
		line1.push_back(vec3(-57.1644, 35.0635, -186.524));
		dir1.push_back(vec3(-0.0612254, 0.115868, 0.991376));
		up1.push_back(vec3(0.879063, 0.476703, -0.00142613));
		line1.push_back(vec3(-54.752, 33.7358, -196.148));
		dir1.push_back(vec3(-0.207955, 0.00457363, 0.978128));
		up1.push_back(vec3(0.44054, 0.893262, 0.0894842));
		line1.push_back(vec3(-52.6833, 33.6839, -206.069));
		dir1.push_back(vec3(-0.153295, 0.0050686, 0.988167));
		up1.push_back(vec3(-0.139005, 0.989933, -0.0266416));
		line1.push_back(vec3(-51.1467, 33.6323, -216.022));
		dir1.push_back(vec3(-0.156015, 0.00375957, 0.987748));
		up1.push_back(vec3(-0.670109, 0.73427, -0.108638));
		line1.push_back(vec3(-49.6108, 33.5807, -225.97));
		dir1.push_back(vec3(-0.157517, 0.00112052, 0.987516));
		up1.push_back(vec3(-0.96354, 0.218845, -0.153941));
		line1.push_back(vec3(-48.0731, 33.5291, -235.93));
		dir1.push_back(vec3(-0.157271, -0.0019183, 0.987554));
		up1.push_back(vec3(-0.915512, -0.374657, -0.146526));
		line1.push_back(vec3(-46.5375, 33.4776, -245.876));
		dir1.push_back(vec3(-0.155365, -0.0042746, 0.987848));
		up1.push_back(vec3(-0.543215, -0.834858, -0.0890474));
		line1.push_back(vec3(-45.0017, 33.4261, -255.824));
		dir1.push_back(vec3(-0.152473, -0.00511895, 0.988294));
		up1.push_back(vec3(0.0215604, -0.999766, -0.00185204));
		line1.push_back(vec3(-43.4654, 33.3745, -265.775));
		dir1.push_back(vec3(-0.149618, -0.00415167, 0.988735));
		up1.push_back(vec3(0.57916, -0.81085, 0.0842355));
		line1.push_back(vec3(-41.1186, 33.3233, -275.466));
		dir1.push_back(vec3(-0.329104, -0.00173315, 0.944292));
		up1.push_back(vec3(0.888743, -0.338495, 0.309123));
		line1.push_back(vec3(-37.7616, 33.2276, -284.951));
		dir1.push_back(vec3(-0.263602, 0.0193749, 0.964437));
		up1.push_back(vec3(0.934056, 0.254852, 0.250179));
		line1.push_back(vec3(-35.6789, 32.1819, -294.64));
		dir1.push_back(vec3(0.0251061, 0.0795853, 0.996512));
		up1.push_back(vec3(0.653934, 0.752666, -0.076586));
		line1.push_back(vec3(-35.2404, 31.1232, -304.643));
		dir1.push_back(vec3(-0.0322102, 0.104537, 0.993999));
		up1.push_back(vec3(0.111874, 0.988643, -0.100349));
		line1.push_back(vec3(-34.8077, 29.5844, -314.513));
		dir1.push_back(vec3(-0.102956, 0.111173, 0.988454));
		up1.push_back(vec3(-0.46254, 0.874407, -0.146523));
		line1.push_back(vec3(-34.3731, 28.3227, -324.428));
		dir1.push_back(vec3(-0.155287, 0.0580948, 0.98616));
		up1.push_back(vec3(-0.874145, 0.456933, -0.164567));
		line1.push_back(vec3(-34.2874, 27.0533, -334.4));
		dir1.push_back(vec3(0.00257542, 0.0214114, 0.999767));
		up1.push_back(vec3(-0.985717, 0.168407, -0.00106744));
		line1.push_back(vec3(-37.9123, 25.792, -343.496));
		dir1.push_back(vec3(0.517512, 0.0214114, 0.855408));
		up1.push_back(vec3(-0.845245, 0.168407, 0.507149));
		line1.push_back(vec3(-46.0509, 24.5211, -349.034));
		dir1.push_back(vec3(0.911998, 0.0214114, 0.409636));
		up1.push_back(vec3(-0.407166, 0.168407, 0.897694));
		line1.push_back(vec3(-55.882, 23.252, -348.968));
		dir1.push_back(vec3(0.983332, 0.0214114, -0.180556));
		up1.push_back(vec3(0.174469, 0.168407, 0.970154));
		line1.push_back(vec3(-63.9418, 21.982, -343.328));
		dir1.push_back(vec3(0.706708, 0.0214114, -0.707182));
		up1.push_back(vec3(0.694687, 0.168407, 0.69932));
		line1.push_back(vec3(-67.3668, 20.7207, -334.176));
		dir1.push_back(vec3(0.18394, 0.0214114, -0.982704));
		up1.push_back(vec3(0.968221, 0.168407, 0.184898));
		line1.push_back(vec3(-64.9837, 19.634, -324.612));
		dir1.push_back(vec3(-0.469419, 0.00947515, -0.882925));
		up1.push_back(vec3(0.870941, 0.169498, -0.461229));
		line1.push_back(vec3(-57.6182, 19.947, -318.088));
		dir1.push_back(vec3(-0.952498, -0.0202304, -0.303873));
		up1.push_back(vec3(0.296336, 0.168553, -0.940093));
		line1.push_back(vec3(-47.855, 21.1458, -316.886));
		dir1.push_back(vec3(-0.95668, -0.0202304, 0.290437));
		up1.push_back(vec3(-0.289602, 0.168553, -0.942189));
		line1.push_back(vec3(-39.1742, 22.3379, -321.399));
		dir1.push_back(vec3(-0.625412, -0.0202304, 0.780032));
		up1.push_back(vec3(-0.771158, 0.168553, -0.613926));
		line1.push_back(vec3(-34.5477, 23.5373, -330.087));
		dir1.push_back(vec3(-0.0742523, -0.0313476, 0.996747));
		up1.push_back(vec3(-0.963197, 0.261178, -0.063539));
		line1.push_back(vec3(-34.486, 24.6199, -339.993));
		dir1.push_back(vec3(0.127046, -0.00530498, 0.991883));
		up1.push_back(vec3(-0.64195, 0.761874, 0.0862994));
		line1.push_back(vec3(-35.6857, 22.5204, -349.707));
		dir1.push_back(vec3(0.0704294, 0.411343, 0.908755));
		up1.push_back(vec3(-0.141365, 0.905939, -0.399112));
		line1.push_back(vec3(-36.7715, 17.8855, -358.499));
		dir1.push_back(vec3(0.174399, 0.501144, 0.847608));
		up1.push_back(vec3(0.0550756, 0.854488, -0.516544));
		line1.push_back(vec3(-37.8356, 12.9183, -367.115));
		dir1.push_back(vec3(0.169792, 0.435837, 0.883865));
		up1.push_back(vec3(0.067964, 0.889574, -0.451708));
		line1.push_back(vec3(-39.1725, 8.49009, -376.045));
		dir1.push_back(vec3(0.268395, 0.435837, 0.859076));
		up1.push_back(vec3(0.0165849, 0.889574, -0.456491));
		line1.push_back(vec3(-41.2715, 4.09, -384.777));
		dir1.push_back(vec3(0.443854, 0.368335, 0.816898));
		up1.push_back(vec3(0.37471, 0.751799, -0.542578));
		line1.push_back(vec3(-43.2384, 0.147687, -393.815));
		dir1.push_back(vec3(0.411873, 0.219043, 0.884523));
		up1.push_back(vec3(0.677774, 0.575173, -0.458037));
		line1.push_back(vec3(-43.161, -2.77774, -403.328));
		dir1.push_back(vec3(0.0128047, 0.13786, 0.990369));
		up1.push_back(vec3(0.870329, 0.486107, -0.0789188));
		line1.push_back(vec3(-39.9488, -6.19006, -412.122));
		dir1.push_back(vec3(-0.202273, 0.150244, 0.967736));
		up1.push_back(vec3(0.913722, 0.384548, 0.131281));
		line1.push_back(vec3(-32.999, -9.85299, -418.201));
		dir1.push_back(vec3(-0.71605, 0.150244, 0.681689));
		up1.push_back(vec3(0.677544, 0.384548, 0.626942));
		line1.push_back(vec3(-23.9083, -13.0458, -420.986));
		dir1.push_back(vec3(-0.933118, 0.0977875, 0.346017));
		up1.push_back(vec3(0.355147, 0.401109, 0.844383));
		line1.push_back(vec3(-14.1512, -15.324, -420.777));
		dir1.push_back(vec3(-0.997031, 0.0696852, -0.0327645));
		up1.push_back(vec3(0.0107222, 0.546984, 0.837075));
		line1.push_back(vec3(-4.38983, -16.3815, -418.639));
		dir1.push_back(vec3(-0.970461, -0.00221223, -0.241246));
		up1.push_back(vec3(-0.0921004, 0.927623, 0.361986));
		line1.push_back(vec3(5.31543, -16.3024, -416.236));
		dir1.push_back(vec3(-0.971192, -0.0799743, -0.224476));
		up1.push_back(vec3(-0.0333194, 0.978322, -0.204392));
		line1.push_back(vec3(14.9126, -14.3928, -413.859));
		dir1.push_back(vec3(-0.960096, -0.197306, -0.198204));
		up1.push_back(vec3(-0.150132, 0.961551, -0.229958));
		line1.push_back(vec3(24.4297, -12.3809, -411.503));
		dir1.push_back(vec3(-0.960096, -0.197306, -0.198204));
		up1.push_back(vec3(-0.150132, 0.961551, -0.229958));
		line1.push_back(vec3(33.9915, -10.3596, -409.135));
		dir1.push_back(vec3(-0.9547, -0.200374, -0.219997));
		up1.push_back(vec3(-0.176433, 0.976502, -0.123755));
		line1.push_back(vec3(43.5659, -8.33564, -406.764));
		dir1.push_back(vec3(-0.9547, -0.200374, -0.219997));
		up1.push_back(vec3(-0.176433, 0.976502, -0.123755));
		line1.push_back(vec3(53.1436, -6.31098, -404.392));
		dir1.push_back(vec3(-0.956122, -0.199815, -0.214253));
		up1.push_back(vec3(-0.169501, 0.973777, -0.151746));

		line2.push_back(vec3(-0, -1.07095, -4.65839));
		dir2.push_back(vec3(0, 0, 1));
		up2.push_back(vec3(0, 1, 0));
		line2.push_back(vec3(-0, -1.07095, -11.2548));
		dir2.push_back(vec3(0, 0.0043428, 0.999991));
		up2.push_back(vec3(0, 0.999991, -0.0043428));
		line2.push_back(vec3(-0, -1.07095, -21.26));
		dir2.push_back(vec3(-0.00968751, 0.115946, 0.993208));
		up2.push_back(vec3(-0.0826963, 0.98976, -0.11635));
		line2.push_back(vec3(-0.238431, -1.68073, -31.3576));
		dir2.push_back(vec3(0.0917861, 0.0119318, 0.995707));
		up2.push_back(vec3(-0.291646, 0.956402, 0.0154237));
		line2.push_back(vec3(-1.82366, -1.1561, -41.2801));
		dir2.push_back(vec3(0.199511, -0.0725427, 0.977207));
		up2.push_back(vec3(-0.369638, 0.91801, 0.143615));
		line2.push_back(vec3(-3.52715, -0.363203, -51.1684));
		dir2.push_back(vec3(0.182168, -0.077677, 0.980194));
		up2.push_back(vec3(-0.150163, 0.982983, 0.105806));
		line2.push_back(vec3(-4.76825, 0.429998, -61.1114));
		dir2.push_back(vec3(0.0158025, -0.0768693, 0.996916));
		up2.push_back(vec3(0.220495, 0.972763, 0.0715118));
		line2.push_back(vec3(-5.4443, 2.61035, -70.8001));
		dir2.push_back(vec3(0.327775, -0.251961, 0.910538));
		up2.push_back(vec3(-0.281376, 0.894009, 0.348677));
		line2.push_back(vec3(-8.69199, 3.77431, -80.2165));
		dir2.push_back(vec3(0.470219, -0.0470755, 0.881293));
		up2.push_back(vec3(-0.558165, 0.757639, 0.338282));
		line2.push_back(vec3(-14.8627, 4.39453, -87.8878));
		dir2.push_back(vec3(0.846286, -0.0470755, 0.530645));
		up2.push_back(vec3(-0.315952, 0.757639, 0.571102));
		line2.push_back(vec3(-23.6424, 5.01916, -92.7799));
		dir2.push_back(vec3(0.881957, -0.0606159, 0.467416));
		up2.push_back(vec3(-0.046738, 0.975561, 0.214702));
		line2.push_back(vec3(-32.0424, 5.6434, -98.2067));
		dir2.push_back(vec3(0.681545, -0.0573311, 0.729527));
		up2.push_back(vec3(0.315218, 0.922694, -0.221974));
		line2.push_back(vec3(-37.4087, 7.72769, -106.362));
		dir2.push_back(vec3(0.17103, -0.290737, 0.941393));
		up2.push_back(vec3(0.694191, 0.713592, 0.0942649));
		line2.push_back(vec3(-40.0659, 11.5012, -115.19));
		dir2.push_back(vec3(-0.126708, -0.290737, 0.948376));
		up2.push_back(vec3(0.631578, 0.713592, 0.303143));
		line2.push_back(vec3(-39.6255, 16.5393, -123.779));
		dir2.push_back(vec3(-0.628317, -0.379463, 0.679137));
		up2.push_back(vec3(0.418869, 0.570622, 0.706356));
		line2.push_back(vec3(-37.1787, 22.0894, -131.816));
		dir2.push_back(vec3(-0.352844, -0.492887, 0.795339));
		up2.push_back(vec3(0.0267276, 0.844353, 0.535119));
		line2.push_back(vec3(-34.452, 25.109, -140.975));
		dir2.push_back(vec3(-0.116942, -0.216869, 0.969171));
		up2.push_back(vec3(-0.41275, 0.898209, 0.151187));
		line2.push_back(vec3(-33.5566, 27.4722, -150.707));
		dir2.push_back(vec3(0.181003, -0.163368, 0.969819));
		up2.push_back(vec3(-0.694844, 0.676624, 0.243662));
		line2.push_back(vec3(-34.0387, 29.8354, -160.483));
		dir2.push_back(vec3(0.251342, -0.11828, 0.960644));
		up2.push_back(vec3(-0.826739, 0.48988, 0.276624));
		line2.push_back(vec3(-35.0883, 33.2453, -169.757));
		dir2.push_back(vec3(0.646513, -0.153955, 0.747207));
		up2.push_back(vec3(-0.67217, 0.348306, 0.653353));
		line2.push_back(vec3(-40.381, 37.2896, -177.053));
		dir2.push_back(vec3(0.955552, -0.153955, 0.251431));
		up2.push_back(vec3(-0.185647, 0.348306, 0.918814));
		line2.push_back(vec3(-48.9254, 41.3588, -180.086));
		dir2.push_back(vec3(0.929313, -0.153955, -0.335671));
		up2.push_back(vec3(0.368956, 0.348306, 0.861716));
		line2.push_back(vec3(-57.6305, 45.4017, -177.761));
		dir2.push_back(vec3(0.577443, -0.153955, -0.801784));
		up2.push_back(vec3(0.791086, 0.348306, 0.502858));
		line2.push_back(vec3(-63.5267, 49.472, -170.87));
		dir2.push_back(vec3(0.0198263, -0.153955, -0.987879));
		up2.push_back(vec3(0.936709, 0.348306, -0.0354818));
		line2.push_back(vec3(-64.4773, 53.5151, -161.91));
		dir2.push_back(vec3(-0.553268, -0.120718, -0.82421));
		up2.push_back(vec3(0.77973, 0.273112, -0.563411));
		line2.push_back(vec3(-60.1643, 57.415, -153.934));
		dir2.push_back(vec3(-0.904582, -0.106902, -0.412679));
		up2.push_back(vec3(0.371132, 0.27881, -0.885735));
		line2.push_back(vec3(-51.9893, 60.996, -149.714));
		dir2.push_back(vec3(-0.979574, -0.106902, 0.170315));
		up2.push_back(vec3(-0.193958, 0.27881, -0.940556));
		line2.push_back(vec3(-42.804, 64.5935, -150.875));
		dir2.push_back(vec3(-0.726741, -0.0998801, 0.679611));
		up2.push_back(vec3(-0.675774, 0.281401, -0.681281));
		line2.push_back(vec3(-35.5852, 65.8506, -157.335));
		dir2.push_back(vec3(-0.623119, 0.0398595, 0.781111));
		up2.push_back(vec3(-0.7393, 0.295929, -0.604865));
		line2.push_back(vec3(-33.29, 63.0538, -166.494));
		dir2.push_back(vec3(-0.329134, 0.118788, 0.936782));
		up2.push_back(vec3(-0.895976, 0.273957, -0.349535));
		line2.push_back(vec3(-35.2934, 57.641, -174.709));
		dir2.push_back(vec3(-0.178209, 0.365174, 0.913723));
		up2.push_back(vec3(-0.760885, 0.537663, -0.36328));
		line2.push_back(vec3(-37.9319, 52.0224, -182.53));
		dir2.push_back(vec3(0.272174, 0.542491, 0.794748));
		up2.push_back(vec3(-0.460198, 0.798734, -0.387609));
		line2.push_back(vec3(-42.7239, 45.6422, -188.579));
		dir2.push_back(vec3(0.572793, 0.713983, 0.402661));
		up2.push_back(vec3(-0.35307, 0.658226, -0.664892));
		line2.push_back(vec3(-47.1439, 38.2416, -193.776));
		dir2.push_back(vec3(0.858022, 0.488024, 0.160095));
		up2.push_back(vec3(-0.0901156, 0.449913, -0.888514));
		line2.push_back(vec3(-51.7398, 32.4148, -200.444));
		dir2.push_back(vec3(0.680195, 0.224842, 0.697697));
		up2.push_back(vec3(0.55461, 0.464507, -0.690392));
		line2.push_back(vec3(-53.0987, 28.7886, -209.545));
		dir2.push_back(vec3(0.140403, 0.14371, 0.979609));
		up2.push_back(vec3(0.796627, 0.571135, -0.197964));
		line2.push_back(vec3(-51.1093, 26.8079, -219.134));
		dir2.push_back(vec3(-0.12755, 0.123421, 0.984123));
		up2.push_back(vec3(0.506689, 0.86109, -0.0423205));
		line2.push_back(vec3(-49.8764, 27.1453, -229.058));
		dir2.push_back(vec3(-0.283901, -0.164393, 0.944656));
		up2.push_back(vec3(0.318833, 0.912949, 0.254695));
		line2.push_back(vec3(-46.5679, 28.9295, -238.393));
		dir2.push_back(vec3(-0.319581, -0.175727, 0.931122));
		up2.push_back(vec3(-0.180909, 0.975893, 0.122085));
		line2.push_back(vec3(-43.1568, 30.7018, -247.625));
		dir2.push_back(vec3(-0.416416, -0.157984, 0.895343));
		up2.push_back(vec3(0.356856, 0.877356, 0.32078));
		line2.push_back(vec3(-39.7524, 31.0301, -257.031));
		dir2.push_back(vec3(-0.19479, 0.0381376, 0.980103));
		up2.push_back(vec3(0.866568, 0.474784, 0.153751));
		line2.push_back(vec3(-40.1021, 30.2294, -266.844));
		dir2.push_back(vec3(0.401372, -0.00827578, 0.915878));
		up2.push_back(vec3(0.910693, -0.103027, -0.400031));
		line2.push_back(vec3(-43.7673, 29.4229, -276.191));
		dir2.push_back(vec3(0.424216, -0.048709, 0.90425));
		up2.push_back(vec3(0.706827, -0.60639, -0.364262));
		line2.push_back(vec3(-47.4691, 28.611, -285.596));
		dir2.push_back(vec3(0.387701, -0.0762897, 0.918623));
		up2.push_back(vec3(0.252245, -0.949749, -0.185334));
		line2.push_back(vec3(-51.1439, 27.8051, -294.932));
		dir2.push_back(vec3(0.343386, -0.0765976, 0.936066));
		up2.push_back(vec3(-0.299447, -0.953582, 0.031818));
		line2.push_back(vec3(-54.7959, 27.0042, -304.211));
		dir2.push_back(vec3(0.306922, -0.0500406, 0.950418));
		up2.push_back(vec3(-0.753394, -0.622968, 0.210496));
		line2.push_back(vec3(-58.419, 26.1982, -313.565));
		dir2.push_back(vec3(0.21422, -0.00569283, 0.976769));
		up2.push_back(vec3(-0.974415, -0.0708715, 0.213291));
		line2.push_back(vec3(-58.7955, 25.3975, -323.452));
		dir2.push_back(vec3(-0.143536, 0.0403972, 0.98882));
		up2.push_back(vec3(-0.852213, 0.502915, -0.144252));
		line2.push_back(vec3(-57.0153, 24.5914, -333.308));
		dir2.push_back(vec3(-0.322805, 0.0725152, 0.943683));
		up2.push_back(vec3(-0.380983, 0.902759, -0.199693));
		line2.push_back(vec3(-52.879, 23.7847, -342.445));
		dir2.push_back(vec3(-0.388303, 0.0795951, 0.918088));
		up2.push_back(vec3(-0.069469, 0.9909, -0.115289));
		line2.push_back(vec3(-52.0051, 22.984, -352.226));
		dir2.push_back(vec3(0.155441, 0.0607846, 0.985973));
		up2.push_back(vec3(-0.65133, 0.756723, 0.0560325));
		line2.push_back(vec3(-56.853, 22.1781, -360.835));
		dir2.push_back(vec3(0.67044, 0.0204388, 0.741682));
		up2.push_back(vec3(-0.720899, 0.254448, 0.644641));
		line2.push_back(vec3(-65.7086, 21.372, -365.284));
		dir2.push_back(vec3(0.94028, -0.0271502, 0.339317));
		up2.push_back(vec3(-0.328085, -0.338, 0.882109));
		line2.push_back(vec3(-75.5487, 20.5713, -365.412));
		dir2.push_back(vec3(0.993262, -0.00668492, -0.115698));
		up2.push_back(vec3(0.114746, -0.0832223, 0.989903));
		line2.push_back(vec3(-85.4703, 19.7648, -364.234));
		dir2.push_back(vec3(0.983397, 0.039839, 0.177041));
		up2.push_back(vec3(-0.173276, 0.495965, 0.850878));
		line2.push_back(vec3(-94.8769, 18.9589, -367.576));
		dir2.push_back(vec3(0.895717, 0.0722337, 0.438717));
		up2.push_back(vec3(-0.248751, 0.899256, 0.359808));
		line2.push_back(vec3(-103.872, 18.1527, -371.987));
		dir2.push_back(vec3(0.816028, 0.0790706, 0.572578));
		up2.push_back(vec3(0.0267586, 0.984369, -0.174073));
		line2.push_back(vec3(-109.767, 17.352, -379.839));
		dir2.push_back(vec3(0.390111, 0.0581444, 0.91893));
		up2.push_back(vec3(0.617431, 0.723854, -0.307918));
		line2.push_back(vec3(-110.743, 16.5458, -389.763));
		dir2.push_back(vec3(0.0121911, 0.0165836, 0.999788));
		up2.push_back(vec3(0.978336, 0.206454, -0.015354));
		line2.push_back(vec3(-107.202, 15.7448, -398.926));
		dir2.push_back(vec3(-0.554693, 0.0165836, 0.83189));
		up2.push_back(vec3(0.815974, 0.206454, 0.539965));
		line2.push_back(vec3(-99.0476, 14.9389, -404.506));
		dir2.push_back(vec3(-0.928975, 0.0165836, 0.369771));
		up2.push_back(vec3(0.365033, 0.206454, 0.907815));
		line2.push_back(vec3(-89.1617, 14.1376, -404.849));
		dir2.push_back(vec3(-0.999344, 0.0315459, -0.0178095));
		up2.push_back(vec3(-0.00399227, 0.392723, 0.919648));
		line2.push_back(vec3(-79.1741, 13.3319, -403.932));
		dir2.push_back(vec3(-0.996518, 0.067748, -0.0485974));
		up2.push_back(vec3(0.0311823, 0.843412, 0.536362));
		line2.push_back(vec3(-69.6225, 10.6403, -403.876));
		dir2.push_back(vec3(-0.923545, 0.359229, 0.134238));
		up2.push_back(vec3(0.349333, 0.932474, -0.0919719));
		line2.push_back(vec3(-60.878, 6.69103, -406.606));
		dir2.push_back(vec3(-0.874582, 0.464675, 0.138505));
		up2.push_back(vec3(0.377271, 0.831574, -0.407617));
		line2.push_back(vec3(-53.121, 1.32891, -410.089));
		dir2.push_back(vec3(-0.818884, 0.542106, 0.188548));
		up2.push_back(vec3(0.411204, 0.7833, -0.466211));
		line2.push_back(vec3(-45.4055, -4.05164, -413.653));
		dir2.push_back(vec3(-0.821849, 0.479017, 0.308394));
		up2.push_back(vec3(0.41633, 0.8745, -0.248837));
		line2.push_back(vec3(-36.6133, -7.75941, -416.566));
		dir2.push_back(vec3(-0.906395, 0.300127, 0.297276));
		up2.push_back(vec3(0.335821, 0.938849, 0.0760657));
		line2.push_back(vec3(-27.152, -10.8461, -418.365));
		dir2.push_back(vec3(-0.94469, 0.281052, 0.169027));
		up2.push_back(vec3(0.324064, 0.879179, 0.349322));
		line2.push_back(vec3(-17.4227, -13.3781, -418.594));
		dir2.push_back(vec3(-0.98158, 0.186518, 0.0413704));
		up2.push_back(vec3(0.183077, 0.856388, 0.482786));
		line2.push_back(vec3(-7.56332, -15.2047, -417.804));
		dir2.push_back(vec3(-0.990932, 0.121838, -0.0566572));
		up2.push_back(vec3(0.0786802, 0.867955, 0.490372));
		line2.push_back(vec3(2.31351, -16.2538, -416.195));
		dir2.push_back(vec3(-0.988035, 0.0642098, -0.140226));
		up2.push_back(vec3(-0.0236696, 0.835311, 0.549268));
		line2.push_back(vec3(12.0582, -16.4844, -414.034));
		dir2.push_back(vec3(-0.955852, -0.109979, -0.27249));
		up2.push_back(vec3(-0.178226, 0.954262, 0.24004));
		line2.push_back(vec3(21.4672, -14.6843, -411.236));
		dir2.push_back(vec3(-0.922131, -0.225247, -0.314546));
		up2.push_back(vec3(-0.212665, 0.9743, -0.0742443));
		line2.push_back(vec3(21.4672, -14.6843, -408.236));
		dir2.push_back(vec3(-0.922131, -0.225247, -0.314546));
		up2.push_back(vec3(-0.212665, 0.9743, -0.0742443));
		line2.push_back(vec3(21.4672, -14.6843, -404.236));
		dir2.push_back(vec3(-0.922131, -0.225247, -0.314546));
		up2.push_back(vec3(-0.212665, 0.9743, -0.0742443));
		line2.push_back(vec3(21.4672, -14.6843, -400.236));
		dir2.push_back(vec3(-0.922131, -0.225247, -0.314546));
		up2.push_back(vec3(-0.212665, 0.9743, -0.0742443));

		line3.push_back(vec3(-0, 9.63987, -0.61293));
		dir3.push_back(vec3(0, -0.161848, 0.986816));
		up3.push_back(vec3(0, 0.986816, 0.161848));
		line3.push_back(vec3(-0, 9.63987, -14.6206));
		dir3.push_back(vec3(0, -0.561251, 0.827646));
		up3.push_back(vec3(0, 0.827646, 0.561251));
		line3.push_back(vec3(-0, 9.63987, -23.2565));
		dir3.push_back(vec3(0, -0.419866, 0.907586));
		up3.push_back(vec3(0, 0.907586, 0.419866));
		line3.push_back(vec3(-0, 13.4705, -32.489));
		dir3.push_back(vec3(0, -0.358536, 0.933516));
		up3.push_back(vec3(0, 0.933516, 0.358536));
		line3.push_back(vec3(-0, 16.9375, -41.8675));
		dir3.push_back(vec3(0, -0.327249, 0.944938));
		up3.push_back(vec3(0, 0.944938, 0.327249));
		line3.push_back(vec3(-0.22516, 20.2324, -51.3744));
		dir3.push_back(vec3(0.0942818, -0.327249, 0.940223));
		up3.push_back(vec3(0.0326515, 0.944938, 0.325616));
		line3.push_back(vec3(-2.83362, 23.5279, -60.4819));
		dir3.push_back(vec3(0.378669, -0.327249, 0.865747));
		up3.push_back(vec3(0.13114, 0.944938, 0.299824));
		line3.push_back(vec3(-6.62293, 26.8026, -69.1454));
		dir3.push_back(vec3(0.378669, -0.327249, 0.865747));
		up3.push_back(vec3(0.13114, 0.944938, 0.299824));
		line3.push_back(vec3(-10.4351, 30.0764, -77.8611));
		dir3.push_back(vec3(0.382829, -0.295578, 0.875258));
		up3.push_back(vec3(0.118448, 0.955319, 0.270807));
		line3.push_back(vec3(-14.2918, 32.7653, -86.6828));
		dir3.push_back(vec3(0.377973, -0.239419, 0.894324));
		up3.push_back(vec3(0.0932047, 0.970916, 0.220532));
		line3.push_back(vec3(-17.5064, 34.9258, -95.9709));
		dir3.push_back(vec3(0.273824, -0.198654, 0.94104));
		up3.push_back(vec3(0.0555025, 0.98007, 0.190743));
		line3.push_back(vec3(-18.7876, 36.9259, -105.724));
		dir3.push_back(vec3(-0.0163173, -0.198654, 0.979934));
		up3.push_back(vec3(-0.00330741, 0.98007, 0.198627));
		line3.push_back(vec3(-16.5609, 38.9126, -115.148));
		dir3.push_back(vec3(-0.484028, -0.198654, 0.852205));
		up3.push_back(vec3(-0.0981095, 0.98007, 0.172737));
		line3.push_back(vec3(-10.1449, 40.9129, -122.573));
		dir3.push_back(vec3(-0.764991, -0.198654, 0.612638));
		up3.push_back(vec3(-0.155059, 0.98007, 0.124178));
		line3.push_back(vec3(-1.99574, 42.9131, -128.13));
		dir3.push_back(vec3(-0.837363, -0.198654, 0.509274));
		up3.push_back(vec3(-0.169729, 0.98007, 0.103227));
		line3.push_back(vec3(6.43692, 44.5166, -133.38));
		dir3.push_back(vec3(-0.795644, -0.137187, 0.590026));
		up3.push_back(vec3(-0.110194, 0.990545, 0.0817164));
		line3.push_back(vec3(12.7375, 45.8979, -140.928));
		dir3.push_back(vec3(-0.385183, -0.137187, 0.912586));
		up3.push_back(vec3(-0.0533465, 0.990545, 0.12639));
		line3.push_back(vec3(13.6713, 47.2794, -150.708));
		dir3.push_back(vec3(0.201477, -0.137187, 0.969839));
		up3.push_back(vec3(0.0279038, 0.990545, 0.134319));
		line3.push_back(vec3(8.88493, 48.6607, -159.286));
		dir3.push_back(vec3(0.716724, -0.137187, 0.68373));
		up3.push_back(vec3(0.0992636, 0.990545, 0.094694));
		line3.push_back(vec3(0.230618, 50.042, -164.058));
		dir3.push_back(vec3(0.95888, -0.137187, 0.248452));
		up3.push_back(vec3(0.132801, 0.990545, 0.0344097));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.951831, -0.295206, 0.0828978));
		up3.push_back(vec3(0.294093, 0.955434, 0.0256134));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.970269, -0.2268, 0.0845037));
		up3.push_back(vec3(0.225944, 0.973941, 0.0196782));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.970269, -0.2268, 0.0845037));
		up3.push_back(vec3(0.225944, 0.973941, 0.0196782));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.970269, -0.2268, 0.0845037));
		up3.push_back(vec3(0.225944, 0.973941, 0.0196782));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.994559, -0.0578764, 0.0866192));
		up3.push_back(vec3(0.0576582, 0.998324, 0.00502163));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.994559, -0.0578764, 0.0866192));
		up3.push_back(vec3(0.0576582, 0.998324, 0.00502163));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.950163, -0.0578764, 0.306336));
		up3.push_back(vec3(0.0550844, 0.998324, 0.0177594));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.846152, 0.0170858, 0.532668));
		up3.push_back(vec3(-0.0144593, 0.999854, -0.00910237));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.846152, 0.0170858, 0.532668));
		up3.push_back(vec3(-0.0144593, 0.999854, -0.00910237));
		line3.push_back(vec3(-2.65748, 50.4618, -164.692));
		dir3.push_back(vec3(0.783545, 0.0170858, 0.6211));
		up3.push_back(vec3(-0.0133894, 0.999854, -0.0106135));
		line3.push_back(vec3(-10.4334, 50.2921, -170.866));
		dir3.push_back(vec3(0.765073, 0.0170858, 0.643717));
		up3.push_back(vec3(-0.0130738, 0.999854, -0.011));
		line3.push_back(vec3(-17.5942, 50.1201, -177.934));
		dir3.push_back(vec3(0.691093, 0.0170858, 0.722564));
		up3.push_back(vec3(-0.0118096, 0.999854, -0.0123474));
		line3.push_back(vec3(-24.5514, 49.8932, -185.208));
		dir3.push_back(vec3(0.687995, 0.0960858, 0.719326));
		up3.push_back(vec3(-0.0664139, 0.995373, -0.0694383));
		line3.push_back(vec3(-31.422, 48.2939, -192.391));
		dir3.push_back(vec3(0.682035, 0.162251, 0.713094));
		up3.push_back(vec3(-0.112147, 0.98675, -0.117254));
		line3.push_back(vec3(-38.3048, 46.8622, -199.588));
		dir3.push_back(vec3(0.691167, 0.00884974, 0.722641));
		up3.push_back(vec3(-0.00611688, 0.999961, -0.00639544));
		line3.push_back(vec3(-45.1871, 47.7621, -206.783));
		dir3.push_back(vec3(0.687772, -0.0993804, 0.719092));
		up3.push_back(vec3(0.0686911, 0.99505, 0.0718192));
		line3.push_back(vec3(-52.1075, 48.7559, -213.931));
		dir3.push_back(vec3(0.745247, -0.0993804, 0.659341));
		up3.push_back(vec3(0.0744314, 0.99505, 0.0658516));
		line3.push_back(vec3(-59.7454, 49.7497, -220.309));
		dir3.push_back(vec3(0.764151, -0.0993804, 0.637335));
		up3.push_back(vec3(0.0763195, 0.99505, 0.0636538));
		line3.push_back(vec3(-67.8902, 50.7504, -226.123));
		dir3.push_back(vec3(0.830868, -0.0993804, 0.547523));
		up3.push_back(vec3(0.0829829, 0.99505, 0.0546838));
		line3.push_back(vec3(-75.4802, 51.7442, -232.529));
		dir3.push_back(vec3(0.725394, -0.0993804, 0.681122));
		up3.push_back(vec3(0.0724486, 0.99505, 0.068027));
		line3.push_back(vec3(-82.789, 52.6191, -239.392));
		dir3.push_back(vec3(0.728837, 0.0212919, 0.684356));
		up3.push_back(vec3(-0.0155219, 0.999773, -0.0145745));
		line3.push_back(vec3(-90.0715, 52.1604, -246.23));
		dir3.push_back(vec3(0.72822, 0.0463131, 0.683776));
		up3.push_back(vec3(-0.0337624, 0.998927, -0.0317019));
		line3.push_back(vec3(-97.3908, 51.694, -253.128));
		dir3.push_back(vec3(0.696158, 0.0463131, 0.716394));
		up3.push_back(vec3(-0.0322759, 0.998927, -0.0332141));
		line3.push_back(vec3(-103.86, 51.2276, -260.829));
		dir3.push_back(vec3(0.581232, 0.0463131, 0.812419));
		up3.push_back(vec3(-0.0269476, 0.998927, -0.0376661));
		line3.push_back(vec3(-109.148, 50.7614, -269.369));
		dir3.push_back(vec3(0.490124, 0.0463131, 0.870422));
		up3.push_back(vec3(-0.0227236, 0.998927, -0.0403553));
		line3.push_back(vec3(-114.05, 50.2982, -278.074));
		dir3.push_back(vec3(0.490124, 0.0463131, 0.870422));
		up3.push_back(vec3(-0.0227236, 0.998927, -0.0403553));
		line3.push_back(vec3(-116.623, 50.0249, -283.363));
		dir3.push_back(vec3(0.0586532, 0.0463131, 0.997204));
		up3.push_back(vec3(-0.00271933, 0.998927, -0.0462332));
		line3.push_back(vec3(-116.623, 50.0249, -283.363));
		dir3.push_back(vec3(-0.441803, 0.223575, 0.868806));
		up3.push_back(vec3(0.101342, 0.974687, -0.199288));
		line3.push_back(vec3(-114.59, 49.1554, -286.561));
		dir3.push_back(vec3(-0.522943, 0.223575, 0.822523));
		up3.push_back(vec3(0.119954, 0.974687, -0.188672));
		line3.push_back(vec3(-109.4, 46.9041, -294.889));
		dir3.push_back(vec3(-0.449125, 0.223575, 0.865044));
		up3.push_back(vec3(0.103021, 0.974687, -0.198425));
		line3.push_back(vec3(-104.901, 44.4716, -303.555));
		dir3.push_back(vec3(-0.435974, 0.323737, 0.839715));
		up3.push_back(vec3(0.149174, 0.946147, -0.28732));
		line3.push_back(vec3(-100.489, 41.3625, -312.052));
		dir3.push_back(vec3(-0.443406, 0.272073, 0.85403));
		up3.push_back(vec3(0.125368, 0.962277, -0.241467));
		line3.push_back(vec3(-95.6743, 38.6231, -320.453));
		dir3.push_back(vec3(-0.502746, 0.272073, 0.820502));
		up3.push_back(vec3(0.142146, 0.962277, -0.231987));
		line3.push_back(vec3(-92.5087, 35.9022, -329.408));
		dir3.push_back(vec3(-0.0478544, 0.272073, 0.961086));
		up3.push_back(vec3(0.0135303, 0.962277, -0.271736));
		line3.push_back(vec3(-94.9106, 33.1626, -338.643));
		dir3.push_back(vec3(0.506594, 0.272073, 0.818131));
		up3.push_back(vec3(-0.143234, 0.962277, -0.231317));
		line3.push_back(vec3(-101.684, 30.9468, -345.681));
		dir3.push_back(vec3(0.727758, 0.13708, 0.671995));
		up3.push_back(vec3(-0.100712, 0.99056, -0.0929947));
		line3.push_back(vec3(-108.772, 29.5665, -352.689));
		dir3.push_back(vec3(0.667726, 0.136283, 0.731826));
		up3.push_back(vec3(-0.0110537, 0.984806, -0.173309));
		line3.push_back(vec3(-113.506, 28.1862, -361.412));
		dir3.push_back(vec3(0.376427, 0.136283, 0.916368));
		up3.push_back(vec3(0.0490208, 0.984806, -0.166598));
		line3.push_back(vec3(-116.022, 26.8056, -371.042));
		dir3.push_back(vec3(0.138142, 0.136283, 0.980991));
		up3.push_back(vec3(0.0886941, 0.984806, -0.149303));
		line3.push_back(vec3(-115.553, 25.4257, -380.946));
		dir3.push_back(vec3(-0.182245, 0.136283, 0.973763));
		up3.push_back(vec3(0.131715, 0.984806, -0.113178));
		line3.push_back(vec3(-111.675, 24.0456, -389.993));
		dir3.push_back(vec3(-0.623525, 0.136283, 0.769833));
		up3.push_back(vec3(0.169696, 0.984806, -0.0368946));
		line3.push_back(vec3(-103.56, 22.1916, -395.248));
		dir3.push_back(vec3(-0.925261, 0.245953, 0.288788));
		up3.push_back(vec3(0.266416, 0.963287, 0.0331748));
		line3.push_back(vec3(-94.4792, 19.0687, -398.224));
		dir3.push_back(vec3(-0.862224, 0.344754, 0.371098));
		up3.push_back(vec3(0.359911, 0.932502, -0.0300725));
		line3.push_back(vec3(-85.83, 15.1278, -401.546));
		dir3.push_back(vec3(-0.843185, 0.39087, 0.369135));
		up3.push_back(vec3(0.4025, 0.914131, -0.0485582));
		line3.push_back(vec3(-77.246, 11.1961, -404.842));
		dir3.push_back(vec3(-0.843185, 0.39087, 0.369135));
		up3.push_back(vec3(0.4025, 0.914131, -0.0485582));
		line3.push_back(vec3(-68.6622, 7.26444, -408.138));
		dir3.push_back(vec3(-0.843185, 0.39087, 0.369135));
		up3.push_back(vec3(0.4025, 0.914131, -0.0485582));
		line3.push_back(vec3(-60.0202, 3.30612, -411.457));
		dir3.push_back(vec3(-0.843185, 0.39087, 0.369135));
		up3.push_back(vec3(0.4025, 0.914131, -0.0485582));
		line3.push_back(vec3(-51.3578, -0.625464, -414.521));
		dir3.push_back(vec3(-0.872208, 0.39087, 0.294065));
		up3.push_back(vec3(0.405203, 0.914131, -0.0132113));
		line3.push_back(vec3(-42.3159, -4.1811, -417.113));
		dir3.push_back(vec3(-0.912176, 0.2858, 0.29369));
		up3.push_back(vec3(0.304748, 0.952225, 0.0198773));
		line3.push_back(vec3(-32.811, -6.7517, -419.131));
		dir3.push_back(vec3(-0.976362, 0.161789, 0.143324));
		up3.push_back(vec3(0.188713, 0.961382, 0.200328));
		line3.push_back(vec3(-22.9604, -8.72569, -419.378));
		dir3.push_back(vec3(-0.968442, 0.24663, -0.0359697));
		up3.push_back(vec3(0.242391, 0.965572, 0.0944334));
		line3.push_back(vec3(-13.2485, -10.8583, -417.902));
		dir3.push_back(vec3(-0.96537, 0.165873, -0.201363));
		up3.push_back(vec3(0.169286, 0.985567, 0.000272013));
		line3.push_back(vec3(-3.42839, -11.9984, -416.444));
		dir3.push_back(vec3(-0.986602, 0.0914765, -0.135085));
		up3.push_back(vec3(0.088498, 0.995685, 0.0279044));
		line3.push_back(vec3(-3.42839, -11.9984, -415.444));
		dir3.push_back(vec3(-0.986602, 0.0914765, -0.135085));
		up3.push_back(vec3(0.088498, 0.995685, 0.0279044));
		line3.push_back(vec3(-3.42839, -11.9984, -414.444));
		dir3.push_back(vec3(-0.986602, 0.0914765, -0.135085));
		up3.push_back(vec3(0.088498, 0.995685, 0.0279044));
		line3.push_back(vec3(-3.42839, -11.9984, -413.444));
		dir3.push_back(vec3(-0.986602, 0.0914765, -0.135085));
		up3.push_back(vec3(0.088498, 0.995685, 0.0279044));
		line3.push_back(vec3(-3.42839, -11.9984, -412.444));
		dir3.push_back(vec3(-0.986602, 0.0914765, -0.135085));
		up3.push_back(vec3(0.088498, 0.995685, 0.0279044));

		linerender1.re_init_line(line1);
		cardinal_curve(cardinal1, line1, 121, 1.0);
		smoothrender1.re_init_line(cardinal1);

		linerender2.re_init_line(line2);
		cardinal_curve(cardinal2, line2, 121, 1.0);
		smoothrender2.re_init_line(cardinal2);

		linerender3.re_init_line(line3);
		cardinal_curve(cardinal3, line3, 121, 1.0);
		smoothrender3.re_init_line(cardinal3);
	
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	}
	
	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		

		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
		glEnable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		skyprog = std::make_shared<Program>();
		skyprog->setVerbose(true);
		skyprog->setShaderNames(resourceDirectory + "/sky_vertex.glsl", resourceDirectory + "/sky_fragment.glsl");
		if (!skyprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		skyprog->addUniform("P");
		skyprog->addUniform("V");
		skyprog->addUniform("M");
		skyprog->addAttribute("vertPos");
		skyprog->addAttribute("vertTex");

		// Initialize the GLSL program.
		prog = std::make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");

		// Initialize the GLSL program.
		heightshader = std::make_shared<Program>();
		heightshader->setVerbose(true);
		heightshader->setShaderNames(resourceDirectory + "/height_vertex.glsl", resourceDirectory + "/height_frag.glsl", resourceDirectory + "/geometry.glsl");
		if (!heightshader->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		heightshader->addUniform("P");
		heightshader->addUniform("V");
		heightshader->addUniform("M");
		heightshader->addUniform("camoff");
		heightshader->addUniform("campos");
		heightshader->addAttribute("vertPos");
		heightshader->addAttribute("vertTex");
		heightshader->addUniform("bgcolor");
		heightshader->addUniform("renderstate");

		// Initialize the GLSL program.
		linesshader = std::make_shared<Program>();
		linesshader->setVerbose(true);
		linesshader->setShaderNames(resourceDirectory + "/lines_height_vertex.glsl", resourceDirectory + "/lines_height_frag.glsl", resourceDirectory + "/lines_geometry.glsl");
		if (!linesshader->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		linesshader->addUniform("P");
		linesshader->addUniform("V");
		linesshader->addUniform("M");
		linesshader->addUniform("camoff");
		linesshader->addUniform("campos");
		linesshader->addAttribute("vertPos");
		linesshader->addAttribute("vertTex");
		linesshader->addUniform("bgcolor");

		pplane = std::make_shared<Program>();
		pplane->setVerbose(true);
		pplane->setShaderNames(resourceDirectory + "/plane_vertex.glsl", resourceDirectory + "/plane_frag.glsl");
		if (!pplane->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		pplane->addUniform("P");
		pplane->addUniform("V");
		pplane->addUniform("M");
		pplane->addUniform("campos");
		pplane->addAttribute("vertPos");
		pplane->addAttribute("vertNor");
		pplane->addAttribute("vertTex");
		
	}


	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/
	void render()
	{
		double frametime = get_last_elapsed_time();
		
	
		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClearColor(0.8f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now
		
		glm::mat4 V,Vi, M, P; //View, Model and Perspective matrix
		
		V = mycam.process(frametime);

		Vi = glm::inverse(V);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 10000.0f);		
		if (width < height)
			{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect,  1.0f / aspect, -2.0f, 10000.0f);
			}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.01f, 100000.0f); //so much type casting... GLM metods are quite funny ones


		
		// Draw the skybox --------------------------------------------------------------
		skyprog->bind();		
		static float w = 0.0;
		w += 1.0 * frametime;//rotation angle
		float trans = 0; // sin(t) * 2;
		w = 0.6;
		glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), w, glm::vec3(0.0f, 1.0f, 0.0f));
		float angle = 3.1415926 / 2.0;
		glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 TransZ = glm::translate(glm::mat4(1.0f), -mycam.pos);
		glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));
		M = TransZ *RotateY * RotateX * S;
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glActiveTexture(GL_TEXTURE0);
		if(renderstate==1)
			glBindTexture(GL_TEXTURE_2D, Texture);
		else if (renderstate == 2)
			glBindTexture(GL_TEXTURE_2D, Texture2);		
		glDisable(GL_DEPTH_TEST);
		shape->draw(prog,false);
		glEnable(GL_DEPTH_TEST);	



		//draw the dot
		//TransZ = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
		//S = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
		//M = Vi * TransZ * S;
		//glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, 0);
		//shape->draw(prog, false);

		static int counter;
		static double repeater;
		static double totalTime;

		totalTime += frametime;

		repeater += frametime;
		//repeater = clamp(repeater, 0.0, 1.0);

		if (repeater >= 1.0)
		{
			repeater = 0.0;
			counter++;
			vec3 dir, pos, up;
			mycam.get_dirpos(up, dir, pos);
			cout << endl;
			cout << "point position:" << -pos.x << "," << -pos.y << "," << -pos.z << endl;
			//cout << "Zbase:" << dir.x << "," << dir.y << "," << dir.z << endl;
			//cout << "Ybase:" << up.x << "," << up.y << "," << up.z << endl;
			cout << "point saved into file!" << endl << endl;
			file1 << "line1.push_back(vec3(" << -pos.x << "," << -pos.y << "," << -pos.z << "));" << endl;
			file1 << "dir1.push_back(vec3(" << dir.x << "," << dir.y << "," << dir.z << ")); " << endl;
			file1 << "up1.push_back(vec3(" << up.x << "," << up.y << "," << up.z << ")); " << endl;
			file2 << "line2.push_back(vec3(" << -pos.x << "," << -pos.y << "," << -pos.z << "));" << endl;
			file2 << "dir2.push_back(vec3(" << dir.x << "," << dir.y << "," << dir.z << ")); " << endl;
			file2 << "up2.push_back(vec3(" << up.x << "," << up.y << "," << up.z << ")); " << endl;
			file3 << "line3.push_back(vec3(" << -pos.x << "," << -pos.y << "," << -pos.z << "));" << endl;
			file3 << "dir3.push_back(vec3(" << dir.x << "," << dir.y << "," << dir.z << ")); " << endl;
			file3 << "up3.push_back(vec3(" << up.x << "," << up.y << "," << up.z << ")); " << endl;
		}

		int i, j;

		i = ((counter + (repeater/1.0)) * 120.0f);
		j = (counter + 1);

		i = i % cardinal1.size();
		j = j % up1.size();		

		milleniumFalconPos = cardinal1[i];
		vec3 milleniumFalconDir1, milleniumFalconDir2, milleniumFalconUp1, milleniumFalconUp2;

		tieFighterPos = cardinal2[i];
		vec3 tieFighterDir1, tieFighterDir2, tieFighterUp1, tieFighterUp2;


		mycam.pos = -cardinal3[i];
		vec3 midpoint = normalize((milleniumFalconPos) + (tieFighterPos));
		glm::mat4 T = glm::translate(glm::mat4(1), -cardinal3[i]);
		glm::mat4 myV = glm::lookAt(normalize(cardinal3[i]), midpoint, vec3(0,1,0));
		V = myV * T;

		milleniumFalconDir2 = dir1[j];
		if (j > 0)
		{
			milleniumFalconDir1 = dir1[j - 1];
		}
		else
		{
			milleniumFalconDir1 = dir1[j];
		}

		tieFighterDir2 = dir2[j];
		if (j > 0)
		{
			tieFighterDir1 = dir2[j - 1];
		}
		else
		{
			tieFighterDir1 = dir2[j];
		}

		milleniumFalconUp2 = up1[j];
		if (j > 1)
		{
			milleniumFalconUp1 = up1[j - 1];
		}
		else
		{
			milleniumFalconUp1 = up1[j];
		}

		tieFighterUp2 = up2[j];
		if (j > 1)
		{
			tieFighterUp1 = up2[j - 1];
		}
		else
		{
			tieFighterUp1 = up2[j];
		}

		milleniumFalconDir1 = normalize(milleniumFalconDir1);
		milleniumFalconDir2 = normalize(milleniumFalconDir2);
		milleniumFalconUp1 = normalize(milleniumFalconUp1);
		milleniumFalconUp2 = normalize(milleniumFalconUp2);

		tieFighterDir1 = normalize(tieFighterDir1);
		tieFighterDir2 = normalize(tieFighterDir2);
		tieFighterUp1 = normalize(tieFighterUp1);
		tieFighterUp2 = normalize(tieFighterUp2);
		
		mat4 m1, m2;
		quat q1, q2;
		vec3 ex, ey, ez;

		ey = milleniumFalconUp2;
		ez = milleniumFalconDir2; 
		ex = cross(milleniumFalconUp2, milleniumFalconDir2);

		ex = normalize(ex);

		m1[0][0] = ex.x;		m1[0][1] = ex.y;		m1[0][2] = ex.z;		m1[0][3] = 0;
		m1[1][0] = ey.x;		m1[1][1] = ey.y;		m1[1][2] = ey.z;		m1[1][3] = 0;
		m1[2][0] = ez.x;		m1[2][1] = ez.y;		m1[2][2] = ez.z;		m1[2][3] = 0;
		m1[3][0] = 0;			m1[3][1] = 0;			m1[3][2] = 0;			m1[3][3] = 1.0f;
		//m1 = transpose(m1);

		ey = milleniumFalconUp1;
		ez = milleniumFalconDir1;
		ex = cross(milleniumFalconUp1, milleniumFalconDir1);

		ex = normalize(ex);

		m2[0][0] = ex.x;		m2[0][1] = ex.y;		m2[0][2] = ex.z;		m2[0][3] = 0;
		m2[1][0] = ey.x;		m2[1][1] = ey.y;		m2[1][2] = ey.z;		m2[1][3] = 0;
		m2[2][0] = ez.x;		m2[2][1] = ez.y;		m2[2][2] = ez.z;		m2[2][3] = 0;
		m2[3][0] = 0;			m2[3][1] = 0;			m2[3][2] = 0;			m2[3][3] = 1.0f;
		//m2 = transpose(m2);

		q1 = quat(m1);
		q2 = quat(m2);

		float f = ((sin((repeater + 0.5) * 3.14159) + 1.) / 2.0);
		
		quat qt = slerp(q1, q2, f);
		mat4 mt = mat4(qt);

		// Draw the plane using GLSL.
		glm::mat4 TransPlane = glm::translate(glm::mat4(1.0f), milleniumFalconPos);
		glm::mat4 SPlane = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
		glm::mat4 RotateXPlane = glm::rotate(glm::mat4(1.0f), (-3.1415926f / 2.0f), vec3(1, 0, 0)); // glm::rotate(glm::mat4(1.0f), sangle, milleniumFalconDir1);
		glm::mat4 RotateYPlane = glm::rotate(glm::mat4(1.0f), (-3.1415926f / 1.0f), vec3(0, 1, 0)); // glm::rotate(glm::mat4(1.0f), sangle, milleniumFalconDir1);



		M = TransPlane * mt * RotateYPlane; // * RotateXPlane;

		pplane->bind();
		glUniformMatrix4fv(pplane->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pplane->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pplane->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pplane->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		plane->draw(pplane, false);			//render!!!!!!!
		pplane->unbind();
		//----------------------------------------------------------------------------------------------------------------
		ey = tieFighterUp2;
		ez = tieFighterDir2;
		ex = cross(tieFighterUp2, tieFighterDir2);

		ex = normalize(ex);

		m1[0][0] = ex.x;		m1[0][1] = ex.y;		m1[0][2] = ex.z;		m1[0][3] = 0;
		m1[1][0] = ey.x;		m1[1][1] = ey.y;		m1[1][2] = ey.z;		m1[1][3] = 0;
		m1[2][0] = ez.x;		m1[2][1] = ez.y;		m1[2][2] = ez.z;		m1[2][3] = 0;
		m1[3][0] = 0;			m1[3][1] = 0;			m1[3][2] = 0;			m1[3][3] = 1.0f;
		//m1 = transpose(m1);

		ey = tieFighterUp1;
		ez = tieFighterDir1;
		ex = cross(tieFighterUp1, tieFighterDir1);

		ex = normalize(ex);

		m2[0][0] = ex.x;		m2[0][1] = ex.y;		m2[0][2] = ex.z;		m2[0][3] = 0;
		m2[1][0] = ey.x;		m2[1][1] = ey.y;		m2[1][2] = ey.z;		m2[1][3] = 0;
		m2[2][0] = ez.x;		m2[2][1] = ez.y;		m2[2][2] = ez.z;		m2[2][3] = 0;
		m2[3][0] = 0;			m2[3][1] = 0;			m2[3][2] = 0;			m2[3][3] = 1.0f;
		//m2 = transpose(m2);

		q1 = quat(m1);
		q2 = quat(m2);

		f = ((sin((repeater + 0.5) * 3.14159) + 1.) / 2.0);

		qt = slerp(q1, q2, f);
		mt = mat4(qt);

		// Draw the plane using GLSL.
		TransPlane = glm::translate(glm::mat4(1.0f), tieFighterPos);
		SPlane = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
		RotateXPlane = glm::rotate(glm::mat4(1.0f), (-3.1415926f / 2.0f), vec3(1, 0, 0)); // glm::rotate(glm::mat4(1.0f), sangle, tieFighterDir1);
		RotateYPlane = glm::rotate(glm::mat4(1.0f), (-3.1415926f / 1.0f), vec3(0, 1, 0)); // glm::rotate(glm::mat4(1.0f), sangle, tieFighterDir1);



		M = TransPlane * mt * RotateYPlane; // * RotateXPlane;

		pplane->bind();
		glUniformMatrix4fv(pplane->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(pplane->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(pplane->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(pplane->getUniform("campos"), 1, &mycam.pos[0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		tieFighter->draw(pplane, false);			//render!!!!!!!
		pplane->unbind();



		// Draw the terrain --------------------------------------------------------------
		heightshader->bind();
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glm::mat4 TransY = glm::translate(glm::mat4(1.0f), glm::vec3(-500.0f, -9.0f, -500));
		M = TransY;
		glUniformMatrix4fv(heightshader->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(heightshader->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(heightshader->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		vec3 offset = mycam.pos;
		offset.y = 0;
		offset.x = (int)offset.x;
		offset.z = (int)offset.z;
		//	offset = vec3(0, 0, 0);
		vec3 bg = vec3(254. / 255., 225. / 255., 168. / 255.);
		if (renderstate == 2)
			bg = vec3(49. / 255., 88. / 255., 114. / 255.);
		glUniform3fv(heightshader->getUniform("camoff"), 1, &offset[0]);
		glUniform3fv(heightshader->getUniform("campos"), 1, &mycam.pos[0]);
		glUniform3fv(heightshader->getUniform("bgcolor"), 1, &bg[0]);
		glUniform1i(heightshader->getUniform("renderstate"), renderstate);
		glBindVertexArray(VertexArrayID);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, HeightTex);
		glDrawArrays(GL_TRIANGLES, 0, MESHSIZE*MESHSIZE * 6);			
		heightshader->unbind();	

		glm::vec3 linecolor = glm::vec3(1, 0, 0);
		linerender1.draw(P, V, linecolor);
		linerender2.draw(P, V, linecolor);
		linerender3.draw(P, V, linecolor);

		linecolor = glm::vec3(0, 1, 1);
		smoothrender1.draw(P, V, linecolor);
		smoothrender2.draw(P, V, linecolor);
		smoothrender3.draw(P, V, linecolor);

		
	}

};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}
	file1.open("pathinfo1.txt");
	if (!file1.is_open())
		{
		cout << "warning! could not open pathinfo.txt file!" << endl;
		}

	file2.open("pathinfo2.txt");
	if (!file1.is_open())
	{
		cout << "warning! could not open pathinfo.txt file!" << endl;
	}

	file3.open("pathinfo3.txt");
	if (!file1.is_open())
	{
		cout << "warning! could not open pathinfo.txt file!" << endl;
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(2560, 1440);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom();

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	file1.close();
	file2.close();
	file3.close();

	// Quit program.
	windowManager->shutdown();
	return 0;
}
