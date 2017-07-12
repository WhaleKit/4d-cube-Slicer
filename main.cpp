// GLEW нужно подключать до GLFW.
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <exception>
#include <algorithm>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/projection.hpp"



#include "SOIL.h"

#include "wk4dcore.h"
#include "wk4dmatrix5.h"
#include "wk4dpointsofview.h"
#include "wk4dgraphics.h"
#include "chunc4d.h"

using namespace std;



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
                                              "resources\\geomshader.geom",
                                              "resources\\fragmshader1.frag");

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
    //model = glm::translate(model, glm::vec3(-0.5f, -0.5f, -0.5f));
    projection = glm::perspective( glm::radians(75.0f) ,
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


    glm::tvec2<double> cursorPos;
    glm::tvec2<double> lastFrameCursorPos = {0,0};


    mesh tempMesh{{},{}};
    tempMesh.m_indices.reserve(30);
    tempMesh.m_points.reserve(30);
    bool pause=false;

    auto myKeyCallback = [&](GLFWwindow* window, int key, int scancode, int action, int mode){
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS){
            pause = !pause;
        }
    };
    glfwSetWindowUserPointer(window, &myKeyCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window,
        [](GLFWwindow* window, int key, int scancode, int action, int mode){

        void* pv = glfwGetWindowUserPointer(window);
        decltype(myKeyCallback)* p = static_cast<decltype(myKeyCallback)*>(pv);
        (*p)(window, key, scancode, action, mode);

    });


    WK4d::chunc4d<> chuncToSlice;
    chuncToSlice.blockSize = 1/8.f;
    glm::ivec4 idx;
    for (idx.w=0; idx.w<8; ++idx.w)
        for (idx.z=0; idx.z<8; ++idx.z)
            for (idx.y=0; idx.y<8; ++idx.y)
                for (idx.x=0; idx.x<8; ++idx.x){

        glm::vec4 posi = glm::vec4(idx) - glm::vec4(3.5);
        if(glm::length(posi) <= 3.9/* && glm::length(posi) > 3.8*/){
            chuncToSlice.at(idx)
                    = WK4d::chunc4d<>::blocks::solid;
        }else{
            chuncToSlice.at(idx)
                    = WK4d::chunc4d<>::blocks::air;
        }
    }

    for (int wi=0; wi<chuncToSlice.aSize; ++wi){
        chuncToSlice.at(wi,1,1,1)
                = WK4d::chunc4d<>::blocks::air;
    }


    float ang[6] = {0., 0., 0., 0., 0.};
    float shift = 0;
    WK4d::SpaceSimPointOfView World4dCenter;
//    startMyCam.myCoord = WK4d::vec4{0.501f, 0.501f, 0.501f, 0.501f};
    World4dCenter.myCoord = chuncToSlice.blockOriginAt(4,4,4,4);


//    WK4d::AATesseract blockToSlice{1};
//    blockToSlice.position = {-0.5, -0.5, -0.5, -0.5};
    WK4d::SpaceSimPointOfView my4dCam;
    my4dCam.myCoord = World4dCenter.myCoord;
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


        float speed = 0.01;
        float anglespeed = 0.03;
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            speed = 0.1;
            //anglespeed = 0.1;
        }else{
            speed = 0.01;
            anglespeed = 0.03;
        }
        my4dCam = World4dCenter;
        my4dCam.rotateForwardAna(ang[3]);
        my4dCam.rotateRightAna(ang[5]);
        my4dCam.rotateUpAna(ang[4]);
        my4dCam.rotateForwardUp(ang[1]);
        my4dCam.rotateRightUp(ang[2]);
        my4dCam.rotateForwardRight(ang[0]);


        my4dCam.myCoord += my4dCam.planeImOn.getNormal()* shift;


        if (pause){
            //ang[1] += anglespeed*0.5;
            //ang[1] += anglespeed*0.5;
            ang[0] -= anglespeed;
            ang[2] -= anglespeed*0.5;
        }
        if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
            shift += (speed/4);
        }
        if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
           shift -= (speed/4);
        }
        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            World4dCenter.rotateUpAna(anglespeed);
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            World4dCenter.rotateUpAna(-anglespeed);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
                World4dCenter.rotateForwardAna(anglespeed);
            }
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
                World4dCenter.rotateForwardAna(-anglespeed);
            }
            my4dCam.normalize();
        }
        else if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS){
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
                World4dCenter.rotateRightAna(anglespeed);
            }
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
                World4dCenter.rotateRightAna(-anglespeed);
            }
        }else{
            if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
                World4dCenter.rotateForwardRight(anglespeed);
            }
            if(glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
                World4dCenter.rotateForwardRight(-anglespeed);
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
        }
        my4dCam.normalize();


