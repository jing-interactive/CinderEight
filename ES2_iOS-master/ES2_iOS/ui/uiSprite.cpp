//
//  uiSprite.cpp
//  EristoffGameServer
//
//  Created by Kris Meeusen on 17/09/12.
//
//

#include "uiSprite.h"
#include "cinder/app/AppBasic.h"
#include "cinder/ImageIo.h"

#include "App.h"

using namespace ci;
using namespace ci::app;
using namespace std;

uiSprite::uiSprite(){
    mWidth = 0;
    mHeight = 0;

    mRotationRadians = 0;
    mScale = 1;
    mAlpha = 0;
    mTextureScale = 1;
	
	mParent = 0;

    isDirty = 0;
	isTextureLoaded = false;
	
	mTotalAnimationframes = 1;
	mCurrentAnimationframe = 0;
	
	data = new float [36];
	
	mModelMatrix.setToIdentity();
	mWorldMatrix.setToIdentity();
	
//	// This will identify our vertex buffer
//	GLuint vertexbuffer;
//		// Generate 1 buffer, put the resulting identifier in vertexbuffer
//	glGenBuffers(1, &vertexbuffer);
//		// The following commands will talk about our 'vertexbuffer' buffer
//	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	
	

}

void uiSprite::setTexureScale(float textureScale){
    mTextureScale = textureScale;
}

void uiSprite::setupByTexture(ci::gl::Texture texture,float x,float y,int align){
    setupByTexture(texture, Vec2f(x,y),align);
}

void uiSprite::setupByTexture(ci::gl::Texture texture,Vec2f position,int align){
//    if(isTextureLoaded)
//		mTexture.reset()
		
	mTexture = texture;
	
	isTextureLoaded = true;
    
    mWidth =  mTexture.getWidth()  / mTextureScale / mTotalAnimationframes;
    mHeight = mTexture.getHeight() / mTextureScale;
    
	mOriginalPoint = position;
    mAlligment = align;
	
    switch (align) {
        case ALIGN_TOPLEFT:
            mCenterPointAnimated().x = position.x + (mWidth / 2.0);
            mCenterPointAnimated().y = position.y + (mHeight / 2.0);
            break;
            
        case ALIGN_TOPRIGHT:
            mCenterPointAnimated().x = position.x - mWidth / 2.0;
            mCenterPointAnimated().y = position.y + mHeight / 2.0;
            break;
			
        case ALIGN_TOPCENTER:
            mCenterPointAnimated().x = position.x;
            mCenterPointAnimated().y = position.y + (mHeight / 2.0);
            break;
            
        case ALIGN_BOTTOMRIGHT:
            mCenterPointAnimated().x = position.x - mWidth / 2.0;
            mCenterPointAnimated().y = position.y - mHeight / 2.0;
            break;
            
        case ALIGN_BOTTOMLEFT:
            mCenterPointAnimated().x = position.x + mWidth / 2.0;
            mCenterPointAnimated().y = position.y - mHeight / 2.0;
            break;

        case ALIGN_CENTERLEFT:
            mCenterPointAnimated().x = position.x + mWidth / 2.0;
            mCenterPointAnimated().y = position.y;
            break;
            
        case ALIGN_CENTERRIGHT:
            mCenterPointAnimated().x = position.x - mWidth / 2.0;
            mCenterPointAnimated().y = position.y;
            break;
                  
			
			
        case ALIGN_CENTER:
        default:
            mCenterPointAnimated().x = position.x;
            mCenterPointAnimated().y = position.y;
            break;
            
            
            // TODO implement all the other aligns
    }
    
	resetData();
//	setDirty();
}


void uiSprite::setParent(uiSprite* parent){
	mParent = parent;
}

// children
void uiSprite::addChild(uiSprite* sprite){
	sprite->setParent(this);
	sprite->setDirty();
	children.push_back(sprite);
	
}

const std::vector<uiSprite*>& uiSprite::getChildren(){
	return children;
}

const ci::Matrix44f& uiSprite::getWorldMatrix(){
	return mWorldMatrix;
}





void uiSprite::setupByResource(std::string fileName,float x,float y,int align){
    gl::Texture texture =  loadImage(gl2::getResourcePath(fileName));
    setupByTexture(texture, x, y, align);
}

void uiSprite::setupByAsset(std::string fileName,float x,float y,int align){
    gl::Texture texture =  loadImage(gl2::getResourcePath(fileName));
    setupByTexture(texture, x, y, align);
}

void uiSprite::setupByImagePath(std::string fileName,float x,float y,int align){
    gl::Texture texture =  loadImage(fileName);
    setupByTexture(texture, x, y, align);
}

void uiSprite::changeImage(std::string fileName){
	setupByTexture(loadImage(fileName), mOriginalPoint.x, mOriginalPoint.y, mAlligment);
}

void uiSprite::setAlpha(float alpha){
	mAlpha = alpha;
	isDirty = true;
}

float uiSprite::getAlpha() const{
    return mAlpha;
}

void uiSprite::setScale(float scale){
	mScale = scale;
	isDirty = true;
}

float uiSprite::getScale() const{
    return mScale;
}



void uiSprite::setTotalAnimationframes(int totalAnimationframes){
	mTotalAnimationframes = totalAnimationframes;
	if(isTextureLoaded) mWidth = mTexture.getWidth() / mTotalAnimationframes;
	isDirty = true;
}

