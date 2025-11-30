#include "moving_objects.h"
#include <algorithm>

void SphereControl(MovingSphereState& sphere, CarState& car, float deltaTime) {
    // Parameters
    const float friction = 8.0f;

    float acceleration = 0.0f;

    // Update speed
    sphere.speed += acceleration * deltaTime;

    // Apply friction

    if (sphere.speed > 0)
    {
        sphere.speed -= friction * deltaTime;
        if (sphere.speed < 0) sphere.speed = 0;
    }
    else if (sphere.speed < 0)
    {
        sphere.speed += friction * deltaTime;
        if (sphere.speed > 0) sphere.speed = 0;
    }

    // Wrap angle
    if(sphere.angle >= 360.0f)
        sphere.angle -= 360.0f;
    else if(sphere.angle <= -360.0f)
        sphere.angle += 360.0f;

    // Local Movement (relative to sphere angle)
    float rad_angle = glm::radians(sphere.angle);
    glm::vec3 forward = glm::vec3(sin(rad_angle), 0.0f, cos(rad_angle));

    sphere.position.x += forward.x * sphere.speed * deltaTime;
    sphere.position.z += forward.z * sphere.speed * deltaTime;
}

void CarMovingSphere(const glm::mat4 sphere_model_matrix, const glm::mat4& car_model_matrix, CarState& car, glm::vec3 largestMtv, MovingSphereState& sphere)
{
    // Pushback
    car.position.x += largestMtv.x;
    car.position.y += largestMtv.y;
    car.position.z += largestMtv.z;

    sphere.speed = car.speed * 0.2;

    // Stop velocity
    car.speed = -car.speed * 0.2f;

}