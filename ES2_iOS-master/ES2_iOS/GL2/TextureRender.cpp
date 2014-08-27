//
//  textureRender.cpp
//  CinderES2
//
//  Created by Kris Meeusen on 15/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#include "TextureRender.h"
#include "shaderLoader.h"
#include "checkError.h"


using namespace ci;

TextureRender* TextureRender::instance;


TextureRender* TextureRender::Instance(){
    if(instance == NULL)
	{
		instance = new TextureRender();
		instance->setup();
	}
    return instance;
}


TextureRender::TextureRender(){
	isSetup = false;
}


void TextureRender::setup(){
		
	
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;
    
	
	gl2::CheckForErrors();
	
    // Load the vertex/fragment shaders
    vertexShader = loadShader( vShaderStr,GL_VERTEX_SHADER);
    fragmentShader = loadShader(fShaderStr, GL_FRAGMENT_SHADER);
	
	gl2::CheckForErrors();
	
	glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindVertexArrayOES(0);

    // Create the program object
    program = glCreateProgram();
	gl2::CheckForErrors();
	
    if(program == 0)
        return;
	
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    //gl2::CheckForErrors();
	
    glBindAttribLocation(program, ATTRIB_VERTEX, "position");
    glBindAttribLocation(program, ATTRIB_UV, "uv");
    
	glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    
//    gl2::CheckForErrors();
	
    if(!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
        
        if(infoLen > 1)
        {
            char* infoLog = (GLchar *) malloc(sizeof(char) * infoLen);
            glGetProgramInfoLog(program, infoLen, NULL, infoLog);
            std::cout << " !! " <<  infoLog << std::endl;
//            gl2::CheckForErrors();
			
            free(infoLog);
        }
        return;
    }else{
		glUseProgram(program);
		
		uWorldMatrix= glGetUniformLocation(program, "modelViewProjectionMatrix");
		//utexture = glGetUniformLocation(program, "vtexture");
		
	}
	
	
	isSetup = true;
	gl2::CheckForErrors();
	
	
    
	
}



void TextureRender::setCameraMatrix(ci::Matrix44f worldMatrix){
	glUseProgram(program);
	glUniformMatrix4fv(uWorldMatrix, 1, 0,worldMatrix.m);
	glUseProgram(0);
	
}


void TextureRender::bindTexture(const ci::gl::Texture &texture)
{
    texture.bind();
	gl2::CheckForErrors();
}

void TextureRender::unbindTexture(const ci::gl::Texture &texture)
{
    texture.unbind();
	gl2::CheckForErrors();
}



void TextureRender::setLineWidth(const float width){
	glLineWidth(width);
	
}


void TextureRender::drawMesh(const ci::TriMesh mesh){
	drawMesh(mesh,GL_TRIANGLES);
}


void TextureRender::drawLine(const ci::Vec3f &start,const ci::Vec3f &end){
	glUseProgram(program);
	
	ci::TriMesh mesh;
	mesh.appendVertex(start);
	mesh.appendVertex(end);
	
	drawMesh(mesh, GL_LINE_LOOP);
	glUseProgram(0);
	
}

void TextureRender::drawTexture(const ci::gl::Texture& texture){

   ci::TriMesh mesh;
//
 //   mesh.appendVertex(Vec3f(100,500,-0.1));
  //  mesh.appendVertex(Vec3f(10,0,-0.1));
  //  mesh.appendVertex(Vec3f(310,0,-0.1));
//
//	// not implemented
//    //    mesh.appendColorRGB(Color(1,1,0));
//    //    mesh.appendColorRGB(Color(1,0,1));
//    //    mesh.appendColorRGB(Color(1,0,1));
//	
//	mesh.appendTexCoord(Vec2f(1,1));
//	mesh.appendTexCoord(Vec2f(0,0));
//	mesh.appendTexCoord(Vec2f(1,0));

    
}


void TextureRender::drawMesh(const ci::TriMesh mesh,GLint shape){


	glUseProgram(program);
	glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, &(mesh.getVertices()[0]));
    glEnableVertexAttribArray(ATTRIB_VERTEX);
	
	
	gl2::CheckForErrors();

	
	glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, 0, &mesh.getTexCoords()[0]);
	glEnableVertexAttribArray(ATTRIB_UV);

	gl2::CheckForErrors();

    glDrawArrays(shape, 0, mesh.getNumVertices());
	gl2::CheckForErrors();

	glDisableVertexAttribArray(ATTRIB_VERTEX);
	glDisableVertexAttribArray(ATTRIB_UV);


	glUseProgram(0);
	//glDisable  (GL_BLEND);

	
	gl2::CheckForErrors();

}

void TextureRender::drawMesh(VboMesh mesh){
//	// 1rst attribute buffer : vertices
//	glUseProgram(program);
//	gl2::CheckForErrors();
//	
//	
//	
//	glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexbuffer);
//	gl2::CheckForErrors();
//	
//	glVertexAttribPointer(
//						  ATTRIB_VERTEX,       // attribute
//						  3,                  // size
//						  GL_FLOAT,           // type
//						  GL_FALSE,           // normalized?
//						  mesh.getStride() *sizeof(GLfloat),                  // stride
//						  (GLvoid*)0            // array buffer offset
//						  );
//	
//	glEnableVertexAttribArray(ATTRIB_VERTEX);
//	
//	glVertexAttribPointer(
//						  ATTRIB_UV,       // attribute
//						  3,                  // size
//						  GL_FLOAT,           // type
//						  GL_FALSE,           // normalized?
//						  mesh.getStride() *sizeof(GLfloat),                  // stride
//						  (GLvoid*) (3 * sizeof(GLfloat))         // array buffer offset
//						  );
//	
//	glEnableVertexAttribArray(ATTRIB_UV);
//	
//    glVertexAttribPointer(
//						  ATTRIB_NORMAL,       // attribute
//						  3,                  // size
//						  GL_FLOAT,           // type
//						  GL_FALSE,           // normalized?
//						  mesh.getStride() *sizeof(GLfloat),                  // stride
//						  (GLvoid*) (6 * sizeof(GLfloat))         // array buffer offset
//						  );
//	
//	glEnableVertexAttribArray(ATTRIB_UV);
//	
//	glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
//	gl2::CheckForErrors();
//	
//	glDisableVertexAttribArray(ATTRIB_VERTEX);
//	glDisableVertexAttribArray(ATTRIB_UV);
//	glUseProgram(0);
//	
//	glBindBuffer(GL_ARRAY_BUFFER,0);
//    glBindVertexArrayOES(0);
	
	
}


void TextureRender::drawSprite(uiSprite& sprite){
	if(!sprite.isTextureLoaded || sprite.getAlpha()==1) return;
	

	glUseProgram(program);
//	glBindBuffer(GL_ARRAY_BUFFER,0);
//    glBindVertexArrayOES(0);


    	
	bindTexture(sprite.getTexture());
	GLfloat *pointer = sprite.data;
    glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, 0, 6 * sizeof(GLfloat), pointer);
	glEnableVertexAttribArray(ATTRIB_VERTEX);

    pointer +=3;
    glVertexAttribPointer(ATTRIB_UV, 3, GL_FLOAT, 0, 6 * sizeof(GLfloat), pointer);
	glEnableVertexAttribArray(ATTRIB_UV);

    
    glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glDisableVertexAttribArray(ATTRIB_VERTEX);
	glDisableVertexAttribArray(ATTRIB_UV);

	glUseProgram(0);
	//glDisable  (GL_BLEND);
	
	sprite.getTexture().unbind();
	gl2::CheckForErrors();

}




