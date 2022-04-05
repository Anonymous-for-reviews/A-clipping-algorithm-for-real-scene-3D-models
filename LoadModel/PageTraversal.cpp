#include <stdlib.h>
#include <crtdbg.h>
#include "PageTraversal.h"
#include "MainVisiter.h"
PageTraversal::PageTraversal() {
	num = 0;
	geodeNum = 0;
}

PageTraversal::~PageTraversal() {

}

void PageTraversal::Traversal(osg::Node* node, osg::Node* pLine , osgViewer::Viewer *viewer) {
	_pLine = pLine;
	_viewer = viewer;
	_projectArray = Common::Utility::worldToWindowArray(pLine, _viewer);
	_lineVertices = Common::Utility::setLines(pLine);
	Traversal(node);
}

void PageTraversal::setPath(string path) {
	_path = path;
}

void PageTraversal::setDirectory(string path) {
	osgDB::makeDirectory(path);
	_directory = path;
}
 
void PageTraversal::Traversal(osg::Node* node) {
	if (node == NULL) {
		return;
	}
	osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(node);
	if (plod) {
		unsigned int fileNames = plod->getNumFileNames();
		if (fileNames == 2) {
			std::string fileName = plod->getFileName(1);
			std::string filePath = plod->getDatabasePath();
			HitCheck hitcheck;
			bool result = hitcheck.BoundingCheck(_pLine, plod);
			if (result) {
				CullOSGB cullOSGB;
				cullOSGB.setLine(_projectArray);
				cullOSGB.setLine3D(_lineVertices);
				cullOSGB.setViewer(_viewer);
				osg::ref_ptr<osg::PagedLOD> pagelod = cullOSGB.cropModel(plod, _boundary, _quadnodeList);
				string path = _path;
				_path = filePath + fileName;
				string name = pagelod->getName();
				osg::ref_ptr<osg::Node> childNode = osgDB::readNodeFile(_path);
				Traversal(childNode);
				
				if (pagelod->getName() != "null") {
					saveAsOSGB(pagelod, path);
				}
				
				num++;
				return;
			}
			else {
				string path = _path;
				_path = filePath + fileName;
				osg::ref_ptr<osg::Node> childNode = osgDB::readNodeFile(_path);
				Traversal(childNode);
				string simpleFilename = osgDB::getSimpleFileName(path);
				osgDB::copyFile(path, _directory + simpleFilename);
				return;
			}
		}
	}
	osg::Geode *geode = node->asGeode();
	if (geode) {
		HitCheck hitcheck;
		bool result = hitcheck.BoundingCheck(_pLine, geode);
		if (result) {
			CullOSGB cullOSGB;
			cullOSGB.setLine(_projectArray);
			cullOSGB.setLine3D(_lineVertices);
			cullOSGB.setViewer(_viewer);
			osg::ref_ptr<osg::Geode> geodeRe = cullOSGB.cropModel(geode, _boundary, _quadnodeList);
			if (geodeRe->getName() != "null") {
				saveAsOSGB(geodeRe, _path);
			}
			num++;
		}
		else {
			string simpleFilename = osgDB::getSimpleFileName(_path);
			osgDB::copyFile(_path, _directory + simpleFilename);
		}
		return; 
	}
	osg::ref_ptr<osg::Group> group = node->asGroup();
	if (group) {
		string onlyname = osgDB::getStrippedName(_path);
		string simpleFilename = osgDB::getSimpleFileName(_path);
		string path = _path;
		osg::ref_ptr<osg::Group> groupRe = TraversalGroup(group);
		if (groupRe->getNumChildren() > 0) {
			saveAsOSGB(groupRe, path);
		}
		
		return;
	}
}

osg::ref_ptr<osg::Node> PageTraversal::Traversal(osg::Node* node, bool isGroup) {
	osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(node);
	if (plod) {
		unsigned int fileNames = plod->getNumFileNames();
		if (fileNames == 2) {
			std::string fileName = plod->getFileName(1);
			std::string filePath = plod->getDatabasePath();
			HitCheck hitcheck;
			bool result = hitcheck.BoundingCheck(_pLine, plod);
			if (result) {
				CullOSGB cullOSGB;
				cullOSGB.setLine(_projectArray);
				cullOSGB.setLine3D(_lineVertices);
				cullOSGB.setViewer(_viewer);
				osg::ref_ptr<osg::PagedLOD> pagelod = cullOSGB.cropModel(plod, _boundary, _quadnodeList);
				string path = _path;
				_path = filePath + fileName;
				osg::ref_ptr<osg::Node> childNode = osgDB::readNodeFile(_path);
				Traversal(childNode);
			
				num++;
				return pagelod;
			}
			else {
				_path = filePath + fileName;
				osg::ref_ptr<osg::Node> childNode = osgDB::readNodeFile(_path);
				Traversal(childNode);
				return node;
			}
		}
	}
	osg::Geode *geode = node->asGeode();
	if (geode) {
		HitCheck hitcheck;
		bool result = hitcheck.BoundingCheck(_pLine, geode);
		if (result) {
			CullOSGB cullOSGB;
			cullOSGB.setLine(_projectArray);
			cullOSGB.setLine3D(_lineVertices);
			cullOSGB.setViewer(_viewer);
			osg::ref_ptr<osg::Geode> geodeRe = cullOSGB.cropModel(geode, _boundary, _quadnodeList);
			num++;
			return geodeRe;
		}
		else {
			string simpleFilename = osgDB::getSimpleFileName(_path);
			return node;
		}
		
	}
	osg::Group *group = node->asGroup();
	if (group) {
		return TraversalGroup(group);
	}
}

