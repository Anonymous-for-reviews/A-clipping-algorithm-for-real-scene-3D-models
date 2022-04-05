#include "MainVisiter.h"
#include "TriangleIndex.h"
#include "ModelAttributeFunctor.h"
#include "Side.h"
#include <osgFX/Outline>
#include <osg/ComputeBoundsVisitor>
#include <osgUtil/DelaunayTriangulator>
#include "DelaunayTriangulator.h"
#include "Utility.h"
using Common::Utility;
using namespace std;
//极小值，趋近于0的判断
#define EXTREMELY_SMALL 0.001 
#define LINE_STEP 4 

osg::ref_ptr<osg::Geode> DrawPoint(osg::Vec3 vec1) {
	osg::Geode *geode = new osg::Geode;
	geode->addDrawable(new osg::ShapeDrawable(new osg::Box(vec1, 0.1)));
	//声明Effect节点并把Geode添加到其中
	osgFX::Outline *outline = new osgFX::Outline;
	outline->addChild(geode);
	outline->setColor(osg::Vec4(1.0, 1.0, 0.0, 1.0));
	outline->setWidth(5.0f);
	return geode;

}

void MainVisitor::apply(osg::Node& node) {

	_indent++;
	traverse(node);
	_indent--;
}

void MainVisitor::apply(osg::Geode& node) {

	/*for (unsigned int n = 0; n < node.getNumDrawables(); ++n) {
		osg::ref_ptr<osg::Drawable> drawable = node.getDrawable(n);
		ModelAttributeFunctor functor;
		drawable->accept(functor);
		osg::TriangleIndexFunctor<TriangleIndex> triangleIndex;
		drawable->accept(triangleIndex);
		dealTriangleInfo(functor, triangleIndex);
	}*/
	_indent++;
	traverse(node);
	_indent--;
}

void MainVisitor::apply(osg::Geometry& geomrtry) {
	osg::ref_ptr<osg::Geometry> geom = &geomrtry;
	ModelAttributeFunctor functor;
	geom->accept(functor);
	osg::TriangleIndexFunctor<TriangleIndex> triangleIndex;
	geom->accept(triangleIndex);
	dealTriangleInfo(functor, triangleIndex);
}

osg::Geometry::PrimitiveSetList MainVisitor::primitiveSetList = {};
osg::Vec3Array* MainVisitor::vertArray = new osg::Vec3Array();

MainVisitor::~MainVisitor(){
	for (Geom* geom : allGeom)
	{
		delete geom;
	}
}

//处理访问器得到的信息，构建三角形关系
void MainVisitor::dealTriangleInfo(ModelAttributeFunctor attributeFunctor, osg::TriangleIndexFunctor<TriangleIndex> indexFunctor) {
	Geom *geom = new Geom;
	if (attributeFunctor.textCoordList->size() != 0
		&& attributeFunctor.textCoordList->size() != attributeFunctor.vertexList->size())
	{
		cout << "纹理坐标和顶点数量不匹配" << endl;
		return;
	}
	//处理顶点信息
	for (size_t i = 0; i < attributeFunctor.vertexList->size(); i++)
	{
		Vertex* vertex = new Vertex;
		vertex->coor = attributeFunctor.vertexList->at(i);
		vertex->index = i;
		if (i < attributeFunctor.normalList->size())
			vertex->normal = attributeFunctor.normalList->at(i);
		if (i < attributeFunctor.textCoordList->size())
			vertex->texCoor = attributeFunctor.textCoordList->at(i);
		geom->vertices.push_back(vertex);
	}
	//处理三角形信息
	for (int i = 0; i < indexFunctor.triangleNum; i++)
	{
		Triangle* triangle = new Triangle;
		triangle->index = i;
		triangle->vertexIndexs[0] = indexFunctor.indexs->at(i * 3);
		triangle->vertexIndexs[1] = indexFunctor.indexs->at(i * 3 + 1);
		triangle->vertexIndexs[2] = indexFunctor.indexs->at(i * 3 + 2);
		//计算法向量
		osg::Vec3 edge1 = geom->vertices.at(triangle->vertexIndexs[1])->coor - geom->vertices.at(triangle->vertexIndexs[0])->coor;
		osg::Vec3 edge2 = geom->vertices.at(triangle->vertexIndexs[2])->coor - geom->vertices.at(triangle->vertexIndexs[0])->coor;
		osg::Vec3 triangleNormal = edge1 ^ edge2;
		triangleNormal.normalize();
		triangle->normal = triangleNormal;
		geom->triangles.push_back(triangle);
	}
	allGeom.push_back(geom);
}

//根据指定的颜色，将geom中的数据创建成osg节点
osg::ref_ptr<osg::Node> MainVisitor::createOsgNode(osg::Vec4 color, int order)
{
	this->geomColor = color;
	short direction = order % 4;
	osg::ref_ptr<osg::Group> result = new osg::Group;
	if (allGeom.size() > 0 && allGeom.size() == 1)
	{
		osg::ref_ptr<osg::Geode> geode = allGeom[0]->createOsgNode(color);
		this->boundingBox = geode->getBoundingBox();
		return geode;
	}
	else
	{
		for (Geom* geom : allGeom)
			result->addChild(geom->createOsgNode(color));
		osg::ComputeBoundsVisitor boundsVisitor;
		result->accept(boundsVisitor);
		this->boundingBox = boundsVisitor.getBoundingBox();
	}
	return result;
}

void MainVisitor::createTriangleTopo()
{
	
	if (allGeom.size() > 0 && allGeom.size() == 1)
	{
		allGeom[0]->createTriangleTopo();
	}
	else
	{
		for (Geom* geom : allGeom)
			geom->createTriangleTopo();
	}
}

//画三角网
osg::ref_ptr<osg::Geode> MainVisitor::DrawTriangle(osg::Vec3 vec1, osg::Vec3 vec2,osg::Vec3 vec3,osg::Vec4 color) {
	osg::Geode* geodeLine = new osg::Geode();
	osg::Geometry* geometryLine = new osg::Geometry();
	osg::Vec3Array * coordsLine = new osg::Vec3Array();
	coordsLine->push_back(vec1);
	coordsLine->push_back(vec2);
	coordsLine->push_back(vec3);
	geometryLine->setVertexArray(coordsLine);
	osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array();
	vc->push_back(color);
	geometryLine->setColorArray(vc.get());
	geometryLine->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, 3));
	osg::ref_ptr<osg::Material> material = new osg::Material;
	material->setAmbient(osg::Material::FRONT_AND_BACK, color);
	material->setDiffuse(osg::Material::FRONT_AND_BACK, color);
	material->setColorMode(osg::Material::AMBIENT);
	geometryLine->getOrCreateStateSet()->setAttributeAndModes(material.get(), osg::StateAttribute::ON);
	geodeLine->addDrawable(geometryLine);
	return geodeLine;
}

osg::ref_ptr<osg::Geode> MainVisitor::DrawLine(osg::Vec3 vec1, osg::Vec3 vec2, osg::Vec4 color) {
	osg::Geode* geodeLine = new osg::Geode();
	osg::Geometry* geometryLine = new osg::Geometry();
	osg::Vec3Array * coordsLine = new osg::Vec3Array();
	coordsLine->push_back(vec1);
	coordsLine->push_back(vec2);
	geometryLine->setVertexArray(coordsLine);
	osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array();
	vc->push_back(color);
	geometryLine->setColorArray(vc.get());
	geometryLine->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, 2));
	osg::ref_ptr<osg::Material> material = new osg::Material;
	material->setAmbient(osg::Material::FRONT_AND_BACK, color);
	material->setDiffuse(osg::Material::FRONT_AND_BACK, color);
	material->setColorMode(osg::Material::AMBIENT);
	geometryLine->getOrCreateStateSet()->setAttributeAndModes(material.get(), osg::StateAttribute::ON);
	geodeLine->addDrawable(geometryLine);
	return geodeLine;
}

osg::ref_ptr<osg::Geode> MainVisitor::DrawTriangle(osg::Vec3 vec1, osg::Vec3 vec2, osg::Vec3 vec3, osg::Vec4 color, float depthValue) {
	osg::Geode* geodeLine = new osg::Geode();
	osg::Geometry* geometryLine = new osg::Geometry();
	osg::Vec3Array * coordsLine = new osg::Vec3Array();
	coordsLine->push_back(vec1);
	coordsLine->push_back(vec2);
	coordsLine->push_back(vec3);
	geometryLine->setVertexArray(coordsLine);
	osg::ref_ptr<osg::Vec4Array> vc = new osg::Vec4Array();
	vc->push_back(color);
	geometryLine->setColorArray(vc.get());
	geometryLine->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, 3));
	osg::ref_ptr<osg::Depth> depth = new osg::Depth();
	depth->setRange(0, depthValue);
	geometryLine->getOrCreateStateSet()->setAttributeAndModes(depth, osg::StateAttribute::ON);
	osg::ref_ptr<osg::Material> material = new osg::Material;
	material->setAmbient(osg::Material::FRONT_AND_BACK, color);
	material->setDiffuse(osg::Material::FRONT_AND_BACK, color);
	material->setColorMode(osg::Material::AMBIENT);
	geometryLine->getOrCreateStateSet()->setAttributeAndModes(material.get(), osg::StateAttribute::ON);
	geodeLine->addDrawable(geometryLine);
	return geodeLine;
}

//判断投影面中，点是否在区域内
vector<int> MainVisitor::triangleInterPoly(Geom* geom, int triangleIndexs, osg::ref_ptr<osg::Vec3Array> ptPolygon, osg::View *view) {
	Intersect inter;
	vector<int> points;
	for (int i = 0; i < 3; i++) {
		osg::Vec3 point0Win = Utility::worldToWindow(geom->vertices.at(geom->triangles.at(triangleIndexs)->vertexIndexs[i])->coor, view);

		bool result = inter.PtInPolygon(point0Win, ptPolygon, false);
		if (result) {
			points.push_back(i);
		}
	}
	return points;
}

//判断三角网triganle是否包含index0和index1
bool lineTriangleToop(int index0, int index1, Triangle* triganle) {
	return ((index0 == triganle->vertexIndexs[0] || index0 == triganle->vertexIndexs[1] || index0 == triganle->vertexIndexs[2]) &&
		(index1 == triganle->vertexIndexs[0] || index1 == triganle->vertexIndexs[1] || index1 == triganle->vertexIndexs[2]));
}  

//遍历属于边界内部的三角网，并标记
void MainVisitor::inlineTraverseTriangle(Geom* geom, Triangle* triganle) {
	for (int i : triganle->neighborTriangles) {
		if (geom->triangles.at(i)->isBorder) {
			geom->triangles.at(i)->isTraversed = true;
			continue;
		}
		if (geom->triangles.at(i)->isTraversed) {
			continue;
		}
	/*	_group->addChild(DrawTriangle(geom->vertices.at(geom->triangles.at(i)->vertexIndexs[0])->coor,
			geom->vertices.at(geom->triangles.at(i)->vertexIndexs[1])->coor, geom->vertices.at(geom->triangles.at(i)->vertexIndexs[2])->coor,
			osg::Vec4(1.0f, 0.0f, 0.0f, 0.8f),0.8));*/
		geom->triangles.at(i)->isTraversed = true;
		geom->triangles.at(i)->isInline = true;
		geom->triangles.at(i)->isChioced = true;
		geom->choicedTriangles.push_back(i);
		inlineTraverseTriangle(geom, geom->triangles.at(i));
		
	}
}

