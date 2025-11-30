#include "moving_objects.h"
#include <algorithm>

void SphereControl(MovingSphereState& sphere, float deltaTime)
{
    const float friction = 2.0f;      // deceleration per second
    const float EPS_SPEED = 1e-4f;
    const float sphereRadius = 1.0f;  // use the same logical radius you render with

    // Apply friction
    if (sphere.speed > EPS_SPEED)
    {
        sphere.speed -= friction * deltaTime;
        if (sphere.speed < 0.0f) sphere.speed = 0.0f;
    }
    else if (sphere.speed < -EPS_SPEED)
    {
        sphere.speed += friction * deltaTime;
        if (sphere.speed > 0.0f) sphere.speed = 0.0f;
    }
    else
    {
        sphere.speed = 0.0f;
    }

    // Nothing to do if not moving
    if (sphere.speed <= 0.0f) return;

    // Move sphere along its direction
    sphere.position += sphere.direction * sphere.speed * deltaTime;

    // Update rolling angle (distance/radius -> radians, convert to degrees because you use degrees)
    float distance = sphere.speed * deltaTime;
    float deltaAngleRad = distance / sphereRadius;
    sphere.angle += glm::degrees(deltaAngleRad);

    // stabilize angle
    if (sphere.angle > 360.0f) sphere.angle = fmod(sphere.angle, 360.0f);
    if (sphere.angle < 0.0f)   sphere.angle = fmod(sphere.angle, 360.0f) + 360.0f;
}



void CarMovingSphere(const glm::mat4 sphere_model_matrix, const glm::mat4& car_model_matrix, CarState& car, glm::vec3 largestMtv, MovingSphereState& sphere)
{
    // --- 1) Separate objects immediately so they aren't overlapping anymore ---
    // We will push the car a bit (as before) and also nudge the sphere the opposite way.
    // Split the correction so both move out of penetration (tweak factors if needed).
    const float carPushFactor = 0.65f;
    const float spherePushFactor = 0.35f;

    glm::vec3 carPush = largestMtv * carPushFactor;
    glm::vec3 spherePush = -largestMtv * spherePushFactor; // push sphere opposite to car push

    car.position += carPush;
    sphere.position += spherePush;

    // --- 2) Compute horizontal push direction for the sphere.
    // The collision MTV (largestMtv) is the vector that pushes the car out.
    // The sphere should be pushed in the opposite horizontal direction.
    glm::vec3 horizontal = glm::vec3(-largestMtv.x, 0.0f, -largestMtv.z); // NOTE the negation

    // Fallback: if horizontal is tiny (vertical hit), use car forward vector
    if (glm::length(horizontal) < 1e-4f)
    {
        glm::vec3 carForward = glm::normalize(glm::vec3(car_model_matrix * glm::vec4(0,0,-1,0)));
        horizontal = glm::vec3(carForward.x, 0.0f, carForward.z);
    }

    if (glm::length(horizontal) > 1e-5f)
        sphere.direction = glm::normalize(horizontal);
    else
        sphere.direction = glm::vec3(0.0f, 0.0f, 1.0f); // safe default

    // --- 3) Transfer speed from car to sphere (with a minimum)
    float transferred = glm::abs(car.speed) * 0.4f;
    const float MIN_PUSH_SPEED = 1.0f; // tune to taste
    sphere.speed = glm::max(transferred, MIN_PUSH_SPEED);

    // --- 4) Damp car speed (impulse)
    car.speed *= -0.25f;
}

void SphereSphereBounce(const glm::mat4& moving_model, float moving_scale, int moving_id, const glm::mat4& static_model, float static_scale, int static_id, MovingSphereState& movingSphere, const Sphere& localHull)
{
    
}
