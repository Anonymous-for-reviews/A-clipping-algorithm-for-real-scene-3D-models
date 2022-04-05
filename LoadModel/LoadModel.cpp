// LoadModel.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//newnode 1 表示展示新的模型，0展示原始模型 增强对比
#define originnode 0
#define newnode 0
#define CULL 1
#define SHOW 0 
#define LINE_STEP 4 
//#include <iostream>
//
//int main()
//{
//    std::cout << "Hello World!\n";
//

#include <time.h>
#include <osgViewer/Viewer>
#include <osgUtil/IntersectVisitor> 
#include <osgDB/ReadFile>
#include <osg/MatrixTransform>
#include <osgUtil/TriStripVisitor>
#include <osgViewer/ViewerEventHandlers> 
#include <osgGA/StateSetManipulator>
#include <osgDB/FileNameUtils>
#include "MainVisiter.h"
#include "space.h"
#include "Intersect.h"
#include "IntersectTest.h"
#include "HitCheck.h"
#include "PageTraversal.h"
#include "Utility.h"
#include "TextureVisitor.h"
#include "Boundary.h"
#include "Quadtree/Quadtree.h"
#include "GetDirectorys.h"

osg::ref_ptr<osg::StateSet> createTexture2DState1(osg::ref_ptr<osg::Image> image) {
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

string LINE_NUM = "11";
string TILE_NUM = "37";

int main(int argc, char **argv)
{
	clock_t start, finish;
	double  duration;
	osg::ref_ptr<osgViewer::Viewer> viewer = new osgViewer::Viewer;
	osg::ref_ptr<osg::Group > root = new osg::Group();

#if newnode
	osg::Node * node = osgDB::readNodeFile("G://culled.osgb");
#endif
	string filePath = "G://Result//Total1//Data";
	vector<string> files;
	vector<string> directoryName;
	GetAllFiles(filePath, files, directoryName);
	int DirectorySize = files.size();
#if CULL
	
	//string rootPath = "G://xuankongsi//Data//Tile_"+ TILE_NUM +"//Tile_"+ TILE_NUM +".osgb";
	//string rootPath = "G://Result//Total1//Data//Tile_" + TILE_NUM + "//Tile_" + TILE_NUM + ".osgb";
	//string rootPath = "G://Result//tile31//osgbtest13//Tile_31.osgb";
#endif
#if SHOW
	string rootPath = "G://Result//tile"+ TILE_NUM +"//osgbtest"+ LINE_NUM +"//Tile_"+ TILE_NUM +".osgb";
	//string rootPath = "G://xuankongsi//Data//Tile_"+ TILE_NUM +"//Tile_" + TILE_NUM + ".osgb";
	//string rootPath = "G://Result//tile" + TILE_NUM + "//osgbtestDouble//Tile_" + TILE_NUM + ".osgb";
#endif // SHOW
	//std::string rootPath = "G://xuankongsi//Data//Tile_31//Tile_31_L21_000510t3.osgb";
	//osg::Node * node = osgDB::readNodeFile(rootPath);
	/*osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(node);
	if (plod) {
		unsigned int filenum = plod->getNumFileNames();
		std::string name1 = plod->getFileName(0);
		std::string name2 = plod->getFileName(1);
		std::string databasePath = plod->getDatabasePath();
	}
	else {
		osg::Group * pGroup = dynamic_cast<osg::Group*>(node);
		for (unsigned int i = 0; i < pGroup->getNumChildren(); i++) {
			osg::ref_ptr<osg::PagedLOD> childLOD = dynamic_cast<osg::PagedLOD*>(pGroup->getChild(i));
			unsigned int filenum = childLOD->getNumFileNames();
			std::string name1 = childLOD->getFileName(0);
			std::string name2 = childLOD->getFileName(1);
			std::string databasePath = childLOD->getDatabasePath();
		}
	}*/
	
	//文件存储，存储为osgb格式
	//osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
	//options->setOptionString("WriteImageHint=IncludeFile");			// 设置压缩
	//osgDB::writeNodeFile(*node, "G://test.osgb", options);
	//osgDB::writeNodeFile(*node, "G://test.osg");
#if originnode
	osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(node);
	osg::ref_ptr<osg::Material> nodeMaterial;
	if (plod) {
		osg::StateSet *state = plod->getChild(0)->asGeode()->getDrawable(0)->getStateSet();
		node = plod;
		//osg::StateSet *state = node->asGeode()->getDrawable(0)->getStateSet();
		osg::StateAttribute * nodeMaterialAttribute = state->getAttribute(osg::StateAttribute::MATERIAL);
		nodeMaterial = dynamic_cast<osg::Material *> (nodeMaterialAttribute);
	}
	else {
		osg::Group * pGroup = dynamic_cast<osg::Group*>(node);
		node = pGroup->getChild(1);
		int kkk = pGroup->getNumChildren();
		for (unsigned int i = 0; i < pGroup->getNumChildren(); i++) {
			
		}
		osg::ref_ptr<osg::PagedLOD> childLOD = dynamic_cast<osg::PagedLOD*>(pGroup->getChild(1));
		osg::StateSet *state = childLOD->getChild(0)->asGeode()->getDrawable(0)->getStateSet();
		//osg::StateSet *state = node->asGeode()->getDrawable(0)->getStateSet();
		osg::StateAttribute * nodeMaterialAttribute = state->getAttribute(osg::StateAttribute::MATERIAL);
		nodeMaterial = dynamic_cast<osg::Material *> (nodeMaterialAttribute);
	}
	/*osg::ref_ptr<osg::Material> nodeMaterial;
	osg::StateSet *state = node->asGeode()->getDrawable(0)->getStateSet();
	osg::StateAttribute * nodeMaterialAttribute = state->getAttribute(osg::StateAttribute::MATERIAL);
	nodeMaterial = dynamic_cast<osg::Material *> (nodeMaterialAttribute);*/
#endif

	std::vector<int> indeces;
	HitCheck hitCheck;

	//osg::Node * nodeLine = osgDB::readNodeFile("G:\\OSG\\Lines\\tile"+ TILE_NUM +"\\Lines"+ LINE_NUM +".osg");
	osg::ref_ptr<osg::Node> nodeLine = osgDB::readNodeFile("G:\\OSG\\Lines\\total\\Lines" + LINE_NUM + ".osg");
	root->addChild(nodeLine);

	/*osg::Node * nodeLineTemp = osgDB::readNodeFile("G:\\OSG\\Lines\\tile" + TILE_NUM + "\\Lines12.osg");
	root->addChild(nodeLineTemp);*/

	//遍历Pagelod节点

	int x = 50;
	int y = 50;
	int width = 1500;
	int height = 800;
	osg::Vec3d eye, center, up;
	viewer->getCamera()->getViewMatrixAsLookAt(eye, center, up);
	osg::ref_ptr<osg::GraphicsContext::Traits> traits =
		new osg::GraphicsContext::Traits;
	traits->x = x;
	traits->y = y;
	traits->width = width;
	traits->height = height;
	traits->windowDecoration = true;
	traits->pbuffer = false;
	traits->doubleBuffer = true;
	osg::ref_ptr<osg::GraphicsContext> gc =
		osg::GraphicsContext::createGraphicsContext(traits.get());
	viewer->getCamera()->setGraphicsContext(gc.get());
	viewer->getCamera()->setViewport(0, 0, width, height);
	viewer->setCameraManipulator(new osgGA::TrackballManipulator);
	// add the state manipulator
	viewer->addEventHandler(new osgGA::StateSetManipulator(viewer->getCamera()->getOrCreateStateSet()));
	viewer->realize();
	////修改相机参数
	eye = osg::Vec3d(21.734262, -70.719444, 932.897095);
	center = osg::Vec3d(21.057886, -70.059731, 932.569519);
	up = osg::Vec3d(-0.275669, 0.185651, 0.943155);
	viewer->getCamera()->setViewMatrixAsLookAt(eye, center, up);
	//关闭光照
	root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	 

	AABB boundbox;
	osg::ref_ptr<osg::Vec3Array> vec3Array0 = Common::Utility::worldToWindowArray(nodeLine, viewer);
	int remainder = vec3Array0->size() % LINE_STEP;
	std::cout << "line point count is " << vec3Array0->size() / LINE_STEP << endl;
	int j = remainder == 0 ? (floor(vec3Array0->size() / LINE_STEP) - 1)*LINE_STEP : floor(vec3Array0->size() / LINE_STEP)*LINE_STEP;
	int i = 0;
	vector<Line> lineList;
	for (; i < vec3Array0->size();) {
		Line line;
		line.linePoint->push_back(vec3Array0->at(j));
		line.linePoint->push_back(vec3Array0->at(i));
		line.lineIndex = i;
		lineList.push_back(line);
		boundbox.addPoint(vec3Array0->at(i));
		j = i;
		i = i + LINE_STEP;
	}

	/*osg::Vec3Array::iterator iter;
	for (iter = vec3Array->begin();
		iter != vec3Array->end();) {
		
		iter++;
	}*/
	Quadtree::Region region;
	region.maxX = boundbox.max.x() + 0.1;
	region.maxY = boundbox.max.y() + 0.1;
	region.minX = boundbox.min.x() - 0.1;
	region.minY = boundbox.min.y() - 0.1;
	Quadtree::QuadTree quadTree = Quadtree::QuadTreeBuild(0, 3, region);
	vector<Quadtree::QuadTreeNode *> quadnodeList = lineDivide(quadTree.root, lineList);

#if CULL
	for (int i = 0; i < DirectorySize; i++) {
		string p;
		string rootPath = p.assign(files[i]).append("//").append(directoryName[i]).append(".osgb");
		osg::ref_ptr<osg::Node> node = osgDB::readNodeFile(rootPath);

		start = clock();
		PageTraversal page;
		page.setPath(rootPath);
		BOUNDARY::Boundary * boundary = new BOUNDARY::Boundary(nodeLine, viewer);
		page.setLineBoundary(boundary);
		page.setLineQuadTree(quadnodeList);
		//page.setDirectory("G://Result//tile"+ TILE_NUM +"//osgbtest" + LINE_NUM + "//");
		string savePath = "G://Result//Total//Data//";
		savePath = savePath.append(directoryName[i]).append("//");
		page.setDirectory(savePath);
		//page.setDirectory("G://Result//tile" + TILE_NUM + "//osgbtestDouble//");
		page.Traversal(node, nodeLine, viewer);
		int num = page.num;
		int geodeNum = page.geodeNum;
		//std::cout << "page num:" << num << "," << "geodeNum num:" << geodeNum << endl;
		finish = clock();
		duration = (double)(finish - start) / CLOCKS_PER_SEC;
		//std::cout << "Time to do cull is " << duration << endl;
		//std::cout << "CULLED Tile Num" << page.num << endl;
		std::cout << rootPath << endl;
		root->addChild(node);
	}
#endif // CULL


#if newnode
	TextureVisitor textureVisitor;
	node->accept(textureVisitor);
	//三角网构建
	MainVisitor mainvisitor;
	node->accept(mainvisitor);
	start = clock();
	mainvisitor.createTriangleTopo();
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	std::cout << "Time to do topo is " << duration << endl;

	/*osg::ref_ptr<osg::Image> g_Image = new osg::Image;
	viewer.getCamera()->attach(osg::Camera::COLOR_BUFFER, g_Image.get());
	osgDB::writeImageFile(*g_Image, "G://rendertoTxt.png");*/
	osgDB::writeNodeFile(*node, "G://exportByCull.osg");
	root->addChild(node);
	osgDB::writeImageFile(*(textureVisitor._image.get()), "G://rebuild.png");
#endif

#if 0
	TextureVisitor textureVisitor;
	node->accept(textureVisitor);
	osg::Node * childNode = plod->getChild(0);
	root->addChild(childNode);
	osgDB::writeImageFile(*(textureVisitor._image.get()), "G://origin.png");
#endif
	//裁剪重构
#if originnode


	TextureVisitor textureVisitor;
	node->accept(textureVisitor);
	//三角网构建
	MainVisitor mainvisitor;
	node->accept(mainvisitor);
	start = clock();
	mainvisitor.createTriangleTopo();
	int count = mainvisitor.getTriangleNum();
	std::cout << "triangle count: " << count << endl;

	/*PageTraversal page;
	page.Traversal(node, nodeLine);
	int num = page.num;
	int geodeNum = page.geodeNum;
	std::cout << "page num:" << num << "," << "geodeNum num:" << geodeNum << endl;*/

	osg::Matrix matParent(1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);
	osg::ref_ptr<osg::Vec3Array> vec3Array = Common::Utility::worldToWindowArray(nodeLine, viewer);
	vector<osg::Vec3> lineVertices = Common::Utility::setLines(nodeLine);
	BOUNDARY::Boundary * boundary = new BOUNDARY::Boundary(nodeLine, viewer);
	mainvisitor.setEye(eye);
	mainvisitor.setLines(lineVertices);
	mainvisitor.setLineBoundary(boundary);
	mainvisitor.setLineQuadTree(quadnodeList);
	//主要是这一步有较长耗时
	mainvisitor.projectFilterTri(vec3Array, viewer, matParent, mainvisitor);

	//遍历边界内的三角网，筛选在边界内的三角网
	//mainvisitor.traverseTriangle(vec3Array, viewer);
	
	mainvisitor.projectRebuild();
	osg::ref_ptr<osg::Node> nodeRe = mainvisitor.createOsgNode(osg::Vec4(0.5f, 0.5f, 0.0f, 1.0f), 1);

	//贴纹理
	//osg::ref_ptr<osg::Image> image = osgDB::readImageFile("G://Tile_56_L22_00111000_0.jpg");
	osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
	osgDB::writeImageFile(*(textureVisitor._image.get()), "G://origin.png");
	stateset = createTexture2DState1(textureVisitor._image.get());
	if (nodeRe->asGeode()) {
		//为了和原始数据结构保持统一
		osg::ref_ptr<osg::Geode> geode = nodeRe->asGeode();
		geode->getDrawable(0)->setStateSet(stateset.get());
		geode->getDrawable(0)->getStateSet()->setDataVariance(osg::Object::STATIC);
		geode->getDrawable(0)->getStateSet()->setAttribute(nodeMaterial.get());
	}
	//nodeRe->setStateSet(stateset.get());

	TextureVisitor textureVisitorRe;
	nodeRe->accept(textureVisitorRe);
	//osgDB::writeImageFile(*(textureVisitorRe._image.get()), "G://rebuild.png");

	////构建pagelod节点
	//osg::ref_ptr<osg::PagedLOD> pagelod = new osg::PagedLOD;
	//pagelod->addChild(nodeRe.get());
	//for (unsigned int i = 0; i < plod->getNumFileNames(); ++i) {
	//	pagelod->setFileName(i, plod->getFileName(i));
	//}
	//pagelod->setRangeList(plod->getRangeList());
	//pagelod->setRangeMode(plod->getRangeMode());
	//osg::ComputeBoundsVisitor boundvisitor;
	//nodeRe->accept(boundvisitor);
	//osg::BoundingBox localBB = boundvisitor.getBoundingBox();
	//osg::Vec3 boxCenter = localBB.center();
	//float radius = localBB.radius();
	//pagelod->setCenter(boxCenter);
	//pagelod->setRadius(radius);
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	std::cout << "Time to do cull is " << duration << endl;


	//osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
	//options->setOptionString("WriteImageHint=IncludeFile");			// 设置压缩
	//osgDB::writeNodeFile(*(pagelod.get()), "G://culled.osgb", options);
	//osgDB::writeNodeFile(*(pagelod.get()), "G://culled.osg");
	root->addChild(mainvisitor._group);
	root->addChild(nodeRe);
#endif
	//CullOSGB cullOSGB;
	//cullOSGB.setLine(nodeLine);
	//cullOSGB.setViewer(viewer);
	//osg::ref_ptr<osg::PagedLOD> pagelod = cullOSGB.cropModel(plod);
	//osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
	//options->setOptionString("WriteImageHint=IncludeFile");			// 设置压缩
	//string simpleFilename = osgDB::getSimpleFileName(rootPath);
	//osgDB::writeNodeFile(*(pagelod.get()), "G://osgbtest//"+ simpleFilename, options);

	//root->addChild(node);

	//root->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
	//root->addChild(mainvisitor._points);
	viewer->setSceneData(root);
	

	
	while (!viewer->done())
	{
		viewer->frame();
	}

	return 1;
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件

