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
                 TrackCursor& cursor);

void AddTurnLeft(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                 const std::map<std::string, SceneObject>& scene,
                 std::vector<AABB>& boxes,
                 int& bboxId,
                 TrackCursor& cursor);

void BuildTrack(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                const std::map<std::string, SceneObject>& scene,
                std::vector<AABB>& boxes,
                int& bboxId,
                TrackCursor& cursor);