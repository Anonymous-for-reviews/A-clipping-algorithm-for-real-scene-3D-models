#include "stdafx.h"
#include "Side.h"

Side::Side() {
	this->vertexIndexs[0] = this->vertexIndexs[1] = -1;
}

Side::~Side() {

}

bool Side::IsSame(const Side & side) {
	return ((this->vertexIndexs[0] == side.vertexIndexs[0] && this->vertexIndexs[1] == side.vertexIndexs[1]) ||
		(this->vertexIndexs[0] == side.vertexIndexs[1] && this->vertexIndexs[1] == side.vertexIndexs[0]));
}