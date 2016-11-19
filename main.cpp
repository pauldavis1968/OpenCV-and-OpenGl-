
//force the application to compile with radians measure only.
#define GLM_FORCE_RADIANS

//#ifdef _WIN32
//#define GLEW_STATIC
//#endif

#include <stdio.h>
#include <stdlib.h>

//GLFW library
#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLM library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Common libraries
#include "shader.hpp"
#include "texture.hpp"
#include "controls.hpp"
#include "common.h"


//Global variables
GLFWwindow* g_window;

const int WINDOWS_WIDTH = 1024;
const int WINDOWS_HEIGHT = 768;

float aspect_ratio = 16/9.0f;
float z_offset = 0.0f;

float rotateY = 0.0f;
float rotateX = 0.0f;

//our vertices
static const GLfloat g_vertex_buffer_data[] = {
	-aspect_ratio,-1.0f,z_offset,
	aspect_ratio,-1.0f,z_offset,
	aspect_ratio,1.0f,z_offset,
	-aspect_ratio,-1.0f,z_offset,
	aspect_ratio,1.0f,z_offset,
	-aspect_ratio,1.0f,z_offset,
};

//UV map for the vertices
static const GLfloat g_uv_buffer_data[] = {
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f
};

// Template function to convert things to strings
template<class T>
std::string toString(const T& t)
{
	std::ostringstream stream;
	stream << t;
	return stream.str();
}

void calcFPS(GLFWwindow* window, uint & frame, double & t0, double & time)
{
	//get the current time
	time = glfwGetTime();

	// Calculate and display the FPS
	if ((time - t0) > 1.0 || frame == 0)
	{
		double fps = (double)frame / (time - t0);

		string windowTitle = "Webcam OpenCV and OpenGL Manipulation  | FPS: ";

		string tempFPS = toString(fps);

		windowTitle += tempFPS;

		const char* pWindowTitle = windowTitle.c_str();

		glfwSetWindowTitle(window, pWindowTitle);

		t0 = time;
		frame = 0;
	}
	frame++; // Increment our frame count
}


//========================================================================
// Handle key strokes
//========================================================================
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS && action != GLFW_REPEAT)
		return;
	switch (key)
	{
		case GLFW_KEY_ESCAPE:
			glfwSetWindowShouldClose(window, GL_TRUE);
			break;
		case GLFW_KEY_SPACE:
			rotateX=0;
			rotateY=0;
			break;
		case GLFW_KEY_Z:
			rotateX+=0.01;
			break;
		case GLFW_KEY_X:
			rotateX-=0.01;
			break;
		case GLFW_KEY_A:
			rotateY+=0.01;
			break;
		case GLFW_KEY_S:
			rotateY-=0.01;
			break;
		default:
			break;
	}
}

