//
//  textureRender.h
//  CinderES2
//
//  Created by Kris Meeusen on 15/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#ifndef __CinderES2__textureRender__
#define __CinderES2__textureRender__

#include <iostream>


#include "cinder/app/AppBasic.h"
#include "cinder/Timeline.h"
#include "cinder/TimelineItem.h"

#include "cinder/Camera.h"
#include "cinder/TriMesh.h"
#include "cinder/gl/Texture.h"


#include "GLConstants.h"
#include "uiSprite.h"
#include "vboMesh.h"


// Standard shaders
const char vShaderStr[] =
"attribute vec4 position;   \n"
"attribute vec3 uv;			\n"

"uniform mat4 modelViewProjectionMatrix;"
"uniform mat3 normalMatrix;"
"varying vec3 uvVarying;			\n"

"void main()                 \n"
"{                           \n"
"   gl_Position = modelViewProjectionMatrix * position; \n"
"	uvVarying.x = uv.x; \n"
"#ifdef GL_ES \n"
"	uvVarying.y = 1.0-uv.y; \n"
"#else	\n"
"	uvVarying.y = uv.y; \n"
"#endif"
"	uvVarying.y = uv.y; \n"
"	uvVarying.z = uv.z; \n"

"}                           \n";

const char fShaderStr[] =
"#ifdef GL_ES \n"
" precision mediump float;\n"
"#endif	\n"
"uniform sampler2D texture;						\n"
"varying  vec3 uvVarying;						\n"

"void main()									\n"
"{												\n"
"  gl_FragColor = texture2D(texture, uvVarying.xy);		\n"
"  gl_FragColor.a   *= 1.0;						\n"
"  gl_FragColor.xyz /= gl_FragColor.a;					\n"

//"	vec2 f1 = gl_FragCoord.xy * 0.01;			"
//"	f1.x = 512.0 * .02 - f1.x;	"
//"	f1.y = 384.0 * .02 - f1.y;	"
//"	f1.x = f1.x * f1.x; "
//"	f1.y = f1.y * f1.y; "
//
//
//
//"	if((f1.x + f1.y) < (4.6 * 4.6) ) gl_FragColor *= 0.0;  "

"}														\n";




class TextureRender{
	
private:
	GLuint program;
	GLint uWorldMatrix;
	
	GLuint interfaceTextureId;
	
	bool isSetup;
	static TextureRender* instance;
	
	
public:
	void drawLine(const ci::Vec3f &start, const ci::Vec3f &end );
	void drawMesh(const ci::TriMesh mesh);
	void drawMesh(const ci::TriMesh mesh,GLint shape);
	void drawMesh(VboMesh mesh);

	void drawTexture(const ci::gl::Texture& texture);
	void drawSprite(uiSprite& texture);
	
	
	void setCameraMatrix(ci::Matrix44f worldMatrix);
    
    
	void bindTexture(const ci::gl::Texture& texture);
	void unbindTexture(const ci::gl::Texture& texture);

	void setLineWidth(const float width);
	//void ::drawArray(GLfloat* dataPointer){

	
	TextureRender();
	
	static TextureRender* Instance();
	
	void setup();
	
};

#endif /* defined(__CinderES2__textureRender__) */