//遍历边界内三角网
void MainVisitor::traverseTriangle(osg::Vec3Array *ptPolygon, osg::View *view) {
	Intersect inter;
	if (allGeom.size() > 0 && allGeom.size() == 1)
	{
		traverseTriangle_Geom(allGeom[0], ptPolygon, view);
	}
	else
	{
		for (Geom* geom : allGeom) {
			traverseTriangle_Geom(geom, ptPolygon, view);
		}
	}
}
//Traverse the triangulation of Geom within the boundary
void MainVisitor::traverseTriangle_Geom(Geom * geom, osg::Vec3Array *ptPolygon, osg::View *view) {
	vector<int > borderTriangles = geom->borderTriangles;
	for (int i = 0; i < borderTriangles.size(); i++) {
		int index = borderTriangles[i];
		Triangle * triangle = geom->triangles.at(index);
	
		if (!triangle->isTraversed) {
			triangle->isTraversed = true;
			vector<int> pointResult = triangleInterPoly(geom, index, ptPolygon, view);
			if (pointResult.size() == 2) {
				for (int j : triangle->neighborTriangles) {
					if (lineTriangleToop(triangle->vertexIndexs[pointResult.at(0)], triangle->vertexIndexs[pointResult.at(1)], geom->triangles.at(j))) {
						inlineTraverseTriangle(geom, geom->triangles.at(j));
					}
				}
			}

		}
	}
}
 
void MainVisitor::outsideTriangle() {
	if (allGeom.size() > 0 && allGeom.size() == 1)
	{
		vector<int > chioceTri = allGeom[0]->choicedTriangles;
		for (int i = 0; i < chioceTri.size(); i++) {
			int index = chioceTri[i];
			if (allGeom[0]->triangles.at(index)->neighborTriangles.size() < 3) {
				if (!allGeom[0]->triangles.at(index)->isAllTraversed&& !allGeom[0]->triangles.at(index)->isInline) {
					_group->addChild(DrawTriangle(allGeom[0]->vertices.at(allGeom[0]->triangles.at(index)->vertexIndexs[0])->coor,
						allGeom[0]->vertices.at(allGeom[0]->triangles.at(index)->vertexIndexs[1])->coor,
						allGeom[0]->vertices.at(allGeom[0]->triangles.at(index)->vertexIndexs[2])->coor,
						osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f)));
					outsideTraverse(allGeom[0], allGeom[0]->triangles.at(index));
				}
				else {

				}
			}
		}
	}
	else
	{
		for (Geom* geom : allGeom) {
			for (size_t i = 0; i < geom->triangles.size(); i++) {
				
			}
		}

	}
}

void MainVisitor::outsideTraverse(Geom* geom, Triangle * triganle) {
	for (int i : triganle->neighborTriangles) {
		if (!geom->triangles.at(i)->isChioced)
			continue;
		if (geom->triangles.at(i)->isInline)
			continue;
		if (geom->triangles.at(i)->isAllTraversed)
			continue;
		geom->triangles.at(i)->isAllTraversed = true;
	/*	_group->addChild(DrawTriangle(geom->vertices.at(geom->triangles.at(i)->vertexIndexs[0])->coor,
			geom->vertices.at(geom->triangles.at(i)->vertexIndexs[1])->coor, 
			geom->vertices.at(geom->triangles.at(i)->vertexIndexs[2])->coor,
			osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f)));*/
		outsideTraverse(geom, geom->triangles.at(i));
	}
}

//判断三角网和线段的交线数量，并返回交点在数组中的索引
std::vector<int> indexOfIntersect(std::vector<int> sideLineIndex) {
	std::vector<int> index;
	for (int i = 0; i < sideLineIndex.size(); i++) {
		if (sideLineIndex[i] != -1) {
			index.push_back(i);
		}
	}
	return index;
}

//判断投影边界的三角网
void MainVisitor::projectFilterTri(osg::ref_ptr<osg::Vec3Array> ptPolygon, osg::View *view, osg::Matrixd &matrix,MainVisitor & mainvisitor) {
	
	
	if (allGeom.size() > 0 && allGeom.size() == 1)
	{
	/*	int count = allGeom[0]->triangles.size();
		int halfLength = allGeom[0]->triangles.size()/2;
		std::thread thread1(&MainVisitor::filterGeomTri, this, allGeom[0], ptPolygon, view, 0, halfLength);
		std::thread thread2(&MainVisitor::filterGeomTri, this, allGeom[0], ptPolygon, view, halfLength, count);
		thread1.join();
		thread2.join();*/
		int count = allGeom[0]->triangles.size();
		filterGeomTri(allGeom[0], ptPolygon, view, 0, count);
	}
	else
	{
		for (Geom* geom : allGeom) {
			/*int count = geom->triangles.size();
			int halfLength = geom->triangles.size() / 2;
			std::thread thread1(&MainVisitor::filterGeomTri,this, geom, ptPolygon, view, 0, halfLength);
			std::thread thread2(&MainVisitor::filterGeomTri,this, geom, ptPolygon, view, halfLength, count);
			thread1.join();
			thread2.join();*/
			int count = geom->triangles.size();
			filterGeomTri(geom, ptPolygon, view, 0, count);
		}

	}

	cout << "Border Triangle Count : " << allGeom[0]->borderTriangles.size() << endl;
}

//filter border triangle of geom
void MainVisitor::filterGeomTri(Geom *geom, osg::ref_ptr<osg::Vec3Array> ptPolygon, osg::View *view, int start, int end) {
	Intersect inter;

	/*为了展示抽析之后的曲线*/
	//inter.SetLines(ptPolygon, view);
	//_group->addChild(inter._group);
	int sideCount = 0;

	for (size_t i = start; i < end; i++) {
		vector<int> points(0);
		Vertex *temp;
		for (int j = 0; j < 3; j++) {
			temp = geom->vertices.at(geom->triangles.at(i)->vertexIndexs[j]);
			if (temp->topology == Topology::UNKNOW) {
				osg::Vec3 point0Win = Utility::worldToWindow(temp->coor, view);
				if (_boundary->boundingBox.contains(point0Win)) {
					bool resultLevel = _boundary->pointTopo(point0Win);
					if (resultLevel) {
						points.push_back(j);
						temp->topology = Topology::Inline;
					}
					else {
						temp->topology = Topology::Outline;
					}
				}
				else {
					temp->topology = Topology::Outline;
				}
			}
			else if (temp->topology == Topology::Inline) {
				points.push_back(j);
				continue;
			}
			else if (temp->topology == Topology::OnLine) {
				osg::Vec3 point0Win = Utility::worldToWindow(temp->coor, view);
				if (_boundary->boundingBox.contains(point0Win)) {
					bool resultLevel = _boundary->pointTopo(point0Win);
					if (resultLevel) {
						points.push_back(j);
					}
				}
				continue;
			}
			else
				continue;
		}


		if (points.size() == 0) {
			geom->triangles.at(i)->isInline = false;
			geom->outTriangleNum = geom->outTriangleNum + 1;
			
		}

		if (points.size() > 0 && points.size() < 3) {

			Triangle *triangle = geom->triangles.at(i);
			triangle->isBorder = true;
			geom->borderTriangles.push_back(i);
			for (int t_index = 0; t_index < 3; t_index++) {
				if (geom->sides.at(triangle->sideIndexs[t_index])->isIntersect) {
					continue;
				}
				Side *side_first = geom->sides.at(triangle->sideIndexs[t_index]);
#if 1
				side_first->intersectionRes = inter.intersect_CurveSgement(Utility::worldToWindow(geom->vertices.at(side_first->vertexIndexs[0])->coor,view),
					Utility::worldToWindow(geom->vertices.at(side_first->vertexIndexs[1])->coor,view), ptPolygon, side_first->vertexIndexs[0], side_first->vertexIndexs[1]);
#else
				side_first->intersectionRes = inter.intersect_CurveSgement(Utility::worldToWindow(geom->vertices.at(side_first->vertexIndexs[0])->coor, view),
					Utility::worldToWindow(geom->vertices.at(side_first->vertexIndexs[1])->coor, view), _quadnodeList, side_first->vertexIndexs[0], side_first->vertexIndexs[1]);
#endif
				side_first->isIntersect = true;
				

				if (side_first->intersectionRes.pointNum > 0) {
					for (int point_i = 0; point_i < side_first->intersectionRes.pointNum; point_i++) {
						if (side_first->intersectionRes.proportions[point_i]<0.01) {
							side_first->intersectionRes.proportions[point_i] = 0;
							side_first->intersections.push_back(geom->vertices.at(side_first->vertexIndexs[0])->coor);
							geom->vertices.at(side_first->vertexIndexs[0])->topology = Topology::OnLine;
						}
						else if (side_first->intersectionRes.proportions[point_i] > 0.99)
						{
							side_first->intersectionRes.proportions[point_i] = 1;
							side_first->intersections.push_back(geom->vertices.at(side_first->vertexIndexs[1])->coor);
							geom->vertices.at(side_first->vertexIndexs[1])->topology = Topology::OnLine;
						}
						else {
							side_first->intersections.push_back(geom->vertices.at(side_first->vertexIndexs[0])->coor +
								(geom->vertices.at(side_first->vertexIndexs[1])->coor - geom->vertices.at(side_first->vertexIndexs[0])->coor)*side_first->intersectionRes.proportions[point_i]);
						}											
					}
				}
			}
			/*_group->addChild(DrawTriangle(geom->vertices.at(triangle->vertexIndexs[0])->coor,
				geom->vertices.at(triangle->vertexIndexs[1])->coor, geom->vertices.at(triangle->vertexIndexs[2])->coor,
				osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f), 0.8));*/

			int num = 0;
			IntersectionRes intersection01 = geom->sides.at(triangle->sideIndexs[0])->intersectionRes;
			IntersectionRes intersection12 = geom->sides.at(triangle->sideIndexs[1])->intersectionRes;
			IntersectionRes intersection20 = geom->sides.at(triangle->sideIndexs[2])->intersectionRes;
			geom->triangles.at(i)->intersectionRess.push_back(intersection01);
			geom->triangles.at(i)->intersectionRess.push_back(intersection12);
			geom->triangles.at(i)->intersectionRess.push_back(intersection20);
			num = intersection01.pointNum + intersection20.pointNum + intersection12.pointNum;
			if (num == 0) {
				if (points.size() == 1) {
					geom->triangles.at(i)->intersectionStatus = 0;
				}
				else {
					geom->triangles.at(i)->intersectionStatus = 1;
				}
			}
			else if (num == 2) {
				if (intersection01.pointNum == 1 || intersection20.pointNum == 1 || intersection12.pointNum == 1) {
					
					geom->triangles.at(i)->intersectionStatus = 2;
				}
				else {
					geom->triangles.at(i)->intersectionStatus = 3;
				}
			}
			else if (num > 2) {
				if (num == intersection01.pointNum || num == intersection20.pointNum || num == intersection12.pointNum) {
					geom->triangles.at(i)->intersectionStatus = 4;
				}
				else if (intersection01.pointNum == 0 || intersection20.pointNum == 0 || intersection12.pointNum == 0) {
					geom->triangles.at(i)->intersectionStatus = 5;
				}
				else if (intersection01.pointNum != 0 && intersection20.pointNum != 0 && intersection12.pointNum != 0) {
					geom->triangles.at(i)->intersectionStatus = 6;
				}
				else {
					geom->triangles.at(i)->intersectionStatus = 7;
				}
			}
		}

		if (points.size() == 3) {
			geom->triangles.at(i)->isInline = true;
			geom->inlineTriangleNum = geom->inlineTriangleNum + 1;
		}
		
	}




	//因为边界三角形的个数可能会动态变化
 	int borderTriangleLength = geom->borderTriangles.size();
	for (size_t borderTriangles_i = 0; borderTriangles_i < borderTriangleLength; borderTriangles_i++) {
		
		for (size_t side_i : geom->triangles.at(geom->borderTriangles[borderTriangles_i])->sideIndexs) {
			if (geom->sides.at(side_i)->intersectionRes.pointNum == 0)
				continue;
			for (int neighborTri_i : geom->sides.at(side_i)->neighborTriangles) {
				if (neighborTri_i != borderTriangles_i) {
					if (geom->triangles.at(neighborTri_i)->isBorder)
						continue;
					//有交点的边 相邻的三角形不是边界三角形
					for (int t_index = 0; t_index < 3; t_index++) {
						if (geom->sides.at(geom->triangles.at(neighborTri_i)->sideIndexs[t_index])->isIntersect) {
							continue;
						}
						Side *side_first = geom->sides.at(geom->triangles.at(neighborTri_i)->sideIndexs[t_index]);
#if 1
						side_first->intersectionRes = inter.intersect_CurveSgement(Utility::worldToWindow(geom->vertices.at(side_first->vertexIndexs[0])->coor,view),
							Utility::worldToWindow(geom->vertices.at(side_first->vertexIndexs[1])->coor,view), ptPolygon, side_first->vertexIndexs[0], side_first->vertexIndexs[1]);
#else
						side_first->intersectionRes = inter.intersect_CurveSgement(Utility::worldToWindow(geom->vertices.at(side_first->vertexIndexs[0])->coor, view),
							Utility::worldToWindow(geom->vertices.at(side_first->vertexIndexs[1])->coor, view), _quadnodeList, side_first->vertexIndexs[0], side_first->vertexIndexs[1]);
#endif
						side_first->isIntersect = true;
						if (side_first->intersectionRes.pointNum > 0) {
							for (int point_i = 0; point_i < side_first->intersectionRes.pointNum; point_i++) {
								if (side_first->intersectionRes.proportions[point_i] < 0.01) {
									side_first->intersectionRes.proportions[point_i] = 0;
									side_first->intersections.push_back(geom->vertices.at(side_first->vertexIndexs[0])->coor);
									geom->vertices.at(side_first->vertexIndexs[0])->topology = Topology::OnLine;
								}
								else if (side_first->intersectionRes.proportions[point_i] > 0.99)
								{
									side_first->intersectionRes.proportions[point_i] = 1;
									side_first->intersections.push_back(geom->vertices.at(side_first->vertexIndexs[1])->coor);
									geom->vertices.at(side_first->vertexIndexs[1])->topology = Topology::OnLine;
								}
								else {
									side_first->intersections.push_back(geom->vertices.at(side_first->vertexIndexs[0])->coor +
										(geom->vertices.at(side_first->vertexIndexs[1])->coor - geom->vertices.at(side_first->vertexIndexs[0])->coor)*side_first->intersectionRes.proportions[point_i]);
								}
							}
						}
					}
					geom->triangles.at(neighborTri_i)->isBorder = true;
					geom->borderTriangles.push_back(neighborTri_i);
					borderTriangleLength = geom->borderTriangles.size();

					int num = 0;
					Triangle * triangle = geom->triangles.at(neighborTri_i);
					IntersectionRes intersection01 = geom->sides.at(triangle->sideIndexs[0])->intersectionRes;
					IntersectionRes intersection12 = geom->sides.at(triangle->sideIndexs[1])->intersectionRes;
					IntersectionRes intersection20 = geom->sides.at(triangle->sideIndexs[2])->intersectionRes;
					triangle->intersectionRess.push_back(intersection01);
					triangle->intersectionRess.push_back(intersection12);
					triangle->intersectionRess.push_back(intersection20);
					num = intersection01.pointNum + intersection20.pointNum + intersection12.pointNum;
					if (num == 0) {
						if (geom->triangles.at(neighborTri_i)->isInline) {
							triangle->intersectionStatus = 0;
						}
						else {
							triangle->intersectionStatus = 1;
						}
					}
					else if (num == 2) {
						if (intersection01.pointNum == 1 || intersection20.pointNum == 1 || intersection12.pointNum == 1) {
							triangle->intersectionStatus = 2;
						}
						else {
							triangle->intersectionStatus = 3;
						}
					}
					else if (num > 2) {
						if (num == intersection01.pointNum || num == intersection20.pointNum || num == intersection12.pointNum) {
							triangle->intersectionStatus = 4;
						}
						else if (intersection01.pointNum == 0 || intersection20.pointNum == 0 || intersection12.pointNum == 0) {
							triangle->intersectionStatus = 5;
						}
						else if (intersection01.pointNum != 0 && intersection20.pointNum != 0 && intersection12.pointNum != 0) {
							triangle->intersectionStatus = 6;
						}
						else {
							triangle->intersectionStatus = 7;
						}
					}
				}
					
			}
		}
	}

}

