#include "simwindow.h"

SimWindow::SimWindow():
    m_program(0),
    m_frame(0)
{
}

//Migrate visitem to simwindow
void SimWindow::initialize()
{
    m_program = std::shared_ptr<QOpenGLShaderProgram>(new QOpenGLShaderProgram(this));
    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, ":/vertexshader.vert");
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, ":/fragmentshader.frag");
    m_program->link();
}


void SimWindow::render()
{
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, width() * retinaScale, height() * retinaScale);

    glClear(GL_COLOR_BUFFER_BIT);

    m_program->bind();
}
