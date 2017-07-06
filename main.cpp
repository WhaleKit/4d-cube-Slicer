// GLEW нужно подключать до GLFW.
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <streambuf>
#include <vector>
#include <chrono>
#include <cmath>
#include <exception>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"



#include "SOIL.h"

#include "mygeomfunctions.h"
#include "chunc4d.h"

using namespace std;

struct mesh;

struct mySimpleShaderProgram;

struct mySimpleShaderProgram
{
    GLuint m_vertexShader;
    GLuint m_fragmentShader;
    GLuint m_shaderProg;
    bool m_correct = false;
    std::string failureReason;
    mySimpleShaderProgram(std::istream& vert, std::istream& fragm)
        :mySimpleShaderProgram(std::string(std::istreambuf_iterator<char>(vert),
                                           std::istreambuf_iterator<char>()),
                               std::string(std::istreambuf_iterator<char>(fragm),
                                           std::istreambuf_iterator<char>())
                               )
    {}
    mySimpleShaderProgram(string const& vert, string const& fragm)
    {
        m_vertexShader   = glCreateShader(GL_VERTEX_SHADER);
        m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        GLchar const*  c = vert.c_str();
        glShaderSource(m_vertexShader, 1, &c, NULL);
        c = fragm.c_str();
        glShaderSource(m_fragmentShader, 1, &c, NULL);


        glCompileShader(m_vertexShader);

        GLint success;
        GLchar infolog[512];
        glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(m_vertexShader, 512, NULL, infolog);
            failureReason = "vertex shader not compiled: ";
            failureReason.append(infolog);
            m_correct = false;
            glDeleteShader(m_vertexShader);
            m_vertexShader = 0;
            return;
        }

        glCompileShader(m_fragmentShader);
        glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(m_fragmentShader, 512, NULL, infolog);
            failureReason = "fragment shader not compiled: ";
            failureReason.append(infolog);
            m_correct = false;
            glDeleteShader(m_vertexShader);
            m_vertexShader = 0;
            glDeleteShader(m_fragmentShader);
            m_fragmentShader = 0;
            return;
        }

        m_shaderProg = glCreateProgram();
        glAttachShader(m_shaderProg, m_vertexShader);
        glAttachShader(m_shaderProg, m_fragmentShader);
        glLinkProgram(m_shaderProg);

        glGetShaderiv(m_shaderProg, GL_LINK_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(m_shaderProg, 512, NULL, infolog);
            failureReason = "shader program not linked: ";
            failureReason.append(infolog);
            m_correct = false;
            glDeleteShader(m_vertexShader);
            m_vertexShader = 0;
            glDeleteShader(m_fragmentShader);
            m_fragmentShader = 0;
            glDeleteProgram(m_shaderProg);
            m_shaderProg = 0;
            return;
        }
        m_correct = true;
    }

    mySimpleShaderProgram(mySimpleShaderProgram && oth)
        :m_vertexShader(oth.m_vertexShader)
        ,m_fragmentShader(oth.m_fragmentShader)
        ,m_shaderProg(oth.m_shaderProg)
        ,m_correct(oth.m_correct)
        ,failureReason(std::move(oth.failureReason))
    {
        oth.m_vertexShader=0;
        oth.m_fragmentShader=0;
        oth.m_shaderProg=0;
        oth.m_correct = false;
    }

    ~mySimpleShaderProgram()
    {
        glDeleteShader(m_vertexShader);
        glDeleteShader(m_fragmentShader);
        glDeleteProgram(m_shaderProg);
    }

    static mySimpleShaderProgram fromFile(string const& vertFile, string const& fragFile)
    {
        ifstream vert("resources\\vertshader1.vert");
        ifstream frag("resources\\fragmshader1.frag");
        return mySimpleShaderProgram{vert, frag};
    }

    operator bool ()
    {
        return m_correct;
    }
};

