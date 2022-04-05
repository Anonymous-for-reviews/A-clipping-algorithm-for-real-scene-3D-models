#pragma once
#include <osg/Vec3>
#include <string>
using namespace std;

//template<typename T>
namespace Common {



	class Utility
	{
	public:
		static bool isVec3Same(const osg::Vec3 & v1, const osg::Vec3 & v2);//�Ƚ�������ά�����Ƿ����
		static string getFileNameFromPath(string path);//��ģ��·���л�ȡ������
		static void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst);
		//��������ת��Ļ����
		static osg::ref_ptr<osg::Vec3Array> worldToWindowArray(osg::Node *node, osg::View *view);
		static osg::Vec3 worldToWindow(osg::Vec3 p, osg::View * view);
		static osg::Vec3 windowToWorld(osg::Vec3 p, osg::View * view);
		//������������������ֻ��0��1��2֮������
		static int customizePlus(int value);
		//�����������ӷ���������2����
		static int customizePlus(int value, int addend);
		//�ж�vector�Ƿ����elementԪ��
		static bool is_element_in_vector(vector<int> v, int element);
		//�Զ���a�Ƿ�С��b
		static bool CustomizeSmall(int a, int b, int Count);
		//Are the three points collinear
		static bool isCollinear(osg::Vec3 a , osg::Vec3 b, osg::Vec3 c);

		//������������
		static vector<osg::Vec3> setLines(osg::Node *node);

		Utility();
		~Utility();
	};

	//*���vector�������Ƿ��к�value��ͬ��ֵ
	template<typename T> 
	bool CheckDuplicates(vector<T>& tList, T value) {
		typename vector<T>::iterator iter; //����һ����������������vector���������ã���������ָ��vector������Ԫ�� 
		for (iter = tList.begin(); iter != tList.end(); iter++) {
			if (*iter == value)
				return true;
		}
		return false;
	}
}