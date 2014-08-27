//
//  colorRender.cpp
//  CinderES2
//
//  Created by Kris Meeusen on 15/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#include "ArcRender.h"
#include "shaderLoader.h"
#include "checkError.h"

#include "cinder/Vector.h"
#include "App.h"
#include "TouchDispatcher.h"

using namespace ci;


ArcRender::ArcRender(){
	isSetup = false;
}


void ArcRender::setup(){
    
    if (isSetup) {
        return;
    }
    
	const char vShaderStr[] =
    "attribute vec4 position;   \n"
	"uniform mat4 modelViewProjectionMatrix; \n"
	"uniform mat4 projectionMatrix;			\n"
	"uniform mat3 normalMatrix;			\n"
	"varying float depth;	"
	""
    "void main()                 \n"
    "{                           \n"
    "   vec4 pos = modelViewProjectionMatrix * position; \n"
	"	depth = (projectionMatrix *position).z;					"
	"	gl_Position = pos;"
    "}                           \n";
    
    const char fShaderStr[] =
	"#ifdef GL_ES \n"
	" precision mediump float;\n"
	"#endif	\n"
	"uniform  vec4 vColor;													\n"
	"uniform  vec3 screenCenter;													\n"
	"uniform  float circleRadiusPow;													\n"
	"varying float depth;												"
    "void main()														\n"
    "{																	\n"
    "  gl_FragColor = vColor;											\n"
	
	"	vec2 f1 = gl_FragCoord.xy * 0.01;			"
	"	f1.x = screenCenter.x - f1.x;	"
	"	f1.y = screenCenter.y - f1.y;	"
	"	f1.x = f1.x * f1.x; "
	"	f1.y = f1.y * f1.y; "
//
//	
//	
	"  if ((f1.x + f1.y) < circleRadiusPow && depth > 0.0) gl_FragColor = vec4(0.4,0.4,0.4,1);					\n"
    "}																		\n";
	
	
    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;
    
	
    radius = 190.0;
	radiusOut = 200.0;
	radiusOutLong = 210.0;
    distance = 0.02;

	lastTouchPoint.set(-1 ,-1);

	
//	gl2::CheckForErrors();

    // Load the vertex/fragment shaders
    vertexShader = loadShader( vShaderStr,GL_VERTEX_SHADER);
    fragmentShader = loadShader(fShaderStr, GL_FRAGMENT_SHADER);
	
//	gl2::CheckForErrors();
	
    // Create the program object
    program = glCreateProgram();
	gl2::CheckForErrors();
	
    if(program == 0)
        return;
	
    glAttachShader(program, vertexShader);
	gl2::CheckForErrors();

    glAttachShader(program, fragmentShader);
    
//    gl2::CheckForErrors();
	
    glBindAttribLocation(program, ATTRIB_VERTEX, "position");
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
            
//            gl2::CheckForErrors();
			
            free(infoLog);
        }
        return;
    }else{
		glUseProgram(program);
		
		uWorldMatrix= glGetUniformLocation(program, "modelViewProjectionMatrix");
		uProjectionMatrix= glGetUniformLocation(program, "projectionMatrix");
		uColor = glGetUniformLocation(program, "vColor");
		
		uScreenCenter = glGetUniformLocation(program, "screenCenter");
		uCircleRadiusPow = glGetUniformLocation(program, "circleRadiusPow");

		
		float scale = gl2::App::Instance()->displayScale;
		
		GLfloat screenCenter[] = { 512.0f * 0.01f  * scale, 384.0f * 0.01f  * scale, 0.0 };
		
		glUniform3fv(uScreenCenter, 1,screenCenter);
		
		if(scale == 1.0){
			glUniform1f(uCircleRadiusPow, (2.35 * 2.35 * scale));
		}else{
			glUniform1f(uCircleRadiusPow, (3.34 * 3.34 * scale));
		}

	}
	
	
	
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glUseProgram(0);
	isSetup = true;
	
	
	
    TouchDispatcher::Instance()->onTouchesBegan.Connect(this,&ArcRender::touchesBegan);
    TouchDispatcher::Instance()->onTouchesMoved.Connect(this,&ArcRender::touchesMoved);
    TouchDispatcher::Instance()->onTouchesEnded.Connect(this,&ArcRender::touchesEnded);

	
}



void ArcRender::touchesBegan(std::vector<ci::Vec2f> touches){
	if(touches.size() > 1) return;
	
	lastTouchPoint = touches[0];
    
    //std::cout << "started" << lastTouchPoint << std::endl;
	//std::cout << lastTouchPoint << std::endl;
	
}

