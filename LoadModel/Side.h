#pragma once
#include <osg/Vec3>
#include <vector>
#include <osg/Vec4>
#include "Intersect.h"

class Side {
public:
	Side();
	~Side();
	int vertexIndexs[2];//顶点索引
	int index;//该边在数组中的索引
	std::vector<int> neighborTriangles;//相邻的三角形的索引
	bool isIntersect = false;
	IntersectionRes intersectionRes;
	std::vector<osg::Vec3> intersections;
	bool IsSame ( const Side & side);
	std::vector<int> intersectionIndexs;
};