#ifndef WK4DGRAPHICS_H
#define WK4DGRAPHICS_H

#include <fstream>
#include <streambuf>
#include <vector>
#include <algorithm>

#include <GL/glew.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/projection.hpp"


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

    mySimpleShaderProgram(std::string const& vert, std::string const& fragm)
        :mySimpleShaderProgram(vert, std::string(""), fragm)
    {    }

    mySimpleShaderProgram(std::string const& vert, std::string const& geom,
                          std::string const& fragm)
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

        glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &success);
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

    static mySimpleShaderProgram fromFile(std::string const& vertFile, std::string const& fragFile)
    {
        std::ifstream vert(vertFile);
        std::ifstream frag(fragFile);
        return mySimpleShaderProgram{vert, frag};
    }
    static mySimpleShaderProgram fromFile(std::string const& vertFile,
                                          std::string const& geomFile,
                                          std::string const& fragFile)
    {
        std::ifstream vert(vertFile);
        std::ifstream geom(geomFile);
        std::ifstream frag(fragFile);
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




void drawLine(glm::vec3 p1, glm::vec3 p2);


//makes index array for convax face, such that
//it will be fully covered with minimal amount of triangles

inline
std::vector<GLuint> makeIndexArrayForFace (std::vector<glm::vec4>const& face,
                            glm::vec4 const&faceCenter, glm::vec4 const&normal,
                                           GLuint offset=0)
{
    std::vector<GLuint> result;
    std::vector<std::pair<float, GLuint>> angsVerts; //angle of vertice and it's index

    angsVerts.push_back(std::make_pair(0.0, 0));
    glm::vec3 locA = face[0] - faceCenter;
    for (int i=1; i<face.size(); ++i){
        float angle;
        glm::vec3 locB = face[i] - faceCenter;
        glm::vec3 bCa= glm::cross(locB, locA);
        float d = glm::dot(locB, locA)
                /(glm::length(locB)*glm::length(locA));
        float tr = glm::dot(bCa, glm::vec3(normal));
        angle = std::acos(std::min(std::max(d, -1.f), 1.f));
        //if (tr>=0){
        if (tr>=0){
            angle = 2*glm::pi<float>() - angle;
        }
        angsVerts.push_back(std::make_pair(angle,i) );
    }
    std::sort(angsVerts.begin(), angsVerts.end(),
              [](auto const& f, auto const& s)->bool{
        return f.first > s.first;
    });
    result.resize((face.size()-2)*3);
    for (size_t i=0; i<(face.size()-2)*3; i+=3){
        result[i+0] = angsVerts[0].second+offset;
        result[i+1] = angsVerts[i/3+1].second+offset;
        result[i+2] = angsVerts[i/3+2].second+offset;
    }
    return result;
}
inline
std::vector<GLuint> makeIndexArrayForFace (std::vector<glm::vec4> const& face,
                            glm::vec4 const&pointAtTheBackOfFace, GLuint offset=0)
{
    assert(face.size()>2);
    glm::vec4 faceCenter{0,0,0,0};
    for (glm::vec4 const& a : face){
        faceCenter+=a;
    }
    faceCenter /= float(face.size());
    glm::vec4 normal = faceCenter-pointAtTheBackOfFace;
    normal -= glm::proj(normal, face[0]-faceCenter);
    normal -= glm::proj(normal, face[1]-faceCenter);//make sure normal is *normal* to face
    return makeIndexArrayForFace(face, faceCenter, normal, offset);
}

#endif // WK4DGRAPHICS_H
