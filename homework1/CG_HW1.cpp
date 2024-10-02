﻿#ifdef _WIN32  // Windows OS
// OpenGL and FreeGlut headers.
#include <freeglut.h>
#include <glew.h>
// GLM.
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#endif

#ifdef __APPLE__  // MacOS
// OpenGL and FreeGlut headers.
#include <GL/glew.h>
// include freeglut.h after glew.h
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
// GLM.
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif

// C++ STL headers.
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <vector>

// My headers.
#include "TriangleMesh.h"

// Global variables.
const int screenWidth = 600;
const int screenHeight = 600;
TriangleMesh* mesh = nullptr;
std::string userInput = "";
std::string filePath = "Triangles.obj";
int menu;

// Function prototypes.
void SetupRenderState();
void SetupScene(const std::string&);
void ReleaseResources();
void RenderSceneCB();
void ReshapeCB(int, int);
void ProcessSpecialKeysCB(int, int, int);
void ProcessKeysCB(unsigned char, int, int);
void ProcessMenuEvents(int);
void CreateGLUTMenus();
void mouse(int, int, int, int);

// Callback function for glutDisplayFunc.
void RenderSceneCB() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render the triangle mesh.
  // Add your code here.
  // ...

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPTN), 0);
  glDrawElements(GL_TRIANGLES, mesh->vertexIndices.size(), GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);

  // Render the user input string
  glColor3f(1.0f, 1.0f, 1.0f);  // Set text color to white
  glRasterPos2f(-0.9f, 0.9f);   // Set text position
  for (char c : userInput) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
  }

  // Render the file path
  glColor3f(1.0f, 1.0f, 1.0f);  // Set text color to white
  glRasterPos2f(-0.9f, 0.8f);   // Set text position
  for (char c : "File: " + filePath) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
  }

  glutSwapBuffers();
}

// Callback function for glutReshapeFunc.
void ReshapeCB(int w, int h) {
  // Adjust camera and projection here.
  // Implemented in HW2.
}

// Callback function for glutSpecialFunc.
void ProcessSpecialKeysCB(int key, int x, int y) {
  // Handle special (functional) keyboard inputs such as F1, spacebar, page up,
  // etc.
  switch (key) {
    case GLUT_KEY_F1:
      // Render with point mode.
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
      break;
    case GLUT_KEY_F2:
      // Render with line mode.
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
    case GLUT_KEY_F3:
      // Render with fill mode.
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
    default:
      break;
  }
}

// Callback function for PopupMenu
void ProcessMenuEvents(int option) {
  switch (option) {
    case 0:
      // Reload the model
      ReleaseResources();
      SetupScene("TestModels_HW1/" + filePath);
      break;
    case 1:
      // Release memory allocation if needed.
      ReleaseResources();
      exit(0);
      break;
    default:
      break;
  }
}

// Create GLUT menus.
void CreateGLUTMenus() {
  menu = glutCreateMenu(ProcessMenuEvents);
  glutAddMenuEntry("Reload", 0);
  glutAddMenuEntry("Quit", 1);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

// Callback function for glutKeyboardFunc.
void ProcessKeysCB(unsigned char key, int x, int y) {
  // Handle other keyboard inputs those are not defined as special keys.
  if (key == 27) {
    // Release memory allocation if needed.
    ReleaseResources();
    exit(0);
  }
  if (key == 13) {
    // Enter key.
    filePath = userInput;
    userInput.clear();
  } else if (key == 8) {
    // Backspace key.
    if (!userInput.empty()) {
      userInput.pop_back();
    }
  } else {
    userInput.push_back(key);
  }

  glutPostRedisplay();
}

void ReleaseResources() {
  // Release memory if needed.
  // Add your code here.
  // ...
  delete mesh;
}

void SetupRenderState() {
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glm::vec4 clearColor = glm::vec4(0.44f, 0.57f, 0.75f, 1.00f);
  glClearColor((GLclampf)(clearColor.r), (GLclampf)(clearColor.g),
               (GLclampf)(clearColor.b), (GLclampf)(clearColor.a));
}

void mouse(int button, int state, int x, int y) {
  // if (state == GLUT_DOWN) {
  //   switch (button) {
  //     case GLUT_RIGHT_BUTTON:
  //       CreateGLUTMenus();
  //       break;
  //   }
  // }
}

// Load a model from obj file and apply transformation.
// You can alter the parameters for dynamically loading a model.
void SetupScene(const std::string& modelPath) {
  mesh = new TriangleMesh();
  mesh->LoadFromFile(modelPath);

  // Please DO NOT TOUCH the following code.
  // ------------------------------------------------------------------------
  // Build transformation matrices.
  // World.
  glm::mat4x4 M(1.0f);
  // Camera.
  glm::vec3 cameraPos = glm::vec3(0.0f, 0.5f, 2.0f);
  glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::mat4x4 V = glm::lookAt(cameraPos, cameraTarget, cameraUp);
  // Projection.
  float fov = 40.0f;
  float aspectRatio = (float)screenWidth / (float)screenHeight;
  float zNear = 0.1f;
  float zFar = 100.0f;
  glm::mat4x4 P = glm::perspective(glm::radians(fov), aspectRatio, zNear, zFar);

  // Apply CPU transformation.
  glm::mat4x4 MVP = P * V * M;

  mesh->ApplyTransformCPU(MVP);

  // Create and upload vertex/index buffers.
  mesh->CreateBuffers();
}

int main(int argc, char** argv) {
  // Setting window properties.
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(screenWidth, screenHeight);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("HW1: OBJ Loader");

  // Initialize GLEW.
  // Must be done after glut is initialized!
  GLenum res = glewInit();
  if (res != GLEW_OK) {
    std::cerr << "GLEW initialization error: " << glewGetErrorString(res)
              << std::endl;
    return 1;
  }

  // Initialization.
  SetupRenderState();
  SetupScene("TestModels_HW1/" + filePath);

  // Register callback functions.
  glutDisplayFunc(RenderSceneCB);
  glutIdleFunc(RenderSceneCB);
  glutReshapeFunc(ReshapeCB);
  glutSpecialFunc(ProcessSpecialKeysCB);
  glutKeyboardFunc(ProcessKeysCB);
  glutMouseFunc(mouse);

  // Create GLUT menus.
  CreateGLUTMenus();

  // Start rendering loop.
  glutMainLoop();

  return 0;
}
