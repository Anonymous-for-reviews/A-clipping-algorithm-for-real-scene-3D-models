#include "Quadtree.h"
#include "../Intersect.h"

using namespace Quadtree;

QuadTreeNode::QuadTreeNode(Region region, int depth, int maxdepth, QuadTreeNode *parent) :
	_depth(depth), _maxDepth(maxdepth), _region(region) {
	_parent = parent;
	UpRight = nullptr;
	UpLeft = nullptr;
	LLeft = nullptr;
	LRight = nullptr;
	_vertices = new osg::Vec3Array;
}

QuadTreeNode::~QuadTreeNode() {

}

void QuadTreeNode::SetVertices(osg::ref_ptr<osg::Vec3Array> vertices) {
	_vertices = vertices;
}

void QuadTreeNode::SetDepth(int depth) {
	_depth = depth;
}

bool QuadTreeNode::isContain(osg::Vec3 & p) {
	return (p._v[0] >= _region.minX) && (p._v[0] <= _region.maxX) && (p._v[1] >= _region.minY) &&
		(p._v[1] <= _region.maxY);
}

void QuadTreeNode::addPoint(osg::Vec3 & p) {
	_vertices->push_back(p);
}

void QuadTreeNode::addLine(Line & line) {
	_lines.push_back(line);
}

bool QuadTreeNode::topologic(osg::Vec3 point1, osg::Vec3 point2) {
	/*bool isContainPoint1 = isContain(line.linePoint->at(0));
	bool isContainPoint2 = isContain(line.linePoint->at(1));
	if (isContainPoint1 && isContainPoint2) return TopoLogic::Contain;
	if (isContainPoint1 == !isContainPoint2) return TopoLogic::Adjacent;
	
	Intersect inter;*/

	double minX = point1.x();
	double maxX = point2.x();

	if (point1.x() > point2.x())
	{
		minX = point2.x();
		maxX = point1.x();
	}

	// Find the intersection of the segment's and rectangle's x-projections
	if (maxX > _region.maxX)
	{
		maxX = _region.maxX;
	}
	if (minX < _region.minX)
	{
		minX = _region.minX;
	}
	if (minX > maxX) {
		return false;
	}

	// Find corresponding min and max Y for min and max X we found before
	double minY = point1.y();
	double maxY = point2.y();

	double dx = point2.x() - point1.x();
	if (abs(dx) > 0.0000001)
	{
		double a = (point2.y() - point1.y()) / dx;
		double b = point1.y() - a * point1.x();
		minY = a * minX + b;
		maxY = a * maxX + b;
	}

	if (minY > maxY)
	{
		double tmp = maxY;
		maxY = minY;
		minY = tmp;
	}

	// Find the intersection of the segment's and rectangle's y-projections
	if (maxY > _region.maxY)
	{
		maxY = _region.maxY;
	}

	if (minY < _region.minY)
	{
		minY = _region.minY;
	}

	if (minY > maxY) // If Y-projections do not intersect return false
	{
		return false;
	}

	return true;
}

void QuadTreeNode::LineNumMatch(std::vector<QuadTreeNode *> & quadnodeList) {
	if (_lines.size() > 0) quadnodeList.push_back(this);
	if (is_leaf != 1) {
		UpRight->LineNumMatch(quadnodeList);
		UpLeft->LineNumMatch(quadnodeList);
		LLeft->LineNumMatch(quadnodeList);
		LRight->LineNumMatch(quadnodeList);
	}
}

QuadTree Quadtree::QuadTreeBuild(int depth, int maxDepth, Region region ) {
	QuadTree quadTree;
	//QuadTreeNode * root;
 	QuadTreeNode* quadRoot = CreateQuadBranch(nullptr, depth, maxDepth, region);
	quadTree.depth = maxDepth;
	quadTree.root = quadRoot;
	return quadTree;
}

