#include "stdafx.h"
#include "Geom.h"
#include <iostream>
#include <queue>
#include "Utility.h"
#include <osg/Geometry>
#include <osgUtil/DelaunayTriangulator> 
using namespace std;
using Common::Utility;

Geom::Geom()
{
}

Geom::~Geom()
{
	for (Vertex* vertex : vertices)
		delete vertex;
	for (Triangle* triangle : triangles)
		delete triangle;
	for (Vertex* vertex : newVertices)
		delete vertex;
	for (Triangle* triangle : newTriangles)
		delete triangle;
	for (Side* side : sides)
		delete side;
}

/**
 * �ж������������Ƿ�����
 */
bool Geom::isTwoTriangleNeighbor(int triangle1Index, int triangle2Index, int & side0, int& side1)
{

	osg::Vec3 pnt11 = vertices[triangles[triangle1Index]->vertexIndexs[0]]->coor;
	osg::Vec3 pnt12 = vertices[triangles[triangle1Index]->vertexIndexs[1]]->coor;
	osg::Vec3 pnt13 = vertices[triangles[triangle1Index]->vertexIndexs[2]]->coor;

	osg::Vec3 pnt21 = vertices[triangles[triangle2Index]->vertexIndexs[0]]->coor;
	osg::Vec3 pnt22 = vertices[triangles[triangle2Index]->vertexIndexs[1]]->coor;
	osg::Vec3 pnt23 = vertices[triangles[triangle2Index]->vertexIndexs[2]]->coor;

	bool pnt0 = Utility::isVec3Same(pnt11, pnt21) || Utility::isVec3Same(pnt11, pnt22) || Utility::isVec3Same(pnt11, pnt23);
	bool pnt1 = Utility::isVec3Same(pnt12, pnt21) || Utility::isVec3Same(pnt12, pnt22) || Utility::isVec3Same(pnt12, pnt23);
	bool pnt2 = Utility::isVec3Same(pnt13, pnt21) || Utility::isVec3Same(pnt13, pnt22) || Utility::isVec3Same(pnt13, pnt23);

	if (pnt0&&pnt1) {
		side0 = 0, side1 = 1;
		return true;
	}
	if (pnt1&&pnt2) {
		side0 = 1, side1 = 2;
		return true;
	}
	if (pnt0&&pnt2) {
		side0 = 0, side1 = 2;
		return true;
	}
	return false;

	/*int pnt1 = Utility::isVec3Same(pnt11, pnt21) || Utility::isVec3Same(pnt11, pnt22) || Utility::isVec3Same(pnt11, pnt23);
	int pnt2 = Utility::isVec3Same(pnt12, pnt21) || Utility::isVec3Same(pnt12, pnt22) || Utility::isVec3Same(pnt12, pnt23);
	int pnt3 = Utility::isVec3Same(pnt13, pnt21) || Utility::isVec3Same(pnt13, pnt22) || Utility::isVec3Same(pnt13, pnt23);

	if (pnt1 + pnt2 + pnt3 == 2)
		return true;
	return false;*/
	

}

/**
 * �������γɵıߺ��������Ƿ�����
 */
bool Geom::isSideTriangleNeighbor(int sideIndex1, int sideIndex2, int triangleIndex) {
	Triangle* triangle = triangles[triangleIndex];

	return (vertices[sideIndex1]->coor == vertices[triangle->vertexIndexs[0]]->coor || vertices[sideIndex1]->coor == vertices[triangle->vertexIndexs[1]]->coor || vertices[sideIndex1]->coor == vertices[triangle->vertexIndexs[2]]->coor) &&
		(vertices[sideIndex2]->coor == vertices[triangle->vertexIndexs[0]]->coor || vertices[sideIndex2]->coor == vertices[triangle->vertexIndexs[1]]->coor || vertices[sideIndex2]->coor == vertices[triangle->vertexIndexs[2]]->coor);
}

/**
 * ����ģ�͵�������֮������ˣ�ͨ���������˹�ϵ��
 * ��ͨ����������˹�ϵ��һ���������ڵ��������������������������ڵ�������������
 * ���ǲ��ܱ�֤�������Ķ��������У����ж�������겻��ͬ��һ��������ͬ������ͻᵼ�����˹�ϵ����©������ڱ���������������
 * ֮�������������������θ���С���������������໥�����Ƿ����ڣ��ﵽ����©���˹�ϵ��Ŀ�ġ�
 */