osg::ref_ptr<osg::Group> PageTraversal::TraversalGroup(osg::Group* group) {
	unsigned int childNum = group->getNumChildren();
	string path = _path;
	osg::ref_ptr<osg::Group> groupRe = new osg::Group;
	for (unsigned int i = 0; i < childNum; i++) {
		osg::Node *childNode = group->getChild(i);
		osg::ref_ptr<osg::Node> node = Traversal(childNode,true);
		if (node->getName() != "null") {
			groupRe->addChild(node);
		}
	
 	}
	return groupRe;
}

void PageTraversal::TraversalGeometry(osg::Geometry* geometry) {
	if (geometry) {
		geodeNum++;
	}
}

void PageTraversal::saveAsOSGB(osg::ref_ptr<osg::Node> node, string path) {
	osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
	options->setOptionString("WriteImageHint=IncludeFile");			// 设置压缩
	string simpleFilename = osgDB::getSimpleFileName(path);
	string onlyname = osgDB::getStrippedName(path);
	osgDB::writeNodeFile(*(node.get()), _directory + simpleFilename, options);
}

void PageTraversal::setLineBoundary(BOUNDARY::Boundary * boundary) {
	_boundary = boundary;
}

void PageTraversal::setLineQuadTree(std::vector<Quadtree::QuadTreeNode *> quadnodeList) {
	_quadnodeList = quadnodeList;
}

inline osg::ref_ptr<osg::StateSet> createTexture2DState(osg::ref_ptr<osg::Image> image) {
	//创建状态集对象
	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
	//创建二维纹理对象
	osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D();
	texture->setDataVariance(osg::Object::STATIC);
	//设置贴图
	texture->setImage(image.get());
	stateset->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::ON);
	return stateset.get();
}

CullOSGB::~CullOSGB() {
	
}

void CullOSGB::setLine(osg::ref_ptr<osg::Vec3Array> projectArray) {
	_projectArray = projectArray;
}

void CullOSGB::setLine3D(vector<osg::Vec3> lineVertices) {
	_lineVertices = lineVertices;
}

void CullOSGB::setViewer(osgViewer::Viewer * viewer) {
	_viewer = viewer;
}



