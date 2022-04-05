#pragma once
#include <vector>
#include "Vertex.h"
#include "Triangle.h"
#include "Side.h"
#include <osg/ref_ptr>
#include <osg/Geode>
using namespace std;


class Geom
{
public:
	vector<Vertex*> vertices;//一个geom中所有的顶点信息
	vector<Triangle*> triangles;//一个geom中的所有三角形信息

	vector<Vertex*> newVertices;//一个geom中裁剪后的顶点信息
	vector<Triangle*> newTriangles;//一个geom中裁剪后的三角形信息

	//vector<Side*> allSides;//边界三角网的边信息，保存边界三角网拓扑关系
	vector<Side*> sides;//边界三角网的边信息，保存边界三角网拓扑关系

	vector<int>  borderTriangles;
	vector<int>  choicedTriangles;
	osg::BoundingBox  boundingBox;//包围盒
	bool isTwoTriangleNeighbor(int triangle1Index, int triangle2Index, int& side0, int& side1);//两个三角形是否相邻
	bool isSideTriangleNeighbor(int sideIndex1, int sideIndex2, int triangleIndex); //两个点形成的边和三角形是否相邻

	void createTriangleTopo();//创建三角形之间的拓扑关系
	void createVertexTopo();//创建顶点之间的拓扑
	osg::ref_ptr<osg::Geode> createOsgNode(osg::Vec4 color);

	void reorderTriangle(); //把边界三角形根据拓扑关系重排序

	void rebuildOutsideTriangle(); //对于不在选择区域内的三角网，重新排序顶点、重新建立索引
	int inlineTriangleNum=0;
	int outTriangleNum=0;
	Geom();
	~Geom();
};