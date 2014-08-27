//
//  colorRender.h
//  CinderES2
//
//  Created by Kris Meeusen on 15/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#ifndef __CinderES2__colorRender__
#define __CinderES2__colorRender__

#include <iostream>


#include "cinder/app/AppBasic.h"
#include "cinder/Timeline.h"
#include "cinder/TimelineItem.h"

#include "cinder/Camera.h"
#include "cinder/TriMesh.h"

#include "VboMesh.h"
#include "GLConstants.h"

class ColorRender{

private:
	GLint uWorldMatrix;
	GLint uColor;
	
	bool isSetup;
	static ColorRender* instance;
	
public:
	GLuint program;

	void drawLine(const ci::Vec3f &start, const ci::Vec3f &end );
	void drawMesh(const ci::TriMesh mesh);
	void drawMesh(const ci::TriMesh mesh,GLint shape);
	void drawMesh(VboMesh mesh);

	

	void setCameraMatrix(ci::Matrix44f worldMatrix);
	void setCamera(ci::Camera camera);
    
	void setColor(const ci::ColorA color);
	void setLineWidth(const float width);

	
	ColorRender();
	
	static ColorRender* Instance();
	
	void setup();
	
};

#endif /* defined(__CinderES2__colorRender__) */