osg::ref_ptr<osg::PagedLOD> CullOSGB::cropModel(osg::PagedLOD * plod, BOUNDARY::Boundary * boundary, std::vector<Quadtree::QuadTreeNode *> quadnodeList) {
	clock_t start, finish;
	double  duration;
	start = clock();
	TextureVisitor textureVisitor;
	plod->accept(textureVisitor);
	//三角网构建
	MainVisitor mainvisitor;
	plod->accept(mainvisitor);
	mainvisitor.createTriangleTopo();
	osg::Matrix matParent(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	osg::Vec3d eye, center, up;
	_viewer->getCamera()->getViewMatrixAsLookAt(eye, center, up);
	mainvisitor.setEye(eye);
	mainvisitor.setLines(_lineVertices);
	mainvisitor.setLineBoundary(boundary);
	mainvisitor.setLineQuadTree(quadnodeList);
	//投影筛选三角网
	mainvisitor.projectFilterTri(_projectArray, _viewer, matParent,mainvisitor);
	//重建三角网
	mainvisitor.projectRebuild();
	//转换成顶点-索引对
	osg::ref_ptr<osg::Node> nodeRe = mainvisitor.createOsgNode(osg::Vec4(0.5f, 0.5f, 0.0f, 1.0f), 1);
	vector<Geom*> allgeom = mainvisitor.GetGeoms();
	bool isSave = true;
	if (allgeom.size() > 0 && allgeom.size() == 1) {
		if (allgeom[0]->inlineTriangleNum == allgeom[0]->triangles.size())
			isSave = false;
	}
	else if (allgeom.size() > 1) {
		//保证所有要素都在边界内的时候，才不保存该文件
		int index = 0;
		for (int i = 0; i < allgeom.size(); i++) {
			if (allgeom[i]->inlineTriangleNum == allgeom[i]->triangles.size())
				index++;	
		}
		if(index== allgeom.size())
			isSave = false;
	}

	if (!isSave) {
		osg::ref_ptr<osg::PagedLOD> pagelod = new osg::PagedLOD;
		pagelod->setName("null");
		return pagelod;
	}

	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet(); 
	stateset = createTexture2DState(textureVisitor._image.get());

	//osg::ref_ptr<osg::PagedLOD> plod = dynamic_cast<osg::PagedLOD*>(node);
	osg::StateSet *state = plod->getChild(0)->asGeode()->getDrawable(0)->getStateSet();
	osg::StateAttribute * nodeMaterialAttribute = state->getAttribute(osg::StateAttribute::MATERIAL);
	osg::ref_ptr<osg::Material> nodeMaterial = dynamic_cast<osg::Material *> (nodeMaterialAttribute);
	if (nodeRe->asGeode()) {
		//为了和原始数据结构保持统一
		osg::ref_ptr<osg::Geode> geode = nodeRe->asGeode();
		geode->getDrawable(0)->setStateSet(stateset.get());
		geode->getDrawable(0)->getStateSet()->setDataVariance(osg::Object::STATIC);
		geode->getDrawable(0)->getStateSet()->setAttribute(nodeMaterial.get());
	}

	//构建pagelod节点
	osg::ref_ptr<osg::PagedLOD> pagelod = new osg::PagedLOD;
	pagelod->addChild(nodeRe.get());
	for (unsigned int i = 0; i < plod->getNumFileNames(); ++i) {
		pagelod->setFileName(i, plod->getFileName(i));
	}
	pagelod->setRangeList(plod->getRangeList());
	pagelod->setRangeMode(plod->getRangeMode());
	osg::ComputeBoundsVisitor boundvisitor;
	nodeRe->accept(boundvisitor);
	osg::BoundingBox localBB = boundvisitor.getBoundingBox();
	osg::Vec3 boxCenter = localBB.center();
	float radius = localBB.radius();
	pagelod->setCenter(boxCenter);
	pagelod->setRadius(radius);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	std::cout << "Time to do cull of thie tile is " << duration << endl;
	return pagelod;
}

osg::ref_ptr<osg::Geode> CullOSGB::cropModel(osg::ref_ptr<osg::Geode> geode, BOUNDARY::Boundary * boundary, std::vector<Quadtree::QuadTreeNode *> quadnodeList) {
	clock_t start, finish;
	double  duration;
	start = clock();

	TextureVisitor textureVisitor;
	geode->accept(textureVisitor);
	//三角网构建
	MainVisitor mainvisitor;
	geode->accept(mainvisitor);
	mainvisitor.createTriangleTopo();
	osg::Matrix matParent(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	osg::Vec3d eye, center, up;
	_viewer->getCamera()->getViewMatrixAsLookAt(eye, center, up);
	mainvisitor.setEye(eye);
	mainvisitor.setLines(_lineVertices);
	mainvisitor.setLineBoundary(boundary);
	mainvisitor.setLineQuadTree(quadnodeList);
	//投影筛选三角网
	mainvisitor.projectFilterTri(_projectArray, _viewer, matParent, mainvisitor);
	//重建三角网
	mainvisitor.projectRebuild();
	osg::ref_ptr<osg::Node> nodeRe = mainvisitor.createOsgNode(osg::Vec4(0.5f, 0.5f, 0.0f, 1.0f), 1);

	vector<Geom*> allgeom = mainvisitor.GetGeoms();
	bool isSave = true;
	if (allgeom.size() > 0 && allgeom.size() == 1) {
		if (allgeom[0]->inlineTriangleNum == allgeom[0]->triangles.size())
			isSave = false;
	}
	else if (allgeom.size() > 1) {
		//保证所有要素都在边界内的时候，才不保存该文件
		int index = 0;
		for (int i = 0; i < allgeom.size(); i++) {
			if (allgeom[i]->inlineTriangleNum == allgeom[i]->triangles.size())
				index++;
		}
		if (index == allgeom.size())
			isSave = false;
	}

	if (!isSave) {
		osg::ref_ptr<osg::Geode> geodeRe = new osg::Geode;
		geodeRe->setName("null");
		return geodeRe;
	}

	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
	stateset = createTexture2DState(textureVisitor._image.get());
	osg::StateSet *state = geode->getDrawable(0)->getStateSet();
	osg::StateAttribute * nodeMaterialAttribute = state->getAttribute(osg::StateAttribute::MATERIAL);
	osg::ref_ptr<osg::Material> nodeMaterial = dynamic_cast<osg::Material *> (nodeMaterialAttribute);
	if (nodeRe->asGeode()) {
		//为了和原始数据结构保持统一
		osg::ref_ptr<osg::Geode> geode = nodeRe->asGeode();
		geode->getDrawable(0)->setStateSet(stateset.get());
		geode->getDrawable(0)->getStateSet()->setDataVariance(osg::Object::STATIC);
		geode->getDrawable(0)->getStateSet()->setAttribute(nodeMaterial.get());
	}
	osg::ref_ptr<osg::Geode> geodeRe = nodeRe->asGeode();
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	std::cout << "Time to do cull of thie tile is " << duration << endl;
	return geodeRe;
}

