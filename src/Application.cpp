#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stb_image.h>
#include <assimp.h>

#include "Mesh.h"
#include "Model.h"
#include "shader.h"
#include "camera.h"

#include <iostream>
#include <math.h>


void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void processInput(GLFWwindow * window);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
void RenderScene( Shader & shader, Model models[]);
unsigned int loadCubemap(vector<std::string> faces);

// screen settings (aspect ratio = 16/9)
const unsigned int SCR_WIDTH = 1706;
const unsigned int SCR_HEIGHT = 960;

// frametime variables
float delta_frametime = 0.0f;
float prev_frametime = 0.0f;
float curr_frametime = 0.0f;

// mouse event variables
float prev_xpos = 0 , prev_ypos = 0;
bool first_enter = true;

// lighting settings
glm::vec3 light_pos(1.0f, 8.0f, 4.0f);

// camera settings
Camera camera(glm::vec3(0.0f, 3.0f, 15.0f), glm::vec3(0.0f, 0.0f, -1.0f));

int main()
{
    // ---- setting the window and GLFW ----

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    string window_title = "Press ESC to exit Demo; FPS = ", FPS = "0";
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, (window_title + FPS).c_str(), NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // -------------------------------------
    



    // -- models load and matrix creation --

    // changing color of screen ( by setting default values for the color buffers )
    glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    Shader LightShader("res/shaders/light_vertex.glsl", "res/shaders/light_fragment.glsl");
    Shader ObjectShader("res/shaders/object_vertex.glsl", "res/shaders/object_fragment.glsl");
    Shader ShadowShader("res/shaders/shadow_mapping_vertex.glsl", "res/shaders/shadow_mapping_fragment.glsl", "res/shaders/shadow_mapping_geometry.glsl");
    Shader SkyboxShader("res/shaders/skybox_vertex.glsl", "res/shaders/skybox_fragment.glsl");
    Shader EnvironmentShader("res/shaders/environment_mapping_vertex.glsl", "res/shaders/environment_mapping_fragment.glsl");
    Shader NormalShader("res/shaders/normal_mapping_vertex.glsl", "res/shaders/normal_mapping_fragment.glsl");
    //Shader TextureShader("res/shaders/texture_vertex.glsl", "res/shaders/texture_fragment.glsl");

    vector<std::string> faces
    {
        "res/textures/japan_park_skybox/posx.jpg",
        "res/textures/japan_park_skybox/negx.jpg",
        "res/textures/japan_park_skybox/posy.jpg",
        "res/textures/japan_park_skybox/negy.jpg",
        "res/textures/japan_park_skybox/posz.jpg",
        "res/textures/japan_park_skybox/negz.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    Model Teapot_model("res/models/Teapot/teapot.obj");
    Model Sphere_model("res/models/sphere.obj");
    Model Plane_model("res/models/Table/plane.obj");
    Model Pumpkin_model("res/models/pumpkin.obj");
    Model Box_model("res/models/box.obj");
    //Model Buddha_model("res/models/buddha.obj");
    //Model Car_model("res/models/Mercedes_300_SL.obj");
    Model models[3] = { Plane_model , Teapot_model , Pumpkin_model };

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    // -------------------------------------




    // ----------- shadow mapping ----------

    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth cubemap texture
    unsigned int depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    for (unsigned int i = 0; i < 6; ++i)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // -------------------------------------


    ObjectShader.use();
    ObjectShader.setInt("diffuse_texture1", 0);
    ObjectShader.setInt("depthMap", 1);

    NormalShader.use();
    NormalShader.setInt("diffuse_texture1", 0);
    NormalShader.setInt("normal_texture1", 1);
    NormalShader.setInt("depthMap", 2);

    // --------- render loop start ---------
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        processInput(window);

        // calculates how much time does it takes to render one frame 
        curr_frametime = (float)glfwGetTime();
        delta_frametime = curr_frametime - prev_frametime;
        prev_frametime = curr_frametime;

        // ---------------- rendering the depth map ----------------

        float near_plane = 1.0f;
        float far_plane = 40.0f;
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadowProj * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        ShadowShader.use();
        for (unsigned int i = 0; i < 6; ++i)
            ShadowShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        ShadowShader.setFloat("far_plane", far_plane);
        ShadowShader.setVec3("lightPos", light_pos);

        // ========================================================= 

        RenderScene(ShadowShader, models);

        // =========================================================

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // ---------- drawing objects of the scene ------------

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(3.0f, 0.0f, -3.0f));
        view = camera.GetViewMatrix();

        ObjectShader.use();
        ObjectShader.setVec3("object_color", 0.8f, 0.35f, 0.54f);
        ObjectShader.setVec3("light_color", 1.0f, 1.0f, 1.0f);
        ObjectShader.setVec3("light_pos", light_pos);
        ObjectShader.setVec3("view_pos", camera.camera_pos);
        ObjectShader.setMat4("model", model);
        ObjectShader.setMat4("view", view);
        ObjectShader.setMat4("projection", projection);
        ObjectShader.setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        Teapot_model.Draw(ObjectShader);

        NormalShader.use();
        model = glm::mat4(1.0f);
        NormalShader.setVec3("light_color", 1.0f, 1.0f, 1.0f);
        NormalShader.setVec3("light_pos", light_pos);
        NormalShader.setVec3("view_pos", camera.camera_pos);
        NormalShader.setMat4("model", model);
        NormalShader.setMat4("view", view);
        NormalShader.setMat4("projection", projection);
        NormalShader.setFloat("far_plane", far_plane);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
        Plane_model.Draw(NormalShader);

        
        EnvironmentShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 10.0f, 0.0f));
        EnvironmentShader.setMat4("model", model);
        EnvironmentShader.setMat4("view", view);
        EnvironmentShader.setMat4("projection", projection);
        EnvironmentShader.setVec3("view_pos", camera.camera_pos);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        Sphere_model.Draw(EnvironmentShader);

        // ------------------- drawing the light --------------------

        LightShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, light_pos);
        model = glm::scale(model, glm::vec3(0.1f));

        LightShader.setMat4("model", model);
        LightShader.setMat4("view", view);
        LightShader.setMat4("projection", projection);

        Sphere_model.Draw(LightShader);
        
        // ------------------ drawing the skybox --------------------

        SkyboxShader.use();
        glDepthFunc(GL_LEQUAL);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        SkyboxShader.setMat4("view", view);
        SkyboxShader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        Box_model.Draw(SkyboxShader);
        glDepthFunc(GL_LESS);
        
        // ----------------------------------------------------------

        FPS = to_string(floor(1 / delta_frametime));
        glfwSetWindowTitle(window, (window_title + FPS).c_str());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // ---------- render loop end ----------

    glfwTerminate();
    return 0;
}



void RenderScene( Shader & shader, Model models[])
{
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::scale(model, glm::vec3(10.0f, 0.0f, 10.0f));
    shader.setMat4("model", model);
    models[0].Draw(shader);
    
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0f, 0.0f, -3.0f));
    shader.setMat4("model", model);
    models[1].Draw(shader);

    
}


// loading the cubemap for skybox
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


// listen for mouse movments for changing camera direction
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

// listen for mouse button press for changing light height
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    
    float multiplier = 50.0f;
    if (button == GLFW_MOUSE_BUTTON_RIGHT) 
        light_pos.y += delta_frametime * multiplier;
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        light_pos.y -= delta_frametime * multiplier;
}

// listen for (ESC) key for exit and (WASD) keys for changing camera position
void processInput(GLFWwindow* window)
{
    float multiplier = 5.0f, number = 7.0f;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        multiplier /= number;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        multiplier *= number;

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

    // use Arrow Keys to change light position
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        light_pos.z -= delta_frametime * multiplier;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        light_pos.z += delta_frametime * multiplier;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        light_pos.x -= delta_frametime * multiplier;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        light_pos.x += delta_frametime * multiplier;
}

// ajust glViewport if the size of window is changing
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
