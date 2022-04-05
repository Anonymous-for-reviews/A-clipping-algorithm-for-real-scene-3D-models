#pragma once
#include <thread>
#include <osg/Node>
#include <osg/Geode>  
#include <osg/Group>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgFX/Scribe>
#include <osgGA/GUIEventHandler>
#include <osgUtil/Optimizer>
#include <iostream>
#include <osg/NodeVisitor>
#include <vector>
#include "ModelAttributeFunctor.h"
#include <osg/TriangleIndexFunctor>
#include "TriangleIndex.h"
#include "Geom.h"
#include "ProjectionIntersect.h"
#include "Intersect.h"
#include "Vec3Custom.h"
#include <algorithm>
#include <cmath>
#include "Boundary.h"
#include "Quadtree/Quadtree.h"

namespace Point {
	struct IntersectPoint
	{
		//Intersection coordinates
		osg::Vec3 coordinate;
		//Index of intersecting boundary line
		int lineIndex;
		//poin's indexs of intersecting triangle edge
		int edge[2];
		//Proportion of intersection on the edge
		float intersectPro;
		//Indexs of intersecting triangle edge
		int edgeIndex;
	};
}

class MainVisitor : public osg::NodeVisitor
{

public:

	MainVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN), _indent(0){
		_group = new osg::Group;
		_points= new osg::Group;
	}
	~MainVisitor();
	virtual void apply(osg::Node& node);
	virtual void apply(osg::Geode& node);
	virtual void apply(osg::Geometry& geomrtry);

	void dealTriangleInfo(ModelAttributeFunctor attributeFunctor, osg::TriangleIndexFunctor<TriangleIndex> indexFunctor);//处理访问器得到的信息，构建三角形关系
	osg::ref_ptr<osg::Node> createOsgNode(osg::Vec4 color, int order);//根据指定的颜色，将geom中的数据创建成osg节点

	//filter border triangle of all Geom
	void projectFilterTri(osg::ref_ptr<osg::Vec3Array> ptPolygon, osg::View * view, osg::Matrixd &matrix, MainVisitor & mainvisitor);
	//filter border triangle of geom
	void filterGeomTri(Geom *geom, osg::ref_ptr<osg::Vec3Array> ptPolygon, osg::View * view, int start,int end);

	//Boundary triangulation reconstruction with projection, rebuild all geom
	void projectRebuild();
	
	void rebuildGeom(Geom *geom);

	//针对三角网，在知道边界内顶点索引，求不在边界内的点索引
	std::vector<int> Negate(std::vector<int> vertexStatus);

	//line顶点
	void setLines(vector<osg::Vec3> lineVertices);


	//判断投影面中，点是否在区域内
	vector<int> triangleInterPoly(Geom* geom, int triangleIndexs, osg::ref_ptr<osg::Vec3Array>ptPolygon, osg::View *view);
	//遍历边界内三角网  Traverse the triangulation within the boundary
	void traverseTriangle(osg::Vec3Array *ptPolygon, osg::View *view);
	//Traverse the triangulation of Geom within the boundary 
	void traverseTriangle_Geom(Geom * geom, osg::Vec3Array *ptPolygon, osg::View *view);

	//筛选模型最边上的三角形，也就是只有两个或者更少拓扑的三角形的三角形
	void outsideTriangle();

	//设置相机视角
	void setEye(osg::Vec3 eye) { _eye = eye; };

	//画三角网
	osg::ref_ptr<osg::Geode> DrawTriangle(osg::Vec3 vec1, osg::Vec3 vec2, osg::Vec3 vec3, osg::Vec4 color);
	osg::ref_ptr<osg::Geode> DrawTriangle(osg::Vec3 vec1, osg::Vec3 vec2, osg::Vec3 vec3, osg::Vec4 color, float depthValue);
	osg::ref_ptr<osg::Geode> DrawLine(osg::Vec3 vec1, osg::Vec3 vec2, osg::Vec4 color);
	//建立三角网拓扑关系
	void createTriangleTopo();

	void filterTriangle(osg::Node* node);

	//重构三角网，有两个顶点需要重建
	vector<int> DelaunayTriangulator(osg::ref_ptr<osg::Vec3Array> coords, osg::Vec3 vertex0, osg::Vec3 vertex1, bool middle);

	//重构三角网，有一个顶点需要重建
	vector<int> DelaunayTriangulator(osg::ref_ptr<osg::Vec3Array> coords, osg::Vec3 vertex0, bool middle);

	//重构三角网，有一个顶点需要重建，并传入一个限制条件
	vector<int> DelaunayTriangulator(osg::ref_ptr<osg::Vec3Array> coords, osg::Vec3 vertex0, osg::ref_ptr<osg::Vec3Array> Constraint);

	vector<Geom*> GetGeoms() { return allGeom; };

	void setLineBoundary(BOUNDARY::Boundary * boundary);
	void setLineQuadTree(std::vector<Quadtree::QuadTreeNode *> quadnodeList);

	int getTriangleNum();

	static osg::Vec3Array* vertArray;
	static osg::Geometry::PrimitiveSetList primitiveSetList;
	osg::ref_ptr<osg::UIntArray> indices;
	osg::ref_ptr<osg::Group> _group;
	osg::ref_ptr<osg::Group> _points;
	vector<osg::Vec3> _lineVertices;
	BOUNDARY::Boundary * _boundary;
	vector<int> triangleOrder;

private:
	//遍历属于边界内部的三角网
	void inlineTraverseTriangle(Geom* geom, Triangle* triganle);
	//遍历最边上不用处理的三角形
	void outsideTraverse(Geom* geom, Triangle * triganle);
	//两个交点下的三角网重建,且交于不同边
	void twoIntersectionOnTwoLine(Geom* geom,int triIndex);
	//多个交点相交在一条边上
	void moreIntersectionOnOneLine(Geom* geom, int triIndex);
	//多个交点在两条边上
	void manyIntersectionOnTwoLine(Geom* geom, int triIndex);
	//多个交点在两条边上
	void manyIntersectionOnThreeLine(Geom* geom, int triIndex);
	//交点在三角网上的映射
	bool pointProjectOnTri(osg::Vec3 middleP, osg::Vec3 vertex0, osg::Vec3 vertex1, osg::Vec3 vertex2, osg::Vec3& projectP);
	//有两个交点，且在不同边界上.但是判断的是三角形的顶点都在边界内或边界外
	void abnormalTwoIntersectionOnTwoLine(Geom* geom, int triIndex);

protected:
	int _indent;
	vector<Geom*> allGeom;//所有的geom
	osg::Vec4 geomColor;//geom的颜色
	osg::BoundingBox boundingBox;//包围盒
	Intersect _inter;
	osg::Vec3 _eye;
	
	std::vector<Quadtree::QuadTreeNode *> _quadnodeList;
};


