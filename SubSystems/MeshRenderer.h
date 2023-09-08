#pragma once

#include "UI/UIManager.h"
using namespace FocalEngine;

class MeshRenderer
{
public:
	SINGLETON_PUBLIC_PART(MeshRenderer)

	void RenderFEMesh(FEMesh* Mesh);
private:
	SINGLETON_PRIVATE_PART(MeshRenderer)

	
};

#define MESH_RENDERER MeshRenderer::getInstance()