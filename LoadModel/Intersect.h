#pragma once
#include "stdafx.h"
#include <osg/Vec3>
#include <osg/Vec2>
#include <osg/Vec4>
#include "Line.h"
#include "Quadtree/Quadtree.h"
#include "Utility.h"
using namespace std;
struct IntersectRes
{
	int isIntersect;
	osg::Vec3 intersectPoint;
	int lineIndex;
};

//*pointNum 是相交的交点个数
//*proportions 是相交点对应在线段中的比例
//*indices 线段在三角中的顶点索引  现在已经不需要了
//* lineIndex 相交部分在曲线的第几段 的索引值
struct IntersectionRes
{
	int pointNum;
	vector<float>  proportions;
	int indices[2];
	vector<int> lineIndex;
};

class Intersect {
public :
	Intersect();
	~Intersect();
	bool IsIntersect(osg::Vec3 start, osg::Vec3 direction, osg::Vec3 triangle0, osg::Vec3 triangle1, osg::Vec3 triangle2, osg::Vec3 &intersection);//射线和三角形相交。前两项是线段端点，后三项是三角网顶点
	void SetLines(osg::Node* node, osg::Matrixd &matParent);
	void SetLines(osg::ref_ptr<osg::Vec3Array> vertices, osg::View *view);
	IntersectRes LineLineIntersect1(osg::Vec3 point0, osg::Vec3 point1);
	int LineLineIntersectTwice(osg::Vec3 point0, osg::Vec3 point1);

	//判断两条线段在三维空间中是否相交，返回交点
	bool LineLineIntersect(osg::Vec3 point0, osg::Vec3 point1, osg::Vec3 point2, osg::Vec3 point3,osg::Vec3 &pointResult);
	//判断两条线段在三维空间中是否相交，不需要返回交点
	int LineLineIntersect(osg::Vec3 point0, osg::Vec3 point1, osg::Vec3 point2, osg::Vec3 point3);

	bool PtInPolygon(osg::Vec3 p, osg::Vec3Array *ptPolygon, bool start);
	bool PtInPolygonBE(osg::Vec3 p, osg::Vec3Array *ptPolygon, bool start);

	IntersectionRes intersect_CurveSgement(osg::Vec3 vertex0, osg::Vec3 vertex1, osg::ref_ptr<osg::Vec3Array> vertices, int pointIndex0, int pointIndex1);//判断线段和曲线是否有交点
	IntersectionRes intersect_CurveSgement(osg::Vec3 vertex0, osg::Vec3 vertex1, std::vector<Quadtree::QuadTreeNode *> quadnodeList, int pointIndex0, int pointIndex1); //判断线段和曲线(四叉树)是否有交点
	int intersect2D_Segments(osg::Vec3 p0, osg::Vec3 p1, osg::Vec3 p2, osg::Vec3 p3, osg::Vec3 &intersection, float &proportion);//二维相交判断
	osg::ref_ptr<osg::Geode> DrawLine(osg::Vec3 vec1, osg::Vec3 vec2);
	osg::ref_ptr<osg::Group> _group;
private:
	void GetAllLineSegment(osg::Geometry *pGeometry, osg::Matrixd &matParent);
	void GetAllLineSegment(osg::Node *pNode, osg::Matrixd &matParent);
	std::vector<Line> lines;
protected:
	osg::ref_ptr<osg::Vec3Array> verticeArray;
};

class AABB {
public:
	AABB();
	~AABB() {};
	void addPoint(osg::Vec3 & );
	bool contains(osg::Vec3 & p) const;
	bool intersectAABB(AABB &box);
	osg::Vec3 min;
	osg::Vec3 max;
};