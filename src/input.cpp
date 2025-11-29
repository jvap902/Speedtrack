#include "input.h"
#include "types.h" // Precisamos da definição de GameState
#include <cmath>

// Helper para pegar o estado a partir da janela GLFW
GameState* GetState(GLFWwindow* window) {
    return static_cast<GameState*>(glfwGetWindowUserPointer(window));
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    GameState* state = GetState(window);
    if (state) {
        state->screenRatio = (float)width / height;
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}


// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    // Recupera o estado global
    GameState* state = GetState(window);
    if (!state) return; // Segurança

    // Se o usuário pressionar a tecla ESC, fechamos a janela.
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Projeção e UI
    if (key == GLFW_KEY_P && action == GLFW_PRESS) state->usePerspective = true;
    if (key == GLFW_KEY_O && action == GLFW_PRESS) state->usePerspective = false;
    if (key == GLFW_KEY_H && action == GLFW_PRESS) state->showInfoText = !state->showInfoText;

    // --- Controles do Carro ---
    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) state->input.w = true;
        else if (action == GLFW_RELEASE) state->input.w = false;
    }
    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS) state->input.s = true;
        else if (action == GLFW_RELEASE) state->input.s = false;
    }
    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS) state->input.a = true;
        else if (action == GLFW_RELEASE) state->input.a = false;
    }
    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS) state->input.d = true;
        else if (action == GLFW_RELEASE) state->input.d = false;
    }

    // Modos de Debug
    if (key == GLFW_KEY_B && action == GLFW_PRESS) state->input.debug_mode = !state->input.debug_mode;
    if (key == GLFW_KEY_V && action == GLFW_PRESS) state->input.camera_mode = !state->input.camera_mode;
}

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    GameState* state = GetState(window);
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            glfwGetCursorPos(window, &state->lastCursorX, &state->lastCursorY);
            state->mouseLeftPressed = true;
        } else if (action == GLFW_RELEASE) {
            state->mouseLeftPressed = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            glfwGetCursorPos(window, &state->lastCursorX, &state->lastCursorY);
            state->mouseRightPressed = true;
        } else if (action == GLFW_RELEASE) {
            state->mouseRightPressed = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            glfwGetCursorPos(window, &state->lastCursorX, &state->lastCursorY);
            state->mouseMiddlePressed = true;
        } else if (action == GLFW_RELEASE) {
            state->mouseMiddlePressed = false;
        }
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    GameState* state = GetState(window);
    if (!state) return;

    // Cálculo do deslocamento (Delta)
    float dx = xpos - state->lastCursorX;
    float dy = ypos - state->lastCursorY;

    // ########## CORREÇÃO AQUI ##########
    // Atualizamos a última posição SEMPRE, não apenas quando clica.
    // Isso evita que a câmera "pule" quando você clica ou ativa o modo câmera.
    state->lastCursorX = xpos;
    state->lastCursorY = ypos;

    // Aplica rotação se: Botão Esquerdo pressionado OU Câmera Livre ativada
    // (Adicionei || true se você quiser que a câmera orbital do carro também mova sempre)
    if (state->mouseLeftPressed || state->input.camera_mode)
    {
        state->cameraTheta -= 0.01f * dx;
        state->cameraPhi   += 0.01f * dy;

        // Limites de Phi para não inverter a câmera
        const float epsilon = 0.0174533f * 5.0f; // ~5 graus
        float phimax = 3.141592f/2 - epsilon;
        float phimin = -phimax;

        if (state->cameraPhi > phimax) state->cameraPhi = phimax;
        if (state->cameraPhi < phimin) state->cameraPhi = phimin;
    }
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    GameState* state = GetState(window);
    if (!state) return;

    state->cameraDistance -= 0.1f * yoffset;

    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (state->cameraDistance < verysmallnumber)
        state->cameraDistance = verysmallnumber;
}
