#pragma once
#include <osg/Vec3>
#include <string>
using namespace std;

//template<typename T>
namespace Common {



	class Utility
	{
	public:
		static bool isVec3Same(const osg::Vec3 & v1, const osg::Vec3 & v2);//比较两个三维向量是否相等
		static string getFileNameFromPath(string path);//从模型路径中获取明名称
		static void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst);
		//世界坐标转屏幕坐标
		static osg::ref_ptr<osg::Vec3Array> worldToWindowArray(osg::Node *node, osg::View *view);
		static osg::Vec3 worldToWindow(osg::Vec3 p, osg::View * view);
		static osg::Vec3 windowToWorld(osg::Vec3 p, osg::View * view);
		//三角形索引自增，即只有0、1、2之间的相加
		static int customizePlus(int value);
		//三角形索引加法，尽量在2以下
		static int customizePlus(int value, int addend);
		//判断vector是否存在element元素
		static bool is_element_in_vector(vector<int> v, int element);
		//自定义a是否小于b
		static bool CustomizeSmall(int a, int b, int Count);
		//Are the three points collinear
		static bool isCollinear(osg::Vec3 a , osg::Vec3 b, osg::Vec3 c);

		//设置坐标数组
		static vector<osg::Vec3> setLines(osg::Node *node);

		Utility();
		~Utility();
	};

	//*检测vector容器中是否有和value相同得值
	template<typename T> 
	bool CheckDuplicates(vector<T>& tList, T value) {
		typename vector<T>::iterator iter; //声明一个迭代器，来访问vector容器，作用：遍历或者指向vector容器的元素 
		for (iter = tList.begin(); iter != tList.end(); iter++) {
			if (*iter == value)
				return true;
		}
		return false;
	}
}