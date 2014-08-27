//
//  VboMesh.cpp
//  CinderES2
//
//  Created by Kris Meeusen on 12/04/13.
//  Copyright (c) 2013 Kris Meeusen. All rights reserved.
//

#include "VboMesh.h"

using namespace ci;


bool VboMesh::hasTexCoords(){
	return mHasTexCoords;
}

int VboMesh::getStride(){
	return mStride;
}



VboMesh::VboMesh(ci::TriMesh mesh){
	
	mHasTexCoords = false;
	
	
	GLuint VertexArrayID;
	glGenVertexArraysOES(1, &VertexArrayID);
	glBindVertexArrayOES(VertexArrayID);
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	
	
	
	mStride  = 3;
	if(mesh.hasTexCoords()){ mStride += 3; mHasTexCoords = true;}
	if(mesh.hasColorsRGBA()) mStride += 4;
	if(mesh.hasNormals()) mStride += 3;
	
	GLfloat g_vertex_buffer_data[mesh.getNumVertices()*mStride];

    
    int arrayIndex = 0;
    int indexOffset = 0;
    
    for (std::vector<Vec3f>::const_iterator it = mesh.getVertices().begin(); it != mesh.getVertices().end(); ++it) {

        
		g_vertex_buffer_data[arrayIndex] = it->x;
        g_vertex_buffer_data[arrayIndex +1] = it->y;
        g_vertex_buffer_data[arrayIndex +2] = it->z;
        arrayIndex+=mStride;
    }
    
    indexOffset += 3;

    if(mesh.hasTexCoords()){
	
        arrayIndex = indexOffset;
        for (std::vector<Vec2f>::const_iterator it = mesh.getTexCoords().begin(); it != mesh.getTexCoords().end(); ++it) {		
            
            g_vertex_buffer_data[arrayIndex] = it->x;
            g_vertex_buffer_data[arrayIndex +1] = it->y;
            g_vertex_buffer_data[arrayIndex +2] = 1;
            arrayIndex+=mStride;
            
        }
        
        indexOffset+=3;
    }

    
    
    if(mesh.hasNormals()){
        arrayIndex = indexOffset;

        for (std::vector<Vec3f>::const_iterator it = mesh.getNormals().begin(); it != mesh.getNormals().end(); ++it) {
            
            g_vertex_buffer_data[arrayIndex] = it->x;
            g_vertex_buffer_data[arrayIndex +1] = it->y;
            g_vertex_buffer_data[arrayIndex +2] = it->z;
            arrayIndex+=mStride;
        }
    }

    
    
    
    
//	{
//		-1.0f, -1.0f, 0.0f,
//		1.0f, -1.0f, 0.0f,
//		0.0f,  1.0f, 0.0f,
//	};
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);


}
