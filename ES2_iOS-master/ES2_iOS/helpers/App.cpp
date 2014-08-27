//
//  AppHelper.cpp
//  CinderES2
//
//  Created by Kris Meeusen on 05/06/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#include "App.h"


using namespace gl2;

App* App::instance;


App::App(){
//    std::string getResourcePath(std::string fileName){
        NSString *NSResourcePath = [[NSBundle mainBundle] resourcePath];
        resourcePath = std::string([NSResourcePath UTF8String]);
    
        currentDeviceOrientation = Unknown;
		displayScale = 1;
        //return resourcePath + "/" +  fileName;
}

App* App::Instance(){
    if(instance == NULL)
	{
		instance = new App();
	}
    return instance;
}



std::string App::getResourcePath(std::string fileName){
    return resourcePath + "/" + fileName;
}


void App::orientationChanged(DeviceOrientation orientation){
    currentDeviceOrientation = orientation;
    
    ci::Vec2f newScreenSize = screenSize;
    
    switch (currentDeviceOrientation) {
        case Portrait:
            newScreenSize.set(screenSize.x,screenSize.y);
            std::cout << "portrait" << std::endl;

            break;
        case PortraitUpsideDown:
            newScreenSize.set(screenSize.x,screenSize.y);
            std::cout << "PortraitUpsideDown" << std::endl;

            break;
        case LandscapeLeft:
            newScreenSize.set(screenSize.y,screenSize.x);
            std::cout << "LandscapeLeft" << std::endl;

            break;
        case LandscapeRight:
            newScreenSize.set(screenSize.y,screenSize.x);
            std::cout << "LandscapeRight" << std::endl;

            break;
            
        default:
            break;
    }
    
    onSizeChanged(newScreenSize);
}
