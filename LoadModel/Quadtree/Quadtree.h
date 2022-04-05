#pragma once
#include "../stdafx.h"
#include "../Line.h"


/* 一个矩形区域的象限划分：:

	   UL(1)   |    UR(0)
	 ----------|-----------
	   LL(2)   |    LR(3)
以下对该象限类型的枚举
*/

namespace Quadtree {
	typedef enum
	{
		UR = 0,
		UL = 1,
		LL = 2,
		LR = 3
	} QuadrantEnum;

	/* 四叉树类型结构 */
	

	struct Region {
		float maxX, minX, maxY, minY;
	};

	std::vector<Region> QuadRegion(Region origin);

	enum TopoLogic
	{
		Separated = 0,  //相离
		Adjacent = 1,   //相邻
		Contain = 2,    //包含
	};

	class QuadTreeNode
	{
	public:
		QuadTreeNode(Region region, int depth, int maxdepth, QuadTreeNode *_parent);
		~QuadTreeNode();
		void SetVertices(osg::ref_ptr<osg::Vec3Array> vertices);
		void SetDepth(int depth);
		//判断点是否在象限内
		bool isContain(osg::Vec3 & p);
		void addPoint(osg::Vec3 & p);
		void addLine(Line & line);
		bool topologic(osg::Vec3 point1, osg::Vec3 point2);
		void LineNumMatch(std::vector<QuadTreeNode *> & quadnodeList);
		QuadTreeNode *_parent; // 父节点指针
		QuadTreeNode *UpRight; // 右上子结点指针
		QuadTreeNode *UpLeft; // 左上子结点指针
		QuadTreeNode *LLeft; // 左下子结点指针
		QuadTreeNode *LRight; // 右下子结点指针
		int is_leaf; // 是否是叶子结点
		std::vector<Line> _lines;
		int _depth; // 结点深度
		int _maxDepth; //最大深度
	private:
		
		
		struct Region _region; // 区域范围

		int ele_num; // 线段顶点数
		osg::ref_ptr<osg::Vec3Array> _vertices;
	};
	typedef struct QuadTree
	{
		QuadTreeNode  *root;
		int         depth;           // 四叉树的深度                    
	} QuadTree;


	QuadTree QuadTreeBuild(int depth, int maxDepth, Region region);
	std::vector<QuadTreeNode *> lineDivide(QuadTreeNode* quadRoot, std::vector<Line> lines);
	QuadTreeNode * CreateQuadBranch( QuadTreeNode * parent, int depth, int maxDepth, Region region);
}

