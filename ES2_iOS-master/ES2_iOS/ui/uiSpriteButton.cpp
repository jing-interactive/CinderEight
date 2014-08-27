//
//  uiSpriteButton.cpp
//  TouchGallery
//
//  Created by Kris Meeusen on 19/01/13.
//
//

#include "uiSpriteButton.h"
#include "cinder/app/AppBasic.h"
#include "cinder/Timeline.h"
#include "TouchDispatcher.h"


using namespace ci;
using namespace std;
using namespace app;


// uiSpriteButton::uiSpriteButton(){
// }

void uiSpriteButton::enable(){
	//AppBasic::get()->registerMouseDown(this,&uiSpriteButton::mouseDown);
	
    TouchDispatcher::Instance()->onTouchesEnded.Connect(this,&uiSpriteButton::endTouches);
}

void uiSpriteButton::disable(){
	//AppBasic::get()->registerMouseDown(this,&uiSpriteButton::mouseDown);
	
    TouchDispatcher::Instance()->onTouchesEnded.Disconnect(this,&uiSpriteButton::endTouches);
}


void uiSpriteButton::endTouches(std::vector<ci::Vec2f> touches){
    
    for(vector<Vec2f>::const_iterator it = touches.begin();it != touches.end();++it){
        if(getBoundingBox().contains(*it)){
          //  std::cout << "hit" << std::endl;
            onClicked(this);
        }
//            else{
//            std::cout << "ended " << *it << std::endl;
//        }
   
    }
}

bool uiSpriteButton::mouseDown(ci::app::MouseEvent event){
	
	if(getBoundingBox().contains(event.getPos()))
	{
//		onClicked(this);
     //   		onClicked();
		timeline().apply( &mScale, 0.9f, 0.1f, EaseInCubic()).updateFn(std::bind( &uiSprite::setDirty, this ));
		timeline().appendTo( &mScale, 1.0f, 0.1f, EaseInCubic()).updateFn(std::bind( &uiSprite::setDirty, this ));
		return true;
	}

	return false;
}