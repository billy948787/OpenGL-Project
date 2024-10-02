#include <GL/freeglut.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <iostream>

void SetupRenderState() {}

void SetupScene() {}

int main(int argc, char** argv) {
  // Setting window properties.
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(640, 360);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("OpenGL Practice");

  // Initialize GLEW
  GLenum err = glewInit();
  if (GLEW_OK != err) {
    std::cerr << "GLEW initialization error: " << glewGetErrorString(err)
              << std::endl;
    return 1;
  }

  // Initialize
  SetupRenderState();
}