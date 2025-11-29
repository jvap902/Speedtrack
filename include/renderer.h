#pragma once

#include <vector>
#include <tuple>
#include <map>
#include <stack>
#include <set>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>


#include "types.h"
#include "matrices.h"

// Container for Shader State (replaces global GLint g_model_uniform, etc.)
struct ShaderProgram {
    GLuint programId;
    GLint model_uniform;
    GLint view_uniform;
    GLint projection_uniform;
    GLint object_id_uniform;
    GLint bbox_min_uniform;
    GLint bbox_max_uniform;
};

// Matrix Stack
// Now accepts the stack as an argument
void PushMatrix(std::stack<glm::mat4>& stack, glm::mat4 M);
void PopMatrix(std::stack<glm::mat4>& stack, glm::mat4& M);

// Model Processing
// Now accepts the scene map as an argument
void BuildTrianglesAndAddToVirtualScene(std::map<std::string, SceneObject>& scene, ObjModel* model);
void ComputeNormals(ObjModel* model);

// Loaders
// Now accepts the ShaderProgram struct to fill
void LoadShadersFromFiles(ShaderProgram& shader);
void LoadTextureImage(const char* filename, GLuint& numLoadedTextures); // Pass counter by ref

// Low-level shader helpers
GLuint LoadShader_Vertex(const char* filename);
GLuint LoadShader_Fragment(const char* filename);
void LoadShader(const char* filename, GLuint shader_id);
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id);

// Debug
void PrintObjModelInfo(ObjModel*);

// Draw
// Now accepts scene and shader data
void DrawVirtualObject(const std::map<std::string, SceneObject>& scene, const char* object_name, const ShaderProgram& shader);

// Note: Debug drawing needs access to the scene to find "the_sphere" or "the_cube" models
void DrawDebugSphere(const Sphere& s, glm::mat4 view, glm::mat4 projection, const ShaderProgram& shader, const std::map<std::string, SceneObject>& scene);
void DrawDebugBox(const AABB& box, glm::mat4 view, glm::mat4 projection, const ShaderProgram& shader, const std::map<std::string, SceneObject>& scene);

void DrawAllObjects(const std::vector<std::tuple<glm::mat4, const char*, int>>& objects, const std::map<std::string, SceneObject>& scene, const ShaderProgram& shader);