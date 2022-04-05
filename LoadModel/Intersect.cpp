#include "Intersect.h"
#include <limits>
#define LINE_STEP 4 
using Common::Utility;
using namespace Quadtree;

Intersect::Intersect() {
	_group = new osg::Group();
}

Intersect::~Intersect() {

}

bool Intersect::IsIntersect(osg::Vec3 start, osg::Vec3 direction, osg::Vec3 triangle0, osg::Vec3 triangle1, osg::Vec3 triangle2, osg::Vec3 &intersection) {
	const float epsilon = 0.000001f;
	osg::Vec3 e1, e2, p, s, q;
	float t, u, v, tmp;
	e1 = triangle1 - triangle0;
	e2 = triangle2 - triangle0;
	p = direction ^ e2;
	tmp = p * e1;
	if (tmp > -epsilon && tmp < epsilon) {
		return false;
	}
	tmp = 1.0f / tmp;
	s = start - triangle0;
	u = tmp *( s*p );
	if (u < 0.0 || u > 1.0)
		return false;
	q = s ^ e1;
	v = tmp * (direction*q);
	if (v < 0.0 || v > 1.0)
		return false;
	if (u + v > 1.0)
		return false;
	t = tmp * (e2*q);
	/*if (t < 0.0)
		return false;*/
	intersection = start + direction*t;
	return true;
}

void Intersect::SetLines(osg::Node* node, osg::Matrixd &matParent) {
	GetAllLineSegment(node, matParent);
}

void Intersect::GetAllLineSegment(osg::Geometry *pGeometry, osg::Matrixd &matParent) {
	if (pGeometry == NULL) {
		return;
	}
	osg::ref_ptr<osg::Vec3Array> vertices = dynamic_cast<osg::Vec3Array*>(pGeometry->getVertexArray());
	//为了检测点是否在边界内,临时修改
	verticeArray = dynamic_cast<osg::Vec3Array*>(pGeometry->getVertexArray());

	if (vertices.valid()) {
		unsigned int uPriNum = pGeometry->getNumPrimitiveSets();
		for (unsigned int i = 0; i < uPriNum; i++) {
			const osg::PrimitiveSet *pPriSet = pGeometry->getPrimitiveSet(i);
			unsigned int NumIndices = pPriSet->getNumIndices();
			for (unsigned int j = 0; j < NumIndices; j=j+4) {
				unsigned index1 = pPriSet->index(j);
				unsigned index2;
				if (j + LINE_STEP >= NumIndices) {
					index2 = pPriSet->index(0);
				}
				else {
					index2 = pPriSet->index(j + LINE_STEP);
				}
				Line line;
				line.linePoint->push_back(vertices->at(index1)*matParent);
				line.linePoint->push_back(vertices->at(index2)*matParent);
				lines.push_back(line);
				//if(j==85*10|| j == 84 * 10)
				_group->addChild(DrawLine(line.linePoint->at(0), line.linePoint->at(1)));
			}
		}
	}
}

void Intersect::GetAllLineSegment(osg::Node *pNode, osg::Matrixd &matParent) {
	osg::Geode * pGeode = dynamic_cast<osg::Geode*>(pNode);
	if (pGeode) {
		unsigned int iDrawNum = pGeode->getNumDrawables();
		for (unsigned int i = 0; i < iDrawNum; i++) {
			osg::Drawable *pDrawble = pGeode->getDrawable(i);
			GetAllLineSegment(pDrawble->asGeometry(), matParent);
		}
	}
	else
	{
		osg::MatrixTransform *pMatrix = dynamic_cast<osg::MatrixTransform*>(pNode);
		if (pMatrix)
		{
			osg::Matrixd mat = pMatrix->getMatrix();
			osg::Matrixd matNow = mat;
			unsigned int uNum = pMatrix->getNumChildren();
			for (unsigned int i = 0; i < uNum; i++)
			{
				osg::Node *pChild = pMatrix->getChild(i);
				GetAllLineSegment(pChild, matParent);
			}
		}
		else
		{
			osg::Group *pGroup = dynamic_cast<osg::Group*>(pNode);
			if (pGroup)
			{
				unsigned int uNum = pGroup->getNumChildren();
				for (unsigned int i = 0; i < uNum; i++)
				{
					osg::Node *pChild = pGroup->getChild(i);
					GetAllLineSegment(pChild, matParent);
				}
			}
		}
	}
}