struct mesh{
public:
    GLuint m_VAO;
    GLuint m_VBO;
    GLuint m_EBO;
    std::vector<GLfloat> m_points;
    std::vector<GLuint> m_indices;
    mesh(mesh && oth)
        : m_points(std::move(oth.m_points)),
          m_indices(std::move(oth.m_indices))
    {
        m_VAO = oth.m_VAO;
        m_VBO = oth.m_VBO;
        m_EBO = oth.m_EBO;
        oth.m_VAO = 0;
        oth.m_VBO = 0;
        oth.m_EBO = 0;
    }
    mesh(mesh const& oth)
        : mesh(oth.m_points, oth.m_indices)
    {}
    mesh(std::vector<GLfloat> const& points, std::vector<GLuint> const& indexes
         ,GLenum mode = GL_STATIC_DRAW)
        :m_points(points), m_indices(indexes)
    {

        glGenVertexArrays(1, &m_VAO);
        glCreateBuffers(1, &m_VBO);
        glCreateBuffers(1, &m_EBO);
        updateBufsInGPU();
        bind();

        glVertexAttribPointer(0,3, GL_FLOAT,GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0 );
        glEnableVertexAttribArray(0);
        unbind();
    }
    ~mesh()
    {
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
        glDeleteVertexArrays(1, &m_VAO);
    }
    void updateBufsInGPU(GLenum mode = GL_STATIC_DRAW)
    {
        //Viva la DSA!
        glNamedBufferData(m_VBO, m_points.size()*sizeof(GLfloat),
                          m_points.data(), mode);
        glNamedBufferData(m_EBO, m_indices.size()*sizeof(GLuint),
                          m_indices.data(), mode);
    }

    void bind()
    {
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    }
    static void unbind()
    {
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
};


int main()
{
    //Инициализация GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK){
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    //шейдорыы

    while(glGetError() != GL_NO_ERROR)
    {    }//free error queue

    //mySimpleShaderProgram shadr{string(vertShader1), string(fragmShader2)};
    mySimpleShaderProgram shadr
            = mySimpleShaderProgram::fromFile("resources\\vertshader1.vert",
                                              "resources\\fragmshader1.frag");
    mesh cube({
                  0, 0, 0,   0, 0, 1, //лн 01
                  0, 1, 0,   0, 1, 1, //лв 23
                  1, 0, 0,   1, 0, 1, //пн 45
                  1, 1, 0,   1, 1, 1  //пв 67
              },{
                  0,2,3,   0,3,1,// левый квадрат 0 1 2 3 /
                  4,7,6,   4,5,7,// правы квадрат 4 5 6 7

                  2,6,7,   2,7,3,// верхн квадрат 2 3 6 7 /
                  0,5,4,   0,1,5,// нижни квадрат 0 1 4 5

                  0,4,6,   0,6,2,// ближн увадрат 0 2 4 6
                  1,7,5,   1,3,7 // дальн квадрат 1 3 5 7
              });

    struct FPSPointOfView{
        glm::vec3 pos;
        float pitch;//in rads
        float yaw;//in rads
        void moveForward(float am){
            glm::vec3 res(0,0,1);
            auto m = glm::rotate(glm::mat4{}, yaw, {0.f, 1.f, 0.f});
            m = glm::rotate(m, pitch, {1.f, 0.f, 0.f});
            pos += -glm::vec3((m*glm::vec4(res, 1.f))*am);
        }
        void moveRight(float am){
            glm::vec3 res(0,0,1);
            auto m = glm::rotate(glm::mat4{}, yaw+glm::radians(-90.f), {0.f, 1.f, 0.f});
            pos += -glm::vec3((m*glm::vec4(res, 1.f))*am);
        }
        void moveUp(float am){
            pos.y += am;
        }
    };
    FPSPointOfView myCamera;
    myCamera.pos = glm::vec3(-1.0f, 0.0f, -1.0f);
    myCamera.pitch =0;
    myCamera.yaw  = glm::radians(270.0f);

    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
    model = glm::translate(model, glm::vec3(-0.5f, -0.5f, -0.5f));
    projection = glm::perspective( glm::radians(45.0f) ,
                                   (GLfloat)width / (GLfloat)height,
                                   0.1f, 100.0f);
    GLint modelLoc= glGetUniformLocation(shadr.m_shaderProg, "model");
    GLint viewLoc = glGetUniformLocation(shadr.m_shaderProg, "view");
    GLint projLoc = glGetUniformLocation(shadr.m_shaderProg, "projection");

    glProgramUniformMatrix4fv(shadr.m_shaderProg, modelLoc,
                              1, GL_FALSE, glm::value_ptr(model));
    glProgramUniformMatrix4fv(shadr.m_shaderProg, viewLoc,
                              1, GL_FALSE, glm::value_ptr(view));
    glProgramUniformMatrix4fv(shadr.m_shaderProg, projLoc,
                              1, GL_FALSE, glm::value_ptr(projection));


    GLint nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum nr of vertex attributes supported: " << nrAttributes
              << "tan( "<< glm::radians(180.f) << ") = "
              << glm::tan(glm::radians(180.f)) << std::endl;
    if (!shadr){
        cout << endl << shadr.failureReason << flush;
        return 0;
    }

    glEnable(GL_DEPTH_TEST);
//    glEnable(GL_CULL_FACE);
//    glFrontFace(GL_CCW);
//    glCullFace(GL_FRONT);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    float prevFrameElapsed=0;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window,
        [](GLFWwindow* window, int key, int scancode, int action, int mode){
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
    });
    glm::tvec2<double> cursorPos;
    glm::tvec2<double> lastFrameCursorPos = {0,0};



