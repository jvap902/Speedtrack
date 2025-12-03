#pragma once

// REGRA DE OURO: O GLAD deve ser incluído ANTES do GLFW.
// Se incluirmos apenas o glfw3.h aqui, ele vai definir as coisas do OpenGL do jeito dele.
// Quando o types.h (que tem o glad) for incluído depois, vai dar conflito.
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "types.h"

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void TimeControl(GLFWwindow* window, float delta);
void Reset(GLFWwindow* window, CarState& Car, MovingSphereState& Sphere, DefaultPositions& DefPos);