int main(int argc, char **argv)
{

	double time, t0;
	uint frame_t = 0;
	t0 = glfwGetTime();

	//Initialize GLFW
	if(!glfwInit()){
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(EXIT_FAILURE);
	}

	//enable anti-alising 4x with GLFW
	glfwWindowHint(GLFW_SAMPLES, 4);
	//specify the client API version that the created context must be compatible with.
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	//make GLFW forward compatible
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	//use the OpenGL Core (http://www.opengl.org/wiki/Core_And_Compatibility_in_Contexts)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create a GLFW windows object
	g_window = glfwCreateWindow(WINDOWS_WIDTH, (WINDOWS_HEIGHT), "Texture Mapping Demo", NULL, NULL);
	if(!g_window){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// Setup our viewport to be the entire size of the window
	glViewport(0, 0, (GLsizei)WINDOWS_WIDTH, (GLsizei)WINDOWS_HEIGHT);

	//make the context of the specified window current for the calling thread
	glfwMakeContextCurrent(g_window);
	glfwSwapInterval(1);

	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Final to Initialize GLEW\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	//keyboard input callback
	glfwSetInputMode(g_window,GLFW_STICKY_KEYS,GL_TRUE);
	glfwSetKeyCallback(g_window, key_callback);

	//Print keyboard shortcuts
    printf("Inputs: a - rotate left, s - rotate right, z - rotate up, x - rotate down, left - reduce FOV, right - increase FOV, up - move in, down - move out, space - reset rotation\n");

	//black background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

	//create and compile our GLSL program from the shaders
	GLuint program_id = LoadShaders( "transform.vert", "texture.frag", "geometry.geom");

	//Handle video input with OpenCV
	VideoCapture cap(0); // open the default camera
	Mat frame;
	if (!cap.isOpened()){  // check if we succeeded
		printf("Cannot open files\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}else{
		cap >> frame; // get a new frame from the camera
		printf("Got Video, %d x %d\n",frame.size().width, frame.size().height);
	}

	cv::Mat theFrame = cv::Mat(frame.size().height, frame.size().width, CV_8UC3, cv::Scalar(0, 255.0));

	cap >> frame; // get a new frame from the camera

	GLuint texture_id = initializeTexture(frame.data, frame.size().width, frame.size().height, GL_BGR);
	aspect_ratio = (float)frame.size().width/(float)frame.size().height;
	

	//get the location for our "MVP" uniform variable
	GLuint matrix_id = glGetUniformLocation(program_id, "MVP");

	//get the location for our "textureSampler" uniform variable
	GLuint texture_sampler_id  = glGetUniformLocation(program_id, "textureSampler");

	//get the location for the attribute variables
	GLint attribute_vertex, attribute_uv;

	attribute_vertex = glGetAttribLocation(program_id, "vertexPosition_modelspace");
	attribute_uv = glGetAttribLocation(program_id, "vertexUV");

	//generate the Vertex Array Object (Depedency: GLEW)
	GLuint vertex_array_id;
	glGenVertexArrays(1, &vertex_array_id);
	glBindVertexArray(vertex_array_id);

	//initialize the vertex buffer memory (similar to malloc)
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	//initialize the UV buffer memory
	GLuint uv_buffer;
	//generate the array (like malloc)
	glGenBuffers(1, &uv_buffer);
	//bind the array buffer to that UV
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	//use our shader
	glUseProgram(program_id);

	//bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glUniform1i(texture_sampler_id, 0);

	//1st attribute buffer : vertices for position
	glEnableVertexAttribArray(attribute_vertex);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(attribute_vertex, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	//2nd attribute buffer : UV mapping
	glEnableVertexAttribArray(attribute_uv);
	glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
	glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	GLsizei sw2 = WINDOWS_WIDTH;
	GLsizei sh2 = WINDOWS_HEIGHT;
	do{

		glViewport(0, 0, (GLsizei)WINDOWS_WIDTH, (GLsizei)WINDOWS_HEIGHT);
		//clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

		//compute the MVP matrix from keyboard and mouse input
		computeViewProjectionMatrices(g_window);

		//get the View and Model Matrix and apply to the rendering
		glm::mat4 projection_matrix = getProjectionMatrix();
		glm::mat4 view_matrix = getViewMatrix();
		glm::mat4 model_matrix = glm::mat4(1.0);
		model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateY, glm::vec3(0.0f, 1.0f, 0.0f));
		model_matrix = glm::rotate(model_matrix, glm::pi<float>() * rotateX, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 mvp = projection_matrix * view_matrix * model_matrix;

		//send our transformation to the currently bound shader,
		//in the "MVP" uniform variable
		glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &mvp[0][0]);

		cap >> frame; // get a new frame from camera
		//update the texture with the new frame
		updateTexture(frame.data, frame.size().width, frame.size().height, GL_BGR);

	//	glViewport(0, 0, sw2, sh2);
	//	glDrawArrays(GL_TRIANGLES, 0, 6); //draw the square
		glDrawArrays(GL_TRIANGLES, 0, 6); //draw the square
	//	glViewport(sw2, sh2, sw2, sh2);
	//	updateTexture(theFrame.data, theFrame.size().width, theFrame.size().height, GL_BGR);
	//	glViewport(sw2, 0, sw2, sh2);
	//	glDrawArrays(GL_TRIANGLES, 0, 6); //draw the square
	//	glViewport(0, sh2, sw2, sh2);
	//	glDrawArrays(GL_TRIANGLES, 0, 6); //draw the square
	//	glViewport(sw2, sh2, sw2, sh2);
	//	glDrawArrays(GL_TRIANGLES, 0, 6); //draw the square

		//swap buffers
		glfwSwapBuffers(g_window);
		glfwPollEvents();
		calcFPS(g_window,frame_t, t0, time);

	} // Check if the ESC key was pressed or the window was closed
	while(!glfwWindowShouldClose(g_window) && glfwGetKey(g_window, GLFW_KEY_ESCAPE )!=GLFW_PRESS);

	glDisableVertexAttribArray(attribute_vertex);
	glDisableVertexAttribArray(attribute_uv);

	// Clean up VBO and shader
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteBuffers(1, &uv_buffer);
	glDeleteProgram(program_id);
	glDeleteTextures(1, &texture_id);
	glDeleteVertexArrays(1, &vertex_array_id);

	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(g_window);
	glfwTerminate();

	exit(EXIT_SUCCESS);
}