//        vector<vector<WK4d::vec4>> ans = WK4d::tesseractCrossSectionByHyperPlane(
//                    blockToSlice, my4dCam.planeImOn);
//        WK4d::matrix5x5 tr = my4dCam.getWorldToHyperplaneLocalTransformMatrix();

//        glm::vec4 figCenter = glm::vec4(tr*(blockToSlice.position + (blockToSlice.size/2.f)));

//        tempMesh.m_points.resize(0);
//        tempMesh.m_indices.resize(0);

//        size_t j = 0;
//        for (vector<WK4d::vec4>& face: ans){
//            tempMesh.m_points.resize(tempMesh.m_points.size() + face.size()*3);
//            for (size_t i=0; i<face.size()*3; i+=3){
//                face[i/3] = tr*face[i/3];
//                tempMesh.m_points[j*3+i+0] = face[i/3].y;
//                tempMesh.m_points[j*3+i+1] = face[i/3].z;
//                tempMesh.m_points[j*3+i+2] = face[i/3].x;
//            }
//            vector<GLuint> toAdd= makeIndexArrayForFace(face, figCenter, j);
//            tempMesh.m_indices.insert(tempMesh.m_indices.end(), toAdd.begin(), toAdd.end());
//            j += face.size();
//        }
//        tempMesh.updateBufsInGPU(GL_STREAM_DRAW);

        vector<std::pair<vector<glm::vec4>, glm::vec4>>
                chuncSlice = chuncToSlice.getSlice(my4dCam.planeImOn);
        WK4d::matrix5x5 tr = my4dCam.getWorldToHyperplaneLocalTransformMatrix();

        tempMesh.m_points.resize(0);
        tempMesh.m_indices.resize(0);
        size_t j = 0;
        for (std::pair<vector<glm::vec4>, glm::vec4>& faceNback: chuncSlice){
            vector<WK4d::vec4>& face = faceNback.first;
            glm::vec4 figCenter = tr*faceNback.second;
            tempMesh.m_points.resize(tempMesh.m_points.size() + face.size()*3);
            for (size_t i=0; i<face.size()*3; i+=3){
                face[i/3] = tr*face[i/3];
                tempMesh.m_points[j*3+i+0] = face[i/3].y;
                tempMesh.m_points[j*3+i+1] = face[i/3].z;
                tempMesh.m_points[j*3+i+2] = face[i/3].x;
            }
            vector<GLuint> toAdd= makeIndexArrayForFace(face, figCenter, j);
            tempMesh.m_indices.insert(tempMesh.m_indices.end(), toAdd.begin(), toAdd.end());
            j += face.size();
        }
        tempMesh.updateBufsInGPU(GL_STREAM_DRAW);


        if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
            auto tr = my4dCam.getHyperplaneLocalToWorldTransformMatrix();
            glm::vec4 coord = tr*glm::vec4{myCamera.pos.z, myCamera.pos.x,
                                                myCamera.pos.y, 0};
            glm::ivec4 myBlock = chuncToSlice.coordToIndex(coord);
            cout << "myBlock: "<<myBlock.x <<", "<<myBlock.y<<", "
                 <<myBlock.z<<", "<< myBlock.w<<" "
                 << (chuncToSlice.at(myBlock)==WK4d::chunc4d<>::blocks::solid) <<endl;
        }


        view = glm::rotate(glm::mat4{}, -myCamera.pitch, glm::vec3{1.0f, 0.0f, 0.0f});
        view = glm::rotate(view, -myCamera.yaw, glm::vec3{0.0f, 1.0f, 0.0f});
        view = glm::translate(view, -myCamera.pos);

        glProgramUniformMatrix4fv(shadr.m_shaderProg, viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

        glUseProgram(shadr.m_shaderProg);
        tempMesh.bind();
        glDrawElements(GL_TRIANGLES, tempMesh.m_indices.size(), GL_UNSIGNED_INT, 0);

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

void printMatrix (WK4d::matrix5x5 const& m)
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
    WK4d::AATesseract ts{1};
    ts.position = WK4d::vec4{-0.5, -0.5, -0.5, -0.5};


    WK4d::FPSPointOfView myPOV;
    myPOV.myFront = WK4d::vec4(0,0,0,1);
    myPOV.planeImOn = WK4d::hyperPlane4(WK4d::vec4{0,0,0,0},WK4d::vec4{1,0,0,0});
    vector<vector<WK4d::vec4>> faces = WK4d::tesseractCrossSectionByHyperPlane(
                ts, myPOV.planeImOn);
    auto tr = myPOV.getWorldToHyperplaneLocalTransformMatrix();

    mesh sliceMesh({},{});
    for (vector<WK4d::vec4>& face: faces){
        sliceMesh.m_points.resize(face.size()*3);
        sliceMesh.m_indices.resize(face.size()*3);
        for (size_t i =0; i<face.size()*3; i+=3){
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







