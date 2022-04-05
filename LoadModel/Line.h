#pragma once
#include "stdafx.h"
#include <osg/Vec3>

class Line
{
public:
	Line();
	~Line();
	osg::ref_ptr<osg::Vec3Array> linePoint;
	int lineIndex;
private:

};

