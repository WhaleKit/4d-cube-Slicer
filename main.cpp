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
#include <algorithm>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/projection.hpp"



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
    GLuint m_geomShader;
    GLuint m_shaderProg;
    bool m_correct = false;
    std::string failureReason;

    mySimpleShaderProgram(std::istream& vert, std::istream& geom,
                          std::istream& fragm)
        :mySimpleShaderProgram(std::string(std::istreambuf_iterator<char>(vert),
                                           std::istreambuf_iterator<char>()),
                               std::string(std::istreambuf_iterator<char>(geom),
                                           std::istreambuf_iterator<char>()),
                               std::string(std::istreambuf_iterator<char>(fragm),
                                           std::istreambuf_iterator<char>())
                               )
    {}
    mySimpleShaderProgram(std::istream& vert, std::istream& fragm)
        :mySimpleShaderProgram(std::string(std::istreambuf_iterator<char>(vert),
                                           std::istreambuf_iterator<char>()),
                               std::string(std::istreambuf_iterator<char>(fragm),
                                           std::istreambuf_iterator<char>())
                               )
    {}

    mySimpleShaderProgram(string const& vert, string const& fragm)
        :mySimpleShaderProgram(vert, string(""), fragm)
    {    }

    mySimpleShaderProgram(string const& vert, string const& geom,
                          string const& fragm)
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
        if(!geom.empty()){
            m_geomShader = glCreateShader(GL_GEOMETRY_SHADER);
            c = geom.c_str();
            glShaderSource(m_geomShader, 1, &c, NULL);
            glCompileShader(m_geomShader);
            glGetShaderiv(m_geomShader, GL_COMPILE_STATUS, &success);
            if(!success){
                glGetShaderInfoLog(m_geomShader, 512, NULL, infolog);
                failureReason = "geometry shader not compiled: ";
                failureReason.append(infolog);
                m_correct = false;
                glDeleteShader(m_geomShader);
                m_geomShader = 0;
                return;
            }
        }else{
            m_geomShader=0;
        }
        glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &success);
        if(!success){
            glGetShaderInfoLog(m_vertexShader, 512, NULL, infolog);
            failureReason = "vertex shader not compiled: ";
            failureReason.append(infolog);
            m_correct = false;
            glDeleteShader(m_vertexShader);
            m_vertexShader = 0;
            glDeleteShader(m_geomShader);
            m_geomShader = 0;
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
            glDeleteShader(m_geomShader);
            m_geomShader = 0;
            return;
        }

        m_shaderProg = glCreateProgram();
        glAttachShader(m_shaderProg, m_vertexShader);
        glAttachShader(m_shaderProg, m_fragmentShader);
        if(!geom.empty()){
            glAttachShader(m_shaderProg, m_geomShader);
        }

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
            glDeleteShader(m_geomShader);
            m_geomShader = 0;
            glDeleteProgram(m_shaderProg);
            m_shaderProg = 0;
            return;
        }
        m_correct = true;
    }

    mySimpleShaderProgram(mySimpleShaderProgram && oth)
        :m_vertexShader(oth.m_vertexShader)
        ,m_fragmentShader(oth.m_fragmentShader)
        ,m_geomShader(oth.m_geomShader)
        ,m_shaderProg(oth.m_shaderProg)
        ,m_correct(oth.m_correct)
        ,failureReason(std::move(oth.failureReason))
    {
        oth.m_vertexShader=0;
        oth.m_fragmentShader=0;
        oth.m_geomShader=0;
        oth.m_shaderProg=0;
        oth.m_correct = false;
    }
    ~mySimpleShaderProgram()
    {
        glDeleteShader(m_vertexShader);
        glDeleteShader(m_fragmentShader);
        glDeleteShader(m_geomShader);
        glDeleteProgram(m_shaderProg);
    }

    static mySimpleShaderProgram fromFile(string const& vertFile, string const& fragFile)
    {
        ifstream vert(vertFile);
        ifstream frag(fragFile);
        return mySimpleShaderProgram{vert, frag};
    }
    static mySimpleShaderProgram fromFile(string const& vertFile,
                                          string const& geomFile,
                                          string const& fragFile)
    {
        ifstream vert(vertFile);
        ifstream geom(geomFile);
        ifstream frag(fragFile);
        return mySimpleShaderProgram{vert,geom, frag};
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


void drawLine(glm::vec3 p1, glm::vec3 p2){
    std::vector<GLfloat> m_points{p1.x, p1.y, p1.z,
                                  p2.x, p2.y, p2.z};
    GLuint m_VAO, m_VBO;
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);
    glCreateBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glVertexAttribPointer(0,3, GL_FLOAT,GL_FALSE, 3*sizeof(GLfloat), (GLvoid*)0 );
    glEnableVertexAttribArray(0);

    glNamedBufferData(m_VBO, m_points.size()*sizeof(GLfloat),
                      m_points.data(), GL_STREAM_DRAW);

    glDrawArrays(GL_LINES, m_VBO, 2);

    glDeleteBuffers(1, &m_VBO);
    glDeleteVertexArrays(1, &m_VAO);

}


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
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_FRONT);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    float prevFrameElapsed=0;


    glm::tvec2<double> cursorPos;
    glm::tvec2<double> lastFrameCursorPos = {0,0};

    WK4dG::AATesseract myBlock(1);
    myBlock.position = WK4dG::vec4{-0.5, -0.5, -0.5, -0.5};
    WK4dG::SpaceSimPointOfView my4dCam;

    mesh tempMesh{{},{}};
    tempMesh.m_indices.reserve(30);
    tempMesh.m_points.reserve(30);
    bool pause=false;
    float ang1, ang2, ang3, ang4, ang5;

    auto myKeyCallback = [&](GLFWwindow* window, int key, int scancode, int action, int mode){
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
        if (key == GLFW_KEY_ENTER && action == GLFW_PRESS){
            void* pv = glfwGetWindowUserPointer(window);
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

    WK4dG::matrix5x5 rot = WK4dG::rotationMatrix(WK4dG::axes::z, WK4dG::axes::w, 0.02);

    float ang[6] = {0., 0., 0., 0., 0.};
    WK4dG::SpaceSimPointOfView startMyCam;
    startMyCam.rotateForwardRight(glm::radians(45.f));
    startMyCam.rotateForwardUp(glm::radians(45.f));
    startMyCam.rotateRightAna(glm::radians(45.f));
    startMyCam.rotateUpAna(glm::radians(-45.f));



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
        my4dCam = startMyCam;
        my4dCam.rotateForwardRight(ang[0]);
        my4dCam.rotateForwardUp(ang[1]);
        my4dCam.rotateRightUp(ang[2]);
        my4dCam.rotateForwardAna(ang[3]);
        my4dCam.rotateUpAna(ang[4]);
        my4dCam.rotateRightAna(ang[5]);


        if (pause){
            //ang[1] += anglespeed*0.5;
            ang[1] += anglespeed;
            ang[4] -= anglespeed*0.5;

//            my4dCam.rotateForwardUp(anglespeed*0.5);
//            my4dCam.rotateRightAna(anglespeed);
//            my4dCam.rotateUpAna(anglespeed);
//            myBlock.position = WK4dG::vec4{-0.5, -0.5, -0.5,
//                        std::sin(anglespeed*0.5)*0.7-0.5};
//            my4dCam.myFront = rot * my4dCam.myFront;
//            my4dCam.myUp = rot * my4dCam.myUp;
//            my4dCam.planeImOn.setNormal( rot*my4dCam.planeImOn.getNormal()  );
        }
        if(glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS){
            //my4dCam.rotateUpAna(anglespeed);
            startMyCam.myCoord.w += (speed/4);

        }
        if(glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS){
            //my4dCam.rotateUpAna(anglespeed);
            startMyCam.myCoord.w -= (speed/4);
        }

        if(glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
            //my4dCam.rotateUpAna(anglespeed);
        }
        if(glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
            //my4dCam.rotateUpAna(-anglespeed);
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS){
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
                //my4dCam.rotateForwardAna(anglespeed);
            }
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
                //my4dCam.rotateForwardAna(-anglespeed);
            }
            my4dCam.normalize();
        }
        else if(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS){
            if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
                //my4dCam.rotateRightAna(anglespeed);
            }
            if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
                //my4dCam.rotateRightAna(-anglespeed);
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
        WK4dG::matrix5x5 tr = my4dCam.getWorldToHyperplaneLocalTransformMatrix();
        glm::vec3 figCenter{0,0,0};
        vector<glm::vec3> faceCenters;
        faceCenters.reserve(ans.size());
        {//calculating center of slice figure and individual faces
            int div=0;
            for (vector<WK4dG::vec4>& face: ans){
                glm::vec3 faceCenter{0,0,0};
                int fdiv=0;
                for (WK4dG::vec4& vert: face){
                    vert = tr*vert;
                    ++fdiv;
                    faceCenter+=glm::vec3{vert.x, vert.y,vert.z};
                }
                faceCenter/=fdiv;
                figCenter+=faceCenter;
                ++div;
                faceCenters.push_back(faceCenter);
            }
            figCenter /= div;
        }
        if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
            cout << "\ncoords: ("<<myCamera.pos.x << ", " << myCamera.pos.y
                                        << ", " << myCamera.pos.z
                 << "), yaw: " << myCamera.yaw
                 << ", pitch: " << myCamera.pitch;

            cout << "\n4dcam front: ("<<my4dCam.myFront.x << ", " << my4dCam.myFront.y
                           << ", " << my4dCam.myFront.z << ", " << my4dCam.myFront.w;
            cout << "\n4dcam ana: ("<<my4dCam.planeImOn.A << ", " << my4dCam.planeImOn.B
                        << ", " << my4dCam.planeImOn.C  << ", " << my4dCam.planeImOn.D;
            for (int i=0; i<faceCenters.size(); ++i){
                glm::vec3 vc = glm::vec3{faceCenters[i].y, faceCenters[i].z,
                                        faceCenters[i].x};
                cout << "   dist. to fc " <<i<<" = "
                     << glm::length(vc-myCamera.pos)<<";";
            }
            if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS){
                cout << "facr: ";
                int i;
                cin >> i;
                glm::vec3 vc = glm::vec3{faceCenters[i].y, faceCenters[i].z,
                        faceCenters[i].x};
                myCamera.pos = vc;
            }

            cout << flush;
        }
        {//sorting verticles in faces to make the go CCW
            std::vector<std::pair<float, glm::vec3>> angsVerts;
            angsVerts.reserve(6);
            for (int i=0; i < ans.size(); ++i){
                angsVerts.resize(0);
                WK4dG::vec4 const& b = ans[i][0];
                glm::vec3 faceCn = faceCenters[i];
                glm::vec3 lb = {b.x-faceCn.x, b.y-faceCn.y, b.z-faceCn.z};
                glm::vec3 lFigCn = figCenter-faceCn;
                lFigCn -= glm::proj(lFigCn, lb);
                lFigCn -= glm::proj(lFigCn,
                            {ans[i][1].x-faceCn.x, ans[i][1].y-faceCn.y,
                             ans[i][1].z-faceCn.z});
                lFigCn = glm::normalize(lFigCn);
                auto ang = [&](WK4dG::vec4 const& a){
                    glm::vec3 la = {a.x-faceCn.x, a.y-faceCn.y, a.z-faceCn.z};
                    glm::vec3 aCb= glm::cross(la, lb);
                    float tr = glm::dot(aCb, lFigCn);
                    float d = glm::dot(la, lb)
                            /(glm::length(la)*glm::length(lb));
                    float ang = std::acos(std::min(std::max(d, -1.f), 1.f));
                    if (tr>=0){
                        return ang;
                    }else{
                        return 2*glm::pi<float>() - ang;
                    }
                };
                for (WK4dG::vec4& vert: ans[i]){
                    angsVerts.push_back(std::make_pair(ang(vert),
                                glm::vec3{vert.x, vert.y, vert.z}));
                }
                std::sort(angsVerts.begin(), angsVerts.end(),
                    [](auto const& f, auto const& s)->bool{
                        return f.first > s.first;
                    });
                for (int j=0; j<ans[i].size(); ++j){
                    glm::vec3& v= angsVerts[j].second;
                    ans[i][j] = WK4dG::vec4{v.x, v.y, v.z, 0};
                }

            }

        }

        view = glm::rotate(glm::mat4{}, -myCamera.pitch, glm::vec3{1.0f, 0.0f, 0.0f});
        view = glm::rotate(view, -myCamera.yaw, glm::vec3{0.0f, 1.0f, 0.0f});
        view = glm::translate(view, -myCamera.pos);

        glProgramUniformMatrix4fv(shadr.m_shaderProg, viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);


        glUseProgram(shadr.m_shaderProg);
        for (vector<WK4dG::vec4>& face : ans){
            tempMesh.m_points.resize(face.size()*3);
            tempMesh.m_indices.resize((face.size()-2)*3);
            for (size_t i=0; i<face.size()*3; i+=3){
                //7face[i/3] = tr*face[i/3];
                tempMesh.m_points[i+0] = face[i/3].y;
                tempMesh.m_points[i+1] = face[i/3].z;
                tempMesh.m_points[i+2] = face[i/3].x;
                if (!WK4dG::fuzzyEqual(face[i/3].w, 0)
                        && !std::isnan(face[i/3].w)){
                    cout << "f "<<face[i/3].w<<" ";
                }
            }
            for (size_t i=0; i<(face.size()-2)*3; i+=3){
                tempMesh.m_indices[i+0] = 0;
                tempMesh.m_indices[i+1] = i/3+1;
                tempMesh.m_indices[i+2] = i/3+2;
            }
            tempMesh.updateBufsInGPU(GL_STREAM_DRAW);
            tempMesh.bind();
            glDrawElements(GL_TRIANGLES, tempMesh.m_indices.size(), GL_UNSIGNED_INT, 0);

//            {
//                glm::vec3 fg = glm::vec3{figCenter.y, figCenter.z,
//                                         figCenter.x};
//                model = glm::mat4{1};
//                model = glm::translate(model, fg);
//                model = glm::scale(model, glm::vec3{0.1, 0.1, 0.1});
//                GLint modelLoc= glGetUniformLocation(shadr.m_shaderProg, "model");
//                glProgramUniformMatrix4fv(shadr.m_shaderProg, modelLoc,
//                                          1, GL_FALSE, glm::value_ptr(model));
//                glDrawElements(GL_TRIANGLES, tempMesh.m_indices.size(), GL_UNSIGNED_INT, 0);
//                for (int i=0; i<faceCenters.size(); ++i){
//                    glm::vec3 vc = glm::vec3{faceCenters[i].y,
//                                            faceCenters[i].z,
//                                            faceCenters[i].x};

//                    model = glm::mat4{1};
//                    model = glm::translate(model, vc);
//                    model = glm::scale(model, glm::vec3{0.1, 0.1, 0.1});
//                    glProgramUniformMatrix4fv(shadr.m_shaderProg, modelLoc,
//                                              1, GL_FALSE, glm::value_ptr(model));
//                    glDrawElements(GL_TRIANGLES, tempMesh.m_indices.size(), GL_UNSIGNED_INT, 0);

//                    //drawLine(fg, vc);
//                }
//                model = glm::mat4{1};
//                glProgramUniformMatrix4fv(shadr.m_shaderProg, modelLoc,
//                                        1, GL_FALSE, glm::value_ptr(model));

//            }
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







