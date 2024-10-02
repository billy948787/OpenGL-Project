// OpenGL and FreeGlut headers.
#include <GL/glew.h>
#include <GL/freeglut.h>

// GLM.
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// C++ STL headers.
#include <iomanip>
#include <iostream>
#include <vector>

// Bad idea, should be encapsulated with Class.
// Global variables.
GLuint pointVBO, circleVBO, triangleVBO, rectVBO;
GLuint rectIBO;

// Drawing Functions -------------------------------------------------------
void DrawOnePoint() {
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
  glDrawArrays(GL_POINTS, 0, 1);  // # vertices = 1.
  glDisableVertexAttribArray(0);
}

void DrawOneCircle() {
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
  glDrawArrays(GL_LINE_LOOP, 0, 36);  // # circle samples = 36.
  glDisableVertexAttribArray(0);
}

void DrawOneTriangle() {
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
  glDrawArrays(GL_TRIANGLES, 0, 3);  // # vertices = 3.
  glDisableVertexAttribArray(0);
}

void DrawOneRectangle() {
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectIBO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  glDisableVertexAttribArray(0);
}

void RenderSceneCB() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  DrawOnePoint();
  DrawOneCircle();
  DrawOneTriangle();
  DrawOneRectangle();

  glutSwapBuffers();
}

// RenderState Initialization ---------------------------------------------
void SetupRenderState() {
  glm::vec4 clearColor = glm::vec4(0.67f, 0.78f, 0.75f, 1.00f);
  glClearColor((GLclampf)(clearColor.r), (GLclampf)(clearColor.g),
               (GLclampf)(clearColor.b), (GLclampf)(clearColor.a));

  glPointSize(5.0f);
  glLineWidth(5.0f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

// Scene Creation Functions ---------------------------------------------
void CreateOnePoint() {
  // Create vertex data for a point.
  std::vector<glm::vec3> pointVertices;
  pointVertices.resize(1);
  pointVertices[0] = glm::vec3(0.0f, 0.0f, 0.0f);
  // Generate the vertex buffer.
  glGenBuffers(1, &pointVBO);
  glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 1, &pointVertices[0],
               GL_STATIC_DRAW);
}

void CreateOneCircle() {
  // Create vertex data for a circle.
  const int numCircleSamples = 36;
  std::vector<glm::vec3> circleVertices;
  circleVertices.resize(numCircleSamples);
  const float thetaOffset = 2.0f * M_PI / (float)numCircleSamples;
  float startTheta = 0.0f;
  float r = 0.25f;
  for (int i = 0; i < numCircleSamples; ++i) {
    float theta = startTheta + i * thetaOffset;
    circleVertices[i].x = r * std::cos(theta);  // x.
    circleVertices[i].y = r * std::sin(theta);  // y.
    circleVertices[i].z = 0.0f;                 // z.
  }
  // Generate the vertex buffer.
  glGenBuffers(1, &circleVBO);
  glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numCircleSamples,
               &circleVertices[0], GL_STATIC_DRAW);
}

void CreateOneTriangle() {
  // Create vertex data for a triangle.
  std::vector<glm::vec3> triangleVertices;
  triangleVertices.resize(3);
  triangleVertices[0] = glm::vec3(-0.7f, -0.7f, 0.0f);
  triangleVertices[1] = glm::vec3(0.0f, 0.7f, 0.0f);
  triangleVertices[2] = glm::vec3(0.7f, -0.7f, 0.0f);
  // Generate the vertex buffer.
  glGenBuffers(1, &triangleVBO);
  glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 3, &triangleVertices[0],
               GL_STATIC_DRAW);
}

void CreateOneRectangle() {
  // Create vertex data for a rectangle.
  std::vector<glm::vec3> rectVertices;
  rectVertices.resize(4);
  rectVertices[0] = glm::vec3(-0.9f, 0.9f, 0.0f);
  rectVertices[1] = glm::vec3(-0.9f, -0.9f, 0.0f);
  rectVertices[2] = glm::vec3(0.9f, 0.9f, 0.0f);
  rectVertices[3] = glm::vec3(0.9f, -0.9f, 0.0f);
  // Generate the vertex buffer.
  glGenBuffers(1, &rectVBO);
  glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * 4, &rectVertices[0],
               GL_STATIC_DRAW);
  // Create index data for a triangle.
  unsigned int vertexIndices[6] = {0, 1, 3, 0, 3, 2};
  // Generate the index buffer.
  glGenBuffers(1, &rectIBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectIBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertexIndices), vertexIndices,
               GL_STATIC_DRAW);
}

void SetupScene() {
  // Draw vertex array.
  CreateOnePoint();
  CreateOneCircle();
  CreateOneTriangle();
  CreateOneRectangle();
}

// Other Callback Functions ---------------------------------------------
void ReshapeCB(int w, int h) {
  // Adjust camera and projection here.
  // TODO.
}

void ProcessSpecialKeysCB(int key, int x, int y) {
  // Handle special (functional) keyboard inputs such as F1, spacebar, page up,
  // etc.
  switch (key) {
    case GLUT_KEY_F1:
      // Render with point mode.
      glPointSize(5);
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
      break;
    case GLUT_KEY_F2:
      // Render with line mode.
      glLineWidth(5);
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

void ProcessKeysCB(unsigned char key, int x, int y) {
  // Handle other keyboard inputs those are not defined as special keys.
  if (key == 27) {
    // Release memory allocation if needed.
    exit(0);
  }
}

// Main--------------------------------------------------------------------------
int main(int argc, char** argv) {
  // Setting window properties.
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  glutInitWindowSize(640, 360);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("OpenGL Renderer");

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
  SetupScene();

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
