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
	int vertexIndexs[3];//顶点索引
	osg::Vec3 normal;//法向量
	int index;//该三角形在数组中的索引
	std::vector<int> neighborTriangles;//相邻的三角形的索引
	std::vector<int> sideIndexs; //当三角形为边界三角形时，三角形的边被分开存储
	bool isTraversed; //是否已经完成遍历
	bool isAllTraversed; //是否已经完成遍历,在查找边界不需要裁剪的三角形时用到
	bool isBorder;	//是否是边界三角网
	bool isInline; //三角网是否是在边界内部
	bool isChioced; //是否被选中了
	//0:是没有交点但是在边界外，不裁剪;1:是没有交点但是在边界内，需要删掉;2:是有两个交点，且两个交点在不同的三角边上;3:是有两个交点，且两个交点在同一条边上;
	//4:交点大于两个且只与一条边相交;5:交点大于两个且与两条边相交;6:交点大于两个且与三条边相交
	int intersectionStatus; 

	//Point opposite the triangle side
	int whichVertex(int edge1_1, int edge1_2, int edge2_1, int edge2_2);

	int oppositePoint(int edge_1, int edge_2);
	
	std::vector<IntersectionRes> intersectionRess;
	std::vector<osg::Vec3> intersectPoints; //交点
};
