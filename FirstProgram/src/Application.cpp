#include <iostream>
#include <math.h>

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

void framebuffer_size_callback(GLFWwindow * window, int width, int height);
void mouse_callback(GLFWwindow * window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow * window, int button, int action, int mods);
void processInput(GLFWwindow* window);
void RenderScene( Shader & shader, Model models[]);
GLuint loadCubemap(vector<std::string> faces);

// screen settings (aspect ratio = 16/9)
 GLuint SCR_WIDTH = 1244, SCR_HEIGHT = 700;

// frametime variables
float delta_frametime = 0.0f, prev_frametime = 0.0f, curr_frametime = 0.0f;

// mouse event variables
float prev_xpos = 0 , prev_ypos = 0;
bool first_enter = true;

// lighting settings
glm::vec3 light_pos(1.0f, 8.0f, 4.0f);

// camera settings
Camera camera(glm::vec3(0.0f, 10.0f, 15.0f), glm::vec3(0.0f, 0.0f, -1.0f));


void Render(int depth_cubemap, int cubemap, float far_plane, Model models[], Shader shaders[]);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

int main()
{

    // -------- setting the GLFW and GLAD --------

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
    glfwSetWindowPos(window, 1200, 100);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // -----------------------------------------
    



    // -------- shader, model and skybox load + matrix creation --------

    // changing color of screen ( by setting default values for the color buffers )
    glClearColor(0.3f, 0.3f, 0.5f, 1.0f);
    glClearStencil(0);
    glEnable(GL_DEPTH_TEST);
    // hide and lock cursor on the window
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    Shader EnvironmentShader("res/shaders/environment_mapping_vertex.glsl", "res/shaders/environment_mapping_fragment.glsl");
    Shader LightShader("res/shaders/light_vertex.glsl", "res/shaders/light_fragment.glsl");
    Shader ObjectShader("res/shaders/object_vertex.glsl", "res/shaders/object_fragment.glsl");
    Shader ShadowShader("res/shaders/shadow_mapping_vertex.glsl", "res/shaders/shadow_mapping_fragment.glsl", "res/shaders/shadow_mapping_geometry.glsl");
    Shader SkyboxShader("res/shaders/skybox_vertex.glsl", "res/shaders/skybox_fragment.glsl");
    Shader NormalShader("res/shaders/normal_mapping_vertex.glsl", "res/shaders/normal_mapping_fragment.glsl");
    Shader MirrorShader("res/shaders/mirror_vertex.glsl", "res/shaders/mirror_fragment.glsl");
    Shader ParallaxShader("res/shaders/parallax_mapping_vertex.glsl", "res/shaders/parallax_mapping_fragment.glsl");
    Shader shaders[5] = {NormalShader, EnvironmentShader, LightShader, SkyboxShader, ParallaxShader};
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
    GLuint cubemapTexture = loadCubemap(faces);

    Model Teapot_model("res/models/Teapot/teapot.obj");
    Model Sphere_model("res/models/sphere.obj");
    Model Plane_model("res/models/Table/plane.obj");
    Model Box_model("res/models/box.obj");
    Model Cup_model("res/models/Cup/cup.obj");
    Model Mirror_model("res/models/Mirror/mirror.obj");
    Model Wall_model("res/models/BrickWall/wall.obj");
    //Model Pumpkin_model("res/models/pumpkin.obj");
    Model models[6] = { Teapot_model , Plane_model , Cup_model, Sphere_model, Box_model, Wall_model };

    // -----------------------------------------------------------------




    // ----------- shadow mapping -----------

    const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    GLuint depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);

    // create depth cubemap texture
    GLuint depthCubemap;
    glGenTextures(1, &depthCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);

    for (GLuint i = 0; i < 6; ++i)
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

    // ----------------  reflection texture ----------------------

    const GLuint REFLECTION_WIDTH = 1024, REFLECTION_HEIGHT = 1024;
    unsigned int reflectionFramebuffer;
    glGenFramebuffers(1, &reflectionFramebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, reflectionFramebuffer);

    // create a color attachment texture
    unsigned int reflectionTexture;
    glGenTextures(1, &reflectionTexture);
    glBindTexture(GL_TEXTURE_2D, reflectionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, REFLECTION_WIDTH, REFLECTION_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, reflectionTexture, 0);

    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, REFLECTION_WIDTH, REFLECTION_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // --------------------------------------

    ObjectShader.use();
    ObjectShader.setInt("diffuse_texture1", 0);
    ObjectShader.setInt("depthMap", 1);

    NormalShader.use();
    NormalShader.setInt("diffuse_texture1", 0);
    NormalShader.setInt("normal_texture1", 1);
    NormalShader.setInt("depthMap", 2);

    MirrorShader.use();
    MirrorShader.setInt("mirrorTexture", 0);

    ParallaxShader.use();
    ParallaxShader.setInt("diffuse_texture1", 0);
    ParallaxShader.setInt("normal_texture1", 1);
    ParallaxShader.setInt("specular_texture1", 2);
    ParallaxShader.setInt("depthMap", 3);

    // ---------------- render loop start ----------------
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glDisable(GL_STENCIL_TEST);

        processInput(window);

        // calculates how much time does it takes to render one frame (delta_frametime)
        curr_frametime = (float)glfwGetTime();
        delta_frametime = curr_frametime - prev_frametime;
        prev_frametime = curr_frametime;


        // ---------------- rendering the shadow cubemap ----------------

        float near_plane = 1.0f;
        float far_plane = 40.0f;
        glm::mat4 shadow_projection = glm::perspective(glm::radians(90.0f), (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT, near_plane, far_plane);
        std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.push_back(shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
        shadowTransforms.push_back(shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
        shadowTransforms.push_back(shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
        shadowTransforms.push_back(shadow_projection * glm::lookAt(light_pos, light_pos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        ShadowShader.use();
        for (GLuint i = 0; i < 6; ++i)
            ShadowShader.setMat4("shadowMatrices[" + std::to_string(i) + "]", shadowTransforms[i]);
        ShadowShader.setFloat("far_plane", far_plane);
        ShadowShader.setVec3("lightPos", light_pos);

        RenderScene(ShadowShader, models);


        // ---------- drawing objects of the scene ------------

        glBindFramebuffer(GL_FRAMEBUFFER, reflectionFramebuffer);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        glViewport(0, 0, REFLECTION_WIDTH, REFLECTION_HEIGHT);

        view = camera.GetMirroredViewMatrix(model);
        Render(depthCubemap, cubemapTexture, far_plane, models, shaders);

        // reset to default values
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glDisable(GL_STENCIL_TEST);

        // ---------- drawing objects of the scene ------------

        view = camera.GetViewMatrix();
        Render(depthCubemap, cubemapTexture, far_plane, models, shaders);

        MirrorShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 5.0f, -15.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(16.0f, 6.0f, 6.0f));
        view = camera.GetViewMatrix();
        MirrorShader.setMat4("model", model);
        MirrorShader.setMat4("view", view);
        MirrorShader.setMat4("projection", projection);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, reflectionTexture);
        Mirror_model.Draw(MirrorShader);

        // ----------------------------------------------------------


        FPS = to_string(floor(1 / delta_frametime));
        glfwSetWindowTitle(window, (window_title + FPS).c_str());
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // ---------------- render loop end ----------------

    glfwTerminate();
    return 0;
}




void Render(int depth_cubemap, int cubemap, float far_plane, Model models[], Shader shaders[])
{
    //------------------ teapot -----------------------
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0f, 0.0f, -3.0f));
    shaders[0].use();
    shaders[0].setVec3("object_color", 0.8f, 0.35f, 0.54f);
    shaders[0].setVec3("light_color", 1.0f, 1.0f, 1.0f);
    shaders[0].setVec3("light_pos", light_pos);
    shaders[0].setVec3("view_pos", camera.camera_pos);
    shaders[0].setMat4("model", model);
    shaders[0].setMat4("view", view);
    shaders[0].setMat4("projection", projection);
    shaders[0].setFloat("far_plane", far_plane);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
    models[0].Draw(shaders[0]);

    //------------------ mirror -----------------------

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(14.0f, 6.0f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3( 6.0f, 6.0f, 6.0f));
    shaders[4].use();
    shaders[4].setVec3("object_color", 0.8f, 0.35f, 0.54f);
    shaders[4].setVec3("light_color", 1.0f, 1.0f, 1.0f);
    shaders[4].setVec3("light_pos", light_pos);
    shaders[4].setVec3("view_pos", camera.camera_pos);
    shaders[4].setMat4("model", model);
    shaders[4].setMat4("view", view);
    shaders[4].setMat4("projection", projection);
    shaders[4].setFloat("far_plane", far_plane);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
    models[5].Draw(shaders[4]);

    //----------------- wooden plane -----------------

    shaders[0].use();
    model = glm::mat4(1.0f);
    shaders[0].setVec3("light_color", 1.0f, 1.0f, 1.0f);
    shaders[0].setVec3("light_pos", light_pos);
    shaders[0].setVec3("view_pos", camera.camera_pos);
    shaders[0].setMat4("model", model);
    shaders[0].setMat4("view", view);
    shaders[0].setMat4("projection", projection);
    shaders[0].setFloat("far_plane", far_plane);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_cubemap);
    models[1].Draw(shaders[0]);

    //-------------------- cup ---------------------

    shaders[1].use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 2.0f));
    shaders[1].setMat4("model", model);
    shaders[1].setMat4("view", view);
    shaders[1].setMat4("projection", projection);
    shaders[1].setVec3("view_pos", camera.camera_pos);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    models[2].Draw(shaders[1]);

    // ------------------- drawing the light --------------------

    shaders[2].use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, light_pos);
    model = glm::scale(model, glm::vec3(0.1f));
    shaders[2].setMat4("model", model);
    shaders[2].setMat4("view", view);
    shaders[2].setMat4("projection", projection);

    models[3].Draw(shaders[2]);

    // ------------------ drawing the skybox --------------------

    shaders[3].use();
    glDepthFunc(GL_LEQUAL);
    // we use mat3 instead of mat4 to remove translation from matrix (only rotation is needed)
    view = glm::mat4(glm::mat3(view));
    shaders[3].setMat4("view", view);
    shaders[3].setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    models[4].Draw(shaders[3]);
    glDepthFunc(GL_LESS);
}


void RenderScene( Shader & shader, Model models[])
{
    glm::mat4 model = glm::mat4(1.0f);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(3.0f, 0.0f, -3.0f));
    shader.setMat4("model", model);
    models[0].Draw(shader);

    model = glm::scale(model, glm::vec3(10.0f, 0.0f, 10.0f));
    shader.setMat4("model", model);
    models[1].Draw(shader);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-5.0f, 0.0f, 2.0f));
    shader.setMat4("model", model);
    models[2].Draw(shader);
}


// loading the cubemap (for skybox)
GLuint loadCubemap(vector<std::string> faces)
{
    GLuint cubemapID;
    glGenTextures(1, &cubemapID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

    int width, height, components;
    for (GLuint i = 0; i < faces.size(); i++)
    {
        unsigned char * data = stbi_load(faces[i].c_str(), &width, &height, &components, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load, path = " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubemapID;
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

// update screen dimentions on window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}
