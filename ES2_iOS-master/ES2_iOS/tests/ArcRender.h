//
//  arcRender
//  CinderES2
//
//  Created by Kris Meeusen on 15/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#ifndef __CinderES2__arcRender__
#define __CinderES2__arcRender__

#include <iostream>


#include "cinder/app/AppBasic.h"

#include "cinder/Camera.h"
#include "cinder/TriMesh.h"

#include "VboMesh.h"
#include "GLConstants.h"
#include "cinder/Timeline.h"

#include "cinder/Quaternion.h"

class ArcRender{

private:
	GLint uWorldMatrix;
	GLint uProjectionMatrix;
	GLint uColor;
	
	GLint uCircleRadiusPow;
	GLint uScreenCenter;
	
	bool isSetup;
	
	void touchesBegan(std::vector<ci::Vec2f> touches);
	void touchesEnded(std::vector<ci::Vec2f> touches);
    void touchesMoved(std::vector<ci::Vec2f> touches);

	ci::Vec2f lastTouchPoint;
	
	
	
public:
	GLuint program;

	void drawLine(const ci::Vec3f &start, const ci::Vec3f &end );

	

	void setCameraMatrix(ci::Matrix44f worldMatrix);
	void setProjectionMatrix(ci::Matrix44f projectionMatrix);
	
	void setCamera(ci::Camera camera);
    
	void setColor(const ci::ColorA color);
	void setLineWidth(const float width);

	
	ArcRender();
	
	ci::CameraPersp* pCamera;
	ci::Quatf rotation;


	void setup();
	void drawShape();
	
	
	float radius;
    float radiusOut;
    float radiusOutLong;
    float distance;

	ci::Anim<ci::Vec2f> rotationVector;

	


};

#endif /* defined(__CinderES2__arcRender__) */
