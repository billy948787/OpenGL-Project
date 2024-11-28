#ifndef HEADERS_H
#define HEADERS_H

#ifdef __APPLE__
// OpenGL and FreeGlut headers.
#include <GL/glew.h>
// include freeglut.h after glew.h
#include <GLFW/glfw3.h>
// GLM.
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#else
// OpenGL and FreeGlut headers.
#include <freeglut.h>
#include <glew.h>

// GLM.
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#endif

// C++ STL headers.
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#endif
