#pragma once
#include "stdafx.h"
#include "HitCheck.h"
#include "TextureVisitor.h"
#include "Boundary.h"
#include "Quadtree/Quadtree.h"

class PageTraversal {
public:
	PageTraversal();
	~PageTraversal();
	int num;
	int geodeNum;
	void Traversal(osg::Node* node, osg::Node* pLine, osgViewer::Viewer *viewer);
	void Traversal(osg::Node* node);
	osg::ref_ptr<osg::Node> Traversal(osg::Node* node, bool isGroup);
	void setPath(string path);
	void setDirectory(string path);
	void saveAsOSGB(osg::ref_ptr<osg::Node> node, string path);
	void setLineBoundary(BOUNDARY::Boundary * boundary);
	void setLineQuadTree(std::vector<Quadtree::QuadTreeNode *> quadnodeList);
private:
	std::string _path;
	std::string _directory;
	osg::Node* _pLine;
	osg::ref_ptr<osg::Vec3Array> _projectArray;
	vector<osg::Vec3> _lineVertices ;
	osg::ref_ptr<osg::Group> TraversalGroup(osg::Group* group);
	void TraversalGeometry(osg::Geometry* geometry);
	osgViewer::Viewer * _viewer;
	BOUNDARY::Boundary * _boundary;
	std::vector<Quadtree::QuadTreeNode *> _quadnodeList;
};

class CullOSGB {
public:
	CullOSGB() {};
	~CullOSGB();
	osg::ref_ptr<osg::PagedLOD> cropModel(osg::PagedLOD * plod , BOUNDARY::Boundary * boundary, std::vector<Quadtree::QuadTreeNode *> quadnodeList);
	osg::ref_ptr<osg::Geode> cropModel(osg::ref_ptr<osg::Geode> geode , BOUNDARY::Boundary * boundary, std::vector<Quadtree::QuadTreeNode *> quadnodeList);
	void setLine(osg::ref_ptr<osg::Vec3Array> projectArray);
	void setLine3D(vector<osg::Vec3> lineVertices);
	void setViewer(osgViewer::Viewer *viewer);
private:
	std::string save_path;
	osgViewer::Viewer * _viewer;
	osg::ref_ptr<osg::Vec3Array> _projectArray;
	vector<osg::Vec3> _lineVertices;
};