//Boundary triangulation reconstruction with projection
void MainVisitor::projectRebuild() {
	Intersect inter;
	if (allGeom.size() > 0 && allGeom.size() == 1)
	{
		rebuildGeom(allGeom[0]);
	}
	else { 
		for (Geom* geom : allGeom) {
			rebuildGeom(geom);
		}

	}
}

void MainVisitor::rebuildGeom(Geom *geom) {
	int num = 0;
	int debug = 0;
	for (size_t i = 0; i < geom->borderTriangles.size(); i++) {
		int index = geom->borderTriangles.at(i);
		int intersectionStatus = geom->triangles.at(index)->intersectionStatus;
		switch (intersectionStatus)
		{
		case 0:
			//边线没有交点,那么三角网不需要重构，这样就把三角网排除再边界三角网外，再重构的时候用于重构
			geom->triangles.at(index)->isBorder = false;
			geom->triangles.at(index)->isInline = false;
			
			num++;
			break;
		case 1:
			//只有一个交点，应该不会有这种情况发生
			/*_group->addChild(DrawTriangle(geom->vertices.at(geom->triangles.at(index)->vertexIndexs[0])->coor,
				geom->vertices.at(geom->triangles.at(index)->vertexIndexs[1])->coor, geom->vertices.at(geom->triangles.at(index)->vertexIndexs[2])->coor,
				osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f), 0.8));*/
			num++;
			break;
		case 2:
			twoIntersectionOnTwoLine(geom, index);
				
			num++;
			break;
		case 3:
			//有两个交点都在同一条边上
			moreIntersectionOnOneLine(geom, index);
			num++;
			break;
		case 4:
			//有多个交点都在同一条边上
			moreIntersectionOnOneLine(geom, index);
			num++;
		case 5:
			//有多个交点都在两条条边上
			manyIntersectionOnTwoLine(geom, index);
			num++;
			break;
		case 6:
			//有多个交点都在三条边上
			manyIntersectionOnThreeLine(geom, index);
			num++;
			break;
		case 8:
			//有两个交点，且在不同边上.但是判断的是三角形的顶点都在边界内或边界外
			abnormalTwoIntersectionOnTwoLine(geom, index);
			num++;
			break;
		default:
			break;
		}
	}
	cout << geom->borderTriangles.size() << "," << num << endl;
}

#if 0
Guidelines MainVisitor::DelaunayTriangulator(osg::ref_ptr<osg::Vec3Array> coords,osg::Vec3 vertex0, osg::Vec3 vertex1, bool middle = false) {
	Guidelines guideline;
	osg::ref_ptr<osg::Group> group = new osg::Group;
	//创建Delaunay三角网对象
	osg::ref_ptr<osgUtil::DelaunayTriangulator> dt = new osgUtil::DelaunayTriangulator(coords);
	//生成三角网，,此过程所用时间和点的量有关系
	if (middle) {
		int num = coords->size();
		//增加剖分限制条件
		osg::ref_ptr<osgUtil::DelaunayConstraint> dc1 = new osgUtil::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		osg::ref_ptr <osg::Vec3Array> bounds = new osg::Vec3Array;
		bounds->push_back(coords->at(0));
		bounds->push_back(coords->at(num-1));
		dc1->setVertexArray(bounds);
		dc1->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		dt->addInputConstraint(dc1);

		osg::ref_ptr<osgUtil::DelaunayConstraint> dc2 = new osgUtil::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		osg::ref_ptr <osg::Vec3Array> bounds2 = new osg::Vec3Array;
		bounds2->push_back(coords->at(1));
		bounds2->push_back(coords->at(num - 1));
		dc2->setVertexArray(bounds2);
		dc2->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		dt->addInputConstraint(dc2);

		osg::ref_ptr<osgUtil::DelaunayConstraint> dc3 = new osgUtil::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		osg::ref_ptr <osg::Vec3Array> bounds3 = new osg::Vec3Array;
		bounds3->push_back(coords->at(2));
		bounds3->push_back(coords->at(num - 1));
		dc3->setVertexArray(bounds3);
		dc3->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		dt->addInputConstraint(dc3);
	}
	
	dt->triangulate();
	//创建几何体
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	//设置顶点数组
	geometry->setVertexArray(coords);
	//加入到绘图基元
	osg::ref_ptr<osg::DrawElementsUInt> test = dt->getTriangles();
	int element = test->getElement(0);
	unsigned int indexNum = test->getNumIndices();

	for (int i = 0; i < coords->size(); i++) {
		guideline.vertices.push_back(coords->at(i));
	}

	for (size_t i = 0; i < indexNum/3; i++) {
		unsigned int startIndex = i * 3;
		if (coords->at(test->index(startIndex)) == vertex0 || coords->at(test->index(startIndex + 1)) == vertex0 || coords->at(test->index(startIndex + 2)) == vertex0
			|| coords->at(test->index(startIndex)) == vertex1 || coords->at(test->index(startIndex + 1)) == vertex1 || coords->at(test->index(startIndex + 2)) == vertex1)
			continue;
		guideline.indexes.push_back(test->index(startIndex));
		guideline.indexes.push_back(test->index(startIndex + 1));
		guideline.indexes.push_back(test->index(startIndex + 2));
		group->addChild(DrawTriangle(coords->at(test->index(startIndex)), coords->at(test->index(startIndex + 1)), coords->at(test->index(startIndex + 2))
			, osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.9));
	
	}
	return guideline;
}
#endif // 0

//不规则三角网化 三角网有两个点在边界外
vector<int> MainVisitor::DelaunayTriangulator(osg::ref_ptr<osg::Vec3Array> coords, osg::Vec3 vertex0, osg::Vec3 vertex1, bool middle = false) {
	vector<int> indexes;
	osg::ref_ptr<osg::Group> group = new osg::Group;
	//创建Delaunay三角网对象
	osg::ref_ptr<osgUtil::DelaunayTriangulator> dt = new osgUtil::DelaunayTriangulator(coords);
	//生成三角网，,此过程所用时间和点的量有关系
	if (middle) {
		int num = coords->size();
		//增加剖分限制条件
		osg::ref_ptr<osgUtil::DelaunayConstraint> dc1 = new osgUtil::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		osg::ref_ptr <osg::Vec3Array> bounds = new osg::Vec3Array;
		bounds->push_back(coords->at(0));
		bounds->push_back(coords->at(num - 1));
		dc1->setVertexArray(bounds);
		dc1->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		dt->addInputConstraint(dc1);

		osg::ref_ptr<osgUtil::DelaunayConstraint> dc2 = new osgUtil::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		osg::ref_ptr <osg::Vec3Array> bounds2 = new osg::Vec3Array;
		bounds2->push_back(coords->at(1));
		bounds2->push_back(coords->at(num - 1));
		dc2->setVertexArray(bounds2);
		dc2->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		dt->addInputConstraint(dc2);

		osg::ref_ptr<osgUtil::DelaunayConstraint> dc3 = new osgUtil::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		osg::ref_ptr <osg::Vec3Array> bounds3 = new osg::Vec3Array;
		bounds3->push_back(coords->at(2));
		bounds3->push_back(coords->at(num - 1));
		dc3->setVertexArray(bounds3);
		dc3->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		dt->addInputConstraint(dc3);
	}

	dt->triangulate();
	//创建几何体
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	//设置顶点数组
	geometry->setVertexArray(coords);
	//加入到绘图基元
	osg::ref_ptr<osg::DrawElementsUInt> test = dt->getTriangles();
	int element = test->getElement(0);
	unsigned int indexNum = test->getNumIndices();

	for (size_t i = 0; i < indexNum / 3; i++) {
		unsigned int startIndex = i * 3;
		if (coords->at(test->index(startIndex)) == vertex0 || coords->at(test->index(startIndex + 1)) == vertex0 || coords->at(test->index(startIndex + 2)) == vertex0
			|| coords->at(test->index(startIndex)) == vertex1 || coords->at(test->index(startIndex + 1)) == vertex1 || coords->at(test->index(startIndex + 2)) == vertex1)
			continue;
		indexes.push_back(test->index(startIndex));
		indexes.push_back(test->index(startIndex + 1));
		indexes.push_back(test->index(startIndex + 2));
		group->addChild(DrawTriangle(coords->at(test->index(startIndex)), coords->at(test->index(startIndex + 1)), coords->at(test->index(startIndex + 2))
			, osg::Vec4(0.0f, 0.0f, 1.0f, 1.0f), 0.9));

	}
	return indexes;
}

