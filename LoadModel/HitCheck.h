#pragma once
#include "stdafx.h"
#include <osgUtil/IntersectVisitor>

class HitCheck
{
public:
	HitCheck();
	~HitCheck();
	bool Check(osg::Node *pLine, osg::Node *node);//����Node�ڵ��Ƿ��ཻ
	bool BoundingCheck(osg::Node *pLine, osg::Node *node);
private:
	void GetAllLineSegment(osg::Geometry *pGeometry, osgUtil::IntersectVisitor *pIv, osg::Matrixd &matParent);
	void GetAllLineSegment(osg::Node *pNode, osgUtil::IntersectVisitor *pIv, osg::Matrixd &matParent);
};