std::vector<QuadTreeNode *> Quadtree::lineDivide(QuadTreeNode* quadRoot, std::vector<Line> lines) {
	for (int i = 0; i < lines.size(); i++) {
		QuadTreeNode * temp = quadRoot;
		int depth = quadRoot->_depth;
		int maxDepth = quadRoot->_maxDepth;
		while (depth < maxDepth-1) {
			if (temp->UpRight->isContain(lines[i].linePoint->at(0)) && temp->UpRight->isContain(lines[i].linePoint->at(1))) {
				if(temp->UpRight->is_leaf==1)
					temp->UpRight->addLine(lines[i]);
				else temp = temp->UpRight;
			}
			else if (temp->UpLeft->isContain(lines[i].linePoint->at(0)) && temp->UpLeft->isContain(lines[i].linePoint->at(1))) {
				if (temp->UpLeft->is_leaf == 1) 
					temp->UpLeft->addLine(lines[i]);
				else temp = temp->UpLeft;
			}
			else if (temp->LLeft->isContain(lines[i].linePoint->at(0)) && temp->LLeft->isContain(lines[i].linePoint->at(1))) {
				if (temp->LLeft->is_leaf == 1) 
					temp->LLeft->addLine(lines[i]);
				else temp = temp->LLeft;
			}
			else if (temp->LRight->isContain(lines[i].linePoint->at(0)) && temp->LRight->isContain(lines[i].linePoint->at(1))) {
				if (temp->LRight->is_leaf == 1) 
					temp->LRight->addLine(lines[i]);
				else temp = temp->LRight;
			}
			else {
				temp->addLine(lines[i]);
				break;
			}
			depth++;
		}
	}

	std::vector<QuadTreeNode *> quadnodeList;
	int depth = quadRoot->_depth;
	int maxDepth = quadRoot->_maxDepth;
	quadRoot->LineNumMatch(quadnodeList);
	int count = 0;
	for (int i = 0; i < quadnodeList.size(); i++) {
		count = count + quadnodeList[i]->_lines.size();
	}
	return quadnodeList;
}



QuadTreeNode * Quadtree::CreateQuadBranch(QuadTreeNode * parent,int depth, int maxDepth, Region region) {
	if (depth < maxDepth-1) {
		QuadTreeNode* quadNode = new QuadTreeNode(region, depth, maxDepth, parent);
		std::vector<Region> quadRegion = QuadRegion(region);
		quadNode->UpRight = CreateQuadBranch(quadNode,depth+1,maxDepth,quadRegion[0]);
		quadNode->UpLeft = CreateQuadBranch(quadNode, depth + 1, maxDepth, quadRegion[1]);
		quadNode->LLeft = CreateQuadBranch(quadNode, depth + 1, maxDepth, quadRegion[2]);
		quadNode->LRight = CreateQuadBranch(quadNode, depth + 1, maxDepth, quadRegion[3]);
		return quadNode;
	}
	else {
		QuadTreeNode* quadNode = new QuadTreeNode(region, depth, maxDepth, parent);
		std::vector<Region> quadRegion = QuadRegion(region);
		quadNode->is_leaf = 1;
		return quadNode;
	}

	
}

std::vector<Region> Quadtree::QuadRegion(Region origin) {
	float splitWidth = (origin.maxX - origin.minX) / 2;
	float splitHeight = (origin.maxY - origin.minY) / 2;
	
	std::vector<Region> quadRegions;

	Region UpRight;
	UpRight.maxX = origin.maxX;
	UpRight.maxY = origin.maxY;
	UpRight.minX = origin.minX + splitWidth;
	UpRight.minY = origin.minY + splitHeight;
	quadRegions.push_back(UpRight);

	Region UpLeft;
	UpLeft.maxX = origin.minX + splitWidth;
	UpLeft.maxY = origin.maxY;
	UpLeft.minX = origin.minX ;
	UpLeft.minY = origin.minY + splitHeight;
	quadRegions.push_back(UpLeft);

	Region LLeft;
	LLeft.maxX = origin.minX + splitWidth;
	LLeft.maxY = origin.minY + splitHeight;
	LLeft.minX = origin.minX;
	LLeft.minY = origin.minY ;
	quadRegions.push_back(LLeft);

	Region LRight;
	LRight.maxX = origin.maxX;
	LRight.maxY = origin.minY + splitHeight;
	LRight.minX = origin.minX + splitWidth;
	LRight.minY = origin.minY;
	quadRegions.push_back(LRight);

	return quadRegions;
}

