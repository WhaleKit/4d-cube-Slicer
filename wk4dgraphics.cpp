#include "wk4dgraphics.h"



void drawLine(glm::vec3 p1, glm::vec3 p2)
{
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
