#include "ProjectionIntersect.h"

ProjectionIntersect::ProjectionIntersect()
{
}

ProjectionIntersect::ProjectionIntersect(osg::Camera *camera, osg::Vec3Array* ptPolygon)
{
	_camera = camera;
	_ptPolygon = ptPolygon;
}

ProjectionIntersect::~ProjectionIntersect()
{
	
}

bool ProjectionIntersect::pointInPoly(osg::Vec3 p) {
	Intersect inter;
	osg::Vec3 point0Win = worldToWindow(p);
	bool result = inter.PtInPolygon(point0Win, _ptPolygon,false);
	return result;
}

osg::Vec3 ProjectionIntersect::worldToWindow(osg::Vec3 p) {
	osg::Vec3 point;
	osg::Matrixd VPW = _camera->getViewMatrix() *
		_camera->getProjectionMatrix() *
		_camera->getViewport()->computeWindowMatrix();
	point = p * VPW;
	return point;
}