#include "track.h"
#include "collisions.h"
#include "matrices.h"
#include <cmath>
#include <cstdio>

bool floatEqual(float a, float b){
    return fabs(a - b) < 0.01f;
}

// Helper to add pieces to the list
void AddStraight(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                 const std::map<std::string, SceneObject>& scene,
                 std::vector<AABB>& boxes,
                 int& bboxId,
                 TrackCursor& cursor,
                 std::vector<AABB>& barrierHulls)
{
    float rad = glm::radians(cursor.angleY);

    // Forward vector logic remains the same (0 deg = -Z)
    glm::vec3 forward = glm::vec3(sin(rad), 0.0f, -cos(rad));

    // Move cursor to CENTER of the piece to draw it
    glm::vec3 drawPos = cursor.position + (forward * (PIECE_LENGTH / 2.0f));

    // This aligns the visual model (Local -Z) with the logical forward vector.
    glm::mat4 model = Matrix_Translate(drawPos.x, drawPos.y, drawPos.z)
                    * Matrix_Rotate_Y(-rad)
                    * Matrix_Identity();

    // Add to visual list
    objects.push_back(std::make_tuple(model, "the_reta", STRAIGHT));

    // Add to collision list
    BuildBBoxArray(scene, boxes, bboxId, "the_reta", model, STRAIGHT);

    // Barreiras
    float radPiece = glm::radians(cursor.angleY);

    // barrier = piece direction + 90 degrees
    float barrierAngle = radPiece + glm::radians(90.0f);

    // normalize to [0, 2π)
    barrierAngle = fmod(barrierAngle + 2.0f*M_PI, 2.0f*M_PI);

    glm::vec3 barrierDraw = drawPos;
    
    if(floatEqual(barrierAngle, glm::radians(90.0f)) || floatEqual(barrierAngle, glm::radians(270.0f))){ // se for positivo, é igual a giro de 90 graus da barreira
        barrierDraw.x -= 5.0f;
        for (int i=0; i<2; i++){
            barrierDraw.z = drawPos.z - 5.0f;
            for (int j=0; j<6; j++){
                BuildBarrier(barrierDraw, barrierAngle, objects, scene, boxes, bboxId, barrierHulls);
                barrierDraw.z += 2.0f;
            }    
            barrierDraw.x = drawPos.x + 5.0f;
        }
    }
    else{
        barrierDraw.z -= 5.0f;
        for (int i=0; i<2; i++){
            barrierDraw.x =  drawPos.x - 5.0f;
            for (int j=0; j<6; j++){
                BuildBarrier(barrierDraw, barrierAngle, objects, scene, boxes, bboxId, barrierHulls);
                barrierDraw.x += 2.0f;
            }    
            barrierDraw.z = drawPos.z + 5.0f;
        }
    }

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

void BuildBarrier(glm::vec3 position, float rotate, 
    std::vector<std::tuple<glm::mat4, const char*, int>>& g_TrackObjects,
    const std::map<std::string, SceneObject>& g_VirtualScene,
    std::vector<AABB>& g_CollisionBoxes,
    int& bbox_id_counter,
    std::vector<AABB>& barrierHulls){

    // Adiciona barreira à cena
    glm::mat4 barrier_model_matrix = Matrix_Translate(position.x, position.y, position.z) 
        * Matrix_Scale(1.0f, 1.0f, 1.0f)
        * Matrix_Rotate_Y(rotate);
    g_TrackObjects.push_back(std::make_tuple(barrier_model_matrix, "concrete_road_barrier", BARRIER));
    BuildBBoxArray(g_VirtualScene, g_CollisionBoxes, bbox_id_counter, "concrete_road_barrier", barrier_model_matrix, BARRIER);

    barrierHulls.push_back(g_CollisionBoxes[bbox_id_counter-1]);
}

void BuildTrack(std::vector<std::tuple<glm::mat4, const char*, int>>& objects,
                const std::map<std::string, SceneObject>& scene,
                std::vector<AABB>& boxes,
                int& bboxId,
                TrackCursor& cursor,
                std::vector<AABB>& barrierHulls) {

    printf("Building Track...\n");

    // START LINE
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);

    // TURN 1
    AddTurnLeft(objects, scene, boxes, bboxId, cursor);

    // SIDE STRAIGHTS
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);

    // TURN 2
    AddTurnLeft(objects, scene, boxes, bboxId, cursor);

    // BACK STRAIGHTS
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);

    // TURN 3
    AddTurnLeft(objects, scene, boxes, bboxId, cursor);

    // SIDE STRAIGHTS
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);

    // TURN 4
    AddTurnLeft(objects, scene, boxes, bboxId, cursor);

    // FINISH LINE
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    AddStraight(objects, scene, boxes, bboxId, cursor, barrierHulls);
    printf("Track built.\n");

}