void Geom::createTriangleTopo()
{
	createVertexTopo();
	vector<int> unfullNeighborTriangle;

	for (size_t i = 0; i < triangles.size(); ++i)
	{
		Triangle* trianglei = triangles[i];
		for (size_t j = 0; j < 2; ++j)
		{
			for (size_t borderIndex : vertices.at(triangles.at(i)->vertexIndexs[j])->neighborTriangle) {
				if (i == borderIndex || trianglei->neighborTriangles.size() == 3)
					continue;
				if (!Utility::is_element_in_vector(trianglei->neighborTriangles, borderIndex)) {
					int side0, side1;
					bool topologic = isTwoTriangleNeighbor(i, borderIndex, side0, side1);
					if (topologic) {
						trianglei->neighborTriangles.push_back(borderIndex);
						triangles[borderIndex]->neighborTriangles.push_back(i);

						//���������˹�ϵ
						Side * side = new Side;
						side->vertexIndexs[0] = trianglei->vertexIndexs[side0];
						side->vertexIndexs[1] = trianglei->vertexIndexs[side1];
						side->index = sides.size();
						sides.push_back(side);
						side->neighborTriangles.push_back(i);
						side->neighborTriangles.push_back(borderIndex);
						trianglei->sideIndexs.push_back(side->index);
						triangles[borderIndex]->sideIndexs.push_back(side->index);
					}
				}
			}
		}
		if (trianglei->neighborTriangles.size() < 3)
			unfullNeighborTriangle.push_back(i);
	}
	for (size_t i = 0; i < unfullNeighborTriangle.size(); i++)
	{
		int index_i = unfullNeighborTriangle[i];
		Triangle* trianglei = triangles[index_i];
		if (trianglei->neighborTriangles.size() == 3)
			continue;
		if (trianglei->sideIndexs.size() < 3) {
			//���������˹�ϵ
			for (int i = 0; i < 3; i++) {
				Side * side = new Side;
				side->vertexIndexs[0] = trianglei->vertexIndexs[i];
				side->vertexIndexs[1] = trianglei->vertexIndexs[Utility::customizePlus(i)];
				bool sideExist = false;
				for (int side_i : trianglei->sideIndexs) {
					if (side->IsSame(*sides.at(side_i))) {
						sideExist = true;
						break;
					}
				}
				if (sideExist)
					delete side;
				else {
					side->index = sides.size();
					sides.push_back(side);
					trianglei->sideIndexs.push_back(side->index);
				}

			}

		}

	}
#if 0
	for (size_t i = 0; i < triangles.size(); ++i)
	{
		Triangle* trianglei = triangles[i];
		bool borderOneTri = true;
		for (size_t j = 0; j < 3; j++)
		{
			int vertexFirst = trianglei->vertexIndexs[j];
			int vertexSecond = trianglei->vertexIndexs[Utility::customizePlus(j)];
			for (size_t borderIndex : vertices.at(trianglei->vertexIndexs[j])->neighborTriangle) {
				if (i == borderIndex || trianglei->neighborTriangles.size() == 3)
					continue;
				if (!Utility::is_element_in_vector(trianglei->neighborTriangles, borderIndex)) {
					if (isSideTriangleNeighbor(vertexFirst, vertexSecond, borderIndex)) {
						trianglei->neighborTriangles.push_back(borderIndex);
						triangles[borderIndex]->neighborTriangles.push_back(i);
						//���������˹�ϵ
						Side * side = new Side;
						side->vertexIndexs[0] = vertexFirst;
						side->vertexIndexs[1] = vertexSecond;
						side->index = sides.size();
						sides.push_back(side);
						side->neighborTriangles.push_back(i);
						side->neighborTriangles.push_back(borderIndex);
						trianglei->sideIndexs.push_back(side->index);
						triangles[borderIndex]->sideIndexs.push_back(side->index);
						borderOneTri = false;
						break;
					}
				}
				else {
					if (isSideTriangleNeighbor(vertexFirst, vertexSecond, borderIndex)) {
						borderOneTri = false;
						break;
					}
					else {
						continue;
					}
				}
			}
			if (borderOneTri) {
				Side * side = new Side;
				side->vertexIndexs[0] = vertexFirst;
				side->vertexIndexs[1] = vertexSecond;
				side->index = sides.size();
				sides.push_back(side);
				trianglei->sideIndexs.push_back(side->index);
			}
			borderOneTri = true;
		}
	}
	
#endif // 0
}

/**
 * ��������֮�������
 */
void Geom::createVertexTopo()
{
	//����Χ��������
	for (size_t i = 0; i < triangles.size(); ++i)
	{
		Vertex *vertex1 = (Vertex*)vertices.at(triangles.at(i)->vertexIndexs[0]);
		vertex1->neighborTriangle.push_back(i);
		
		Vertex *vertex2 = (Vertex*)vertices.at(triangles.at(i)->vertexIndexs[1]);
		vertex2->neighborTriangle.push_back(i);
		
		Vertex *vertex3 = (Vertex*)vertices.at(triangles.at(i)->vertexIndexs[2]);
		vertex3->neighborTriangle.push_back(i);
		
		//cout << "	����Χ�������Σ�" << int(i*1.0 / triangles.size() * 100) << "%\r";
	}

}

/**
 * ��Geom�е����ݴ�����osg�ڵ�
 */
