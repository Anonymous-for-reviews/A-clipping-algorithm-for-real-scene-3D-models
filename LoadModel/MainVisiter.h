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

	void dealTriangleInfo(ModelAttributeFunctor attributeFunctor, osg::TriangleIndexFunctor<TriangleIndex> indexFunctor);//����������õ�����Ϣ�����������ι�ϵ
	osg::ref_ptr<osg::Node> createOsgNode(osg::Vec4 color, int order);//����ָ������ɫ����geom�е����ݴ�����osg�ڵ�

	//filter border triangle of all Geom
	void projectFilterTri(osg::ref_ptr<osg::Vec3Array> ptPolygon, osg::View * view, osg::Matrixd &matrix, MainVisitor & mainvisitor);
	//filter border triangle of geom
	void filterGeomTri(Geom *geom, osg::ref_ptr<osg::Vec3Array> ptPolygon, osg::View * view, int start,int end);

	//Boundary triangulation reconstruction with projection, rebuild all geom
	void projectRebuild();
	
	void rebuildGeom(Geom *geom);

	//�������������֪���߽��ڶ������������ڱ߽��ڵĵ�����
	std::vector<int> Negate(std::vector<int> vertexStatus);

	//line����
	void setLines(vector<osg::Vec3> lineVertices);


	//�ж�ͶӰ���У����Ƿ���������
	vector<int> triangleInterPoly(Geom* geom, int triangleIndexs, osg::ref_ptr<osg::Vec3Array>ptPolygon, osg::View *view);
	//�����߽���������  Traverse the triangulation within the boundary
	void traverseTriangle(osg::Vec3Array *ptPolygon, osg::View *view);
	//Traverse the triangulation of Geom within the boundary 
	void traverseTriangle_Geom(Geom * geom, osg::Vec3Array *ptPolygon, osg::View *view);

	//ɸѡģ������ϵ������Σ�Ҳ����ֻ���������߸������˵������ε�������
	void outsideTriangle();

	//��������ӽ�
	void setEye(osg::Vec3 eye) { _eye = eye; };

	//��������
	osg::ref_ptr<osg::Geode> DrawTriangle(osg::Vec3 vec1, osg::Vec3 vec2, osg::Vec3 vec3, osg::Vec4 color);
	osg::ref_ptr<osg::Geode> DrawTriangle(osg::Vec3 vec1, osg::Vec3 vec2, osg::Vec3 vec3, osg::Vec4 color, float depthValue);
	osg::ref_ptr<osg::Geode> DrawLine(osg::Vec3 vec1, osg::Vec3 vec2, osg::Vec4 color);
	//�������������˹�ϵ
	void createTriangleTopo();

	void filterTriangle(osg::Node* node);

	//�ع���������������������Ҫ�ؽ�
	vector<int> DelaunayTriangulator(osg::ref_ptr<osg::Vec3Array> coords, osg::Vec3 vertex0, osg::Vec3 vertex1, bool middle);

	//�ع�����������һ��������Ҫ�ؽ�
	vector<int> DelaunayTriangulator(osg::ref_ptr<osg::Vec3Array> coords, osg::Vec3 vertex0, bool middle);

	//�ع�����������һ��������Ҫ�ؽ���������һ����������
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
	//�������ڱ߽��ڲ���������
	void inlineTraverseTriangle(Geom* geom, Triangle* triganle);
	//��������ϲ��ô����������
	void outsideTraverse(Geom* geom, Triangle * triganle);
	//���������µ��������ؽ�,�ҽ��ڲ�ͬ��
	void twoIntersectionOnTwoLine(Geom* geom,int triIndex);
	//��������ཻ��һ������
	void moreIntersectionOnOneLine(Geom* geom, int triIndex);
	//�����������������
	void manyIntersectionOnTwoLine(Geom* geom, int triIndex);
	//�����������������
	void manyIntersectionOnThreeLine(Geom* geom, int triIndex);
	//�������������ϵ�ӳ��
	bool pointProjectOnTri(osg::Vec3 middleP, osg::Vec3 vertex0, osg::Vec3 vertex1, osg::Vec3 vertex2, osg::Vec3& projectP);
	//���������㣬���ڲ�ͬ�߽���.�����жϵ��������εĶ��㶼�ڱ߽��ڻ�߽���
	void abnormalTwoIntersectionOnTwoLine(Geom* geom, int triIndex);

protected:
	int _indent;
	vector<Geom*> allGeom;//���е�geom
	osg::Vec4 geomColor;//geom����ɫ
	osg::BoundingBox boundingBox;//��Χ��
	Intersect _inter;
	osg::Vec3 _eye;
	
	std::vector<Quadtree::QuadTreeNode *> _quadnodeList;
};