void ArcRender::touchesEnded(std::vector<ci::Vec2f> touches){
	if(touches.size() > 1) return;
	
	//std::cout << "ended" << lastTouchPoint << std::endl;
	
	lastTouchPoint.set(-1 ,-1);
}

void ArcRender::touchesMoved(std::vector<ci::Vec2f> touches){
	if(touches.size() > 1) return;
	
	// perspectiveCamera.lookAt( mesh.getVertices()[2] + Vec3f(touches[0].x /100 ,touches[0].y/100 ,-1),mesh.getVertices()[2], Vec3f::yAxis() );
	
	//std::cout << lastTouchPoint << std::endl;
	
	// check if first touchpoint
	if (lastTouchPoint.x == -1 && lastTouchPoint.y == -1) {
		return;
	}
	
    
    Vec2f diff = lastTouchPoint - touches[0];
    
	
	float newX = rotationVector().x + (diff.y / 250.0);
	if(fabs(newX) < 1.2)
		rotationVector().x = newX;
	
	float newY = rotationVector().y + (diff.x / -250.0);
	if(fabs(newY) < 1.2)
		rotationVector().y  = newY;
    
	
	lastTouchPoint = touches[0];
	
}


void ArcRender::setCamera(ci::Camera camera){
 	glUseProgram(program);
	glUseProgram(0);
   
}

void ArcRender::drawShape(){
	

	setColor(ColorA(1,1,1,1));
	setLineWidth(1.0);
	
	
	glUseProgram(program);
	
	
    rotation.set(rotationVector().x, 0, 0);
	
	Matrix44f world = pCamera->getProjectionMatrix() * pCamera->getModelViewMatrix() * rotation.toMatrix44();

	glUniformMatrix4fv(uWorldMatrix, 1, 0,world.m);
	glUniformMatrix4fv(uProjectionMatrix, 1, 0,pCamera->getProjectionMatrix().m);

	
    glEnableVertexAttribArray(ATTRIB_VERTEX);

	
	
	
	// horizontal
	float r2=radiusOut;
	float longStick=0;
	float longStickTick = 6.28 /  8.0;

	for(float i=0;i < 6.28; i+=distance){

		if((longStick += distance) >= longStickTick || i == 0){
			r2 = radiusOutLong;
			longStick= 0;
		}else{
			r2 = radiusOut;
		}
		
		//std::cout << r2 << std::endl;
		
        drawLine(Vec3f(cos(i) * radius,0,sin(i) * radius), Vec3f(cos(i) * r2,0,sin(i) * r2) );

    }
	
	
	rotation.set(0.0, rotationVector().y, 0);
	world = pCamera->getProjectionMatrix() * pCamera->getModelViewMatrix() * rotation.toMatrix44();

	glUniformMatrix4fv(uWorldMatrix, 1, 0,world.m);
	
	longStick=0;
	
	// vertical
	for(float i=0;i < 6.28; i+=distance){
		if((longStick += distance) >= longStickTick || i == 0){
			r2 = radiusOutLong;
			longStick= 0;
		}else{
			r2 = radiusOut;
		}

        drawLine(Vec3f(0,sin(i) * radius,cos(i) * radius), Vec3f(0,sin(i) * r2,cos(i) * r2) );
    }

	
	
	glDisableVertexAttribArray(ATTRIB_VERTEX);

	glUseProgram(0);


}


void ArcRender::setCameraMatrix(ci::Matrix44f worldMatrix){
	glUseProgram(program);
	glUniformMatrix4fv(uWorldMatrix, 1, 0,worldMatrix.m);
	glUseProgram(0);
}


void ArcRender::setProjectionMatrix(ci::Matrix44f projectionMatrix)
{
	glUseProgram(program);
	glUniformMatrix4fv(uProjectionMatrix, 1, 0,projectionMatrix.m);
	glUseProgram(0);

}

void ArcRender::setColor(const ci::ColorA color){
	glUseProgram(program);
	glUniform4fv (uColor,1, color);
	glUseProgram(0);

	gl2::CheckForErrors();
}

void ArcRender::setLineWidth(const float width){
	glUseProgram(program);
	glLineWidth(width);
	glUseProgram(0);

}



void ArcRender::drawLine(const ci::Vec3f &start,const ci::Vec3f &end){

	ci::TriMesh mesh;
	mesh.appendVertex(start);
	mesh.appendVertex(end);
	
	
	glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, 0, &(mesh.getVertices()[0]));
    glDrawArrays(GL_LINE_LOOP, 0, mesh.getNumVertices());
	

}



