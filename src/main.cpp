/*
 *  Copyright 2022 Au Doan Ngoc
 *
 *  Author: Au Doan Ngoc
 *  Date: Dec 21, 2022
 *  Describe: Draw the cube with openGL
*/

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <cstring>
#include <GL/glew.h>
#include <GL/freeglut.h>

#define ToRadian(x) (float)(((x) * M_PI / 180.0f))
#define WINDOW_WIDTH    1200
#define WINDOW_HEGIHT   900

GLuint VBO;
GLuint IBO;
GLint gRotationLocation;

const char* pVSFileName = "shader.vs";
const char* pFSFileName = "shader.fs";

struct Vector3f
{
    float x;
    float y;
    float z;

    Vector3f() {}

    Vector3f(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }
};

struct Vector4f
{
    float x;
    float y;
    float z;
    float w;

    Vector4f()
    {
    }

    Vector4f(float _x, float _y, float _z, float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    Vector4f(const Vector3f& v, float _w)
    {
        x = v.x;
        y = v.y;
        z = v.z;
        w = _w;
    }

    void Print(bool endl = true) const
    {
        printf("(%f, %f, %f, %f)", x, y, z, w);

        if (endl) {
            printf("\n");
        }
    }

    Vector3f to3f() const
    {
        Vector3f v(x, y, z);
        return v;
    }

    float Length() const
    {
        float len = sqrtf(x * x + y * y + z * z + w * w);
        return len;
    }

    Vector4f& Normalize();

    float Dot(const Vector4f& v) const
    {
        float ret = x * v.x + y * v.y + z * v.z + w * v.w;
        return ret;
    }
};

struct Vertex 
{
    Vector3f pos;
    Vector3f color;

    Vertex() {}
    Vertex(float x, float y, float z)
    {
        pos = Vector3f(x, y, z);

        float red = (float)rand() / (float)RAND_MAX;
        float green = (float)rand() / (float)RAND_MAX;
        float blue = (float)rand() / (float)RAND_MAX;
        color = Vector3f(red, green, blue);
    }
};

class Matrix4f
{
public:
    float m[4][4];

    Matrix4f(){}

    Matrix4f(float a00, float a01, float a02, float a03,
             float a10, float a11, float a12, float a13,
             float a20, float a21, float a22, float a23,
             float a30, float a31, float a32, float a33)
    {
        m[0][0] = a00; m[0][1] = a01; m[0][2] = a02; m[0][3] = a03;
        m[1][0] = a10; m[1][1] = a11; m[1][2] = a12; m[1][3] = a13;
        m[2][0] = a20; m[2][1] = a21; m[2][2] = a22; m[2][3] = a23;
        m[3][0] = a30; m[3][1] = a31; m[3][2] = a32; m[3][3] = a33;
    }

    inline Matrix4f operator*(const Matrix4f& Right) const
    {
        Matrix4f Ret;

        for (unsigned int i = 0 ; i < 4 ; i++)
        {
            for (unsigned int j = 0 ; j < 4 ; j++)
            {
                Ret.m[i][j] = m[i][0] * Right.m[0][j] +
                              m[i][1] * Right.m[1][j] +
                              m[i][2] * Right.m[2][j] +
                              m[i][3] * Right.m[3][j];
            }
        }

        return Ret;
    }

