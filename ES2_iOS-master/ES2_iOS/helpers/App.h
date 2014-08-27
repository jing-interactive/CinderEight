//
//  AppHelper.h
//  CinderES2
//
//  Created by Kris Meeusen on 05/06/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#ifndef __CinderES2__AppHelper__
#define __CinderES2__AppHelper__

#include <iostream>
#include <string.h>

#include "Signal.h"
#include "Delegate.h"

#include "cinder/Vector.h"
#include  "QuartzCore/QuartzCore.h"

using namespace Gallant;

namespace gl2 {


enum DeviceOrientation
{
	Unknown					= 0,
	Portrait				= 1 << 0,
	PortraitUpsideDown		= 1 << 1,
	LandscapeLeft			= 1 << 2,
	LandscapeRight			= 1 << 3,
};
    
    

class App{
  
private:
    static App* instance;
    
    std::string resourcePath;
    
public:
    static App* Instance();

	App();

    std::string getResourcePath(std::string fileName);

    DeviceOrientation currentDeviceOrientation;
    ci::Vec2f screenSize;
	float displayScale;

    
    Signal1< ci::Vec2f > onSizeChanged;
    
    void orientationChanged(DeviceOrientation orientation);


};



    inline std::string getResourcePath(std::string fileName){
        return App::Instance()->getResourcePath(fileName);
    }
	
	inline float getCurrentTime(){
        return CACurrentMediaTime() * 1000.0f;
    }



}
#endif /* defined(__CinderES2__AppHelper__) */
