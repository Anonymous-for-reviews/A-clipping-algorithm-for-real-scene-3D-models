#pragma once

#include <osg/ref_ptr>
#include <osg/Array>
#include <osg/TriangleIndexFunctor>

class TriangleIndex
{
public:
	osg::ref_ptr<osg::UIntArray> indexs;//所有的索引
	int triangleNum;//三角形的数量
	TriangleIndex();
	~TriangleIndex();
	void operator()(const unsigned int& v1, const unsigned int& v2, const unsigned int& v3);
};