//不规则三角网化 三角网有一个点在边界外
vector<int> MainVisitor::DelaunayTriangulator(osg::ref_ptr<osg::Vec3Array> coords, osg::Vec3 vertex0, bool middle=false) {
	vector<int> indexes;
	osg::ref_ptr<osg::Group> group = new osg::Group;
	//创建Delaunay三角网对象
	osg::ref_ptr<osgUtil::DelaunayTriangulator> dt = new osgUtil::DelaunayTriangulator(coords);
	//生成三角网，,此过程所用时间和点的量有关系
	if (middle) {
		int num = coords->size(); 
		//增加剖分限制条件
		osg::ref_ptr<osgUtil::DelaunayConstraint> dc1 = new osgUtil::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		osg::ref_ptr <osg::Vec3Array> bounds = new osg::Vec3Array;
		bounds->push_back(coords->at(0));
		bounds->push_back(coords->at(num-1));
		dc1->setVertexArray(bounds);
		dc1->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		dt->addInputConstraint(dc1);

		osg::ref_ptr<osgUtil::DelaunayConstraint> dc2 = new osgUtil::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		osg::ref_ptr <osg::Vec3Array> bounds2 = new osg::Vec3Array;
		bounds2->push_back(coords->at(1));
		bounds2->push_back(coords->at(num - 1));
		dc2->setVertexArray(bounds2);
		dc2->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		dt->addInputConstraint(dc2);

		osg::ref_ptr<osgUtil::DelaunayConstraint> dc3 = new osgUtil::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		osg::ref_ptr <osg::Vec3Array> bounds3 = new osg::Vec3Array;
		bounds3->push_back(coords->at(2));
		bounds3->push_back(coords->at(num - 1));
		dc3->setVertexArray(bounds3);
		dc3->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 2));
		dt->addInputConstraint(dc3);
	}
	dt->triangulate();
	//创建几何体
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	//设置顶点数组
	geometry->setVertexArray(coords);
	//加入到绘图基元
	osg::ref_ptr<osg::DrawElementsUInt> test = dt->getTriangles();
	int element = test->getElement(0);
	unsigned int indexNum = test->getNumIndices();

	for (size_t i = 0; i < indexNum / 3; i++) {
		unsigned int startIndex = i * 3;
		int index1 = test->index(startIndex);
		int index2 = test->index(startIndex+1);
		int index3 = test->index(startIndex+2);
		if (coords->at(test->index(startIndex)) == vertex0 || coords->at(test->index(startIndex + 1)) == vertex0
			|| coords->at(test->index(startIndex + 2)) == vertex0)
			continue;
		indexes.push_back(test->index(startIndex));
		indexes.push_back(test->index(startIndex + 1));
		indexes.push_back(test->index(startIndex + 2));
	}
	return indexes;
}

//不规则三角网化 三角网有一个点在重建范围，且可以定义限制条件
vector<int> MainVisitor::DelaunayTriangulator(osg::ref_ptr<osg::Vec3Array> coords, osg::Vec3 vertex0, osg::ref_ptr<osg::Vec3Array> Constraint) {
	vector<int> indexes;
	osg::ref_ptr<osg::Group> group = new osg::Group;
	//创建Delaunay三角网对象
	osg::ref_ptr<osgTest::DelaunayTriangulator> dt = new osgTest::DelaunayTriangulator(coords);
	//生成三角网，,此过程所用时间和点的量有关系
	int constraintSize = Constraint->size();
	if (constraintSize >0) {
		int num = coords->size();
		//增加剖分限制条件
		osg::ref_ptr<osgTest::DelaunayConstraint> dc1 = new osgTest::DelaunayConstraint;
		//限制条件是顶点和拐点连线
		dc1->setVertexArray(Constraint);
		dc1->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, constraintSize));
		dt->addInputConstraint(dc1);
	}
	dt->triangulate();
	//加入到绘图基元
	osg::ref_ptr<osg::DrawElementsUInt> test = dt->getTriangles();
	if (!test)
		return indexes;
	int element = test->getElement(0);
	unsigned int indexNum = test->getNumIndices();

	for (size_t i = 0; i < indexNum / 3; i++) {
		unsigned int startIndex = i * 3;
		int index0 = test->index(startIndex);
		int index1 = test->index(startIndex + 1);
		int index2 = test->index(startIndex + 2);
		/*osg::Vec3 n_vector = (coords->at(index0) - coords->at(index1)) ^ (coords->at(index0) - coords->at(index2));
		float length = n_vector.length2();*/
	/*	if (abs(n_vector._v[0]) < EXTREMELY_SMALL && abs(n_vector._v[1]) < EXTREMELY_SMALL&& abs(n_vector._v[2]) < EXTREMELY_SMALL) {
			continue;
		}*/
		if (Utility::isCollinear(coords->at(index0), coords->at(index1), coords->at(index2)))
			continue;

		indexes.push_back(index0);
		indexes.push_back(index1);
		indexes.push_back(index2);
	}
	return indexes;
}

//改变后坐标数组中的坐标在 顶点数组中的对应索引位置。即coords中的坐标在vertexs的对应索引   
vector<int> ContrastIndex(vector<Vertex *> vertexs, osg::ref_ptr<osg::Vec3Array> coords) {
	const int vertexs_num = vertexs.size();
	const int coords_num = coords->size();
	vector<int> contrastArray(coords_num);
	for (int i = 0; i < coords_num; i++) {
		for (int j = 0; j < vertexs_num; j++) {
			if (coords->at(i) == vertexs[j]->coor) {
				contrastArray[i] = j;
				break;
			}
		}
	}
	return contrastArray;
}

//Calculate UV coordinates of 3D point (coord) on the mesh's triangle
osg::Vec2 CalculateUVCoordinates(vector<Vertex *> vertexs, osg::Vec3 coord) {
	// calculate vectors from point f to vertices p1, p2 and p3:
	osg::Vec3 f1 = vertexs[0]->coor - coord;
	osg::Vec3 f2 = vertexs[1]->coor - coord;
	osg::Vec3 f3 = vertexs[2]->coor - coord;
	// calculate the areas and factors (order of parameters doesn't matter):
	float a = ((vertexs[0]->coor- vertexs[1]->coor)^(vertexs[0]->coor - vertexs[2]->coor)).length(); // main triangle area a
	float a1 = (f2^f3).length() / a;  // p1's triangle area / a
	float a2 = (f3^f1).length() / a;  // p2's triangle area / a
	float a3 = (f1^f2).length() / a;  // p3's triangle area / a
	// find the uv corresponding to point f (uv1/uv2/uv3 are associated to p1/p2/p3):
	return  (vertexs[0]->texCoor * a1 + vertexs[1]->texCoor * a2 + vertexs[2]->texCoor * a3);
}


//The boundary line has an intersection with each of the two sides of the triangle
void MainVisitor::twoIntersectionOnTwoLine(Geom* geom, int triIndex) {
	Triangle * triangle = geom->triangles.at(triIndex);
	std::vector<IntersectionRes> intersection = triangle->intersectionRess;
	//需要重构三角网所有顶点集合
	vector<Vertex *> vertexs;
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[0])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[1])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[2])));

	vector<Vertex *> vertexs1;
	vertexs1.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[0])));
	vertexs1.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[1])));
	vertexs1.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[2])));


	vector<Point::IntersectPoint> intersectPoints;
	for (int tri_i = 0; tri_i < 3; tri_i++) {
		for (int point_i = 0; point_i < intersection[tri_i].pointNum; point_i++) {
			Point::IntersectPoint point;
			point.coordinate = geom->sides.at(triangle->sideIndexs[tri_i])->intersections[point_i];
			point.edgeIndex = triangle->sideIndexs[tri_i];
			point.edge[0] = intersection[tri_i].indices[0];
			point.edge[1] = intersection[tri_i].indices[1];
			point.lineIndex = intersection[tri_i].lineIndex[point_i];
			point.intersectPro = intersection[tri_i].proportions[point_i];
			intersectPoints.push_back(point);
		}
	}

	//which vertex is the intersection of edge1 and edge2 ; return 0;1;2 
	int intersectVertex = geom->triangles[triIndex]->whichVertex(intersectPoints[0].edge[0], intersectPoints[0].edge[1], intersectPoints[1].edge[0], intersectPoints[1].edge[1]);
	if (intersectVertex == -1) return;
	//vertex index in vertices
	int interInVertices = geom->triangles[triIndex]->vertexIndexs[intersectVertex];

	//将交点存入点数组，并放入顶点数组中
	vector<Point::IntersectPoint>::iterator inter;
	for (inter = intersectPoints.begin(); inter != intersectPoints.end(); ) {
		if ((*inter).intersectPro != 0 && (*inter).intersectPro != 1) {
			Side * side = geom->sides.at((*inter).edgeIndex);
			if (side->intersectionIndexs.size() == 0) {
				Vertex * vertex = new Vertex;
				vertex->coor = (*inter).coordinate;
				vertex->index = geom->vertices.size();
				geom->vertices.push_back(vertex);
				//UV coordinates
				vertex->texCoor = CalculateUVCoordinates(vertexs1, vertex->coor);
				vertexs.push_back(vertex);
				side->intersectionIndexs.push_back(vertex->index);
			}
			else {
				vertexs.push_back(geom->vertices[side->intersectionIndexs[0]]);
			}
		}
		inter++;
	}
	
	//根据边界线顺序排序交点
	int minIndex;
	Point::IntersectPoint inter_temp;
	osg::Vec3 tempVec;
	for (int i = 0; i < intersectPoints.size(); i++) {
		minIndex = i;
		for (int j = i + 1; j < intersectPoints.size(); j++) {
			if (intersectPoints[j].lineIndex == intersectPoints[minIndex].lineIndex) {
				int nextIndex = intersectPoints[j].lineIndex + LINE_STEP < _lineVertices.size() ? intersectPoints[j].lineIndex + LINE_STEP : 0;
				osg::Vec3 direction1 = _lineVertices[nextIndex] - _lineVertices[intersectPoints[j].lineIndex];
				osg::Vec3 direction2 = intersectPoints[j].coordinate - intersectPoints[minIndex].coordinate;
				if (direction1*direction2 < 0) {
					minIndex = j;
				}
			}

			if (Utility::CustomizeSmall(intersectPoints[j].lineIndex, intersectPoints[minIndex].lineIndex, _lineVertices.size())) {     // 寻找最小的数
				minIndex = j;                 // 将最小数的索引保存	
			}
		}
		inter_temp = intersectPoints[i];
		intersectPoints[i] = intersectPoints[minIndex];
		intersectPoints[minIndex] = inter_temp;
	}
	osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> constraintCoords = new osg::Vec3Array;
	coords->push_back(vertexs[0]->coor);
	coords->push_back(vertexs[1]->coor);
	coords->push_back(vertexs[2]->coor);

	//temp variable for index of intersecting boundary line
	int tempIndex;
	//在有多个交点的情况下，获取交点和拐点
	for (int i = 0; i < intersectPoints.size(); i++) {
		if (intersectPoints[i].coordinate != vertexs[0]->coor && intersectPoints[i].coordinate != vertexs[1]->coor && intersectPoints[i].coordinate != vertexs[2]->coor) 
			coords->push_back(intersectPoints[i].coordinate);
		constraintCoords->push_back(intersectPoints[i].coordinate);
		if (i < intersectPoints.size() - 1) {
			osg::Vec3 turningPoint;
			osg::Vec3 projectionPoint;
			tempIndex = intersectPoints[i].lineIndex;
 			bool turning = true;
			while (tempIndex != intersectPoints[i + 1].lineIndex && turning)
			{
				tempIndex = (tempIndex + LINE_STEP) < _lineVertices.size() ? (tempIndex + LINE_STEP) : 0;
				turningPoint = _lineVertices[tempIndex];
				bool isOntri = pointProjectOnTri(turningPoint, vertexs[0]->coor, vertexs[1]->coor, vertexs[2]->coor, projectionPoint);
				if (isOntri) {
 					coords->push_back(projectionPoint);
					constraintCoords->push_back(projectionPoint);
					Vertex * vertex_middle = new Vertex;
					vertex_middle->coor = projectionPoint;
					vertex_middle->index = geom->vertices.size();
					vertex_middle->texCoor = CalculateUVCoordinates(vertexs, vertex_middle->coor);
					geom->vertices.push_back(vertex_middle);
					vertexs.push_back(vertex_middle);
				}
				else {
					turning = false;
				}
			
			};
		}
	}

	/*for (int i = 0; i < constraintCoords->size()-1; i++) {
		_group->addChild(DrawLine(constraintCoords->at(i), constraintCoords->at(i + 1), osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f)));
	}*/
	int vNum = vertexs.size();
	int cNum = coords->size();
	vector<int> triang_indexes = DelaunayTriangulator(coords, vertexs[intersectVertex]->coor, constraintCoords);
	vector<int> contrastArray = ContrastIndex(vertexs, coords);
	Intersect intersect;
	int validVertex = interInVertices;
	int indexInTri = intersectVertex;
	int loopNum = 0;
	while (geom->vertices[validVertex]->topology == Topology::OnLine && loopNum < 3) {
		indexInTri = Utility::customizePlus(indexInTri);
		validVertex = geom->triangles[triIndex]->vertexIndexs[indexInTri];
		loopNum++;
	}
	if (loopNum == 3) return;
	if (geom->vertices[validVertex]->topology == Topology::Outline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (vertexs[contrastArray[triang_indexes[i * 3]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor) / 3;
			int intersect_Count = 0, constraint_i = 0;
			if (constraintCoords->size() > 1) {
				for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
					osg::Vec3 aaa;
					int result = intersect.LineLineIntersect(centroid, geom->vertices[validVertex]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1));
					if (result)
						intersect_Count += result;
				}
			}
			if (intersect_Count % 2 == 0) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
			}
		}
	}
	else if (geom->vertices[validVertex]->topology == Topology::Inline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (vertexs[contrastArray[triang_indexes[i * 3]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor) / 3;
			int intersect_Count = 0, constraint_i = 0;
			if (constraintCoords->size() > 1) {
				for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
					osg::Vec3 aaa;
					int result = intersect.LineLineIntersect(centroid, geom->vertices[validVertex]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1));
					if (result)
						intersect_Count += result;
				}
			}
			if (intersect_Count % 2 == 1) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
			}
		}

	}
}