void uiSprite::setCurrentAnimationFrame(int animationframe){
	mCurrentAnimationframe = animationframe;
	isDirty = true;
}


Vec2f uiSprite::getCenterPosition( ){
    return mCenterPointAnimated;
}

//properties
void uiSprite::setCenterPosition(Vec2f newPosition){
    mCenterPointAnimated = newPosition;
    isDirty = true;
}

void uiSprite::setRotation(float rotation){
	mRotationRadians = rotation *  M_PI / 180.0;
	isDirty = true;
}

float uiSprite::getRotation() const{
    return mRotationRadians / (M_PI/180.0);
}




void uiSprite::update(){
   	
	for(vector<uiSprite*>::const_iterator it = children.begin() ;it != children.end();++it){
		if(isDirty) (*it)->setDirty();
		(*it)->update();
	}
	
	if(isDirty){
        isDirty = false;
        resetData();
    }


}

void uiSprite::moveTo(ci::Vec2f position,float durationSec,float delay){
	timeline().apply( &mCenterPointAnimated, position, durationSec, EaseInCubic()).updateFn( std::bind( &uiSprite::setDirty, this ) );
}

void uiSprite::fadeTo(float newAlpha,float durationSec,float delay){
	timeline().apply( &mAlpha, newAlpha, durationSec, EaseInCubic()).updateFn( std::bind( &uiSprite::setDirty, this )).delay(delay);
}

void uiSprite::scaleTo(float newScale,float durationSec,float delay){
	timeline().apply( &mScale, newScale, durationSec, EaseInCubic()).updateFn( std::bind( &uiSprite::setDirty, this )).delay(delay);
}



void uiSprite::setDirty(){
	isDirty = true;
}

void uiSprite::resetData(){
     
    float uvHeight = 1.0;
	float uvWidth = 1.0 / mTotalAnimationframes;
	float uvLeft = mCurrentAnimationframe * uvWidth;
	float uvRight = (mCurrentAnimationframe+1) * uvWidth;

    leftBottom = Vec4f(- mWidth/2, mHeight/2,0.0f,1.0f);
    leftTop = Vec4f( - mWidth/2, -mHeight/2,0.0f,1.0f);
    rightBottom = Vec4f(  mWidth/2,mHeight/2,0.0f,1.0f);
    rightTop = Vec4f(  mWidth/2,-mHeight/2,0.0f,1.0f);
    
    mModelMatrix.setToIdentity();
	mModelMatrix.setTranslate(Vec4f(mCenterPointAnimated().x,mCenterPointAnimated().y,0,1));

    mModelMatrix.rotate(Vec3f(0,0,1), mRotationRadians);
	mModelMatrix.scale(Vec3f(mScale,mScale,1));

	if(mParent != 0){
		mWorldMatrix = mParent->getWorldMatrix();
		mWorldMatrix *= mModelMatrix;
	}else{
		mWorldMatrix = mModelMatrix;
	}

	
    leftBottom = mWorldMatrix.postMultiply(leftBottom);
    leftTop=mWorldMatrix.postMultiply(leftTop);
    rightBottom=mWorldMatrix.postMultiply(rightBottom);
    rightTop=mWorldMatrix.postMultiply(rightTop);
    
    data[0] = leftTop.x;
    data[1] = leftTop.y;
    data[2] = 0;
    
    data[3] = uvLeft;
    data[4] = 0;
    data[5] = mAlpha;
    

	data[6] = rightTop.x ;
    data[7] = rightTop.y ;
    data[8] = 0;
    
	data[9] = uvRight;
    data[10] = 0;
    data[11] = mAlpha;
    
    
    data[12] = leftBottom.x ;
    data[13] = leftBottom.y ;
    data[14] = 0;
    
    data[15] = uvLeft;
    data[16] = uvHeight;
    data[17] = mAlpha;
    
    
    data[18] = rightBottom.x ;
    data[19] = rightBottom.y ;
    data[20] = 0;
    
    data[21] = uvRight;
    data[22] = uvHeight;
    data[23] = mAlpha;
    
    
    data[24] = data[12];
    data[25] = data[13];
    data[26] = data[14];
    
    data[27] = data[15];
    data[28] = data[16];
    data[29] = data[17];
    
    
    data[30] = data[6];
    data[31] = data[7];
    data[32] = data[8];
    
    data[33] = data[9];
    data[34] = data[10];
    data[35] = data[11];

}

 ci::gl::Texture& uiSprite::getTexture(){
    return mTexture;
}


void uiSprite::setTexture(ci::gl::Texture texture){
    mTexture = texture;
}


ci::Rectf uiSprite::getBoundingBox(){
	
	Vec3f corner1;
	Vec3f corner2;
	
	corner1.x = min(leftTop.x,rightBottom.x);
	corner1.y = min(leftTop.y,rightBottom.y);

	corner2.x = max(leftTop.x,rightBottom.x);
	corner2.y = max(leftTop.y,rightBottom.y);
	
	Rectf boundingBox(corner1.x,corner1.y,corner2.x,corner2.y );

	return boundingBox;
}


bool uiSprite::isHit(ci::Vec2f point){
    return getBoundingBox().contains(point);
}
