/*******************************************************************************
 *
 * EcoSynth - Data-driven Authoring of Large-Scale Ecosystems (Undergrowth simulator)
 * Copyright (C) 2020  J.E. Gain  (jgain@cs.uct.ac.za)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 ********************************************************************************/


#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <fstream>
#include <string>

#include "glheaders.h"
//#include "glwidget.h"
//#include <QGLWidget>
//#include <QTextStream>
//#include <QResource>
//#include <QtGui>
//#include <GL/gl.h>
//#include <GL/glu.h>

#include "shaderProgram.h"

namespace PMrender
{

  // Function to convert OpenGL error codes to a string
  QString getOpenGLErrorString(GLenum error) {
    switch (error) {
    case GL_NO_ERROR:
      return "GL_NO_ERROR: No error has been recorded.";
    case GL_INVALID_ENUM:
      return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
    case GL_INVALID_VALUE:
      return "GL_INVALID_VALUE: A numeric argument is out of range.";
    case GL_INVALID_OPERATION:
      return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
    case GL_STACK_OVERFLOW:
      return "GL_STACK_OVERFLOW: This command would cause a stack overflow.";
    case GL_STACK_UNDERFLOW:
      return "GL_STACK_UNDERFLOW: This command would cause a stack underflow.";
    case GL_OUT_OF_MEMORY:
      return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
    default:
      return QString("Unknown error code: %1").arg(error);
    }
  }

  // Function to check for OpenGL errors and print them
  void shaderProgram::checkGLError(const char* function) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
			qCritical() << "OpenGL error in" << function << ":" << getOpenGLErrorString(error);
		}
	}

GLenum shaderProgram::compileProgram(GLenum target, GLchar* sourcecode, GLuint & shader)
{
        GLint   logLength = 0;
        GLint   compiled  = 0;

        if (sourcecode != 0)
        {
          QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

        shader = f->glCreateShader(target);
        checkGLError("Failed to create fragment shader");
                f->glShaderSource(shader,1,(const GLchar **)&sourcecode,0);
                checkGLError("Failed glShaderSource");
                  f->glCompileShader(shader);
                  checkGLError("Failed glCompileShader");


                  f->glGetShaderiv(shader,GL_COMPILE_STATUS,&compiled);
                f->glGetShaderiv(shader,GL_INFO_LOG_LENGTH,&logLength);

        if (logLength > 1)
                {
            GLint       charsWritten;
                        GLchar *log = new char [logLength+128];
                        f->glGetShaderInfoLog(shader, logLength, &charsWritten, log);
            std::cerr << "Compilation log: nchars=(" << logLength << "): "<< (char*)log << std::endl;
                        delete [] log;
                }

        if (compiled == 0)
          checkGLError("shader could not compile");

        }
        return GL_NO_ERROR;
}

GLenum shaderProgram::linkProgram(GLuint program)
{
        GLint   logLength = 0;
        GLint linked = 0;

        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

        f->glLinkProgram(program);
        checkGLError("Failed glLinkProgram");
          f->glGetProgramiv(program,GL_LINK_STATUS ,&linked);
        f->glGetProgramiv(program,GL_INFO_LOG_LENGTH,&logLength);

    if (logLength > 1)
        {
                GLint   charsWritten;
        GLchar *log = new char [logLength+128];

        f->glGetProgramInfoLog(program, logLength, &charsWritten, log);
        std::cerr << "Link GetProgramInfoLog: nchars=(" << charsWritten << "): " << (char*)log << std::endl;
                delete [] log;
        }

    if (linked == 0)
      checkGLError("shader did not link");

        return GL_NO_ERROR;
}

shaderProgram::shaderProgram(const std::string& fragSource, const std::string& vertSource)
{
        fragSrc = fragSource;
        vertSrc = vertSource;
        frag_ID = vert_ID = program_ID = 0;

        fileInput = false;
        shaderReady = false;

}

shaderProgram::shaderProgram(const char * fragSourceFile, const char * vertSourceFile)
{
        fragSrc = fragSourceFile;
        vertSrc = vertSourceFile;
        fileInput = true;

        frag_ID = vert_ID = program_ID = 0;

        shaderReady = false;
}

  // use Bruce's shader/kernel source code bake...
/*
    void shaderProgram::setShaderSources(const std::string& fragSource, const std::string& vertSource)
    {
        if (shaderReady)
        {
            std::cerr << "setShaderSources: shader sources already set\n";
            return ;
        }

        fragSrc = getSource(fragSource); // fragSource;
        vertSrc = getSource(vertSource); //vertSource;
        frag_ID = vert_ID = program_ID = 0;

        fileInput = false;
        shaderReady = false;
    }
*/

    void shaderProgram::setShaderSources(const char * fragSourceFile, const char * vertSourceFile)
    {
        if (shaderReady)
        {
            std::cerr << "setShaderSources: shader sources already set\n";
            return ;
        }

        fragSrc = fragSourceFile;
        vertSrc = vertSourceFile;
        fileInput = true;

        frag_ID = vert_ID = program_ID = 0;

        shaderReady = false;
    }

bool shaderProgram::compileAndLink(void)
{
        if (shaderReady) return true;

        if (fileInput)
        {
        std::ifstream vshader(vertSrc.c_str(), std::ios::binary),
                fshader(fragSrc.c_str(), std::ios::binary);

                if (!vshader)
                {
                        std::cerr << "could not open shader source: " << vertSrc << std::endl;
                        return false;
                }
                if (!fshader)
                {
            std::cerr << "could not open shader source: " << fragSrc << std::endl;
                        return false;
                }

        vertSrc.clear(); fragSrc.clear();

        std::string tempstr;

        while (std::getline(vshader, tempstr))
        {
            vertSrc += tempstr;
            vertSrc += "\n";
        }


        //std::cout << "vertSrc=\n" << vertSrc << std::endl;

        while (std::getline(fshader, tempstr))
        {
            fragSrc += tempstr;
            fragSrc += "\n";
        }


        // std::cout << "fragSrc=\n" << fragSrc << std::endl;
                vshader.close();
                fshader.close();
        }
    GLuint err = compileProgram(GL_VERTEX_SHADER, const_cast<GLchar*>(vertSrc.c_str()), vert_ID);
        if (0 != err)
        {
                std::cerr << "Vertex Shader could not compile\n";
                return false;
        }

    err = compileProgram(GL_FRAGMENT_SHADER, const_cast<GLchar*>(fragSrc.c_str()), frag_ID);
        if (0 != err)
        {
                std::cerr <<"Fragment Shader could not compile\n";
                return false;
        }


        QOpenGLFunctions* f = QOpenGLContext::currentContext()->functions();

        program_ID = f->glCreateProgram();
        //std::cout << "Shader ID = " << program_ID << std::endl;
        f->glAttachShader(program_ID, vert_ID);
        f->glAttachShader(program_ID, frag_ID);

        err = linkProgram(program_ID);
        if (GL_NO_ERROR != err)
        {
                std::cerr << "Program could not link\n";
                return false;
        }

        // detach and delete shader objects, we don't need them anymore (NB: may cause issues with dodgy drivers)
        if (frag_ID != 0)
        {
          f->glDetachShader(program_ID, frag_ID);
          f->glDeleteShader(frag_ID);
                frag_ID = 0;
        }
        if (vert_ID != 0)
        {
          f->glDetachShader(program_ID, vert_ID);
          f->glDeleteShader(vert_ID);
                vert_ID = 0;
        }
    shaderReady = true;
    return true;
}
    // end namespace PMrender
}
