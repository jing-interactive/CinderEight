//
//  checkError.h
//  CinderES2
//
//  Created by Kris Meeusen on 14/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//


// Based on
// http://www.lastrayofhope.com/2011/02/02/checking-for-errors/


#pragma once

#ifdef DEBUG
#	define GL_CHECK_FOR_ERRORS()	GL::CheckForErrors()
#else
#	define GL_CHECK_FOR_ERRORS()
#endif

namespace gl2
{
	//
	//
	inline bool CheckForErrors()
	{
		static bool checkForErrors = true;
        
		//
		if( !checkForErrors )
		{
			return false;
		}
        
		//
		const char * errorString = NULL;
		bool retVal = false;
        
		switch( glGetError() )
		{
            case GL_NO_ERROR:
                retVal = true;
                break;
                
            case GL_INVALID_ENUM:
                errorString = "GL_INVALID_ENUM";
                break;
                
            case GL_INVALID_VALUE:
                errorString = "GL_INVALID_VALUE";
                break;
                
            case GL_INVALID_OPERATION:
                errorString = "GL_INVALID_OPERATION";
                break;
                
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                errorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
                
                // OpenGLES Specific Errors
#ifdef ATHENA_OPENGLES
            case GL_STACK_OVERFLOW:
                errorString = "GL_STACK_OVERFLOW";
                break;
                
            case GL_STACK_UNDERFLOW:
                errorString = "GL_STACK_UNDERFLOW";
                break;
#endif
                
            case GL_OUT_OF_MEMORY:
                errorString = "GL_OUT_OF_MEMORY";
                break;
                
            default:
                errorString = "UNKNOWN";
                break;
		}
        
		//
		if( !retVal )
		{
			printf( "%s\n", errorString );
			assert( retVal );
		}
        
		//
		return retVal;
	}
    
	//
	//
	inline void ClearErrors()
	{
		while( glGetError() != GL_NO_ERROR );
	}
}

