#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <array>

static void updateFPS() {
    static DWORD s = GetTickCount64(), e;
    static int count = 0;
    count++;
    if (count == 100) {
        e = GetTickCount64();
        std::cout << float((100000.0f) / (e - s)) << std::endl;
        count = 0;
        s = e;
    }
}

GLFWwindow* window;

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}
int main(void) {

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 2);

    window = glfwCreateWindow(512, 512, "Window", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwSetKeyCallback(window, key_callback);
    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    const GLubyte* OpenGLVersion = glGetString(GL_VERSION);
    const GLubyte* OpenGLVender = glGetString(GL_VENDOR);
    std::cout << OpenGLVersion << std::endl;
    std::cout << OpenGLVender << std::endl;

    glfwSwapInterval(0);


    while (!glfwWindowShouldClose(window)) {

        glClearColor(0, 1, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 10);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
