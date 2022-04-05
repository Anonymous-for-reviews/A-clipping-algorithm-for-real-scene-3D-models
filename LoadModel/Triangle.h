#pragma once
#include <osg/Vec3>
#include <vector>
#include <osg/Vec4>
#include "Intersect.h"
#include "Side.h"

class Triangle
{
public:
	void init();
	Triangle();
	~Triangle();
	int vertexIndexs[3];//��������
	osg::Vec3 normal;//������
	int index;//���������������е�����
	std::vector<int> neighborTriangles;//���ڵ������ε�����
	std::vector<int> sideIndexs; //��������Ϊ�߽�������ʱ�������εı߱��ֿ��洢
	bool isTraversed; //�Ƿ��Ѿ���ɱ���
	bool isAllTraversed; //�Ƿ��Ѿ���ɱ���,�ڲ��ұ߽粻��Ҫ�ü���������ʱ�õ�
	bool isBorder;	//�Ƿ��Ǳ߽�������
	bool isInline; //�������Ƿ����ڱ߽��ڲ�
	bool isChioced; //�Ƿ�ѡ����
	//0:��û�н��㵫���ڱ߽��⣬���ü�;1:��û�н��㵫���ڱ߽��ڣ���Ҫɾ��;2:�����������㣬�����������ڲ�ͬ�����Ǳ���;3:�����������㣬������������ͬһ������;
	//4:�������������ֻ��һ�����ཻ;5:����������������������ཻ;6:����������������������ཻ
	int intersectionStatus; 

	//Point opposite the triangle side
	int whichVertex(int edge1_1, int edge1_2, int edge2_1, int edge2_2);

	int oppositePoint(int edge_1, int edge_2);
	
	std::vector<IntersectionRes> intersectionRess;
	std::vector<osg::Vec3> intersectPoints; //����
};
