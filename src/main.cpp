#include <stdio.h>
#include "include/glad/glad.h"
#include "glad.c"
#include "GLFW/glfw3.h"
#include "glm/common.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "Image.h"
#include "cubemap.h"
#include "shader.eg"
#include "Windows.h"
#define RJSON_IMPLEMENTATION
#include "rj_obj.h"
typedef unsigned int UI;
UI Pgm;
UI WND_WIDTH = 800;
UI WND_HEIGHT = 600;

struct Data
{
    UI VAO;
    UI VBO;
    UI Tex;
    obj Model;
};

struct VecV
{
    float X;
    float Y;
    float Z;
};

Data SphereData()
{
    Data result = {};
    HANDLE File = CreateFileA("../Sphere.obj", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    LARGE_INTEGER Size = {};
    GetFileSizeEx(File, &Size);

    char *Buffer = (char *)malloc(Size.QuadPart);
    ReadFile(File, Buffer, Size.QuadPart, 0, 0);
    result.Model = ParseObj((u8 *)Buffer, Size.QuadPart);

    glGenVertexArrays(1, &result.VAO);
    glBindVertexArray(result.VAO);

    glGenBuffers(1, &result.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, result.VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * result.Model.VertexCount * 3, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * result.Model.VertexCount * 3, result.Model.Vertex);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    return result;
}

void DrawSphere(Data a, float transX, float transY, float transZ, float SizeofCanon)
{
    glm::mat4 Model(1.0f);
    Model = glm::translate(Model, glm::vec3(transX, transY, transZ));
    Model = glm::scale(Model, glm::vec3(SizeofCanon, SizeofCanon, SizeofCanon));
    glUniformMatrix4fv(glGetUniformLocation(Pgm, "Model"), 1, 0, glm::value_ptr(Model));

    glUniform3f(glGetUniformLocation(Pgm, "ObjColor"), 1, 0.1, 0.1);
    glBindVertexArray(a.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, a.VBO);
    glDrawArrays(GL_TRIANGLES, 0, a.Model.VertexCount);
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(WND_WIDTH, WND_HEIGHT, "Canon", 0, 0);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    UI vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexShaderSource, 0);
    glCompileShader(vs);

    char info[1000];
    glGetShaderInfoLog(vs, 1000, 0, info);
    printf("%s", info);

    UI fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentShaderSource, 0);
    glCompileShader(fs);
    glGetShaderInfoLog(fs, 1000, 0, info);
    printf("%s", info);

    Pgm = glCreateProgram();
    glAttachShader(Pgm, vs);
    glAttachShader(Pgm, fs);
    glLinkProgram(Pgm);
    glGetProgramInfoLog(Pgm, 1000, 0, info);
    printf("%s", info);

    UI VScube = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VScube, 1, &VSsource, 0);
    glCompileShader(VScube);

    char in[1000];
    glGetShaderInfoLog(VScube, 1000, 0, in);
    printf("%s", in);

    UI FScube = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FScube, 1, &FSsource, 0);
    glCompileShader(FScube);
    glGetShaderInfoLog(FScube, 1000, 0, in);
    printf("%s", in);

    UI CubePgm = glCreateProgram();
    glAttachShader(CubePgm, VScube);
    glAttachShader(CubePgm, FScube);
    glLinkProgram(CubePgm);

    glGetProgramInfoLog(CubePgm, 1000, 0, in);
    printf("%s", in);

    float CubePos[] =
        {
            -5.0f, -1.0f, -5.0f,
            -5.0f, -1.0f, 5.0f,
            5.0f, -1.0f, -5.0f,
            5.0f, -1.0f, -5.0f,
            -5.0f, -1.0f, 5.0f,
            5.0f, -1.0f, 5.0f};

    Data Cube;
    glGenVertexArrays(1, &Cube.VAO);
    glBindVertexArray(Cube.VAO);

    glGenBuffers(1, &Cube.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, Cube.VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);

    UI Cubetexture;
    glGenTextures(1, &Cubetexture);
    glBindTexture(GL_TEXTURE_2D, Cubetexture);
    int ImgWid, ImgHeig, ColorChan;
    unsigned char *data = stbi_load("../bottom.jpg", &ImgWid, &ImgHeig, &ColorChan, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ImgWid, ImgHeig, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else
    {
        printf("Failed to load data\n");
    }
    stbi_image_free(data);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    VecV Eye;
    Eye.X = 0.2;
    Eye.Y = 0.5;
    Eye.Z = 5;

    VecV Center;
    Center.X = 0;
    Center.Y = 1;
    Center.Z = 0;

    VecV Up;
    Up.X = 0;
    Up.Y = 1;
    Up.Z = 0;

    glfwSwapInterval(6);

    HANDLE File = CreateFileA("../Canon.obj", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    LARGE_INTEGER Size = {};
    GetFileSizeEx(File, &Size);

    char *Buffer = (char *)malloc(Size.QuadPart);
    ReadFile(File, Buffer, Size.QuadPart, 0, 0);
    obj Model = ParseObj((u8 *)Buffer, Size.QuadPart);

    UI ModelVao, ModelVbo;
    glGenVertexArrays(1, &ModelVao);
    glBindVertexArray(ModelVao);

    glGenBuffers(1, &ModelVbo);
    glBindBuffer(GL_ARRAY_BUFFER, ModelVbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * Model.VertexCount * 3, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * Model.VertexCount * 3, Model.Vertex);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    Data Sphere = SphereData();
    float IntialPos = 0.35;
    float angle = (45 * 3.14) / 180;
    float IntialVel = (80 * 16.0f) / 1000.0f;
    float FlightTime = 0;
    float IntialPosY = 0.40;
    bool Start = false;
    float PathX = 0.35;
    float PathY = 0.40;
    float PathTime = 0;

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0, 0, 0, 1);
        glEnable(GL_DEPTH_TEST);

        glUseProgram(Pgm);

        glm::mat4 Proj(1.0f);
        Proj = glm::perspective(glm::radians(65.0f), (float)WND_WIDTH / (float)WND_HEIGHT, 0.1f, 500.0f);
        glUniformMatrix4fv(glGetUniformLocation(Pgm, "Proj"), 1, 0, glm::value_ptr(Proj));

        glm::mat4 CanonModel(1.0f);
        CanonModel = glm::translate(CanonModel, glm::vec3(0, 0, 0));
        CanonModel = glm::scale(CanonModel, glm::vec3(5, 5, 5));
        glUniformMatrix4fv(glGetUniformLocation(Pgm, "Model"), 1, 0, glm::value_ptr(CanonModel));
        glm::mat4 View(1.0f);
        View = glm::lookAt(glm::vec3(1, 1, Eye.Z), glm::vec3(Center.X, Center.Y, Center.Z), glm::vec3(Up.X, Up.Y, Up.Z));
        glUniformMatrix4fv(glGetUniformLocation(Pgm, "View"), 1, 0, glm::value_ptr(View));

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            Eye.Z -= 1;
        }
        else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        {
            Eye.Z += 1;
        }

        glUniform3f(glGetUniformLocation(Pgm, "ObjColor"), 0.01, 1, 0.5);

        glBindVertexArray(ModelVao);
        glBindBuffer(GL_ARRAY_BUFFER, ModelVbo);
        glDrawArrays(GL_TRIANGLES, 0, Model.VertexCount);

        //* Projectile
        if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
        {
            Start = true;
        }
        else if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            Start = false;
        }
        else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            IntialPos = 0.35;
            IntialPosY = 0.40;
            FlightTime = 0;
        }
        else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            IntialVel += 0.1;
        }
        else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            IntialVel -= 0.1;
        }
        else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            angle += 0.1;
        }
        else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            angle -= 0.1;
        }

        DrawSphere(Sphere, -1 * IntialPos, IntialPosY, -0.2, 0.06);

        if (Start == true)
        {
            IntialPos = IntialPos + IntialVel * cos(angle) * FlightTime;
            IntialPosY = IntialPosY + IntialVel * sin(angle) * FlightTime - 0.5 * (9.8) * FlightTime * FlightTime;

            FlightTime += 16 / 1000.0f;
            if (IntialPosY <= 0)
            {
                Start = false;
            }
        }
        else
        {
            DrawSphere(Sphere, -1 * IntialPos, IntialPosY, -0.2, 0.06);
        }

        //* path
        while (PathTime <= 1)
        {
            PathX = PathX + IntialVel * cos(angle) * PathTime;
            PathY = PathY + IntialVel * sin(angle) * PathTime - 0.5 * (9.8) * PathTime * PathTime;

            DrawSphere(Sphere, -1 * PathX, PathY, -0.2, 0.01);
            PathTime += 16 / 1000.0f;
        }

        PathTime = 0;
        PathX = 0.35;
        PathY = 0.40;

        //* Surface
        glm::mat4 SurfaceModel(1.0f);
        SurfaceModel = glm::translate(SurfaceModel, glm::vec3(0, 0, 0));
        SurfaceModel = glm::scale(SurfaceModel, glm::vec3(0.5, 0, 0.5));
        glUniformMatrix4fv(glGetUniformLocation(Pgm, "Model"), 1, 0, glm::value_ptr(SurfaceModel));
        glUniform3f(glGetUniformLocation(Pgm, "ObjColor"), 1, 5, 5);
        glBindVertexArray(Cube.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, Cube.VBO);
        glBindTexture(GL_TEXTURE_2D, Cubetexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 07;
}