#include "TextureVisitor.h"
#include "Utility.h"

TextureVisitor::~TextureVisitor() {}

void TextureVisitor::apply(osg::Node& node) {
	if (node.getStateSet())
	{
		apply(node.getStateSet());
	}
	traverse(node);
}

void TextureVisitor::apply(osg::Geode& geode) {
	if (geode.getStateSet())
	{
		apply(geode.getStateSet());
	}
	unsigned int cnt = geode.getNumDrawables();
	for (unsigned int i = 0; i < cnt; i++)
	{
		osg::Drawable* draw = geode.getDrawable(i);
		if (draw)
		{
			osg::StateSet *state = draw->getStateSet();
			if (state)
				apply(state);
		}
	}
	traverse(geode);
}

void TextureVisitor::apply(osg::StateSet* state) {
	if (state == NULL)
		return;
	osg::StateSet::TextureAttributeList& textureAttributeList = state->getTextureAttributeList();
	for (unsigned int i = 0; i < textureAttributeList.size(); i++) {
		osg::Texture2D* tex2D = dynamic_cast<osg::Texture2D*>(state->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
		if (tex2D) {
			osg::Image *image = tex2D->getImage();
			if (image && image->valid())
			{
				std::string path = tex2D->getImage()->getFileName();
				if (!Common::CheckDuplicates(_pathList, path)) {
					_image = image;
					/*_pathList.push_back(path);
					string filePath = "G://" + path;
					osgDB::writeImageFile(*image, filePath);*/
				}
			}
		}
	}
}