#ifndef SIMWINDOW
#define SIMWINDOW

#include "openglwindow.h"

#include <memory>
#include <fstream>

#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QScreen>

#include <QtCore/qmath.h>

class SimWindow : public OpenGLWindow
{
public:
    SimWindow();

    void initialize() override;
    void render() override;

private:
    GLuint m_posAttr;
    GLuint m_colAttr;
    GLuint m_matrixUniform;

    std::shared_ptr<QOpenGLShaderProgram> m_program;
    int m_frame;
};


#endif // SIMWINDOW

