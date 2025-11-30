//     Universidade Federal do Rio Grande do Sul
//             Instituto de Informática
//       Departamento de Informática Aplicada
//
//    INF01047 Fundamentos de Computação Gráfica
//               Prof. Eduardo Gastal
//
//                   (Refatorado)
//

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>
#include <stack>
#include <string>
#include <tuple>

// Headers das bibliotecas externas
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Nossos Módulos Refatorados
#include "types.h"
#include "utils.h"
#include "matrices.h"
#include "renderer.h"
#include "collisions.h"
#include "input.h"
#include "car.h"
#include "track.h"
#include "moving_objects.h"

// Declaração de funções auxiliares para renderizar texto dentro da janela

// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".

void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().

void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// --- VARIÁVEIS GLOBAIS DE ESTADO (State Management) ---
// Estas substituem as variáveis soltas antigas para a lógica do jogo.
GameState     g_State;
CarState      g_Car;
ShaderProgram g_Shader;
MovingSphereState g_Sphere;

// Estruturas de Dados da Cena
std::map<std::string, SceneObject> g_VirtualScene; // Geometria visual
std::vector<AABB> g_CollisionBoxes;                // Caixas de colisão estáticas (Pista)
std::vector<std::tuple<glm::mat4, const char*, int>> g_TrackObjects; // Lista de objetos estáticos para desenhar
std::stack<glm::mat4> g_MatrixStack;               // Pilha de matrizes (usada pelo Renderer)

// Cache de Colisão
Sphere g_localSphereHull;
std::vector<OBB> g_localCarHulls;
AABB g_localBarrierHull;

// Contador de Texturas
GLuint g_NumLoadedTextures = 0;

// --- CÂMERA SUAVE (Novas Variáveis) ---
// Precisamos lembrar onde a câmera estava no frame anterior para interpolar
glm::vec3 g_CurrentCameraPos = glm::vec3(0.0f, 5.0f, 10.0f);
glm::vec3 g_CurrentCameraLookAt = glm::vec3(0.0f, 0.0f, 0.0f);

// --- VARIÁVEIS LEGADO (Para compatibilidade com TextRenderer) ---
// O TextRenderer original espera encontrar essas variáveis globais.
// Nós as manteremos aqui e as sincronizaremos com o g_State se necessário.
float g_ScreenRatio = 1.0f;
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;
float g_CameraTheta = 0.0f;
float g_CameraPhi = 0.0f;
float g_CameraDistance = 5.0f;
bool g_UsePerspectiveProjection = true;
bool g_ShowInfoText = true;
// Variáveis dummy para o TextRenderer não quebrar se ele as referenciar
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false;
bool g_MiddleMouseButtonPressed = false;
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;


