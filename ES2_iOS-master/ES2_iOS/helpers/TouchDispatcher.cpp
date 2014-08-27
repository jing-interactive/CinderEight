//
//  TouchDispatcher.cpp
//  CinderES2
//
//  Created by Kris Meeusen on 26/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#include "TouchDispatcher.h"

TouchDispatcher* TouchDispatcher::instance;

TouchDispatcher* TouchDispatcher::Instance(){
    if(instance == NULL)
	{
        instance = new TouchDispatcher();
        //instance->setup();
	}
    return instance;
}


TouchDispatcher::TouchDispatcher(){
}


const std::vector<ci::Vec2f> TouchDispatcher::getActiveTouches(){
    return mActiveTouches;
}


void TouchDispatcher::setActiveTouches(std::vector<ci::Vec2f> touches){
	mActiveTouches.clear();
	mActiveTouches = touches;
 //   std::cout << mActiveTouches.size() << std::endl;
}

void TouchDispatcher::touchesBegan(std::vector<ci::Vec2f> touches){
	onTouchesBegan(touches);

}

void TouchDispatcher::touchesEnded(std::vector<ci::Vec2f> touches){
    onTouchesEnded(touches);
}

void TouchDispatcher::touchesMoved(std::vector<ci::Vec2f> touches){
    onTouchesMoved(touches);
}