//The boundary line has more than two intersections with one side of the triangle
void MainVisitor::moreIntersectionOnOneLine(Geom* geom, int triIndex) {
	Triangle * triangle = geom->triangles.at(triIndex);
	std::vector<IntersectionRes> intersection = triangle->intersectionRess;
	//需要重构三角网所有顶点集合
	vector<Vertex *> vertexs;
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[0])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[1])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[2])));

	vector<Point::IntersectPoint> intersectPoints;
	int intersecIndex;
	for (int tri_i = 0; tri_i < 3; tri_i++) {
		if (intersection[tri_i].pointNum > 0) {
			intersecIndex = tri_i;
			for (int point_i = 0; point_i < intersection[tri_i].pointNum; point_i++) {
				Point::IntersectPoint point;
				point.coordinate = geom->sides.at(triangle->sideIndexs[tri_i])->intersections[point_i];
				point.edgeIndex = triangle->sideIndexs[tri_i];
				point.edge[0] = intersection[tri_i].indices[0];
				point.edge[1] = intersection[tri_i].indices[1];
				point.lineIndex = intersection[tri_i].lineIndex[point_i];
				point.intersectPro = intersection[tri_i].proportions[point_i];
				intersectPoints.push_back(point);
			}
		}
	}

	//which point opposite the triangle side ; return 0;1;2 
	int intersectVertex = geom->triangles[triIndex]->oppositePoint(intersection[intersecIndex].indices[0], intersection[intersecIndex].indices[1]);
	if (intersectVertex == -1) return;
	
	Side * side_corner = geom->sides.at(triangle->sideIndexs[intersecIndex]);
	//如果边的交点已经添加到了顶点数组中，则不用在顶点数组中再添加，只需要调用
	for (int i = 0; i < side_corner->intersections.size(); i++) {
		if (side_corner->intersectionIndexs.size()==i) {
			Vertex * vertex = new Vertex;
			vertex->coor = side_corner->intersections[i];
			vertex->index = geom->vertices.size();
			geom->vertices.push_back(vertex);
			//UV coordinates
			vertex->texCoor = CalculateUVCoordinates(vertexs, vertex->coor);
			vertexs.push_back(vertex);
			side_corner->intersectionIndexs.push_back(vertex->index);

		}
		else {
			vertexs.push_back(geom->vertices[side_corner->intersectionIndexs[i]]);
		}
	}

	//根据边界线顺序排序交点
	int minIndex;
	Point::IntersectPoint inter_temp;
	osg::Vec3 tempVec;
	for (int i = 0; i < intersectPoints.size(); i++) {
		minIndex = i;
		for (int j = i + 1; j < intersectPoints.size(); j++) {
			if (intersectPoints[j].lineIndex == intersectPoints[minIndex].lineIndex) {
				int nextIndex = intersectPoints[j].lineIndex + LINE_STEP < _lineVertices.size() ? intersectPoints[j].lineIndex + LINE_STEP : 0;
				osg::Vec3 direction1 = _lineVertices[nextIndex] - _lineVertices[intersectPoints[j].lineIndex];
				osg::Vec3 direction2 = intersectPoints[j].coordinate - intersectPoints[minIndex].coordinate;
				if (direction1*direction2 < 0) {
					minIndex = j;
				}
			}

			if (Utility::CustomizeSmall(intersectPoints[j].lineIndex, intersectPoints[minIndex].lineIndex, _lineVertices.size())) {     // 寻找最小的数
				minIndex = j;                 // 将最小数的索引保存	
			}
		}
		inter_temp = intersectPoints[i];
		intersectPoints[i] = intersectPoints[minIndex];
		intersectPoints[minIndex] = inter_temp;
	}
	osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> constraintCoords = new osg::Vec3Array;
	coords->push_back(vertexs[0]->coor);
	coords->push_back(vertexs[1]->coor);
	coords->push_back(vertexs[2]->coor);
	
	int tempIndex;
	//在有多个交点的情况下，获取交点和拐点
	for (int i = 0; i < intersectPoints.size(); i++) {
		coords->push_back(intersectPoints[i].coordinate);
		constraintCoords->push_back(intersectPoints[i].coordinate);
		if (i < intersectPoints.size() - 1) {
			osg::Vec3 turningPoint;
			osg::Vec3 projectionPoint;
			tempIndex = intersectPoints[i].lineIndex;
			bool turning = true;
			while (tempIndex != intersectPoints[i + 1].lineIndex && turning)
			{
				tempIndex = (tempIndex + LINE_STEP) < _lineVertices.size() ? (tempIndex + LINE_STEP) : 0;
				turningPoint = _lineVertices[tempIndex];
				bool isOntri = pointProjectOnTri(turningPoint, vertexs[0]->coor, vertexs[1]->coor, vertexs[2]->coor, projectionPoint);
				if (isOntri) {
					coords->push_back(projectionPoint);
					constraintCoords->push_back(projectionPoint);
					Vertex * vertex_middle = new Vertex;
					vertex_middle->coor = projectionPoint;
					vertex_middle->index = geom->vertices.size();
					vertex_middle->texCoor = CalculateUVCoordinates(vertexs, vertex_middle->coor);
					geom->vertices.push_back(vertex_middle);
					vertexs.push_back(vertex_middle);
				}
				else {
					turning = false;
				}

			};
			/*while (tempIndex != intersectPoints[i + 1].lineIndex)
			{
				tempIndex = (tempIndex + LINE_STEP) < _lineVertices.size() ? (tempIndex + LINE_STEP) : 0;
				turningPoint = _lineVertices[tempIndex];
				bool isOntri = pointProjectOnTri(turningPoint, vertexs[0]->coor, vertexs[1]->coor, vertexs[2]->coor, projectionPoint);
				if (isOntri) {
					coords->push_back(projectionPoint);
					constraintCoords->push_back(projectionPoint);
					Vertex * vertex_middle = new Vertex;
					vertex_middle->coor = projectionPoint;
					vertex_middle->index = geom->vertices.size();
					vertex_middle->texCoor = CalculateUVCoordinates(vertexs, vertex_middle->coor);
					geom->vertices.push_back(vertex_middle);
					vertexs.push_back(vertex_middle);
				}
				
			};*/
		}
	}

	vector<int> triang_indexes = DelaunayTriangulator(coords, vertexs[intersectVertex]->coor, constraintCoords);
	vector<int> contrastArray = ContrastIndex(vertexs, coords);
	Intersect intersect;

	// Exclude points on the boundary as judgment vertices
	int indexInTri = intersectVertex;
	int loopNum = 0;
	while (vertexs[indexInTri]->topology == Topology::OnLine && loopNum < 3) {
		indexInTri = Utility::customizePlus(indexInTri);
		loopNum++;
	}
	if (loopNum == 3) return;
	if (vertexs[indexInTri]->topology == Topology::Outline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (vertexs[contrastArray[triang_indexes[i * 3]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor) / 3;
			int intersect_Count = 0, constraint_i = 0;
			for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
				osg::Vec3 aaa;
				int result = intersect.LineLineIntersect(centroid, vertexs[indexInTri]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1));
				if (result)
					intersect_Count += result;
			}
			if (intersect_Count % 2 == 0) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
			}
		}
	}
	else if (vertexs[indexInTri]->topology == Topology::Inline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (vertexs[contrastArray[triang_indexes[i * 3]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor) / 3;
			int intersect_Count = 0, constraint_i = 0;
			for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
				osg::Vec3 aaa;
				int result = intersect.LineLineIntersect(centroid, vertexs[indexInTri]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1));
				if (result)
					intersect_Count += result;
			}
			if (intersect_Count % 2 == 1) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
			}
		}
	}
}