bool Intersect::LineLineIntersect(osg::Vec3 point0, osg::Vec3 point1, osg::Vec3 point2, osg::Vec3 point3, osg::Vec3 &pointResult) {
	const float epsilon = 0.000001f;
	osg::Vec3 direction1 = point1 - point0;
	osg::Vec3 direction2 = point3 - point2;
	osg::Vec3 e = point2 - point0;
	osg::Vec3 n = direction1 ^ direction2;
	float tmp = n * n;
	if (tmp > -epsilon && tmp < epsilon)
		return false;
	float u = 1 / tmp;
	float t1 = (e^direction2)*n*u;
	if (t1 < 0.0 || t1 > 1.0)
		return false;
	float t2 = (e^direction1)*n*u;
	if (t2 < 0.0 || t2 > 1.0)
		return false;
	osg::Vec3 intersect1 = point0 + direction1 * t1;
	osg::Vec3 intersect2 = point2 + direction2 * t2;
	osg::Vec3 difference = intersect1 - intersect2;
	float distance = difference * difference;
	if (distance >0.01) {
		return false;
	}
	pointResult = intersect2;
	return true;
}

int Intersect::LineLineIntersect(osg::Vec3 point0, osg::Vec3 point1, osg::Vec3 point2, osg::Vec3 point3) {
	if (point0 == point2 || point0 == point3 || point1 == point2 || point1 == point3)
		return 2;
	const float epsilon = 0.00000001f;
	const float distance_e = 0.000001f;
	osg::Vec3 direction1 = point1 - point0;
	osg::Vec3 direction2 = point3 - point2;
	osg::Vec3 e = point2 - point0;
	osg::Vec3 n = direction1 ^ direction2;
	float tmp = n * n;
	if (tmp > -epsilon && tmp < epsilon)
		return 0;
	float u = 1 / tmp;
	float t1 = (e^direction2)*n*u;
	if (t1 < 0.0 || t1 > 1.0)
		return 0;
	float t2 = (e^direction1)*n*u;
	if (t2 < 0.0 || t2 > 1.0)
		return 0;
	osg::Vec3 intersect1 = point0 + direction1 * t1;
	osg::Vec3 intersect2 = point2 + direction2 * t2;
	osg::Vec3 difference = intersect1 - intersect2;
	float distance = difference * difference;
	if (distance > distance_e) {
		return 0;
	}
	if (t2 == 0 || t1 == 0 || t2 == 1 || t1 == 1)
		return 2;
	return 1;
}

IntersectRes Intersect::LineLineIntersect1(osg::Vec3 point0, osg::Vec3 point1) {
	IntersectRes result;
	osg::Vec3 intersectPoint;
	for (int i = 0; i < lines.size(); i++) {
		if (LineLineIntersect(lines.at(i).linePoint->at(0), lines.at(i).linePoint->at(1), point0, point1, intersectPoint)) {
			result.isIntersect = true;
			result.intersectPoint = intersectPoint;
			result.lineIndex = i;
			return result;
		}
	}
	result.isIntersect = false;
	result.intersectPoint = osg::Vec3();
	result.lineIndex = -1;
	return result;
}

//作用：判断点是否在多边形内
//p指目标点， ptPolygon指多边形的点集合
bool Intersect::PtInPolygon(osg::Vec3 p, osg::Vec3Array *ptPolygon,bool start)
{
	// 交点个数  
	int nCross = start;
	//for (int i = 0,j= ptPolygon->size() - 1; i < ptPolygon->size(); j=i++)
	int i = 0; 
	int remainder = ptPolygon->size() % LINE_STEP;
	
	int j = remainder==0 ? (floor(ptPolygon->size() / LINE_STEP)-1)*LINE_STEP : floor(ptPolygon->size() / LINE_STEP)*LINE_STEP;
	for (; i < ptPolygon->size(); )
	{
		osg::Vec3 p1 = ptPolygon->at(i);
		osg::Vec3 p2 = ptPolygon->at(j);// 点P1与P2形成连线  

		// 求交点的x坐标（由直线两点式方程转化而来）   
		if ((p1._v[1] <= p._v[1] && p._v[1] < p2._v[1]) || (p2._v[1] <= p._v[1] && p._v[1] < p1._v[1])) {
			float x = (p._v[1] - p1._v[1]) * (p2._v[0] - p1._v[0]) / (p2._v[1] - p1._v[1]) + p1._v[0];

			// 只统计p1p2与p向右射线的交点  
			if (x == p._v[0]) {
				int aaa = 0;
			}
			if (x > p._v[0])
			{
				nCross = !nCross;
			}
		}
		j = i;
		i = i + LINE_STEP;
	}

	// 交点为偶数，点在多边形之外  
	// 交点为奇数，点在多边形之内
	return nCross;
}

