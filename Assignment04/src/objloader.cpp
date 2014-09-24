#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <iostream>


#include <glm/glm.hpp>
#include "objloader.h"

static int numFaces = 0;

int loadOBJ(const char * path,  std::vector<Vertex> & out_vertices, 
		std::vector<Vertex> & out_uvs, std::vector<Vertex> & out_normals){

	printf("Loading OBJ file %s...\n", path);

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<Vertex> temp_vertices; 
	std::vector<Vertex> temp_uvs;
	std::vector<Vertex> temp_normals;


	FILE * file = fopen(path, "r");
	if( file == NULL ){
		printf("Can't open file\n");
		getchar();
		return 0;
	}

	while( 1 ){

		char lineHeader[128];
		// read the first word of the line
		int res = fscanf(file, "%s", lineHeader);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.

		//else : parse lineHeader
		
		if ( strcmp( lineHeader, "v" ) == 0 ){
			Vertex vertex;
			fscanf(file, "%f %f %f\n", &vertex.position[0], &vertex.position[1], &vertex.position[2] );
			temp_vertices.push_back(vertex);
		}else if ( strcmp( lineHeader, "vt" ) == 0 ){
			Vertex uv;
			fscanf(file, "%f %f\n", &uv.position[0], &uv.position[1] );
			temp_uvs.push_back(uv);
		}else if ( strcmp( lineHeader, "vn" ) == 0 ){
			Vertex normal;
			fscanf(file, "%f %f %f\n", &normal.position[0], &normal.position[1], &normal.position[2] );
			temp_normals.push_back(normal);
		}else if ( strcmp( lineHeader, "f" ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3] , normalIndex[3];
			int matches = fscanf(file, "%d//%d %d//%d %d//%d \n", &vertexIndex[0],  &normalIndex[0], &vertexIndex[1],  &normalIndex[1] , &vertexIndex[2],  &normalIndex[2] );
			if (matches != 6){
				printf("File can't be read\n");
				return false;
			}
			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			
			normalIndices.push_back(normalIndex[0]);
			normalIndices.push_back(normalIndex[1]);
			normalIndices.push_back(normalIndex[2]);			
			
			//increment numFaces
			numFaces++;
		}

	}
	
	// For each vertex of each triangle
	for( unsigned int i=0; i<vertexIndices.size(); i++ ){

		// Get the indices of its attributes
		unsigned int vertexIndex = vertexIndices[i];
		
		unsigned int normalIndex = normalIndices[i];
		
		// Get the attributes thanks to the index
		Vertex vertex = temp_vertices[ vertexIndex-1 ];
		
		Vertex normal = temp_normals[ normalIndex-1 ];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_normals .push_back(normal);
		
		
		
	}

	return numFaces;
}
