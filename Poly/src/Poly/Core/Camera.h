#pragma once

#include "polypch.h"

class Camera
{
public:
	Camera();
	~Camera();

	void update(float dt);

	glm::mat4 getView();
private:
	glm::quat quat;
};