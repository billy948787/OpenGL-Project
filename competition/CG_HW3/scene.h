#pragma once
#include "headers.h"
#include "light.h"
#include "scene_obj.h"
#include "camera.h"

struct Scene {
	std::vector<SceneObject> objects;
	std::vector<AreaLight*> areaLights;
	std::vector<DirectionalLight*> dirLights;
	std::vector<PointLight*> pointLights;
	std::vector<SpotLight*> spotLights;

	glm::vec3 ambientLight;

	Camera* camera;
};