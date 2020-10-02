#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <math.h>
#include "../res/headers/Mesh.h"
#include "../res/headers/Model.h"
#include "../res/headers/shader.h"
#include "../res/headers/camera.h"
#include "../res/headers/stb_image.h"
#include "../res/headers/assimp.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// screen settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// frametime variables
float delta_frametime = 0.0f;
float prev_frametime = 0.0f;
float curr_frametime = 0.0f;

// mouse event variables
float prev_xpos = 0 , prev_ypos = 0;
bool first_enter = true;

// lighting settings
glm::vec3 light_pos(5.0f, 6.0f, 10.0f);

// camera settings
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 0.0f, -1.0f));

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "PRESS ESC KEY TO EXIT THE GAME", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // changing color of screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // loading vertices
    float vertices[] = {
        // positions          // normal
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    unsigned int VBO, cubeVAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(cubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    unsigned int lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBindVertexArray(lightVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    
    Shader LightingShader("res/shaders/shader.vs", "res/shaders/shader.fs");
    Shader LightCubeShader("res/shaders/light_cube.vs", "res/shaders/light_cube.fs");

    Model Teapot_model("res/models/teapot.obj");

    // ---- 3D transformation matrices ----

    glEnable(GL_DEPTH_TEST);

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection;
    
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // --------- render loop start ---------
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        processInput(window);

        // calculates how much time does it takes to render one frame 
        curr_frametime = (float)glfwGetTime();
        delta_frametime = curr_frametime - prev_frametime;
        prev_frametime = curr_frametime;
        
        //-----------------------------------------------------------

        LightingShader.use();
        LightingShader.setVec3("object_color", 0.8f, 0.35f, 0.54f);
        LightingShader.setVec3("light_color", 1.0f, 1.0f, 1.0f);
        LightingShader.setVec3("light_pos", light_pos);
        LightingShader.setVec3("view_pos", camera.camera_pos);

        model = glm::mat4(1.0f);
        view = camera.GetViewMatrix();

        LightingShader.setMat4("model", model);
        LightingShader.setMat4("view", view);
        LightingShader.setMat4("projection", projection);

        Teapot_model.Draw();

        //glBindVertexArray(cubeVAO);
        //glDrawArrays(GL_TRIANGLES, 0, 36);

        //-----------------------------------------------------------

        LightCubeShader.use();

        model = glm::mat4(1.0f);
        model = glm::translate(model, light_pos);
        model = glm::scale(model, glm::vec3(0.3f));

        LightCubeShader.setMat4("model", model);
        LightCubeShader.setMat4("view", view);
        LightCubeShader.setMat4("projection", projection);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        //-----------------------------------------------------------
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // ---------- render loop end ----------

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    
    glfwTerminate();
    return 0;
}




// listen for mouse movments for changing camera diraction
void mouse_callback(GLFWwindow* window, double xpos, double ypos) 
{
    if (first_enter)
    {
        prev_xpos = (float)xpos;
        prev_ypos = (float)ypos;
        first_enter = false;
    }

    float delta_xpos = (float)xpos - prev_xpos;
    float delta_ypos = prev_ypos - (float)ypos;
    prev_xpos = (float)xpos;
    prev_ypos = (float)ypos;

    camera.UpdateDir(delta_xpos, delta_ypos);
}

// listen for (ESC) key for exit and (WASD) keys for changing camera position
void processInput(GLFWwindow* window)
{
    float multiplier = 5.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        multiplier /= 5;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // use WASD to change camera position
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) 
        camera.UpdatePos(FORWARD, delta_frametime, multiplier);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.UpdatePos(BACKWARD, delta_frametime, multiplier);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.UpdatePos(LEFT, delta_frametime, multiplier);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.UpdatePos(RIGHT, delta_frametime, multiplier);
}

// ajust glViewport if the size of window is changing
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
