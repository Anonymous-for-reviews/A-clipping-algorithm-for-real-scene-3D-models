#pragma once

#include <osg/Geode>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/LineWidth> 
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/PolygonMode>
#include <osg/CullFace>

#include <osg/TriangleFunctor>
#include <osg/ClipNode>
#include <osg/Depth>
#include <osg/NodeCallback>
#include <osgUtil/PlaneIntersector>
#include <osg/DrawPixels>
#include <osg/Image>

#include<osgManipulator/TabBoxTrackballDragger >  
#include<osgManipulator/CommandManager>  
#include<osgManipulator/TabBoxDragger>
#include <osgManipulator/TranslateAxisDragger>
#include<osgManipulator/Selection>
#include <osgManipulator/TranslatePlaneDragger>
#include <osgManipulator/TabPlaneTrackballDragger>

#include <osg/ComputeBoundsVisitor>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <Windows.h>
#include <osgViewer\api\Win32\GraphicsWindowWin32>

#include <iostream>
#include <fstream>
#include <string>
