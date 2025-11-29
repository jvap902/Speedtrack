#include "track.h"
#include "collisions.h"
#include "matrices.h"
#include <cmath>
#include <cstdio>

// Helper to add pieces to the list
void AddStraight(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                 const std::map<std::string, SceneObject>& scene,
                 std::vector<AABB>& boxes,
                 int& bboxId,
                 TrackCursor& cursor)
{
    float rad = glm::radians(cursor.angleY);

    // Forward vector logic remains the same (0 deg = -Z)
    glm::vec3 forward = glm::vec3(sin(rad), 0.0f, -cos(rad));

    // Move cursor to CENTER of the piece to draw it
    glm::vec3 drawPos = cursor.position + (forward * (PIECE_LENGTH / 2.0f));

    // ########## FIX: Use -rad for Matrix Rotation ##########
    // This aligns the visual model (Local -Z) with the logical forward vector.
    glm::mat4 model = Matrix_Translate(drawPos.x, drawPos.y, drawPos.z)
                    * Matrix_Rotate_Y(-rad)
                    * Matrix_Identity();

    // Add to visual list
    objects.push_back(std::make_tuple(model, "the_reta", STRAIGHT));

    // Add to collision list
    BuildBBoxArray(scene, boxes, bboxId, "the_reta", model, STRAIGHT);

    // Update Cursor to the END of the piece
    cursor.position += (forward * PIECE_LENGTH);
}

void AddTurnLeft(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                 const std::map<std::string, SceneObject>& scene,
                 std::vector<AABB>& boxes,
                 int& bboxId,
                 TrackCursor& cursor)
{
    float rad = glm::radians(cursor.angleY);

    // ########## FIX: Use -rad for Matrix Rotation ##########
    glm::mat4 model = Matrix_Translate(cursor.position.x, cursor.position.y, cursor.position.z)
                    * Matrix_Rotate_Y(-rad);

    // Add to list
    objects.push_back(std::make_tuple(model, "the_turn", TURN));
    BuildBBoxArray(scene, boxes, bboxId, "the_turn", model, TURN);

    // --- UPDATE CURSOR LOGIC ---
    float R = TURN_RADIUS;

    glm::vec3 forward = glm::vec3(sin(rad), 0.0f, -cos(rad));
    glm::vec3 left    = glm::vec3(-cos(rad), 0.0f, -sin(rad));

    // Apply displacement: Move R Forward AND R Left
    cursor.position += (forward * R) + (left * R);

    // Rotate cursor 90 degrees LEFT for the next piece
    cursor.angleY -= 90.0f;
}

void BuildTrack(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                const std::map<std::string, SceneObject>& scene,
                std::vector<AABB>& boxes,
                int& bboxId,
                TrackCursor& cursor) {

    printf("Building Track...\n");

    // START LINE
    AddStraight(objects, scene, boxes, bboxId, cursor);
    AddStraight(objects, scene, boxes, bboxId, cursor);
    AddStraight(objects, scene, boxes, bboxId, cursor);

    // TURN 1
    AddTurnLeft(objects, scene, boxes, bboxId, cursor);

    // SIDE STRAIGHTS
    AddStraight(objects, scene, boxes, bboxId, cursor);
    AddStraight(objects, scene, boxes, bboxId, cursor);

    // TURN 2
    AddTurnLeft(objects, scene, boxes, bboxId, cursor);

    // BACK STRAIGHTS
    AddStraight(objects, scene, boxes, bboxId, cursor);
    AddStraight(objects, scene, boxes, bboxId, cursor);
    AddStraight(objects, scene, boxes, bboxId, cursor);
    AddStraight(objects, scene, boxes, bboxId, cursor);
    AddStraight(objects, scene, boxes, bboxId, cursor);

    // TURN 3
    AddTurnLeft(objects, scene, boxes, bboxId, cursor);

    // SIDE STRAIGHTS
    AddStraight(objects, scene, boxes, bboxId, cursor);
    AddStraight(objects, scene, boxes, bboxId, cursor);

    // TURN 4
    AddTurnLeft(objects, scene, boxes, bboxId, cursor);

    // FINISH LINE
    AddStraight(objects, scene, boxes, bboxId, cursor);
    AddStraight(objects, scene, boxes, bboxId, cursor);
    printf("Track built.\n");

}