    WK4dG::AATesseract myBlock(1);
    myBlock.position = WK4dG::vec4{-0.5, -0.5, -0.5, -0.5};
//    WK4dG::hyperPlane4 myPlane(WK4dG::vec4{0,0,0,0},
//                               WK4dG::vec4{0,0,0,1} );
    WK4dG::FPSPointOfView my4dCam;

    mesh tempMesh{{},{}};
    tempMesh.m_indices.reserve(30);
    tempMesh.m_points.reserve(30);
    while(!glfwWindowShouldClose(window))
    {
        //timey-whiney stuff
        end = std::chrono::system_clock::now();
        float elapsed = ((end - start)/std::chrono::milliseconds(1))/1000.0f;
        prevFrameElapsed = elapsed;

        lastFrameCursorPos = cursorPos;
        glfwGetCursorPos(window, &cursorPos.x, &cursorPos.y);
        glm::tvec2<double> cursorMovement = cursorPos-lastFrameCursorPos;

        myCamera.yaw += -cursorMovement.x/1000;
        myCamera.pitch += -cursorMovement.y/1000;

        if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
            cout << "\ncoords: ("<<myCamera.pos.x << ", " << myCamera.pos.y
                                        << ", " << myCamera.pos.z
                 << "), yaw: " << myCamera.yaw
                 << ", pitch: " << myCamera.pitch;

            cout << "\n4dcam front: ("<<my4dCam.myFront.x << ", " << my4dCam.myFront.y
                           << ", " << my4dCam.myFront.z << ", " << my4dCam.myFront.w;
            cout << "\n4dcam ana: ("<<my4dCam.planeImOn.A << ", " << my4dCam.planeImOn.B
                        << ", " << my4dCam.planeImOn.C  << ", " << my4dCam.planeImOn.D;
            cout << flush;
        }
        float speed = 0.01;
        float anglespeed = 0.1;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            speed = 0.1;
            anglespeed = 0.1;
        }else{
            speed = 0.01;
            anglespeed = 0.001;
        }

        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