//有多个交点交在两条边上，也就是所可能每一条边上都有大于等于一个得交点。
//The boundary line has more than two intersections with two sides of the triangle
void MainVisitor::manyIntersectionOnTwoLine(Geom* geom, int triIndex) {
	Triangle * triangle = geom->triangles.at(triIndex);
	std::vector<IntersectionRes> intersection = triangle->intersectionRess;
	//需要重构三角网所有顶点集合
	vector<Vertex *> vertexs;
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[0])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[1])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[2])));

	vector<Point::IntersectPoint> intersectPoints;
	vector<int> interEdges;
	for (int tri_i = 0; tri_i < 3; tri_i++) {
		if (intersection[tri_i].pointNum > 0) {
			interEdges.push_back(tri_i);
		}
		for (int point_i = 0; point_i < intersection[tri_i].pointNum; point_i++) {
			Point::IntersectPoint point;
			point.coordinate = geom->sides.at(triangle->sideIndexs[tri_i])->intersections[point_i];
			point.edgeIndex = triangle->sideIndexs[tri_i];
			point.edge[0] = intersection[tri_i].indices[0];
			point.edge[1] = intersection[tri_i].indices[1];
			point.lineIndex = intersection[tri_i].lineIndex[point_i];
			point.intersectPro = intersection[tri_i].proportions[point_i];
			intersectPoints.push_back(point);
		}
	}

	
	if (interEdges.size() < 2) {
		moreIntersectionOnOneLine(geom, triIndex);
		return;
	}
	//which vertex is the intersection of edge1 and edge2 ; return 0;1;2 
	int intersectVertex = geom->triangles[triIndex]->whichVertex(intersection[interEdges[0]].indices[0], intersection[interEdges[0]].indices[1], intersection[interEdges[1]].indices[0], intersection[interEdges[1]].indices[1]);
	if (intersectVertex == -1) return;
	//vertex index in vertices
	int interInVertices = geom->triangles[triIndex]->vertexIndexs[intersectVertex];

	osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> constraintCoords = new osg::Vec3Array;
	coords->push_back(vertexs[0]->coor);
	coords->push_back(vertexs[1]->coor);
	coords->push_back(vertexs[2]->coor);

	//根据边界线顺序排序交点
	int minIndex;
	Point::IntersectPoint inter_temp;
	osg::Vec3 tempVec;
	for (int i = 0; i < intersectPoints.size(); i++) {
		minIndex = i;
		for (int j = i + 1; j < intersectPoints.size(); j++) {
			if (intersectPoints[j].lineIndex == intersectPoints[minIndex].lineIndex) {
				int nextIndex = intersectPoints[j].lineIndex + LINE_STEP < _lineVertices.size() ? intersectPoints[j].lineIndex + LINE_STEP : 0;
				osg::Vec3 direction1 =_lineVertices[nextIndex] - _lineVertices[intersectPoints[j].lineIndex];
				osg::Vec3 direction2 = intersectPoints[j].coordinate - intersectPoints[minIndex].coordinate;
				if (direction1*direction2 < 0) {
					minIndex = j;
				}
			}

			if (Utility::CustomizeSmall(intersectPoints[j].lineIndex, intersectPoints[minIndex].lineIndex, _lineVertices.size())) {     // 寻找最小的数
					minIndex = j;                 // 将最小数的索引保存	
			}
		}
		inter_temp = intersectPoints[i];
		intersectPoints[i] = intersectPoints[minIndex];
		intersectPoints[minIndex] = inter_temp;
	}

	Side * side_corner = geom->sides.at(triangle->sideIndexs[interEdges[0]]);
	//如果边的交点已经添加到了顶点数组中，则不用在顶点数组中再添加，只需要调用
	for (int i = 0; i < side_corner->intersections.size(); i++) {
		if (side_corner->intersectionIndexs.size()==i) {
			Vertex * vertex = new Vertex;
			vertex->coor = side_corner->intersections[i];
			vertex->index = geom->vertices.size();
			geom->vertices.push_back(vertex);
			//UV coordinates
			vertex->texCoor = CalculateUVCoordinates(vertexs, vertex->coor);
			vertexs.push_back(vertex);
			side_corner->intersectionIndexs.push_back( vertex->index);

		}
		else {
			vertexs.push_back(geom->vertices[side_corner->intersectionIndexs[i]]);
		}
	}
	Side * side_previous = geom->sides.at(triangle->sideIndexs[interEdges[1]]);
	//如果边的交点已经添加到了顶点数组中，则不用在顶点数组中再添加，只需要调用
	for (int i = 0; i < side_previous->intersections.size(); i++) {
		if (side_previous->intersectionIndexs.size()==i) {
			Vertex * vertex = new Vertex;
			vertex->coor = side_previous->intersections[i];
			vertex->index = geom->vertices.size();
			geom->vertices.push_back(vertex);
			//UV coordinates
			vertex->texCoor = CalculateUVCoordinates(vertexs, vertex->coor);
			vertexs.push_back(vertex);
			side_previous->intersectionIndexs.push_back(vertex->index);

		}
		else {
			vertexs.push_back(geom->vertices[side_previous->intersectionIndexs[i]]);
		}
	}

	int tempIndex;
	//在有多个交点的情况下，获取交点和拐点
	for (int i = 0; i < intersectPoints.size(); i++) {
		if (intersectPoints[i].coordinate != vertexs[0]->coor&&intersectPoints[i].coordinate != vertexs[1]->coor&&intersectPoints[i].coordinate != vertexs[2]->coor) {
			coords->push_back(intersectPoints[i].coordinate);
			constraintCoords->push_back(intersectPoints[i].coordinate);
		}
		if (i < intersectPoints.size() - 1) {
			if (intersectPoints[i].lineIndex != intersectPoints[i + 1].lineIndex) {
				osg::Vec3 turningPoint;
				osg::Vec3 projectionPoint;
				bool turning = true;
				int turnIndex = 0;
				tempIndex = intersectPoints[i].lineIndex;
				while (tempIndex != intersectPoints[i + 1].lineIndex && turning) {
					tempIndex = (tempIndex + LINE_STEP) < _lineVertices.size() ? (tempIndex + LINE_STEP) : 0;
					turningPoint = _lineVertices[tempIndex];
					bool isOntri = pointProjectOnTri(turningPoint, vertexs[0]->coor, vertexs[1]->coor, vertexs[2]->coor, projectionPoint);
					if (isOntri) {
						coords->push_back(projectionPoint);
						constraintCoords->push_back(projectionPoint);
						Vertex * vertex_middle = new Vertex;
						vertex_middle->coor = projectionPoint;
						vertex_middle->index = geom->vertices.size();
						vertex_middle->texCoor = CalculateUVCoordinates(vertexs, vertex_middle->coor);
						geom->vertices.push_back(vertex_middle);
						vertexs.push_back(vertex_middle);
						turnIndex++;
					}
					else {
						turning = false;
					}
				}
				if (turnIndex == 0) {
					int vertex_index = geom->triangles[triIndex]->whichVertex(intersectPoints[i].edge[0], intersectPoints[i].edge[1], intersectPoints[i + 1].edge[0], intersectPoints[i + 1].edge[1]);
					if (vertex_index != -1) {
						constraintCoords->push_back(vertexs[vertex_index]->coor);
					}
				}
				
			}
		}
	}	

	//for (int i = 0; i < constraintCoords->size() - 1; i++) {
	//	_group->addChild(DrawLine(constraintCoords->at(i), constraintCoords->at(i + 1), osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f)));
	//}

	vector<int> triang_indexes = DelaunayTriangulator(coords, vertexs[intersectVertex]->coor, constraintCoords);
	vector<int> contrastArray = ContrastIndex(vertexs, coords);
	Intersect intersect;
	int validVertex = interInVertices;
	int indexInTri = intersectVertex;
	int loopNum = 0;
	while (geom->vertices[validVertex]->topology == Topology::OnLine && loopNum < 3) {
		indexInTri = Utility::customizePlus(indexInTri);
		validVertex = geom->triangles[triIndex]->vertexIndexs[indexInTri];
		loopNum++;
	}
	if (loopNum == 3) return;
	if (geom->vertices[validVertex]->topology == Topology::Outline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (coords->at(triang_indexes[i * 3]) + coords->at(triang_indexes[i * 3 + 1]) + coords->at(triang_indexes[i * 3 + 2])) / 3;
			int intersect_Count=0, constraint_i = 0;
			if (constraintCoords->size() > 1) {
				for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
					if (intersect.LineLineIntersect(centroid, geom->vertices[validVertex]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1))==1)
						intersect_Count++;
				}
			}
			if (intersect_Count % 2 == 0) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
			}
		}
	}
	else if (geom->vertices[validVertex]->topology == Topology::Inline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (coords->at(triang_indexes[i * 3]) + coords->at(triang_indexes[i * 3 + 1]) + coords->at(triang_indexes[i * 3 + 2])) / 3;
			int intersect_Count = 0, constraint_i = 0;
			if (constraintCoords->size() > 1) {
				for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
					osg::Vec3 aaa;
					if (intersect.LineLineIntersect(centroid, geom->vertices[validVertex]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1))==1)
						intersect_Count++;
				}
			}
			if (intersect_Count % 2 == 1) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
			}
		}
	}

}

