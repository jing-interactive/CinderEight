//
//  main.cpp
//  CinderES2
//
//  Created by Kris Meeusen on 13/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#include "mainController.h"
#include "cinder/app/AppBasic.h"
#include "QuartzCore/QuartzCore.h"
#include "cinder/ImageIo.h"
#include "TouchDispatcher.h"
#include "checkError.h"
#include "TouchDispatcher.h"

#include "App.h"

using namespace std;
using namespace ci;
using namespace gl2;



void mainController::setup(){
	
	
	
    mesh1.appendVertex(Vec3f(10,200,0));
    mesh1.appendVertex(Vec3f(10,10,0));
    mesh1.appendVertex(Vec3f(200,10,0));

	// not implemented
//    mesh.appendColorRGB(Color(1,1,0));
//    mesh.appendColorRGB(Color(1,0,1));
//    mesh.appendColorRGB(Color(1,0,1));
	
	mesh1.appendTexCoord(Vec2f(0,1));
	mesh1.appendTexCoord(Vec2f(0,0));
	mesh1.appendTexCoord(Vec2f(1,0));

	
	//mesh2
    // vertexes
	mesh2.appendVertex(Vec3f(-100,100,0));
    mesh2.appendVertex(Vec3f(-100,-100,0));
    mesh2.appendVertex(Vec3f(100,-100,0));
	// texture coordinates
	mesh2.appendTexCoord(Vec2f(0,1));
	mesh2.appendTexCoord(Vec2f(0,0));
	mesh2.appendTexCoord(Vec2f(1,0));
    // normals
	mesh2.appendNormal(Vec3f(0,0,-1));
	mesh2.appendNormal(Vec3f(0,0,-1));
	mesh2.appendNormal(Vec3f(0,0,-1));
    
	
	
	glUseProgram(ColorRender::Instance()->program);
	vboMesh = new VboMesh(mesh2);
	gl2::CheckForErrors();

	glUseProgram(0);
	

	setLineWidth(8);
    

    // used by custom mesh.
    texture = ci::loadImage(gl2::getResourcePath("1.png"));
	std::cout << "texture loaded " << texture.getWidth() << std::endl;
	
    // double scale for retina images
    
    background.setTexureScale(2);
    background.setupByResource("background@2x.png", 0, 0, ALIGN_TOPLEFT);
    
    button.setTexureScale(2);
	button.setupByResource("btnStart.png", 320/2, 568/2, ALIGN_CENTER);
	button.onClicked.Connect(this,&mainController::clicked);
	button.argument = "arg test";
	button.enable();
	button.resetData();
	button.update();
    
    globe.setTexureScale(2);
    globe.setupByResource("globe@2x.png", 1024/2.0, 768/ 2.0, ALIGN_CENTER);

    globeDots.setTexureScale(2);
    globeDots.setupByResource("globe_dots@2x.png", 1024/2.0, 768/ 2.0, ALIGN_CENTER);
        
    
    
    perspectiveRender.setup();
    //perspectiveRender.setColor(ColorA(1.0,0.0,1.0,1.0));
	
    App::Instance()->onSizeChanged.Connect(this,&mainController::setSize);
    
    // setup arc test
    arc.setup();
    arc.pCamera = &perspectiveCamera;
    
    
    mTimeline->step(mTimer.getSeconds());
}



void mainController::clicked(uiSpriteButton* button){
	std::cout << button->argument;
//    button->setAlpha(1);
//    button->setCenterPosition(Vec2f(-button->getBoundingBox().getWidth(),button->getCenterPosition().y));
	//"clicked";
    
    
    mTimeline->apply(&arc.rotationVector, Vec2f(0.5,0.5), 2000.0f,EaseInOutQuad());
    
    
}

void mainController::setSize(ci::Vec2f size){
    
    
    // Set default renders to orthocamera
    orthoCamera.setOrtho(0, size.x, 0,size.y, -1, 1);
	gl2::setCamera(orthoCamera.getProjectionMatrix());

    
    
    // Create perspective camera for perspective render
    perspectiveCamera.setPerspective( 60.0f, size.x / size.y, 1.0f, 1000.0f );
	perspectiveCamera.lookAt(Vec3f(10,10.0,500.0),Vec3f(0.0,0.0,0.0),Vec3f::yAxis());

    
    perspectiveRender.setCameraMatrix(perspectiveCamera.getProjectionMatrix() * perspectiveCamera.getModelViewMatrix());

    
    
	std::cout << "set size " << size <<  std::endl;
}


void mainController::update(){
	
	perspectiveRender.setCameraMatrix(perspectiveCamera.getProjectionMatrix() * perspectiveCamera.getModelViewMatrix() * localMatrix);

    float time = sin(gl2::getCurrentTime() / 300.0);
    globeDots.setScale(1 + time / 80.0);
    
    globeDots.update();
	button.update();
    
    mTimeline->step(mTimer.getSeconds());

}



void mainController::draw(){

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.7f, 0.7, 0.75f, 1.0f);
	glClearColor(0.f, 0., 0.f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_BLEND);
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    
    
    TextureRender::Instance()->drawSprite(background);


    //VBO is buggy needs some checks
    //	  bindTexture(texture);
    //    perspectiveRender.drawMesh(*vboMesh);
    //    unbindTexture(texture);
    
	
	bindTexture(texture);
	drawMesh(mesh1);
    unbindTexture(texture);

	
    const vector<Vec2f> activeTouches =  TouchDispatcher::Instance()->getActiveTouches();
    
    for(vector<Vec2f>::const_iterator it = activeTouches.begin(); it != activeTouches.end();++it){
        for(vector<Vec2f>::const_iterator it2 = activeTouches.begin(); it2 != activeTouches.end();++it2){
            setLineWidth(4.0);
            setColor(ColorA(1,1,1,1));
            drawLine(Vec3f(it->x + 2,it->y+2,0), Vec3f(it2->x +2,it2->y +2,0));

            setColor(ColorA(0,1,1,1));
            drawLine(Vec3f(it->x,it->y,0), Vec3f(it2->x,it2->y,0));
        }
    }
	
    
	
    // render sprites
	TextureRender::Instance()->drawSprite(globe);
	TextureRender::Instance()->drawSprite(globeDots);
    TextureRender::Instance()->drawSprite(button);
    
    glEnable(GL_DEPTH_TEST);
    arc.drawShape();
    glDisable(GL_DEPTH_TEST);


	glDisable(GL_BLEND);

}