//                my4dCam.planeImOn.setNormal(WK4dG::rotationMatrix(WK4dG::axes::y,WK4dG::axes::w,
//                                                        glm::radians(5.f))
//                                    *my4dCam.planeImOn.getNormal()  );
                my4dCam.rotateForwardAna(anglespeed);
            }
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
//                my4dCam.planeImOn.setNormal(WK4dG::rotationMatrix(WK4dG::axes::y,WK4dG::axes::w,
//                                                        glm::radians(-5.f))
//                                    *my4dCam.planeImOn.getNormal()  );
                my4dCam.rotateForwardAna(-anglespeed);
            }
            my4dCam.normalize();
        }
        else if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS){
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
//                my4dCam.planeImOn.setNormal(WK4dG::rotationMatrix(WK4dG::axes::x,WK4dG::axes::w,
//                                                        glm::radians(5.f))
//                                    *my4dCam.planeImOn.getNormal()  );
                my4dCam.rotateRightAna(anglespeed);
            }
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
//                my4dCam.planeImOn.setNormal(WK4dG::rotationMatrix(WK4dG::axes::x,WK4dG::axes::w,
//                                                        glm::radians(-5.f))
//                                    *my4dCam.planeImOn.getNormal()  );
                my4dCam.rotateRightAna(-anglespeed);
            }
        }else{
            if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
                my4dCam.rotateForwardRight(anglespeed);
            }
            if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
                my4dCam.rotateForwardRight(-anglespeed);
            }
            if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
                myCamera.moveForward(speed);
            }
            if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
                myCamera.moveForward(-speed);
            }
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
                myCamera.moveRight(-speed);
            }
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
                myCamera.moveRight(speed);
            }
            if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
                myCamera.moveUp(speed);
            }
            if(glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS ||
                    glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
                myCamera.moveUp(-speed);
            }
            my4dCam.normalize();
        }
        vector<vector<WK4dG::vec4>> ans = WK4dG::tesseractCrossSectionByHyperPlane(
                                            myBlock, my4dCam.planeImOn);



        view = glm::rotate(glm::mat4{}, -myCamera.pitch, glm::vec3{1.0f, 0.0f, 0.0f});
        view = glm::rotate(view, -myCamera.yaw, glm::vec3{0.0f, 1.0f, 0.0f});
        view = glm::translate(view, -myCamera.pos);

        glProgramUniformMatrix4fv(shadr.m_shaderProg, viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);


        glUseProgram(shadr.m_shaderProg);
        WK4dG::matrix5x5 tr = my4dCam.getWorldToHyperplaneLocalTransformMatrix();

        for (vector<WK4dG::vec4>& face : ans){
            tempMesh.m_points.resize(face.size()*3);
            tempMesh.m_indices.resize((face.size()-2)*3);
            for (size_t i=0; i<face.size()*3; i+=3){
                face[i/3] = tr*face[i/3];
                tempMesh.m_points[i+0] = face[i/3].y;
                tempMesh.m_points[i+1] = face[i/3].z;
                tempMesh.m_points[i+2] = face[i/3].x;
            }
            for (size_t i=0; i<(face.size()-2)*3; i+=3){
                tempMesh.m_indices[i+0] = 0;
                tempMesh.m_indices[i+1] = i/3+1;
                tempMesh.m_indices[i+2] = i/3+2;
            }
            tempMesh.updateBufsInGPU(GL_STREAM_DRAW);
            tempMesh.bind();
            glDrawElements(GL_TRIANGLES, tempMesh.m_indices.size(), GL_UNSIGNED_INT, 0);
        }

//        cube.bind();
//        glDrawElements(GL_TRIANGLES, cube.m_indexes.size(), GL_UNSIGNED_INT, 0);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}






//#include <iostream>
//#include "mygeomfunctions.h"
//#include "GLFW/glfw3.h"
//using namespace myGeomFunctions;
//using namespace std;

void printMatrix (WK4dG::matrix5x5 const& m)
{
    cout << "\n{";
    for (int r=0; r<5; ++r){
        cout << "\n\t" << "{";
        for (int c=0; c<5; ++c){
            cout << m[r][c] << ", ";
        }
        cout << "}";
    }
    cout <<"\n}";
}

int main_(int argc, char *argv[])
{

    //Инициализация GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK){
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    while(glGetError() != GL_NO_ERROR)
    {    }//free error queue

    mySimpleShaderProgram shadr
            = mySimpleShaderProgram::fromFile("resources\\vertshader1.vert",
                                              "resources\\fragmshader1.frag");
    WK4dG::AATesseract ts{1};
    ts.position = WK4dG::vec4{-0.5, -0.5, -0.5, -0.5};


    WK4dG::FPSPointOfView myPOV;
    myPOV.myFront = WK4dG::vec4(0,0,0,1);
    myPOV.planeImOn = WK4dG::hyperPlane4(WK4dG::vec4{0,0,0,0},WK4dG::vec4{1,0,0,0});
    vector<vector<WK4dG::vec4>> faces = WK4dG::tesseractCrossSectionByHyperPlane(
                ts, myPOV.planeImOn);
    auto tr = myPOV.getWorldToHyperplaneLocalTransformMatrix();

    mesh sliceMesh({},{});
    for (vector<WK4dG::vec4>& face: faces){
        sliceMesh.m_points.resize(face.size()*3);
        sliceMesh.m_indices.resize(face.size()*3);
        for (int i=0; i<face.size()*3; i+=3){
            face[i/3] = tr*face[i/3];
            sliceMesh.m_points[i+0] = face[i/3].y;
            sliceMesh.m_points[i+1] = face[i/3].z;
            sliceMesh.m_points[i+2] = face[i/3].x;

            sliceMesh.m_indices[i+0]=0;
            sliceMesh.m_indices[i+1]=i/3+1;
            sliceMesh.m_indices[i+2]= (i/3)+2;
        }
    }

    return 0;
}