int main(int argc, char* argv[])
{
    // 1. Inicialização da Janela e Contexto
    int success = glfwInit();
    if (!success) { fprintf(stderr, "ERROR: glfwInit() failed.\n"); std::exit(EXIT_FAILURE); }

    glfwSetErrorCallback(ErrorCallback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Fullscreen ou Janela
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    // Para janela em modo janela, use:
    // GLFWwindow* window = glfwCreateWindow(1600, 900, "SpeedTrack", NULL, NULL);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "SpeedTrack", monitor, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // --- CONEXÃO CRÍTICA DO INPUT ---
    // Conecta nosso GameState à janela para que os callbacks em input.cpp possam acessá-lo.
    glfwSetWindowUserPointer(window, &g_State);

    // Callbacks (Definidos em input.h/cpp)
    glfwSetKeyCallback(window, KeyCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    glfwSetCursorPosCallback(window, CursorPosCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Inicializa estado da janela
    FramebufferSizeCallback(window, mode->width, mode->height);

    // 2. Carregamento de Assets (Shaders, Texturas, Modelos)
    const GLubyte *vendor      = glGetString(GL_VENDOR);
    const GLubyte *renderer    = glGetString(GL_RENDERER);
    const GLubyte *glversion   = glGetString(GL_VERSION);
    const GLubyte *glslversion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("GPU: %s, %s, OpenGL %s, GLSL %s\n", vendor, renderer, glversion, glslversion);

    // Carrega Shaders para a struct g_Shader
    LoadShadersFromFiles(g_Shader);

    //carregando texturas
    LoadTextureImage("../../models/sphere_textures/rock_boulder_dry_diff_4k.jpg", g_NumLoadedTextures); // 0
    LoadTextureImage("../../models/sphere_textures/rock_boulder_dry_disp_4k.png", g_NumLoadedTextures); // 1
    LoadTextureImage("../../data/asfalto.jpg", g_NumLoadedTextures);                      // 2
    LoadTextureImage("../../models/Jeep_Renegade_2016/Jeep_Renegade_2016/car_jeep_ren.jpg", g_NumLoadedTextures); // 3
    LoadTextureImage("../../models/concrete_road_barrier/textures/concrete_road_barrier_arm_4k.jpg", g_NumLoadedTextures); // 4
    LoadTextureImage("../../models/concrete_road_barrier/textures/concrete_road_barrier_diff_4k.jpg", g_NumLoadedTextures); // 5
    LoadTextureImage("../../models/concrete_road_barrier/textures/concrete_road_barrier_nor_gl_4k.jpg", g_NumLoadedTextures); // 6
    LoadTextureImage("../../models/sphere_textures/beige_wall_001_diff_4k.jpg", g_NumLoadedTextures); // 7
    LoadTextureImage("../../models/sphere_textures/beige_wall_001_disp_4k.png", g_NumLoadedTextures); // 8

    // Carrega Modelos
    // Esfera
    ObjModel spheremodel("../../data/sphere.obj");
    ComputeNormals(&spheremodel);
    BuildTrianglesAndAddToVirtualScene(g_VirtualScene, &spheremodel);

    // Cubo
    ObjModel cubemodel("../../data/cube.obj");
    ComputeNormals(&cubemodel);
    BuildTrianglesAndAddToVirtualScene(g_VirtualScene, &cubemodel);

    // Plano
    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(g_VirtualScene, &planemodel);

    // Carro
    ObjModel carmodel("../../models/Jeep_Renegade_2016/Jeep_Renegade_2016.obj");
    ComputeNormals(&carmodel);
    BuildTrianglesAndAddToVirtualScene(g_VirtualScene, &carmodel);

    // Pista (Novos modelos)
    ObjModel rampa("../../models/pista/ramp.obj");
    ComputeNormals(&rampa);
    BuildTrianglesAndAddToVirtualScene(g_VirtualScene, &rampa);

    ObjModel reta("../../models/pista/straight.obj");
    ComputeNormals(&reta);
    BuildTrianglesAndAddToVirtualScene(g_VirtualScene, &reta);

    ObjModel curva("../../models/pista/turn.obj");
    ComputeNormals(&curva);
    BuildTrianglesAndAddToVirtualScene(g_VirtualScene, &curva);

    // Barreira
    ObjModel barriermodel("../../models/concrete_road_barrier/barrier.obj");
    ComputeNormals(&barriermodel);
    BuildTrianglesAndAddToVirtualScene(g_VirtualScene, &barriermodel);

    // 3. Inicialização da Pista (Level Design)
    TrackCursor cursor;
    cursor.position = glm::vec3(0.0f, TRACK_Y, 5.0f);
    cursor.angleY = 0.0f;
    int bbox_id_counter = 0;

    // Constrói a pista e popula g_TrackObjects e g_CollisionBoxes
    BuildTrack(g_TrackObjects, g_VirtualScene, g_CollisionBoxes, bbox_id_counter, cursor);

    // Adiciona esferas estáticas à cena
    float sphere1UniformScale = 1.0f;
    glm::mat4 sphere_model_matrix = Matrix_Translate(-1.0f, 0.0f, 0.0f) * Matrix_Scale(sphere1UniformScale, sphere1UniformScale, sphere1UniformScale);
    g_TrackObjects.push_back(std::make_tuple(sphere_model_matrix, "the_sphere", SPHERE1));
    BuildBBoxArray(g_VirtualScene, g_CollisionBoxes, bbox_id_counter, "the_sphere", sphere_model_matrix, SPHERE1);

    // Adiciona barreira à cena
    glm::mat4 barrier_model_matrix = Matrix_Translate(4.0f, -1.0f, 0.0f) * Matrix_Scale(1.0f, 1.0f, 1.0f);
    g_TrackObjects.push_back(std::make_tuple(barrier_model_matrix, "concrete_road_barrier", BARRIER));
    BuildBBoxArray(g_VirtualScene, g_CollisionBoxes, bbox_id_counter, "concrete_road_barrier", barrier_model_matrix, BARRIER);
    int barrierHullId = bbox_id_counter-1;

    // 4. Inicialização de Física e Colisão
    printf("Building local-space collision hulls...\n");
    g_localSphereHull = BoundingSphere(spheremodel, SPHERE1);
    g_localCarHulls   = BuildCompoundHitbox(carmodel, Matrix_Identity(), CAR);
    g_localBarrierHull = g_CollisionBoxes[barrierHullId];
    printf("Hulls built.\n");


    // Inicialização Final
    TextRendering_Init();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Setup inicial do Carro
    g_Car.position = glm::vec3(2.0f, -1.0f, 0.0f);
    g_Car.angle    = 0.0f;
    g_Car.speed    = 0.0f;

    // Setup inicial da esfera que se move
    g_Sphere.position = glm::vec3(-30.0f, -0.3f, 0.0f);
    g_Sphere.angle    = 0.0f;
    g_Sphere.speed    = 0.0f;

    // Controle de Tempo
    float prev_time = (float)glfwGetTime();
    float deltaTime = 0.0f;

    // --- LOOP PRINCIPAL ---
    while (!glfwWindowShouldClose(window))
    {
        // 1. Gestão de Tempo
        float current_time = (float)glfwGetTime();
        deltaTime = current_time - prev_time;
        prev_time = current_time;

        // 2. Sincronização com Globais Legado (Para TextRenderer funcionar)
        // O Input atualiza g_State, mas o TextRenderer lê g_CameraTheta, etc.
        g_CameraTheta = g_State.cameraTheta;
        g_CameraPhi   = g_State.cameraPhi;
        g_CameraDistance = g_State.cameraDistance;
        g_UsePerspectiveProjection = g_State.usePerspective;
        g_ShowInfoText = g_State.showInfoText;
        g_ScreenRatio = g_State.screenRatio;


        // 3. Limpeza e Preparação do Frame
        // Clonamos as caixas estáticas (pista) para adicionar as dinâmicas (carro) neste frame
        std::vector<AABB> currentFrameBoxes = g_CollisionBoxes;
        int current_bbox_id = bbox_id_counter; // Continua a contagem de IDs

        glClearColor(0.6f, 0.8f, 1.0f, 1.0f); // Cor do céu
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(g_Shader.programId);

        // 4. Lógica e Física (Carro)
        glm::vec4 camera_position_c;
        glm::vec4 camera_lookat_l;
        glm::vec4 camera_view_vector;
        glm::vec4 camera_up_vector;

        float carUniformScale = 0.5f;

        // Guarda posição anterior para resolução de colisão
        glm::vec3 last_car_pos = g_Car.position;

        CarControl(g_Car, g_State.input, deltaTime);
        SphereControl(g_Sphere, g_Car, deltaTime);

        if (g_State.input.camera_mode) // Câmera Livre
        {
            // Lógica de câmera livre (Orbital/Free)
            // Usa g_State.cameraTheta/Phi atualizados pelo input
            float r = g_State.cameraDistance;
            float y = r * sin(g_State.cameraPhi);
            float z = r * cos(g_State.cameraPhi) * cos(g_State.cameraTheta);
            float x = r * cos(g_State.cameraPhi) * sin(g_State.cameraTheta);

            // Implementação simples de Free Cam baseada na posição atual
            // (Para simplificar, mantivemos a orbital focada no carro, mas você pode expandir)
            camera_position_c = glm::vec4(x,y,z,1.0f) + glm::vec4(g_Car.position, 0.0f);
            camera_lookat_l   = glm::vec4(g_Car.position, 1.0f);
            camera_view_vector = camera_lookat_l - camera_position_c;
            camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f);
        }
        else // Câmera do Jogo (Segue o Carro)
        {
            /* Atualiza Física do Carro
            CarControl(g_Car, g_State.input, deltaTime);

            float r = g_State.cameraDistance;
            float y = r * sin(g_State.cameraPhi);
            float z = r * cos(g_State.cameraPhi) * cos(g_State.cameraTheta);
            float x = r * cos(g_State.cameraPhi) * sin(g_State.cameraTheta);

            camera_position_c  = glm::vec4(x,y,z,1.0f) + glm::vec4(g_Car.position, 0.0f);
            camera_lookat_l    = glm::vec4(g_Car.position, 1.0f); // Olha para o carro
            camera_view_vector = camera_lookat_l - camera_position_c;
            camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f);
            */

            // --- LÓGICA DE SUAVIZAÇÃO (INTERPOLAÇÃO) ---

            // 1. Calcula Onde a Câmera DEVERIA estar (Ideal)
            float cam_distance = 3.5f;
            float cam_height   = 1.5f;
            float ang_rad = glm::radians(g_Car.angle);
            glm::vec3 car_forward = glm::vec3(sin(ang_rad), 0.0f, cos(ang_rad));

            glm::vec3 ideal_pos = g_Car.position - (car_forward * cam_distance);
            ideal_pos.y += cam_height;

            glm::vec3 ideal_lookat = g_Car.position;
            ideal_lookat.y += 0.5f;

            // 2. Interpolação (Lerp)
            // O fator determina o quão rápido a câmera segue.
            // 2.0 = Lento/Pesado, 10.0 = Rápido/Grudado.
            // Multiplicamos por deltaTime para ser independente do framerate.
            float lerp_speed = 15.0f;

            g_CurrentCameraPos = glm::mix(g_CurrentCameraPos, ideal_pos, lerp_speed * deltaTime);
            g_CurrentCameraLookAt = glm::mix(g_CurrentCameraLookAt, ideal_lookat, lerp_speed * deltaTime);

            // 3. Aplica
            camera_position_c  = glm::vec4(g_CurrentCameraPos, 1.0f);
            camera_lookat_l    = glm::vec4(g_CurrentCameraLookAt, 1.0f);
            camera_view_vector = camera_lookat_l - camera_position_c;
            camera_up_vector   = glm::vec4(0.0f, 1.0f, 0.0f, 0.0f);

        }

        // Matriz do Carro Atualizada
        glm::mat4 car_model_matrix = Matrix_Translate(g_Car.position.x, g_Car.position.y, g_Car.position.z)
                                   * Matrix_Rotate_Y(glm::radians(g_Car.angle))
                                   * Matrix_Scale(carUniformScale, carUniformScale, carUniformScale)
                                   * Matrix_Identity();

        // 5. Matrizes de Câmera e Projeção
        glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
        glm::mat4 projection;

        if (g_State.usePerspective)
            projection = Matrix_Perspective(3.141592f / 3.0f, g_State.screenRatio, -0.1f, -500.0f);
        else
            projection = Matrix_Orthographic(-10.0f * g_State.screenRatio, 10.0f * g_State.screenRatio, -10.0f, 10.0f, -0.1f, -500.0f);

        glUniformMatrix4fv(g_Shader.view_uniform, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(g_Shader.projection_uniform, 1, GL_FALSE, glm::value_ptr(projection));

        // 6. Colisão
        // Adiciona as caixas do carro à lista de colisão deste frame
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh1 Group1 Model", car_model_matrix, CAR);
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh2 Group2 Model", car_model_matrix, CAR);
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh3 Group3 Model", car_model_matrix, CAR);
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh4 Group4 Model", car_model_matrix, CAR);
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh5 Group5 Model", car_model_matrix, CAR);
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh6 Group6 Model", car_model_matrix, CAR);
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh7 Group7 Model", car_model_matrix, CAR);
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh8 Group8 Model", car_model_matrix, CAR);
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh9 Group9 Model", car_model_matrix, CAR);
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, current_bbox_id, "Mesh10 Group10 Model", car_model_matrix, CAR);

        // Esfera que pode se mexer
        float sphere2UniformScale = 0.6f;
        glm::mat4 sphere_model_matrix2 = Matrix_Translate(g_Sphere.position.x, g_Sphere.position.y, g_Sphere.position.z) 
            * Matrix_Rotate_Y(glm::radians(g_Sphere.angle))
            * Matrix_Scale(sphere2UniformScale, sphere2UniformScale, sphere2UniformScale)
            * Matrix_Identity();
        g_TrackObjects.push_back(std::make_tuple(sphere_model_matrix2, "the_sphere", SPHERE2));
        BuildBBoxArray(g_VirtualScene, currentFrameBoxes, bbox_id_counter, "the_sphere", sphere_model_matrix2, SPHERE2);

        // Broadphase
        auto possibleCollisions = SweepAndPrune(currentFrameBoxes);

        if (possibleCollisions.count({CAR, SPHERE1})){
            // Verifica colisão contra a Esfera 1
            TreatCarSphereCollision(sphere_model_matrix, sphere1UniformScale, car_model_matrix, {CAR, SPHERE1}, g_Car, last_car_pos, g_localSphereHull, g_localCarHulls, g_Sphere);
        }
        if(possibleCollisions.count({CAR, SPHERE2})){
            // Verifica colisão contra a Esfera 2
            TreatCarSphereCollision(sphere_model_matrix2, sphere2UniformScale, car_model_matrix, {CAR, SPHERE2}, g_Car, last_car_pos, g_localSphereHull, g_localCarHulls, g_Sphere);
        }
        if (possibleCollisions.count({CAR, BARRIER})) {
            TreatCarBarrierCollision(car_model_matrix, {CAR, BARRIER}, g_Car, last_car_pos, g_localCarHulls, g_localBarrierHull);
        }

        // Nota: Se você tiver colisão CAR vs PLANE ou CAR vs WALL, adicione aqui usando TreatCarCollision (adaptando para OBB vs OBB se necessário)

        // 7. Desenho (Render)

        // Desenha Pista Estática (incluindo esferas)
        DrawAllObjects(g_TrackObjects, g_VirtualScene, g_Shader);

        // Desenha Carro Dinâmico
        // (Poderíamos otimizar não recriando o vetor todo frame, mas é ok para poucas partes)
        std::vector<std::tuple<glm::mat4, const char*, int>> carParts;
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh1 Group1 Model", CAR));
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh2 Group2 Model", CAR));
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh3 Group3 Model", CAR));
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh4 Group4 Model", CAR));
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh5 Group5 Model", CAR));
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh6 Group6 Model", CAR));
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh7 Group7 Model", CAR));
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh8 Group8 Model", CAR));
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh9 Group9 Model", CAR));
        carParts.push_back(std::make_tuple(car_model_matrix, "Mesh10 Group10 Model", CAR));

        DrawAllObjects(carParts, g_VirtualScene, g_Shader);

        // Debug Drawing
        if (g_State.input.debug_mode)
        {
            for (const auto& box : currentFrameBoxes)
                DrawDebugBox(box, view, projection, g_Shader, g_VirtualScene);

            // Debug Spheres
            Sphere s1 = { glm::vec3(sphere_model_matrix * glm::vec4(g_localSphereHull.center, 1.0f)), g_localSphereHull.radius * sphere1UniformScale, 0 };
            DrawDebugSphere(s1, view, projection, g_Shader, g_VirtualScene);

            Sphere s2 = { glm::vec3(sphere_model_matrix2 * glm::vec4(g_localSphereHull.center, 1.0f)), g_localSphereHull.radius * sphere1UniformScale, 0 };
            DrawDebugSphere(s2, view, projection, g_Shader, g_VirtualScene);
        }

        // 8. Interface de Texto
        TextRendering_ShowFramesPerSecond(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);

        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}