//作用：判断点是否在多边形内
//p指目标点， ptPolygon指多边形的点集合
bool Intersect::PtInPolygonBE(osg::Vec3 p, osg::Vec3Array *ptPolygon, bool start)
{
	// 交点个数  
	int nCross = start;
	for (int i = 0,j= i+1; i < ptPolygon->size()-1; )
	{
		osg::Vec3 p1 = ptPolygon->at(i);
		osg::Vec3 p2 = ptPolygon->at(j);// 点P1与P2形成连线  

		// 求交点的x坐标（由直线两点式方程转化而来）   
		if ((p1._v[1] <= p._v[1] && p._v[1] < p2._v[1]) || (p2._v[1] <= p._v[1] && p._v[1] < p1._v[1])) {
			float x = (p._v[1] - p1._v[1]) * (p2._v[0] - p1._v[0]) / (p2._v[1] - p1._v[1]) + p1._v[0];

			// 只统计p1p2与p向右射线的交点  
			if (x == p._v[0]) {
				int aaa = 0;
			}
			if (x > p._v[0])
			{
				nCross = !nCross;
			}
		}
		i++;
		j = i + 1;
	}
	

	// 交点为偶数，点在多边形之外  
	// 交点为奇数，点在多边形之内
	return nCross;
}


osg::ref_ptr<osg::Geode> Intersect::DrawLine(osg::Vec3 vec1, osg::Vec3 vec2) {
	osg::Geode* geodeLine = new osg::Geode();
	osg::Geometry* geometryLine = new osg::Geometry();
	osg::Vec3Array * coordsLine = new osg::Vec3Array();
	coordsLine->push_back(vec1);
	coordsLine->push_back(vec2);
	geometryLine->setVertexArray(coordsLine);
	geometryLine->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 2));
	geodeLine->addDrawable(geometryLine);
	return geodeLine;
}

int Intersect::LineLineIntersectTwice(osg::Vec3 point0, osg::Vec3 point1) {
	int index = 0;
	osg::Vec3 intersectPoint;
	for (int i = 0; i < lines.size(); i++) {
		if (LineLineIntersect(lines.at(i).linePoint->at(0), lines.at(i).linePoint->at(1), point0, point1, intersectPoint)) {
			index++;
		}
	}
	return index;
}

void Intersect::SetLines(osg::ref_ptr<osg::Vec3Array> vertices, osg::View * view) {
	if (vertices.valid()) {
		unsigned NumIndices = vertices->size();
		for (unsigned int j = 0; j < NumIndices; j = j + LINE_STEP) {
			osg::Vec3 point0 = vertices->at(j);
			osg::Vec3 point1;
			if (j + LINE_STEP >= NumIndices) {
				point1 = vertices->at(0);
			}
			else {
				point1 = vertices->at(j + LINE_STEP);
			}
			_group->addChild(DrawLine(Utility::windowToWorld(point0,view), Utility::windowToWorld(point1, view)));
		}
	}
}

IntersectionRes Intersect::intersect_CurveSgement(osg::Vec3 vertex0, osg::Vec3 vertex1,osg::ref_ptr<osg::Vec3Array> vertices,int pointIndex0, int pointIndex1) {
	IntersectionRes intersectionRes;
	osg::Vec3 temp;
	int num = 0;
	float proportion;
	if (vertices.valid()) {
		unsigned NumIndices = vertices->size();
		for (unsigned int j = 0; j < NumIndices; j = j + LINE_STEP) {
			osg::Vec3 point0 = vertices->at(j);
			osg::Vec3 point1;
			if (j + LINE_STEP >= NumIndices) {
				point1 = vertices->at(0);
			}
			else {
				point1 = vertices->at(j + LINE_STEP);
			}
			if (intersect2D_Segments(vertex0, vertex1, point0, point1, temp, proportion)) {
				num++;
				intersectionRes.proportions.push_back(proportion);
				intersectionRes.lineIndex.push_back(j);
			}
		}	
	}
	intersectionRes.pointNum = num;
	intersectionRes.indices[0]= pointIndex0;
	intersectionRes.indices[1] = pointIndex1;
	return intersectionRes;
}

