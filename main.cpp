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
    std::vector<GLuint> m_indexes;
    mesh(mesh && oth)
        : m_points(std::move(oth.m_points)),
          m_indexes(std::move(oth.m_indexes))
    {
        m_VAO = oth.m_VAO;
        m_VBO = oth.m_VBO;
        m_EBO = oth.m_EBO;
        oth.m_VAO = 0;
        oth.m_VBO = 0;
        oth.m_EBO = 0;
    }
    mesh(mesh const& oth)
        : mesh(oth.m_points, oth.m_indexes)
    {}
    mesh(std::vector<GLfloat> const& points, std::vector<GLuint> const& indexes
         ,GLenum mode = GL_STATIC_DRAW)
        :m_points(points), m_indexes(indexes)
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
        glNamedBufferData(m_EBO, m_indexes.size()*sizeof(GLuint),
                          m_indexes.data(), mode);
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
    //Настройка GLFW
    //Задается минимальная требуемая версия OpenGL.
    //Мажорная
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    //Минорная
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    //Установка профайла для которого создается контекст
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //Выключение возможности изменения размера окна
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
    auto vertShader1 = R"lollypop(
       #version 330 core
       layout (location = 0) in vec3 position;


       uniform mat4 model;
       uniform mat4 view;
       uniform mat4 projection;
       out float depth;
       void main()
       {
           gl_Position =  projection * view * model * vec4(position, 1.0f);
            depth = gl_Position.z;
       }
    )lollypop";
    auto fragmShader2 = R"Carthage(
        #version 330 core

        out vec4 color;
        in float depth;
        void main()
        {
            color = mix(vec4(0.02f, 0.4f, 0.9f, 0.5f), vec4(0.1f, 0.1f, 0.1f, 0.1f), depth/5);
        }
    )Carthage";

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
        if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
            speed = 0.1;
        }else{
            speed = 0.01;
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
        if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS){
            cout << "\ncoords: ("<<myCamera.pos.x << ", " << myCamera.pos.y
                                        << ", " << myCamera.pos.z
                 << "), yaw: " << myCamera.yaw
                 << ", pitch: " << myCamera.pitch << flush;
        }

        view = glm::rotate(glm::mat4{}, -myCamera.pitch, glm::vec3{1.0f, 0.0f, 0.0f});
        view = glm::rotate(view, -myCamera.yaw, glm::vec3{0.0f, 1.0f, 0.0f});
        view = glm::translate(view, -myCamera.pos);

        glProgramUniformMatrix4fv(shadr.m_shaderProg, viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT| GL_DEPTH_BUFFER_BIT);

        glUseProgram(shadr.m_shaderProg);
        cube.bind();
        glDrawElements(GL_TRIANGLES, cube.m_indexes.size(), GL_UNSIGNED_INT, 0);



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

//void printMatrix (matrix5x5 const& m)
//{
//    cout << "\n{";
//    for (int r=0; r<5; ++r){
//        cout << "\n\t" << "{";
//        for (int c=0; c<5; ++c){
//            cout << m[r][c] << ", ";
//        }
//        cout << "}";
//    }
//    cout <<"\n}";
//}

//int main(int argc, char *argv[])
//{
//    cout << "Hello World!" << endl;

//    vec4 v(0,0,0,0);

//    matrix5x5 m(1);

//    printMatrix(m);

//    m = moveMatrix(vec4(1, 0, 0, 0)) *m;
//    printMatrix(m);
//    m = rotationMatrix(axes::x, axes::z, mathPi)*m;
//    printMatrix(m);
//    v = m*v;

//    cout << mathPi << '\n';
//    cout << v.x << " "<< v.y << " "<< v.z << " "<< v.w;

//    v = orthogonalToThree(vec4{1,0,0,0}, vec4{0,0,1,0}, vec4{0,0,0,1});
//    cout << '\n' <<endl<< v.y << endl;

//    return 0;
//}







