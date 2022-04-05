#pragma once
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Array>
#include <vector>

class Vec3Custom : public osg::Vec3f
{
public:
	Vec3Custom ();
	~Vec3Custom ();
	int index;
private:

};

class Vec3CustomArray :public osg::Vec3Array {
public:
	Vec3CustomArray() {};
	~Vec3CustomArray() {};
	std::vector<int> indexs;
private:

};
