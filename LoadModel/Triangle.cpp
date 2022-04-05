#include "stdafx.h"
#include "Triangle.h"


Triangle::Triangle()
{
	init();
}


Triangle::~Triangle()
{
}

void Triangle::init()
{
	this->vertexIndexs[0] = this->vertexIndexs[1] = this->vertexIndexs[2] = -1;
	this->normal.set(0.0f, 0.0f, 0.0f);
	this->isTraversed = false;
	this->isBorder = false;
	this->isInline = false;
	this->isAllTraversed = false;
	this->isChioced = false;
}

//which vertex is the intersection of edge1 and edge2 ; return 0;1;2 which represents three vertex. -1 is false
int Triangle::whichVertex(int edge1_1, int edge1_2, int edge2_1, int edge2_2) {
	int vertex_index;
	//this edges are same 
	if ((edge1_1 == edge2_1 && edge1_2 == edge2_2) ||
		(edge1_1 == edge2_2 && edge1_2 == edge2_1))
		return -1;

	if (edge1_1 == edge2_1 || edge1_1 == edge2_2) vertex_index = edge1_1;
	else if (edge1_2 == edge2_1 || edge1_2 == edge2_2) vertex_index = edge1_2;
	else return -1;
	// return 0;1;2 
	// intersectiong vertex is 0-1-2 
	if (vertex_index == vertexIndexs[0])
		return 0;
	else if (vertex_index == vertexIndexs[1])
		return 1;
	else return 2;
}

int Triangle::oppositePoint(int edge_1, int edge_2) {
	if ((vertexIndexs[0] == edge_1 && vertexIndexs[1] == edge_2) || (vertexIndexs[0] == edge_2 && vertexIndexs[1] == edge_1))
		return 2;
	if ((vertexIndexs[1] == edge_1 && vertexIndexs[2] == edge_2) || (vertexIndexs[1] == edge_2 && vertexIndexs[2] == edge_1))
		return 0;
	if ((vertexIndexs[2] == edge_1 && vertexIndexs[0] == edge_2) || (vertexIndexs[2] == edge_2 && vertexIndexs[0] == edge_1))
		return 1;
	return -1;
}