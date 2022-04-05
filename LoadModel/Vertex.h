#pragma once
#include <osg/Vec3>
#include <osg/Vec2>
#include <osg/Vec4>
#include <vector>
using namespace std;
enum Topology {
	UNKNOW = 0,
	Inline = 1,
	Outline = 2,
	OnLine = 3,
};
/**
 * �����࣬��¼ģ���ж���ĸ�����Ϣ�������������ꡢ�����������������
 */
class Vertex
{
public:
	osg::Vec3 coor;//��������
	osg::Vec3 normal;//������
	osg::Vec2 texCoor;//��������
	int index;//�ö����������е��±�
	vector<int> neighborTriangle;//�������ڵ�������
	int newIndex=-1; //�����½��������ڶ��������е�����
	int topology = 0;

	Vertex();
	~Vertex();
};

