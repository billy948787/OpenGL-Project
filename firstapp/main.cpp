#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>

void RenderSceneCB() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render something here.
  // TODO.

  glutSwapBuffers();
}

void ReshapeCB(int w, int h) {
  // Adjust camera and projection here.
  // TODO.
}

void ProcessSpecialKeysCB(int key, int x, int y) {
  // Handle special (functional) keyboard inputs such as F1, spacebar, page up,
  // etc.
  // TODO.
}

void ProcessKeysCB(unsigned char key, int x, int y) {
  // Handle other keyboard inputs those are not defined as special keys.
  if (key == 27) {
    // Release memory allocation if needed.
    exit(0);
  }
}

void SetupRenderState() {
  float clearColor[4] = {0.44f, 0.57f, 0.75f, 1.00f};
  glClearColor((GLclampf)(clearColor[0]), (GLclampf)(clearColor[1]),
               (GLclampf)(clearColor[2]), (GLclampf)(clearColor[3]));
}

int main(int argc, char** argv) {
  // Setting window properties.
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(640, 360);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("OpenGL Renderer");

  // Initialization.
  SetupRenderState();

  // Register callback functions.
  glutDisplayFunc(RenderSceneCB);
  glutIdleFunc(RenderSceneCB);
  glutReshapeFunc(ReshapeCB);
  glutSpecialFunc(ProcessSpecialKeysCB);
  glutKeyboardFunc(ProcessKeysCB);

  // Start rendering loop.
  glutMainLoop();

  return 0;
}
