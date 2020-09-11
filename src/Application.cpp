#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <math.h>
#include "../res/headers/shader.h"
#include "../res/headers/stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

glm::mat4 view = glm::mat4(1.0f);
float camera_speed = 0.05;

float delta_frametime = 0.0f;
float prev_frametime = 0.0f;
float curr_frametime = 0.0f;

float pitch = 0.0f, yaw = -90.0f;
glm::vec3 camera_dir;

double prev_xpos = SCR_WIDTH / 2, prev_ypos = SCR_HEIGHT / 2;


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
    glClearColor(0.0f, 0.1f, 0.0f, 1.0f);
    //glClear(GL_COLOR_BUFFER_BIT);
    
    
    // loading vertices
    
    float vertices[] = {
        // positions          // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
    };

    
    unsigned int VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    //glGenBuffers(1, &IBO);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    Shader myShader("res/shaders/shader.vs", "res/shaders/shader.fs");

    // load and create a texture 
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("res/textures/wooden_crate.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);


    // ---- 3D transformation matrices ----

    glEnable(GL_DEPTH_TEST);

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
        curr_frametime = glfwGetTime();
        delta_frametime = curr_frametime - prev_frametime;
        prev_frametime = curr_frametime;
        
        glBindTexture(GL_TEXTURE_2D, texture);
        
        myShader.use();

        camera_dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        camera_dir.y = sin(glm::radians(pitch));
        camera_dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        view = glm::lookAt(camera_pos, camera_pos + camera_dir, up);

        myShader.setMat4("model", model);
        myShader.setMat4("view", view);
        myShader.setMat4("projection", projection);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // ---------- render loop end ----------
    
    glfwTerminate();
    return 0;
}




// listen for mouse movments for changing camera diraction
void mouse_callback(GLFWwindow* window, double xpos, double ypos) 
{
    double delta_xpos, delta_ypos;

    //std::cout << xpos  << ", " << ypos  << std::endl;

    delta_xpos = xpos - prev_xpos;
    delta_ypos = prev_ypos - ypos;
    prev_xpos = xpos;
    prev_ypos = ypos;

    yaw += delta_xpos * 0.1;
    pitch += delta_ypos * 0.1;
}

// listen for (ESC) key for exit and (WASD) keys for changing camera position
void processInput(GLFWwindow* window)
{
    camera_speed = delta_frametime * 5.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera_speed /= 3;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // use WASD to change camera position

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_pos += camera_dir * camera_speed;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_pos -= camera_dir * camera_speed;

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_pos += glm::normalize(glm::cross(up, camera_dir)) * camera_speed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_pos -= glm::normalize(glm::cross(up, camera_dir)) * camera_speed;
}

// ajust glViewport if the size of window is changing
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
