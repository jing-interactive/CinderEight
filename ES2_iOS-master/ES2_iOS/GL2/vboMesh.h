//
//  vboMesh.h
//  CinderES2
//
//  Created by Kris Meeusen on 12/04/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#ifndef __CinderES2__VboMesh__
#define __CinderES2__VboMesh__

#include <iostream>
#include "cinder/TriMesh.h"

class VboMesh{
	

public:
	VboMesh(ci::TriMesh mesh);
	GLuint vertexbuffer;

	bool hasTexCoords();
	int getStride();
	
	
private:
	int mStride;
	bool mHasTexCoords;
};




#endif /* defined(__CinderES2__vboMesh__) */
