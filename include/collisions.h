#pragma once

#include <vector>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <algorithm>
#include <cmath>
#include "types.h"


std::set<std::pair<int, int>> SweepAndPrune(const std::vector<AABB>& boxes);
bool RaySphere(const glm::vec3& rayOrigin, const glm::vec3& rayDir, const Sphere& sphere, float & t_hit);
Sphere BoundingSphere(const ObjModel& model, int id);
bool SSCollision(const Sphere& s1, const Sphere& s2);
std::vector<OBB> BuildCompoundHitbox(const ObjModel& model, const glm::mat4& transform, int id);
bool CHitboxSphereCollision(const OBB& box, const Sphere& sphere, glm::vec3& mtv);

// REFACTORED: Now accepts the scene map, the boxes vector, and the ID counter
void BuildBBoxArray(const std::map<std::string, SceneObject>& virtualScene, std::vector<AABB>& boxes, int& bboxId, const std::string& name, const glm::mat4& modelMatrix, int objectId);
// REFACTORED: Removed debug drawing (Renderer dependency) and fixed pass-by-value
bool SphereSphereCollision(const ObjModel& obj1, const ObjModel& obj2, glm::mat4 object1_model, int object1_id, glm::mat4 object2_model, int object2_id, float object1_uniformScale, float object2_UniformScale);
OBB TransformOBB(const OBB& localBox, const glm::mat4& transform);
bool AabbObbCollision(const AABB& aabb, const OBB& obb, glm::vec3& mtv);