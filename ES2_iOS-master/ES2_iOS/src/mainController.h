//
//  main.h
//  CinderES2
//
//  Created by Kris Meeusen on 13/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#ifndef __CinderES2__main__
#define __CinderES2__main__

//#include <iostream>
#include "cinder/app/AppBasic.h"
#include "cinder/Timeline.h"
#include "cinder/TimelineItem.h"

#include "cinder/Camera.h"
#include "cinder/TriMesh.h"

#include "cinder/gl/Texture.h"

#include "gl2.h"
#include "uiSpriteButton.h"
#include "uiSprite.h"
#include "VboMesh.h"

//tests
#include "ArcRender.h"

//#include "colorRender.h"

class mainController {

private:
	
    ci::gl::Texture texture;
	ci::Matrix44f localMatrix;

	// classic mesh
    ci::TriMesh mesh1;
    ci::TriMesh mesh2;
	VboMesh* vboMesh;
	ci::CameraOrtho orthoCamera;
    ci::CameraPersp perspectiveCamera;

	TextureRender perspectiveRender;
	

    


	uiSpriteButton button;

    uiSprite background;
    uiSprite globe;
    uiSprite globeDots;
    
    
    ArcRender arc;
    
    //timeline
    ci::TimelineRef  mTimeline;
    ci::Timer mTimer;

public:
	
	//mainController();
	void update();
	void setup();
	void setSize(ci::Vec2f size);

	
    GLuint LoadShader(const char *shaderSrc, GLenum type);

	void draw();


	void clicked(uiSpriteButton* button);


	
	void touchesBegan(std::vector<ci::Vec2f> touches);
	void touchesEnded(std::vector<ci::Vec2f> touches);
    void touchesMoved(std::vector<ci::Vec2f> touches);

    
    
    mainController() : mTimer(true), mTimeline(ci::Timeline::create())
    {
        
    };
//	void touchesMoved(std::vector<ci::Vec2f> touches);



};
#endif /* defined(__CinderES2__main__) */
