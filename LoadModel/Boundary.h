#pragma once
#include "stdafx.h"
#include "Utility.h"
#include "Intersect.h"


namespace BOUNDARY {

	class AABB {
	public:
		AABB();
		~AABB() {};
		void addPoint(osg::Vec3 &);
		bool contains(osg::Vec3 & p) const;
		bool intersectAABB(AABB &box);
		osg::Vec3 min;
		osg::Vec3 max;
	};

	class Boundary {
	public:
		Boundary(osg::Node * Line, osg::View * view);
		Boundary();
		~Boundary();
		//return level of vertex y
		int judgeLevel(float y);
		bool pointTopo(osg::Vec3 vertex);
		AABB boundingBox;
	private:
		osg::ref_ptr<osg::Vec3Array> windowArray;
		osg::Vec3 maxVertex;
		osg::Vec3 minVertex;
		vector<vector<float>> _critical;
		//Curve segments at various levels
		vector<vector<osg::ref_ptr<osg::Vec3Array>>> _levelsLine;
		
		
	};
}

