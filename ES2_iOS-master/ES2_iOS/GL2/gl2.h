//
//  gl2.h
//  CinderES2
//
//  Created by Kris Meeusen on 22/02/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//
#pragma once

#include <iostream>
#include "ColorRender.h"
#include "TextureRender.h"

#include "cinder/TriMesh.h"

namespace gl2 {

	inline void drawLine(const ci::Vec3f &start, const ci::Vec3f &end ){
		ColorRender::Instance()->drawLine(start, end);
	}
	
	inline void setColor(const ci::ColorA color){
		ColorRender::Instance()->setColor(color);
	}

	inline void setLineWidth(const float lineWidth){
		ColorRender::Instance()->setLineWidth(lineWidth);
	}
	
	inline void drawMesh(const ci::TriMesh mesh){
		if(mesh.hasTexCoords()){
			TextureRender::Instance()->drawMesh(mesh);
		}else{
			ColorRender::Instance()->drawMesh(mesh);
		}
	}

	inline void setCamera(const ci::Matrix44f worldMatrix){
		ColorRender::Instance()->setCameraMatrix(worldMatrix);
		TextureRender::Instance()->setCameraMatrix(worldMatrix);
	}
	

    inline void bindTexture(const ci::gl::Texture& texture){
        TextureRender::Instance()->bindTexture(texture);
    }

    inline void unbindTexture(const ci::gl::Texture& texture){
        TextureRender::Instance()->unbindTexture(texture);
    }
    
    inline void drawTexture(const ci::gl::Texture &texture){
        TextureRender::Instance()->drawTexture(texture);
    }

}

