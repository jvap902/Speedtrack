#pragma once

#include <vector>
#include <tuple>
#include <map>
#include <string>
#include "types.h"

// Forward declarations to avoid including huge headers if possible,
// but since we use SceneObject in the map, we need types.h (already included).

void AddStraight(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                 const std::map<std::string, SceneObject>& scene,
                 std::vector<AABB>& boxes,
                 int& bboxId,
                 TrackCursor& cursor,
                 std::vector<AABB>& barrierHulls,
                 bool barrier=false);

void AddTurnLeft(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                 const std::map<std::string, SceneObject>& scene,
                 std::vector<AABB>& boxes,
                 int& bboxId,
                 TrackCursor& cursor,
                 std::vector<AABB>& barrierHulls,
                 bool barrier=false);

void AddFinishLine(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                 const std::map<std::string, SceneObject>& scene,
                 std::vector<AABB>& boxes,
                 int& bboxId,
                 TrackCursor& cursor);

void BuildTrack(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                const std::map<std::string, SceneObject>& scene,
                std::vector<AABB>& boxes,
                int& bboxId,
                TrackCursor& cursor,
                std::vector<AABB>& barrierHulls,
                std::vector<glm::mat4>& wall_matrices);

void BuildBarrier(glm::vec3 position, float rotate, 
    std::vector<std::tuple<glm::mat4, const char*, int>>& g_TrackObjects,
    const std::map<std::string, SceneObject>& g_VirtualScene,
    std::vector<AABB>& g_CollisionBoxes,
    int& bbox_id_counter,
    std::vector<AABB>& barrierHulls);

bool floatEqual(float a, float b);