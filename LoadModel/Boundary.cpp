#include "Boundary.h"
#define LINE_STEP 4 
#define CRITICAL_NUM  4
using namespace BOUNDARY;

Boundary::Boundary() {}

Boundary::~Boundary() {

}
Boundary::Boundary(osg::Node* Line, osg::View *view) {
	windowArray = Common::Utility::worldToWindowArray(Line, view);
	maxVertex = windowArray->at(0);
	minVertex = windowArray->at(0);
	osg::Vec3Array::iterator iter;
	for (iter = windowArray->begin();
		iter != windowArray->end();) {
		if (maxVertex.y() < (*iter).y()) {
			maxVertex = *iter;
		}
		if (minVertex.y() > (*iter).y()) {
			minVertex = *iter;
		}
		boundingBox.addPoint(*iter);
		iter++;
	}
	float step = (maxVertex.y() - minVertex.y()) / CRITICAL_NUM;

	for (int i = 0; i < CRITICAL_NUM; i++) {
		vector<float> temp(2);
		temp[0] = minVertex.y() + step * i;
		temp[1] = minVertex.y() + step * (i + 1);
		_critical.push_back(temp);
	}

	int level = judgeLevel(windowArray->at(0).y());
	int currentLevel = level;
	osg::ref_ptr<osg::Vec3Array>  vec3Array = new osg::Vec3Array;
	_levelsLine.resize(CRITICAL_NUM);

	for (int i = 0; i < windowArray->size();) {
		int nextIndex = (i + LINE_STEP) >= windowArray->size() ? 0 : i + LINE_STEP;
		int level = judgeLevel(windowArray->at(i).y());
		int nextLevel = judgeLevel(windowArray->at(nextIndex).y());
		
		if (level == nextLevel) {
			currentLevel = level;
			vec3Array->push_back(windowArray->at(i));
		}
		else {
			vec3Array->push_back(windowArray->at(i));
			vec3Array->push_back(windowArray->at(nextIndex));
			_levelsLine[currentLevel].push_back(vec3Array);
			vec3Array = new osg::Vec3Array;
			vec3Array->push_back(windowArray->at(i));
			currentLevel = nextLevel;
		}

		if (nextIndex == 0) {
			i = windowArray->size();
			if (level == nextLevel)
				vec3Array->push_back(windowArray->at(nextIndex));
			if(vec3Array->size()!=0)
				_levelsLine[currentLevel].push_back(vec3Array);
		}
		else {
			i = nextIndex;
		}
		
	}
}

bool Boundary::pointTopo(osg::Vec3 vertex) {		
	int level = judgeLevel(vertex.y());
	if (level == -1) return false;
	bool result = false;
	Intersect inter;
	for (int i = 0; i < _levelsLine[level].size(); i++) {
		result = inter.PtInPolygonBE(vertex, _levelsLine[level][i], result);
	}
	return result;
}



int Boundary::judgeLevel(float y) {
	for (int i = 0; i < _critical.size(); i++) {
		if (y >= _critical[i][0] && (y < _critical[i][1] || y == _critical[_critical.size() - 1][1]))
			return i;
	}
	return -1;
}

/** AABB包围盒
	主要是在投影坐标判断点是否在边界内中。
	先用包围盒大致判断。
**/
BOUNDARY::AABB::AABB() {
	const float infinity = std::numeric_limits<float>::infinity();
	max.set(-infinity, -infinity, -infinity);
	min.set(infinity, infinity, infinity);
}

void BOUNDARY::AABB::addPoint(osg::Vec3 & p) {
	if (p._v[0] < min._v[0]) min._v[0] = p._v[0];
	if (p._v[0] > max._v[0]) max._v[0] = p._v[0];
	if (p._v[1] < min._v[1]) min._v[1] = p._v[1];
	if (p._v[1] > max._v[1]) max._v[1] = p._v[1];
	if (p._v[2] < min._v[2]) min._v[2] = p._v[2];
	if (p._v[2] > max._v[2]) max._v[2] = p._v[2];
}

bool BOUNDARY::AABB::contains(osg::Vec3 & p) const {
	return (p._v[0] >= min._v[0]) && (p._v[0] <= max._v[0]) && (p._v[1] >= min._v[1]) &&
		(p._v[1] <= max._v[1]);
} //const 表示在该函数内，所有类成员是const变量，不能改变

bool BOUNDARY::AABB::intersectAABB(AABB &box) {
	//判断是否又重叠
	if (this->min._v[0] > box.max._v[0]) return false;
	if (this->max._v[0] < box.min._v[0]) return false;
	if (this->min._v[1] > box.max._v[1]) return false;
	if (this->max._v[1] < box.min._v[1]) return false;
	if (this->min._v[2] > box.max._v[2]) return false;
	if (this->max._v[2] < box.min._v[2]) return false;
	return true;
}