    Vector4f operator*(const Vector4f& v) const
    {
        Vector4f r;

        r.x = m[0][0]* v.x + m[0][1]* v.y + m[0][2]* v.z + m[0][3]* v.w;
        r.y = m[1][0]* v.x + m[1][1]* v.y + m[1][2]* v.z + m[1][3]* v.w;
        r.z = m[2][0]* v.x + m[2][1]* v.y + m[2][2]* v.z + m[2][3]* v.w;
        r.w = m[3][0]* v.x + m[3][1]* v.y + m[3][2]* v.z + m[3][3]* v.w;

        return r;
    }
};

bool ReadFile(const char* pFileName, std::string& outFile)
{
    std::ifstream f(pFileName);

    bool ret = false;

    if (f.is_open())
    {
        std::string line;
        while (getline(f, line))
        {
            outFile.append(line);
            outFile.append("\n");
        }

        f.close();

        ret = true;
    }
    else
    {
        printf("Error read file!\n");
        ret = false;
    }

    return ret;
}

static void RenderSceneCB()
{
    glClear(GL_COLOR_BUFFER_BIT);

    static float Scale = 0.0f;
    static float Delta = 0.02f;

    Scale += Delta;

    Matrix4f Rotation(cosf(Scale), 0.0f, -sinf(Scale),  0.0f,
                      0.0f,        1.0f, 0.0f,          0.0f,
                      sinf(Scale), 0.0f, cosf(Scale),   0.0f,
                      0.0f,        0.0f, 0.0f,          1.0f);

    Matrix4f Translation(1.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 1.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 1.0f, 2.0f,
                         0.0f, 0.0f, 0.0f, 1.0f);

    float FOV = 90.0f;
    float tanHalfFOV = tanf(ToRadian(FOV / 2.0f));
    float f = 1/tanHalfFOV;
    float ar = (float)WINDOW_WIDTH / (float)WINDOW_HEGIHT;

    Matrix4f Projection(f/ar, 0.0f, 0.0f, 0.0f,
                        0.0f, f,    0.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f,
                        0.0f, 0.0f, 1.0f, 0.0f);

    Matrix4f FinalMatrix = Projection * Translation * Rotation;

    glUniformMatrix4fv(gRotationLocation, 1, GL_TRUE, &FinalMatrix.m[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glutPostRedisplay();

    glutSwapBuffers();

    usleep(15000);
}

static void CreateIndexBuffer()
{
    unsigned int Indices[] = {
                              0, 1, 2,
                              1, 3, 4,
                              5, 6, 3,
                              7, 3, 6,
                              2, 4, 7,
                              0, 7, 6,
                              0, 5, 1,
                              1, 5, 3,
                              5, 0, 6,
                              7, 4, 3,
                              2, 1, 4,
                              0, 2, 7
                             };
    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

static void CreateVertexBuffer()
{
    Vertex Vertices[8];

    Vertices[0] = Vertex(0.5f, 0.5f, 0.5f);
    Vertices[1] = Vertex(-0.5f, 0.5f, -0.5f);
    Vertices[2] = Vertex(-0.5f, 0.5f, 0.5f);
    Vertices[3] = Vertex(0.5f, -0.5f, -0.5f);
    Vertices[4] = Vertex(-0.5f, -0.5f, -0.5f);
    Vertices[5] = Vertex(0.5f, 0.5f, -0.5f);
    Vertices[6] = Vertex(0.5f, -0.5f, 0.5f);
    Vertices[7] = Vertex(-0.5f, -0.5f, 0.5f);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0)
    {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    const GLchar* p[1];
    p[0] = pShaderText;

    GLint Lengths[1];
    Lengths[0] = (GLint)strlen(pShaderText);

    glShaderSource(ShaderObj, 1, p, Lengths);

    glCompileShader(ShaderObj);

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders()
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0)
    {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    std::string vs, fs;

    if (!ReadFile(pVSFileName, vs))
    {
        exit(1);
    };

    AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

    if (!ReadFile(pFSFileName, fs))
    {
        exit(1);
    };

    AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = {0};

    glLinkProgram(ShaderProgram);

    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    gRotationLocation = glGetUniformLocation(ShaderProgram, "gRotation");
    if(gRotationLocation == -1)
    {
        printf("Error getting uniform location of 'gScale'\n");
        exit(1);
    }

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success)
    {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);
}

int main(int argc, char** argv)
{
    srandom(getpid());

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);

    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEGIHT);

    int x = 200;
    int y = 100;
    glutInitWindowPosition(x, y);
    int win = glutCreateWindow("tutorial");
    printf("window id: %d\n", win);

    GLenum res = glewInit();
    if (res != GLEW_OK)
    {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    GLclampf Red = 0.0f, Green = 0.0f, Blue = 0.0f, Alpha = 0.0f;
    glClearColor(Red, Green, Blue, Alpha);

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    CreateVertexBuffer();
    CreateIndexBuffer();

    CompileShaders();

    glutDisplayFunc(RenderSceneCB);

    glutMainLoop();

    return 0;
}
