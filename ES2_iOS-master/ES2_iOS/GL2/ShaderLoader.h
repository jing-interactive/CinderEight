//
//  shaderLoader.h
//  CinderES2
//
//  Created by Kris Meeusen on 15/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#ifndef CinderES2_shaderLoader_h
#define CinderES2_shaderLoader_h
#include "checkError.h"

inline GLuint loadShader(const char *shaderSrc, GLenum type)
{
    GLuint shader;
    GLint compiled;
    
    // Create the shader object
    shader = glCreateShader(type);
	gl2::CheckForErrors();
	
    if(shader == 0)
        return 0;
    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);
	gl2::CheckForErrors();

    // Compile the shader
    glCompileShader(shader);
	gl2::CheckForErrors();

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
        
        if(infoLen > 1)
        {
            char* infoLog = (GLchar *) malloc(sizeof(char) * infoLen);
            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			// esLogMessage("Error compiling shader:\n%s\n", infoLog);
            gl2::CheckForErrors();
            free(infoLog);
        }
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}





#endif