//The boundary line intersects the three sides of the triangle
void MainVisitor::manyIntersectionOnThreeLine(Geom* geom, int triIndex) {
	Triangle * triangle = geom->triangles.at(triIndex);
	std::vector<IntersectionRes> intersection = triangle->intersectionRess;
	//需要重构三角网所有顶点集合
	vector<Vertex *> vertexs;
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[0])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[1])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[2])));

	osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> constraintCoords = new osg::Vec3Array;
	coords->push_back(vertexs[0]->coor);
	coords->push_back(vertexs[1]->coor);
	coords->push_back(vertexs[2]->coor);

	vector<Point::IntersectPoint> intersectPoints;
	for (int tri_i = 0; tri_i < 3; tri_i++) {
		for (int point_i = 0; point_i < intersection[tri_i].pointNum; point_i++) {
			Point::IntersectPoint point;
			point.coordinate = geom->sides.at(triangle->sideIndexs[tri_i])->intersections[point_i];
			point.edge[0] = intersection[tri_i].indices[0];
			point.edge[1] = intersection[tri_i].indices[1];
			point.lineIndex = intersection[tri_i].lineIndex[point_i];
			intersectPoints.push_back(point);
		}
	}

	for (int side_i = 0; side_i < 3; side_i++) {
		Side * side_corner = geom->sides.at(triangle->sideIndexs[side_i]);
		//如果边的交点已经添加到了顶点数组中，则不用在顶点数组中再添加，只需要调用
		for (int i = 0; i < side_corner->intersections.size(); i++) {
			if (side_corner->intersectionIndexs.size() == i) {
				Vertex * vertex = new Vertex;
				vertex->coor = side_corner->intersections[i];
				vertex->index = geom->vertices.size();
				geom->vertices.push_back(vertex);
				//UV coordinates
				vertex->texCoor = CalculateUVCoordinates(vertexs, vertex->coor);
				vertexs.push_back(vertex);
				side_corner->intersectionIndexs.push_back(vertex->index);

			}
			else {
				vertexs.push_back(geom->vertices[side_corner->intersectionIndexs[i]]);
			}
		}
	}
	
	//根据边界线顺序排序交点
	int minIndex;
	Point::IntersectPoint inter_temp;
	osg::Vec3 tempVec;
	for (int i = 0; i < intersectPoints.size(); i++) {
		minIndex = i;
		for (int j = i + 1; j < intersectPoints.size(); j++) {
			if (intersectPoints[j].lineIndex == intersectPoints[minIndex].lineIndex) {
				int nextIndex = intersectPoints[j].lineIndex + LINE_STEP < _lineVertices.size() ? intersectPoints[j].lineIndex + LINE_STEP : 0;
				osg::Vec3 direction1 = _lineVertices[nextIndex] - _lineVertices[intersectPoints[j].lineIndex];
				osg::Vec3 direction2 = intersectPoints[j].coordinate - intersectPoints[minIndex].coordinate;
				if (direction1*direction2 < 0) {
					minIndex = j;
				}
			}

			if (Utility::CustomizeSmall(intersectPoints[j].lineIndex, intersectPoints[minIndex].lineIndex, _lineVertices.size())) {     // 寻找最小的数
				minIndex = j;                 // 将最小数的索引保存	
			}
		}
		inter_temp = intersectPoints[i];
		intersectPoints[i] = intersectPoints[minIndex];
		intersectPoints[minIndex] = inter_temp;

	}
	
	int temp_line_index;
	//在有多个交点的情况下，获取交点和拐点
	for (int i = 0; i < intersectPoints.size(); i++) {
		if (intersectPoints[i].coordinate != vertexs[0]->coor&&intersectPoints[i].coordinate != vertexs[1]->coor&&intersectPoints[i].coordinate != vertexs[2]->coor) {
			coords->push_back(intersectPoints[i].coordinate);
			constraintCoords->push_back(intersectPoints[i].coordinate);
		}
		if (i < intersectPoints.size() - 1) {
			if (intersectPoints[i].lineIndex != intersectPoints[i + 1].lineIndex) {
				osg::Vec3 turningPoint;
				osg::Vec3 projectionPoint;
				temp_line_index = intersectPoints[i].lineIndex;
				bool turning=true;
				int turnIndex = 0;
				while (temp_line_index != intersectPoints[i + 1].lineIndex && turning) {
					temp_line_index = (temp_line_index + LINE_STEP) < _lineVertices.size() ? (temp_line_index + LINE_STEP) : 0;
					turningPoint = _lineVertices[temp_line_index];
					bool isOntri = pointProjectOnTri(turningPoint, vertexs[0]->coor, vertexs[1]->coor, vertexs[2]->coor, projectionPoint);
					if (isOntri) {
						coords->push_back(projectionPoint);
						constraintCoords->push_back(projectionPoint);
						Vertex * vertex_middle = new Vertex;
						vertex_middle->coor = projectionPoint;
						vertex_middle->index = geom->vertices.size();
						vertex_middle->texCoor = CalculateUVCoordinates(vertexs, vertex_middle->coor);
						geom->vertices.push_back(vertex_middle);
						vertexs.push_back(vertex_middle);
						turnIndex++;
					}
					else {
						turning = false;
					}
				}
				if (turnIndex == 0) {
					int vertex_index = geom->triangles[triIndex]->whichVertex(intersectPoints[i].edge[0], intersectPoints[i].edge[1], intersectPoints[i + 1].edge[0], intersectPoints[i + 1].edge[1]);
					if (vertex_index != -1) {
						constraintCoords->push_back(vertexs[vertex_index]->coor);
					}
				}
				
			}
		}
	}
	if (constraintCoords->size() < 2) {
		for (int i = 0; i < intersectPoints.size(); i++) {
			constraintCoords->push_back(intersectPoints[i].coordinate);
			if (i < intersectPoints.size() - 1) {
				osg::Vec3 turningPoint;
				osg::Vec3 projectionPoint;
				temp_line_index = intersectPoints[i].lineIndex;
				if (intersectPoints[i].lineIndex != intersectPoints[i + 1].lineIndex) {
					temp_line_index = (temp_line_index + LINE_STEP) < _lineVertices.size() ? (temp_line_index + LINE_STEP) : 0;
					turningPoint = _lineVertices[temp_line_index];
					bool isOntri = pointProjectOnTri(turningPoint, vertexs[0]->coor, vertexs[1]->coor, vertexs[2]->coor, projectionPoint);
					if (turningPoint != projectionPoint) {
						constraintCoords->push_back(projectionPoint);
					}
				}
			}
		}
	}

	vector<int> triang_indexes = DelaunayTriangulator(coords, vertexs[0]->coor, constraintCoords);
	vector<int> contrastArray = ContrastIndex(vertexs, coords);
	Intersect intersect;

	//Exclude points on the boundary as judgment vertices
	int indexInTri = 0;
	int loopNum = 0;
	while (vertexs[indexInTri]->topology == Topology::OnLine && loopNum < 3) {
		indexInTri = Utility::customizePlus(indexInTri);
		loopNum++;
	}
	if (loopNum == 3) return;

	if (vertexs[indexInTri]->topology == Topology::Outline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (coords->at(triang_indexes[i * 3]) + coords->at(triang_indexes[i * 3 + 1]) + coords->at(triang_indexes[i * 3 + 2])) / 3;
			int intersect_Count = 0, constraint_i = 0;
			for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
				osg::Vec3 aaa;
				if (intersect.LineLineIntersect(centroid, vertexs[indexInTri]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1))==1)
					intersect_Count++;
			}
			if (intersect_Count % 2 == 0) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
			}
		}
	}
	else if (vertexs[indexInTri]->topology == Topology::Inline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (coords->at(triang_indexes[i * 3]) + coords->at(triang_indexes[i * 3 + 1]) + coords->at(triang_indexes[i * 3 + 2])) / 3;
			int intersect_Count = 0, constraint_i = 0;
			for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
				osg::Vec3 aaa;
				if (intersect.LineLineIntersect(centroid, vertexs[indexInTri]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1)) == 1)
					intersect_Count++;
			}
			if (intersect_Count % 2 == 1) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
			}
		}
	}
}

