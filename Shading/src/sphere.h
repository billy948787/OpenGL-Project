#ifndef SPHERE_H
#define SPHERE_H

#include "headers.h"

// VertexPN Declarations.
struct VertexPN
{
	VertexPN() {
		position = glm::vec3(0.0f, 0.0f, 0.0f);
		normal = glm::vec3(0.0f, 1.0f, 0.0f);
	}
	VertexPN(glm::vec3 p, glm::vec3 n) {
		position = p;
		normal = n;
	}
	glm::vec3 position;
	glm::vec3 normal;
};


// Sphere Declarations.
class Sphere
{
public:
	// Sphere Public Methods.
	Sphere(const int nSlices, const int nStacks, const float radius);
	~Sphere();
	void Render();
	
private:
	// Sphere Private Methods.
	static void CreateSphere3D(const int nSlices, const int nStacks, const float radius, 
					std::vector<VertexPN>& vertices, std::vector<unsigned int>& indices);

	// Sphere Private Data.
	GLuint vboId;
	GLuint iboId;
	std::vector<VertexPN> vertices;
	std::vector<unsigned int> indices;
};

#endif

