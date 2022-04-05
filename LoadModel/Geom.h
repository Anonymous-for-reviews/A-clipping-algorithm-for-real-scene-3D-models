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
	vector<Vertex*> vertices;//һ��geom�����еĶ�����Ϣ
	vector<Triangle*> triangles;//һ��geom�е�������������Ϣ

	vector<Vertex*> newVertices;//һ��geom�вü���Ķ�����Ϣ
	vector<Triangle*> newTriangles;//һ��geom�вü������������Ϣ

	//vector<Side*> allSides;//�߽��������ı���Ϣ������߽����������˹�ϵ
	vector<Side*> sides;//�߽��������ı���Ϣ������߽����������˹�ϵ

	vector<int>  borderTriangles;
	vector<int>  choicedTriangles;
	osg::BoundingBox  boundingBox;//��Χ��
	bool isTwoTriangleNeighbor(int triangle1Index, int triangle2Index, int& side0, int& side1);//�����������Ƿ�����
	bool isSideTriangleNeighbor(int sideIndex1, int sideIndex2, int triangleIndex); //�������γɵıߺ��������Ƿ�����

	void createTriangleTopo();//����������֮������˹�ϵ
	void createVertexTopo();//��������֮�������
	osg::ref_ptr<osg::Geode> createOsgNode(osg::Vec4 color);

	void reorderTriangle(); //�ѱ߽������θ������˹�ϵ������

	void rebuildOutsideTriangle(); //���ڲ���ѡ�������ڵ����������������򶥵㡢���½�������
	int inlineTriangleNum=0;
	int outTriangleNum=0;
	Geom();
	~Geom();
};