//There are two intersections, and they are on different boundaries. 
//But it is judged that the vertices of the triangle are all inside or outside the boundary
void MainVisitor::abnormalTwoIntersectionOnTwoLine(Geom* geom, int triIndex) {
	Triangle * triangle = geom->triangles.at(triIndex);
	std::vector<IntersectionRes> intersection = triangle->intersectionRess;
	//需要重构三角网所有顶点集合
	vector<Vertex *> vertexs;
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[0])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[1])));
	vertexs.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[2])));

	vector<Vertex *> vertexs1;
	vertexs1.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[0])));
	vertexs1.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[1])));
	vertexs1.push_back(geom->vertices.at((geom->triangles[triIndex]->vertexIndexs[2])));

	int linePoint[2];
	if (intersection[0].pointNum > 0) {
		linePoint[0] = 0;
		if (intersection[1].pointNum > 0) {
			linePoint[1] = 1;
		}
		else {
			linePoint[1] = 2;
		}
	}
	else {
		linePoint[0] = 1;
		linePoint[1] = 2;
	}

	//交点在顶点数组中的索引
	int cornerVertexIndex, nextVertexIndex;
	if (geom->sides[triangle->sideIndexs[linePoint[0]]]->vertexIndexs[0] == geom->sides[triangle->sideIndexs[linePoint[1]]]->vertexIndexs[0] ||
		geom->sides[triangle->sideIndexs[linePoint[0]]]->vertexIndexs[0] == geom->sides[triangle->sideIndexs[linePoint[1]]]->vertexIndexs[1]) {
		cornerVertexIndex = geom->sides[triangle->sideIndexs[linePoint[0]]]->vertexIndexs[0];
		nextVertexIndex = geom->sides[triangle->sideIndexs[linePoint[0]]]->vertexIndexs[1];
	}
	else {
		cornerVertexIndex = geom->sides[triangle->sideIndexs[linePoint[0]]]->vertexIndexs[1];
		nextVertexIndex = geom->sides[triangle->sideIndexs[linePoint[0]]]->vertexIndexs[0];
	}

	//*判断两条有交点的边的顶点索引
	//*0-1边和1-2边，则顶点索引为1（边以第一个顶点索引值代替）
	//*0-1边和2-0边，则顶点索引为0
	//*1-2边和2-0边，则顶点索引为2
	int cornerPoint = -1;
	if (intersection[0].pointNum > 0) {
		if (intersection[1].pointNum > 0) {
			cornerPoint = 1;
		}
		else {
			cornerPoint = 0;
		}
	}
	else {
		cornerPoint = 2;
	}

	//相交两点边的交点的下一个顶点和上一个顶点
	int corner_Previous_Point = Utility::customizePlus(cornerPoint, 2);
	//合并两条边，线索引
	vector<int> lineIndexTotal;
	lineIndexTotal.reserve(intersection.at(linePoint[0]).lineIndex.size() + intersection.at(linePoint[1]).lineIndex.size());
	lineIndexTotal.insert(lineIndexTotal.end(), intersection.at(linePoint[0]).lineIndex.begin(), intersection.at(linePoint[0]).lineIndex.end());
	lineIndexTotal.insert(lineIndexTotal.end(), intersection.at(linePoint[1]).lineIndex.begin(), intersection.at(linePoint[1]).lineIndex.end());
	//合并两条边的交点
	vector<osg::Vec3> intersectionsTotal;
	intersectionsTotal.reserve(geom->sides.at(triangle->sideIndexs[linePoint[0]])->intersections.size() + geom->sides.at(triangle->sideIndexs[linePoint[1]])->intersections.size());
	intersectionsTotal.insert(intersectionsTotal.end(), geom->sides.at(triangle->sideIndexs[linePoint[0]])->intersections.begin(), geom->sides.at(triangle->sideIndexs[linePoint[0]])->intersections.end());
	intersectionsTotal.insert(intersectionsTotal.end(), geom->sides.at(triangle->sideIndexs[linePoint[1]])->intersections.begin(), geom->sides.at(triangle->sideIndexs[linePoint[1]])->intersections.end());

	Side * side_corner = geom->sides.at(triangle->sideIndexs[linePoint[0]]);
	//如果边的交点已经添加到了顶点数组中，则不用在顶点数组中再添加，只需要调用
	if (side_corner->intersectionRes.proportions[0] != 0 && side_corner->intersectionRes.proportions[0] != 1) {
		if (side_corner->intersectionIndexs.size() == 0) {
			Vertex * vertex = new Vertex;
			vertex->coor = side_corner->intersections[0];
			vertex->index = geom->vertices.size();
			geom->vertices.push_back(vertex);
			//UV coordinates
			vertex->texCoor = CalculateUVCoordinates(vertexs1, vertex->coor);
			vertexs.push_back(vertex);
			side_corner->intersectionIndexs.push_back(vertex->index);

		}
		else {
			vertexs.push_back(geom->vertices[side_corner->intersectionIndexs[0]]);
		}
	}
	

	Side * side_back = geom->sides.at(triangle->sideIndexs[linePoint[1]]);
	if (side_back->intersectionRes.proportions[0] != 0 && side_back->intersectionRes.proportions[0] != 1) {
		if (side_back->intersectionIndexs.size() == 0) {
			Vertex * vertex = new Vertex;
			vertex->coor = side_back->intersections[0];
			vertex->index = geom->vertices.size();
			geom->vertices.push_back(vertex);
			vertex->texCoor = CalculateUVCoordinates(vertexs1, vertex->coor);
			vertexs.push_back(vertex);
			side_back->intersectionIndexs.push_back(vertex->index);
		}
		else {
			vertexs.push_back(geom->vertices[side_back->intersectionIndexs[0]]);
		}
	}

	//根据边界线顺序排序交点
	int minIndex, temp;
	osg::Vec3 tempVec;
	for (int i = 0; i < lineIndexTotal.size(); i++) {
		minIndex = i;
		for (int j = i + 1; j < lineIndexTotal.size(); j++) {
			if (lineIndexTotal[j] == lineIndexTotal[minIndex]) {
				int nextIndex = lineIndexTotal[j] + LINE_STEP < _lineVertices.size() ? lineIndexTotal[j] + LINE_STEP : 0;
				osg::Vec3 direction1 = _lineVertices[nextIndex] - _lineVertices[lineIndexTotal[j]];
				osg::Vec3 direction2 = intersectionsTotal[j] - intersectionsTotal[minIndex];
				if (direction1*direction2 < 0) {
					minIndex = j;
				}
			}

			if (Utility::CustomizeSmall(lineIndexTotal[j], lineIndexTotal[minIndex], _lineVertices.size())) {     // 寻找最小的数
				minIndex = j;                 // 将最小数的索引保存	
			}
		}
		temp = lineIndexTotal[i];
		lineIndexTotal[i] = lineIndexTotal[minIndex];
		lineIndexTotal[minIndex] = temp;

		tempVec = intersectionsTotal[i];
		intersectionsTotal[i] = intersectionsTotal[minIndex];
		intersectionsTotal[minIndex] = tempVec;
	}
	osg::ref_ptr<osg::Vec3Array> coords = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> constraintCoords = new osg::Vec3Array;
	coords->push_back(vertexs[0]->coor);
	coords->push_back(vertexs[1]->coor);
	coords->push_back(vertexs[2]->coor);
	int tempIndex;

	//在有多个交点的情况下，获取交点和拐点
	for (int i = 0; i < lineIndexTotal.size(); i++) {
		if (intersectionsTotal[i] != vertexs[0]->coor&&intersectionsTotal[i] != vertexs[1]->coor&&intersectionsTotal[i] != vertexs[2]->coor)
			coords->push_back(intersectionsTotal[i]);
		constraintCoords->push_back(intersectionsTotal[i]);
		if (i < lineIndexTotal.size() - 1) {
			osg::Vec3 turningPoint;
			osg::Vec3 projectionPoint;
			tempIndex = lineIndexTotal[i];
			while (tempIndex != lineIndexTotal[i + 1])
			{
				tempIndex = (tempIndex + LINE_STEP) < _lineVertices.size() ? (tempIndex + LINE_STEP) : 0;
				turningPoint = _lineVertices[tempIndex];
				bool isOntri = pointProjectOnTri(turningPoint, vertexs[0]->coor, vertexs[1]->coor, vertexs[2]->coor, projectionPoint);
				if (isOntri) {
					coords->push_back(projectionPoint);
					constraintCoords->push_back(projectionPoint);
					Vertex * vertex_middle = new Vertex;
					vertex_middle->coor = projectionPoint;
					vertex_middle->index = geom->vertices.size();
					vertex_middle->texCoor = CalculateUVCoordinates(vertexs, vertex_middle->coor);
					geom->vertices.push_back(vertex_middle);
					vertexs.push_back(vertex_middle);
				}

			};
		}
	}

	/*for (int i = 0; i < constraintCoords->size()-1; i++) {
		_group->addChild(DrawLine(constraintCoords->at(i), constraintCoords->at(i + 1), osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f)));
	}*/
	int vNum = vertexs.size();
	int cNum = coords->size();
	vector<int> triang_indexes = DelaunayTriangulator(coords, vertexs[cornerPoint]->coor, constraintCoords);
	vector<int> contrastArray = ContrastIndex(vertexs, coords);
	Intersect intersect;
	if (geom->vertices[cornerVertexIndex]->topology == Topology::Inline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (vertexs[contrastArray[triang_indexes[i * 3]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor) / 3;
			int intersect_Count = 0, constraint_i = 0;
			for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
				osg::Vec3 aaa;
				int result = intersect.LineLineIntersect(centroid, geom->vertices[cornerVertexIndex]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1));
				if (result)
					intersect_Count += result;
			}
			if (intersect_Count % 2 == 0) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
				/*_group->addChild(DrawTriangle(vertexs[contrastArray[triang_indexes[i * 3]]]->coor, vertexs[contrastArray[triang_indexes[i * 3+1]]]->coor,
					vertexs[contrastArray[triang_indexes[i * 3+2]]]->coor, osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.95));*/
			}
		}
	}
	else if (geom->vertices[cornerVertexIndex]->topology == Topology::Outline) {
		for (int i = 0; i < triang_indexes.size() / 3; i++) {
			osg::Vec3 centroid = (vertexs[contrastArray[triang_indexes[i * 3]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor) / 3;
			int intersect_Count = 0, constraint_i = 0;
			for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
				osg::Vec3 aaa;
				int result = intersect.LineLineIntersect(centroid, geom->vertices[cornerVertexIndex]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1));
				if (result)
					intersect_Count += result;
			}
			if (intersect_Count % 2 == 1) {
				Triangle * triangle = new Triangle;
				triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
				triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
				triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
				geom->triangles.push_back(triangle);
				/*_group->addChild(DrawTriangle(vertexs[contrastArray[triang_indexes[i * 3]]]->coor, vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor,
					vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor, osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.95));*/
			}
		}

	}
	else {
		if (geom->vertices[nextVertexIndex]->topology == Topology::Inline) {
			for (int i = 0; i < triang_indexes.size() / 3; i++) {
				osg::Vec3 centroid = (vertexs[contrastArray[triang_indexes[i * 3]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor) / 3;
				int intersect_Count = 0, constraint_i = 0;
				for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
					osg::Vec3 aaa;
					int result = intersect.LineLineIntersect(centroid, geom->vertices[nextVertexIndex]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1));
					if (result)
						intersect_Count += result;
				}
				if (intersect_Count % 2 == 0) {
					Triangle * triangle = new Triangle;
					triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
					triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
					triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
					geom->triangles.push_back(triangle);
					/*_group->addChild(DrawTriangle(vertexs[contrastArray[triang_indexes[i * 3]]]->coor, vertexs[contrastArray[triang_indexes[i * 3+1]]]->coor,
						vertexs[contrastArray[triang_indexes[i * 3+2]]]->coor, osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.95));*/
				}
			}
		}
		else if (geom->vertices[nextVertexIndex]->topology == Topology::Outline) {
			for (int i = 0; i < triang_indexes.size() / 3; i++) {
				osg::Vec3 centroid = (vertexs[contrastArray[triang_indexes[i * 3]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor + vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor) / 3;
				int intersect_Count = 0, constraint_i = 0;
				for (; constraint_i < constraintCoords->size() - 1; constraint_i++) {
					osg::Vec3 aaa;
					int result = intersect.LineLineIntersect(centroid, geom->vertices[nextVertexIndex]->coor, constraintCoords->at(constraint_i), constraintCoords->at(constraint_i + 1));
					if (result)
						intersect_Count += result;
				}
				if (intersect_Count % 2 == 1) {
					Triangle * triangle = new Triangle;
					triangle->vertexIndexs[0] = vertexs[contrastArray[triang_indexes[i * 3]]]->index;
					triangle->vertexIndexs[1] = vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->index;
					triangle->vertexIndexs[2] = vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->index;
					geom->triangles.push_back(triangle);
					/*_group->addChild(DrawTriangle(vertexs[contrastArray[triang_indexes[i * 3]]]->coor, vertexs[contrastArray[triang_indexes[i * 3 + 1]]]->coor,
						vertexs[contrastArray[triang_indexes[i * 3 + 2]]]->coor, osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.95));*/
				}
			}

		}
		else {
			int debug = 0;
		}
	}

}

//Map the intersection to the triangulation
bool MainVisitor::pointProjectOnTri(osg::Vec3 middleP, osg::Vec3 vertex0, osg::Vec3 vertex1, osg::Vec3 vertex2, osg::Vec3& projectP){

	osg::Vec3 direction = middleP - _eye; //!!!!!替换eye坐标
	return _inter.IsIntersect(middleP, direction, vertex0, vertex1, vertex2, projectP);
}

std::vector<int> MainVisitor::Negate(std::vector<int> vertexStatus) {
	std::vector<int> negate;
	for (int i = 0; i < 3; i++) {
		bool isEqual = false;
		for (int j = 0; j < vertexStatus.size(); j++) {
			if (i == vertexStatus[j]) {
				isEqual = true;
				break;
			}
		}
		if (!isEqual)
			negate.push_back(i);
	}
	return negate;
}

//set lines stream
void MainVisitor::setLines(vector<osg::Vec3> lineVertices) {
	_lineVertices = lineVertices;
}

//筛选边界的三角网
void MainVisitor::filterTriangle(osg::Node* node) {
	Intersect inter;
	osg::Matrix matParent;
	// 设置矩阵的所有16 个值：
	inter.SetLines(node, matParent);
	_group->addChild(inter._group);
	if (allGeom.size() > 0 && allGeom.size() == 1)
	{
		for (size_t i = 0; i < allGeom[0]->triangles.size(); i++) {
			osg::Vec3 point0 = allGeom[0]->vertices.at(allGeom[0]->triangles.at(i)->vertexIndexs[0])->coor;
			osg::Vec3 point1 = allGeom[0]->vertices.at(allGeom[0]->triangles.at(i)->vertexIndexs[1])->coor;
			osg::Vec3 point2 = allGeom[0]->vertices.at(allGeom[0]->triangles.at(i)->vertexIndexs[2])->coor;
			IntersectRes res[3];
			res[0] = inter.LineLineIntersect1(point0, point1);
			res[1] = inter.LineLineIntersect1(point0, point2);
			res[2] = inter.LineLineIntersect1(point1, point2);
			if (res[0].isIntersect || res[1].isIntersect || res[2].isIntersect) {
				allGeom[0]->borderTriangles.push_back(i);
				allGeom[0]->choicedTriangles.push_back(i);
				allGeom[0]->triangles.at(i)->isBorder = true;
				allGeom[0]->triangles.at(i)->isChioced = true;
				for (int j = 0; j < 3; j++) {
					allGeom[0]->triangles.at(i)->intersectPoints.push_back(res[j].intersectPoint);
				}
				_group->addChild(DrawTriangle(point0, point1, point2, osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f)));

				//验证只有一条边有交点的三角网
				//if ((res[0].lineIndex + res[1].lineIndex == -2) ||
				//	(res[0].lineIndex + res[2].lineIndex == -2) ||
				//	(res[2].lineIndex + res[1].lineIndex == -2)) {
				//	/*std::cout << res[0].lineIndex << "," << res[1].lineIndex << "," << res[2].lineIndex << std::endl;
				//	_group->addChild(DrawTriangle(point0, point1, point2));*/
				//	int num;
				//	if (res[0].lineIndex != -1) {
				//		num = inter.LineLineIntersectTwice(point0, point1);
				//		_points->addChild(DrawPoint(res[0].intersectPoint));
				//	}
				//	else if (res[1].lineIndex != -1) {
				//		num = inter.LineLineIntersectTwice(point0, point2);
				//		_points->addChild(DrawPoint(res[1].intersectPoint));
				//	}
				//	else {
				//		num = inter.LineLineIntersectTwice(point1, point2);
				//		_points->addChild(DrawPoint(res[2].intersectPoint));
				//	}
				//	std::cout << res[0].lineIndex << "," << res[1].lineIndex << "," << res[2].lineIndex << "," << num << std::endl;
				//}

				/*if (res[0].lineIndex *  res[1].lineIndex * res[2].lineIndex < 0) {
					_group->addChild(DrawTriangle(point0, point1, point2, osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f)));
				}*/

			}
		}
	}
	else
	{
		for (Geom* geom : allGeom) {
			for (size_t i = 0; i < geom->triangles.size(); i++) {
				osg::Vec3 point0 = geom->vertices.at(geom->triangles.at(i)->vertexIndexs[0])->coor;
				osg::Vec3 point1 = geom->vertices.at(geom->triangles.at(i)->vertexIndexs[1])->coor;
				osg::Vec3 point2 = geom->vertices.at(geom->triangles.at(i)->vertexIndexs[2])->coor;
				IntersectRes res[3];
				res[0] = inter.LineLineIntersect1(point0, point1);
				res[1] = inter.LineLineIntersect1(point0, point2);
				res[2] = inter.LineLineIntersect1(point1, point2);
				if (res[0].isIntersect || res[1].isIntersect || res[2].isIntersect) {
					geom->borderTriangles.push_back(i);
					geom->choicedTriangles.push_back(i);
					geom->triangles.at(i)->isBorder = true;
					geom->triangles.at(i)->isChioced = true;
					for (int j = 0; j < 3; j++) {
						geom->triangles.at(i)->intersectPoints.push_back(res[j].intersectPoint);
					}
					_group->addChild(DrawTriangle(point0, point1, point2, osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f)));
				}
			}
		}

	}
}

void MainVisitor::setLineBoundary(BOUNDARY::Boundary * boundary) {
	_boundary = boundary;
}

void MainVisitor::setLineQuadTree(std::vector<Quadtree::QuadTreeNode *> quadnodeList) {
	_quadnodeList = quadnodeList;
}

int MainVisitor::getTriangleNum() {
	int i = 0;
	if (allGeom.size() > 0 && allGeom.size() == 1)
	{
		i = i + allGeom[0]->triangles.size();
	}
	else {
		for (Geom* geom : allGeom) {
			i = i + geom->triangles.size();
		}
	}
	return i;
}