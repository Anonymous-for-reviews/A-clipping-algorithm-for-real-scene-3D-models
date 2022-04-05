#pragma once
#include <osg/Node>
#include <osg/Geode>  
#include <osg/Group>
#include <osg/Depth>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgFX/Scribe>
#include <osgUtil/Optimizer>
#include <iostream>

namespace Space {

	static void drawSpace(osg::Vec3Array *arrayList, osg::Group* group, osg::ref_ptr<osg::UIntArray> indexs) {
		osg::ref_ptr<osg::Geode> geode = new osg::Geode();
		osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
		geom->setVertexArray(arrayList);

		osg::Vec4Array *quadColorArray = new osg::Vec4Array;
		quadColorArray->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
		geom->setColorArray(quadColorArray);
		geom->setColorBinding(osg::Geometry::BIND_OVERALL);

		osg::ref_ptr<osg::DrawElementsUInt> indices = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
		for (int i = 0; i < indexs->size(); i++) {
			indices->push_back(indexs->index(i));
		}
		//geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, arrayList->size()));
		geom->addPrimitiveSet(indices);



		/*osg::ref_ptr<osg::Depth> depth = new osg::Depth();
		depth->setRange(0, 0.95);
		geom->getOrCreateStateSet()->setAttributeAndModes(depth, osg::StateAttribute::ON);*/


		/*osg::ref_ptr<osg::StateSet> stateset = geom->getOrCreateStateSet();
		stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
		osg::ref_ptr<osg::Material> material = new osg::Material;
		osg::Vec4 changeColor(1,0,0, 122 / 255);
		material->setAmbient(osg::Material::FRONT_AND_BACK, changeColor);
		material->setDiffuse(osg::Material::FRONT_AND_BACK, changeColor);
		material->setTransparency(osg::Material::FRONT_AND_BACK, 0.2);
		material->setColorMode(osg::Material::AMBIENT);
		stateset->setAttributeAndModes(material, osg::StateAttribute::ON);
		stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
		geode->getOrCreateStateSet()->setAttributeAndModes(material.get(), osg::StateAttribute::ON);*/
		//geom->addPrimitiveSet(primitiveset);
		geode->addDrawable(geom.get());
		geode->setName("mian");
		group->addChild(geode);
	}

}
