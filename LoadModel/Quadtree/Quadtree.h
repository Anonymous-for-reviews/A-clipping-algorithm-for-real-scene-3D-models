#pragma once
#include "../stdafx.h"
#include "../Line.h"


/* һ��������������޻��֣�:

	   UL(1)   |    UR(0)
	 ----------|-----------
	   LL(2)   |    LR(3)
���¶Ը��������͵�ö��
*/

namespace Quadtree {
	typedef enum
	{
		UR = 0,
		UL = 1,
		LL = 2,
		LR = 3
	} QuadrantEnum;

	/* �Ĳ������ͽṹ */
	

	struct Region {
		float maxX, minX, maxY, minY;
	};

	std::vector<Region> QuadRegion(Region origin);

	enum TopoLogic
	{
		Separated = 0,  //����
		Adjacent = 1,   //����
		Contain = 2,    //����
	};

	class QuadTreeNode
	{
	public:
		QuadTreeNode(Region region, int depth, int maxdepth, QuadTreeNode *_parent);
		~QuadTreeNode();
		void SetVertices(osg::ref_ptr<osg::Vec3Array> vertices);
		void SetDepth(int depth);
		//�жϵ��Ƿ���������
		bool isContain(osg::Vec3 & p);
		void addPoint(osg::Vec3 & p);
		void addLine(Line & line);
		bool topologic(osg::Vec3 point1, osg::Vec3 point2);
		void LineNumMatch(std::vector<QuadTreeNode *> & quadnodeList);
		QuadTreeNode *_parent; // ���ڵ�ָ��
		QuadTreeNode *UpRight; // �����ӽ��ָ��
		QuadTreeNode *UpLeft; // �����ӽ��ָ��
		QuadTreeNode *LLeft; // �����ӽ��ָ��
		QuadTreeNode *LRight; // �����ӽ��ָ��
		int is_leaf; // �Ƿ���Ҷ�ӽ��
		std::vector<Line> _lines;
		int _depth; // ������
		int _maxDepth; //������
	private:
		
		
		struct Region _region; // ����Χ

		int ele_num; // �߶ζ�����
		osg::ref_ptr<osg::Vec3Array> _vertices;
	};
	typedef struct QuadTree
	{
		QuadTreeNode  *root;
		int         depth;           // �Ĳ��������                    
	} QuadTree;


	QuadTree QuadTreeBuild(int depth, int maxDepth, Region region);
	std::vector<QuadTreeNode *> lineDivide(QuadTreeNode* quadRoot, std::vector<Line> lines);
	QuadTreeNode * CreateQuadBranch( QuadTreeNode * parent, int depth, int maxDepth, Region region);
}