//四叉树判断线段与闭合曲线相交
IntersectionRes Intersect::intersect_CurveSgement(osg::Vec3 vertex0, osg::Vec3 vertex1, std::vector<Quadtree::QuadTreeNode *> quadnodeList, int pointIndex0, int pointIndex1) {
	IntersectionRes intersectionRes;
	osg::Vec3 temp;
	int num = 0;
	float proportion;


	for (int i = 0; i < quadnodeList.size(); i++) {
		if (quadnodeList[i]->topologic(vertex0, vertex1)) {
			if (quadnodeList[i]->_lines.size() > 0) {
				unsigned NumIndices = quadnodeList[i]->_lines.size();
				for (unsigned int j = 0; j < NumIndices; j++) {
					osg::Vec3 point0 = quadnodeList[i]->_lines[j].linePoint->at(0);
					osg::Vec3 point1 = quadnodeList[i]->_lines[j].linePoint->at(1);
					if (intersect2D_Segments(vertex0, vertex1, point0, point1, temp, proportion)) {
						num++;
						intersectionRes.proportions.push_back(proportion);
						intersectionRes.lineIndex.push_back(quadnodeList[i]->_lines[j].lineIndex);
					}
				}
			}
		}
	}

	
	intersectionRes.pointNum = num;
	intersectionRes.indices[0] = pointIndex0;
	intersectionRes.indices[1] = pointIndex1;
	return intersectionRes;
}

//二维相交判断
int Intersect::intersect2D_Segments(osg::Vec3 p0, osg::Vec3 p1, osg::Vec3 p2, osg::Vec3 p3, osg::Vec3 &intersection,float &proportion) {
	const float epsilon = 0.000001f;
	osg::Vec3 direction0 = p1 - p0;
	osg::Vec3 direction1 = p3 - p2;
	osg::Vec3 subtract = p2 - p0;
	float kross = direction0.x()*direction1.y() - direction0.y()*direction1.x();
	float sqrKross = kross * kross;
	float sqrLen0 = direction0.x()*direction0.x() + direction0.y()*direction0.y();
	float sqrLen1 = direction1.x()*direction1.x() + direction1.y()*direction1.y();
	if (sqrKross > epsilon*sqrLen0*sqrLen1) {
		float s = (subtract.x()*direction1.y() - subtract.y()*direction1.x()) / kross;
		if (s < 0 || s>1) {
			//intersection of lines is not a point on segment "p1-p0"
			return 0;
		}
		float t = (subtract.x()*direction0.y() - subtract.y()*direction0.x()) / kross;
		if (t < 0 || t>1) {
			//intersection of lines is not a point on segment "p3-p2"
			return 0;
		}
		intersection.set(p0.x() + s * direction0.x(), p0.y() + s * direction0.y(), p0.z()+s*(p0.z()-p1._v[2]));
		proportion = s;
		return 1;
	}

	float sqrLenS = subtract.x()*subtract.x() + subtract.y()*subtract.y();
	kross = subtract.x()*direction0.y() - subtract.y()*direction0.x();
	sqrKross = kross * kross;
	if (sqrKross > epsilon*sqrLen0*sqrLenS) {
		//lines of the segments are different
		return 0;
	}
	//lines of the segments are same
	return 2;
}



/** AABB包围盒
	主要是在投影坐标判断点是否在边界内中。
	先用包围盒大致判断。
**/
AABB::AABB() {
	const float infinity = std::numeric_limits<float>::infinity();
	max.set(-infinity, -infinity, -infinity);
	min.set(infinity, infinity, infinity);
}

void AABB::addPoint(osg::Vec3 & p) {
	if (p._v[0] < min._v[0]) min._v[0] = p._v[0];
	if (p._v[0] > max._v[0]) max._v[0] = p._v[0];
	if (p._v[1] < min._v[1]) min._v[1] = p._v[1];
	if (p._v[1] > max._v[1]) max._v[1] = p._v[1];
	if (p._v[2] < min._v[2]) min._v[2] = p._v[2];
	if (p._v[2] > max._v[2]) max._v[2] = p._v[2];
}

bool AABB::contains(osg::Vec3 & p) const {
	return (p._v[0] >= min._v[0]) && (p._v[0] <= max._v[0]) && (p._v[1] >= min._v[1]) &&
		(p._v[1] <= max._v[1]) ;
} //const 表示在该函数内，所有类成员是const变量，不能改变

bool AABB::intersectAABB(AABB &box) {
	//判断是否又重叠
	if (this->min._v[0] > box.max._v[0]) return false;
	if (this->max._v[0] < box.min._v[0]) return false;
	if (this->min._v[1] > box.max._v[1]) return false;
	if (this->max._v[1] < box.min._v[1]) return false;
	if (this->min._v[2] > box.max._v[2]) return false;
	if (this->max._v[2] < box.min._v[2]) return false;
	return true;
}