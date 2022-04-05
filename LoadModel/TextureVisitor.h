#pragma once
#include "stdafx.h"
#include "Utility.h"
using namespace std;

class TextureVisitor :public osg::NodeVisitor
{
public:
	TextureVisitor() : osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {

	};
	~TextureVisitor();
	virtual void apply(osg::Node& node);
	virtual void apply(osg::Geode& geode);
	virtual void apply(osg::StateSet* state);
	osg::ref_ptr<osg::Image> _image;
protected:
	std::vector<string> _pathList;
};

