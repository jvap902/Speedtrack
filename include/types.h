#pragma once

#include <glad/glad.h>
#include <string>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <tiny_obj_loader.h>
#include <stb_image.h>

// Object IDs
#define CAR    0
#define SPHERE1 1
#define PLANE  2
#define SPHERE2 3
#define BARRIER  4
#define STRAIGHT 5
#define RAMP 6
#define TURN 7
#define SPHEREBEZIER 8

// Constants
const float TRACK_Y = -0.99f;
const float PIECE_LENGTH = 10.0f;
const float TURN_RADIUS = 6.366197f;

#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

// --- Basic Structs ---
struct SceneObject {
    std::string  name;
    size_t       first_index;
    size_t       num_indices;
    GLenum       rendering_mode;
    GLuint       vertex_array_object_id;
    glm::vec3    bbox_min;
    glm::vec3    bbox_max;
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;
    int id;
    int objectId;
};

struct Sphere {
    glm::vec3 center;
    float radius;
    int id;
};

struct OBB {
    glm::vec3 center;
    glm::vec3 halfSize;
    glm::vec3 axis[3];
    int id;
};

struct TrackCursor {
    glm::vec3 position;
    float angleY;
};

// --- Logic Structs ---

// Input flags specifically for the Car physics logic
struct InputState {
    bool w = false, s = false, a = false, d = false;
    bool run_time = false;
    bool debug_mode = false;
    bool camera_mode = false;
};

struct CarState {
    glm::vec3 position;
    float angle;
    float speed;
};

struct MovingSphereState {
    glm::vec3 position;
    float angle;
    float speed;
    glm::vec3 direction;
};

// --- The "God Struct" for Callbacks ---
// This holds everything the Input callbacks need to modify.
struct GameState {
    // State Sub-structs
    InputState input;

    // Camera State
    float cameraTheta = 0.0f;
    float cameraPhi = 0.0f;
    float cameraDistance = 5.0f;

    // Window State
    float screenRatio = 1.0f;
    bool usePerspective = true;
    bool showInfoText = true;

    // Mouse State (Internal logic)
    bool mouseLeftPressed = false;
    bool mouseRightPressed = false;
    bool mouseMiddlePressed = false;
    double lastCursorX = 0.0;
    double lastCursorY = 0.0;

    float time = 0.0f;
};

struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true);
};