#include "HitCheck.h"

HitCheck::HitCheck() {

}

HitCheck::~HitCheck() {

}

bool HitCheck::Check(osg::Node *pLine,osg::Node *node) {
	if (pLine)
	{
		// 获取包围盒
	/*	osg::MatrixTransform* rotateMT = new osg::MatrixTransform;
		rotateMT->setMatrix(
			osg::Matrix::translate(-507507, -4320220, 0));
		rotateMT->addChild(pLine);*/
		osg::ComputeBoundsVisitor boundvisitor;
		pLine->accept(boundvisitor);
		osg::BoundingBox& bb1 = boundvisitor.getBoundingBox();
		osg::ref_ptr<osgUtil::IntersectVisitor> iv = new osgUtil::IntersectVisitor();
		iv->setLODSelectionMode(osgUtil::IntersectVisitor::LODSelectionMode::USE_SEGMENT_START_POINT_AS_EYE_POINT_FOR_LOD_LEVEL_SELECTION);
		osg::Matrix matParent;
		matParent.set(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			-507507, -4320220, 0, 1);
		// 设置矩阵的所有16 个值：
		GetAllLineSegment(pLine, iv, matParent);
		unsigned int j = 0;
		if (node)
		{
			osg::ComputeBoundsVisitor boundvisitor2;
			node->accept(boundvisitor2);
			osg::BoundingBox& bb2 = boundvisitor2.getBoundingBox();
			if (bb1.intersects(bb2))
			{
				// 包围盒有相交
				// 判断两个第一个节点的所有线是否和第二个节点有相交
				node->accept(*iv);
				if (iv->hits())
				{
					/*osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(node);
					std::string path = plod->getDatabasePath();
					unsigned int num = plod->getNumChildren();*/
					return true;
				}
				return false;
			}
			return false;
		}
		return false;
	}
	return false;
}

bool HitCheck::BoundingCheck(osg::Node *pLine, osg::Node *node) {
	if (pLine)
	{
		// 获取包围盒
		osg::MatrixTransform* rotateMT = new osg::MatrixTransform;
		rotateMT->setMatrix(
			osg::Matrix::translate(0, 0, 0));
		rotateMT->addChild(pLine);
		osg::ComputeBoundsVisitor boundvisitor;
		pLine->accept(boundvisitor);
		osg::BoundingBox& bb1 = boundvisitor.getBoundingBox();
		if (node)
		{
			osg::ComputeBoundsVisitor boundvisitor2;
			node->accept(boundvisitor2);
			osg::BoundingBox& bb2 = boundvisitor2.getBoundingBox();
			if (bb1.intersects(bb2))
			{
				// 包围盒有相交
				return true;
			}
			return false;
		}
		return false;
	}
	return false;
}



void HitCheck::GetAllLineSegment(osg::Geometry *pGeometry, osgUtil::IntersectVisitor *pIv, osg::Matrixd &matParent) {
	if (pGeometry == NULL) {
		return;
	}
	osg::ref_ptr<osg::Vec3Array> vertices = dynamic_cast<osg::Vec3Array*>(pGeometry->getVertexArray());
	if (vertices.valid()) {
		unsigned int uPriNum = pGeometry->getNumPrimitiveSets();
		for (unsigned int i = 0; i < uPriNum; i++) {
			const osg::PrimitiveSet *pPriSet = pGeometry->getPrimitiveSet(i);
			for (unsigned int j = 0; j < pPriSet->getNumIndices() - 1; j++) {
				unsigned index1 = pPriSet->index(j);
				unsigned index2 = pPriSet->index(j + 1);
				osg::ref_ptr<osg::LineSegment> ls = new osg::LineSegment();
				ls->set(vertices->at(index1)*matParent, vertices->at(index2)*matParent);
				pIv->addLineSegment(ls);
			}
		}
	}
}

void HitCheck::GetAllLineSegment(osg::Node *pNode, osgUtil::IntersectVisitor *pIv, osg::Matrixd &matParent) {
	osg::Geode * pGeode = dynamic_cast<osg::Geode*>(pNode);
	if (pGeode) {
		unsigned int iDrawNum = pGeode->getNumDrawables();
		for (unsigned int i = 0; i < iDrawNum; i++) {
			osg::Drawable *pDrawble = pGeode->getDrawable(i);
			GetAllLineSegment(pDrawble->asGeometry(), pIv, matParent);
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
				GetAllLineSegment(pChild, pIv, matParent);
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
					GetAllLineSegment(pChild, pIv, matParent);
				}
			}
		}
	}
}