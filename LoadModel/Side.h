#pragma once
#include <osg/Vec3>
#include <vector>
#include <osg/Vec4>
#include "Intersect.h"

class Side {
public:
	Side();
	~Side();
	int vertexIndexs[2];//��������
	int index;//�ñ��������е�����
	std::vector<int> neighborTriangles;//���ڵ������ε�����
	bool isIntersect = false;
	IntersectionRes intersectionRes;
	std::vector<osg::Vec3> intersections;
	bool IsSame ( const Side & side);
	std::vector<int> intersectionIndexs;
};