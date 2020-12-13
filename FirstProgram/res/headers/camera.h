#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Pos_Update_Dir
{
	FORWARD, BACKWARD, LEFT, RIGHT
};

class Camera 
{

public:
	glm::vec3 camera_pos, camera_dir, world_up;
	float camera_speed = 0.05f, pitch = 0.0f, yaw = -90.0f, sensitivity = 0.1f;

	Camera(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f))
	{
		camera_pos = position;
		camera_dir = direction;
		world_up = up;
	}

	void UpdatePos(Pos_Update_Dir direction, float delta_frametime, float multiplier)
	{
		camera_speed = delta_frametime * multiplier;
		switch (direction)
		{
			case FORWARD:  
				camera_pos += camera_dir * camera_speed;
				break;
			case BACKWARD: 
				camera_pos -= camera_dir * camera_speed;
				break;
			case LEFT:
				camera_pos += glm::normalize(glm::cross(world_up, camera_dir)) * camera_speed;
				break;
			case RIGHT:	   
				camera_pos -= glm::normalize(glm::cross(world_up, camera_dir)) * camera_speed;
		}
	}

	void UpdateDir(float delta_xpos, float delta_ypos)
	{
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		yaw += delta_xpos * sensitivity;
		pitch += delta_ypos * sensitivity;

		UpdateVectors();
	}

	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(camera_pos, camera_pos + camera_dir, world_up);
	}

	glm::mat4 GetMirroredViewMatrix()
	{
		glm::vec3 mirrored_camera_pos = glm::vec3(camera_pos.x, camera_pos.y, -30-camera_pos.z), mirrored_camera_dir = glm::vec3(camera_dir.x, camera_dir.y, -camera_dir.z);
		return glm::lookAt(mirrored_camera_pos, mirrored_camera_pos + mirrored_camera_dir, world_up);
	}

	void UpdateVectors()
	{
		camera_dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		camera_dir.y = sin(glm::radians(pitch));
		camera_dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	}

private:

};

#endif