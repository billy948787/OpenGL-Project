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

#include <glew.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// GLM.
#include <glm.hpp>
#include <gtc/type_ptr.hpp>
#endif

// C++ STL headers.
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <vector>
// imgui headers
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#endif