#if 0
osg::ref_ptr<osg::Geode> Geom::createOsgNode(osg::Vec4 color)
{
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	//���㡢������
	osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec2Array> vt = new osg::Vec2Array;
	for (Vertex* vertex : vertices)
	{
		vertexArray->push_back(vertex->coor);
		normalArray->push_back(vertex->normal);
		vt->push_back(vertex->texCoor);
	}
	//��ɫ
	//����
	osg::ref_ptr<osg::DrawElementsUInt> indexs = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	for (Triangle* triangle : triangles)
	{
		if (triangle->isBorder) continue;
		if (triangle->isInline) 
			continue;
		indexs->push_back(triangle->vertexIndexs[0]);
		indexs->push_back(triangle->vertexIndexs[1]);
		indexs->push_back(triangle->vertexIndexs[2]);
		
		
	}
	geometry->setVertexArray(vertexArray);
	geometry->setTexCoordArray(0, vt.get());
	std::cout << vt->size() <<","<< vertexArray->size() << endl;
	geometry->setNormalArray(normalArray, osg::Array::BIND_PER_VERTEX);
	geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	
	
	geometry->addPrimitiveSet(indexs);
	geode->addDrawable(geometry);
	return geode;
}
#endif // 0

/*������û����ӽ�ȥ����Ϊԭʼ���ļ���û�з�����*/
osg::ref_ptr<osg::Geode> Geom::createOsgNode(osg::Vec4 color)
{
	rebuildOutsideTriangle();
	osg::ref_ptr<osg::Geode> geode = new osg::Geode;
	osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
	//���㡢������
	osg::ref_ptr<osg::Vec3Array> vertexArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec3Array> normalArray = new osg::Vec3Array;
	osg::ref_ptr<osg::Vec2Array> vt = new osg::Vec2Array;
	for (Vertex* vertex : newVertices)
	//for (Vertex* vertex : vertices)
	{
		vertexArray->push_back(vertex->coor);
		
		//normalArray->push_back(vertex->normal);
		vt->push_back(vertex->texCoor);
	}
	//��ɫ
	//����
	osg::ref_ptr<osg::DrawElementsUInt> indexs = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	for (Triangle* triangle : newTriangles)
	//for (Triangle* triangle : triangles)
	{
		indexs->push_back(triangle->vertexIndexs[0]);
		indexs->push_back(triangle->vertexIndexs[1]);
		indexs->push_back(triangle->vertexIndexs[2]);
	}
	geometry->setVertexArray(vertexArray);
	geometry->setTexCoordArray(0, vt.get());
	//geometry->setNormalArray(normalArray, osg::Array::BIND_PER_VERTEX);
	//geometry->setNormalBinding(osg::Geometry::BIND_OVERALL);
	geometry->setDataVariance(osg::Object::STATIC);

	geometry->addPrimitiveSet(indexs);
	geode->addDrawable(geometry);
	return geode;
}

/**
 * ��Geom�еĶ�����������ع�
 */
void Geom::rebuildOutsideTriangle() {
	int newVertexIndex = 0;
	for (Triangle* triangle : triangles) {
		if (!triangle->isBorder && !triangle->isInline) {
			Triangle * newTriangle = new Triangle;
			for (int i = 0; i < 3;i++) {
				//����������û�б���ӹ�,��ô������������Ϣ���Ƶ��µĶ������飬��ԭ�����������Ӧ�Ķ�������Ϊ�Ѿ�����ӹ��ұ������ڶ��������
				int vertexIndex = triangle->vertexIndexs[i];
				if (vertices[vertexIndex]->newIndex==-1) {
					Vertex *vertex = new Vertex;
					vertex->coor = vertices[vertexIndex]->coor;
					vertex->normal = vertices[vertexIndex]->normal;
					vertex->texCoor = vertices[vertexIndex]->texCoor;
					vertex->index = vertexIndex;
					vertices[vertexIndex]->newIndex = newVertexIndex;
					newTriangle->vertexIndexs[i] = newVertexIndex;
					newVertexIndex++;
					newVertices.push_back(vertex);
				}
				//����Ѿ�����ӹ���������ֻ��Ҫ���������
				else {
					newTriangle->vertexIndexs[i] = vertices[vertexIndex]->newIndex;
				}
			}
			//���㷨����
			osg::Vec3 edge1 = newVertices.at(newTriangle->vertexIndexs[1])->coor - newVertices.at(newTriangle->vertexIndexs[0])->coor;
			osg::Vec3 edge2 = newVertices.at(newTriangle->vertexIndexs[2])->coor - newVertices.at(newTriangle->vertexIndexs[0])->coor;
			osg::Vec3 triangleNormal = edge1 ^ edge2;
			triangleNormal.normalize();
			newTriangle->normal = triangleNormal;
			newTriangles.push_back(newTriangle);
		}
	}
	
}

//Reorder triangles with toop
void Geom::reorderTriangle() {
	 
}