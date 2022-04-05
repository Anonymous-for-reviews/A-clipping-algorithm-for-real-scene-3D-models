#pragma once
#include "stdafx.h"
#include "Utility.h"
#include "Intersect.h"

class ProjectionIntersect
{
public:
	ProjectionIntersect();
	ProjectionIntersect(osg::Camera *camera, osg::Vec3Array* ptPolygon);
	~ProjectionIntersect();
	osg::Vec3 worldToWindow(osg::Vec3 p);
	bool pointInPoly(osg::Vec3 p);

private:
	osg::Camera* _camera;
	osg::Vec3Array* _ptPolygon;
};

