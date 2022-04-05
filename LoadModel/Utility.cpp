#include "stdafx.h"
#include "Utility.h"
#include <iostream>

#define EXTREMELY_SMALL 0.00000001 
using namespace Common;

Utility::Utility()
{
}


Utility::~Utility()
{
}

/**
 * 比较两个三维向量是否相同
 */
bool Utility::isVec3Same(const osg::Vec3 &v1, const osg::Vec3 &v2)
{
	return (v1._v[0] == v2._v[0]) && (v1._v[1] == v2._v[1]) && (v1._v[2] == v2._v[2]);
}

/**
 * 用一个字符替换原字符中的另一个字符
 */
void Utility::string_replace(std::string& strBig, const std::string& strsrc, const std::string& strdst)
{
	std::string::size_type pos = 0;
	std::string::size_type srclen = strsrc.size();
	std::string::size_type dstlen = strdst.size();

	while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}

/**
 * 从路径中获取文件名（不包括后缀名）
 */
string Utility::getFileNameFromPath(string path)
{
	if (path.empty())
	{
		return "";
	}
	string_replace(path, "/", "\\");
	std::string::size_type iPos = path.find_last_of('\\') + 1;
	std::string::size_type dPos = path.find_last_of('.') + 1;
	if (dPos == 0)
		dPos = path.length();
	return path.substr(iPos, dPos - iPos - 1);
}

//世界坐标转屏幕坐标
osg::ref_ptr<osg::Vec3Array> Utility::worldToWindowArray(osg::Node *node, osg::View * view) {
	osg::ref_ptr<osg::Vec3Array> winVertices = new osg::Vec3Array;
	osg::Camera *camera = view->getCamera();
	osg::Viewport *viewport = camera->getViewport();
	osg::Matrixd VPW = camera->getViewMatrix() *
		camera->getProjectionMatrix() *
		camera->getViewport()->computeWindowMatrix();
	osg::Group * pGroup = dynamic_cast<osg::Group*>(node);
	if (pGroup)
	{
		unsigned int uNum = pGroup->getNumChildren();
		for (unsigned int i = 0; i < uNum; i++)
		{
			osg::Node *pChild = pGroup->getChild(i);
			osg::Geode * pGeode = dynamic_cast<osg::Geode*>(pChild);
			if (pGeode) {
				unsigned int iDrawNum = pGeode->getNumDrawables();
				for (unsigned int i = 0; i < iDrawNum; i++) {
					osg::Drawable *pDrawble = pGeode->getDrawable(i);
					osg::Geometry *pGeometry = pDrawble->asGeometry();
					osg::ref_ptr<osg::Vec3Array> vertices = dynamic_cast<osg::Vec3Array*>(pGeometry->getVertexArray());
					if (vertices.valid()) {
						for (int i = 0; i < vertices->size(); i++) {
							osg::Vec3 temp = vertices->at(i);
							winVertices->push_back(temp*VPW);
						}
					}
				}
			}
		}
	}
	return winVertices;
}

osg::Vec3 Utility::worldToWindow(osg::Vec3 p, osg::View * view) {
	osg::Vec3 point;
	osg::Camera *camera = view->getCamera();
	osg::Matrixd VPW = camera->getViewMatrix() *
		camera->getProjectionMatrix() *
		camera->getViewport()->computeWindowMatrix();
	point = p * VPW;
	return point;
}

osg::Vec3 Utility::windowToWorld(osg::Vec3 p, osg::View * view) {
	osg::Vec3 point;
	osg::Camera *camera = view->getCamera();
	osg::Matrixd VPW = camera->getViewMatrix() *
		camera->getProjectionMatrix() *
		camera->getViewport()->computeWindowMatrix();
	osg::Matrix inverseVPW = osg::Matrix::inverse(VPW);
	point = p * inverseVPW;
	return point;
}


int Utility::customizePlus(int value) {
	int result = value + 1;
	if (result == 3)
		result = 0;
	return result;
}

int Utility::customizePlus(int value, int addend) {
	int result = value + addend;
	if (result >= 3)
		result = result % 3;
	return result;
}

bool Utility::is_element_in_vector(vector<int> v, int element) {
	vector<int>::iterator it;
	it = find(v.begin(), v.end(), element);
	if (it != v.end()) {
		return true;
	}
	else {
		return false;
	}
}

bool Utility::CustomizeSmall(int a, int b, int Count) {
	if (a < b && (b - a) < Count / 2)
		return true;
	if (a > b && (a - b) > Count / 2)
		return true;
	return false;
}

//Are the three points collinear
bool Utility::isCollinear(osg::Vec3 a, osg::Vec3 b, osg::Vec3 c) {
	osg::Vec3 ab = a - b;
	osg::Vec3 ac = a - c;
	osg::Vec3 bc = b - c;
	float edge_ab = ab.length();
	float edge_ac = ac.length();
	float edge_bc = bc.length();
	float p = 0.5*(edge_ab + edge_ac + edge_bc);
	float result = p * (p - edge_ab)*(p - edge_ac)*(p - edge_bc);
	if (result> EXTREMELY_SMALL) return false;
	return true;
}

vector<osg::Vec3> Utility::setLines(osg::Node *node) {
	vector<osg::Vec3> lineVertices;
	osg::Group * pGroup = dynamic_cast<osg::Group*>(node);
	if (pGroup)
	{
		unsigned int uNum = pGroup->getNumChildren();
		for (unsigned int i = 0; i < uNum; i++)
		{
			osg::Node *pChild = pGroup->getChild(i);
			osg::Geode * pGeode = dynamic_cast<osg::Geode*>(pChild);
			if (pGeode) {
				unsigned int iDrawNum = pGeode->getNumDrawables();
				for (unsigned int i = 0; i < iDrawNum; i++) {
					osg::Drawable *pDrawble = pGeode->getDrawable(i);
					osg::Geometry *pGeometry = pDrawble->asGeometry();
					osg::ref_ptr<osg::Vec3Array> vertices = dynamic_cast<osg::Vec3Array*>(pGeometry->getVertexArray());
					if (vertices.valid()) {
						for (int i = 0; i < vertices->size(); i++) {
							lineVertices.push_back(vertices->at(i));
						}
					}
				}
			}
		}
	}
	return lineVertices;
}