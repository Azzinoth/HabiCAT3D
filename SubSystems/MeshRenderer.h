#pragma once

#include "UI/UIManager.h"
using namespace FocalEngine;

class MeshRenderer
{
public:
	SINGLETON_PUBLIC_PART(MeshRenderer)

	void RenderFEMesh(FEMesh* Mesh);

	// Copyright 2019 Google LLC.
	// SPDX-License-Identifier: Apache-2.0
	// Author: Anton Mikhailov
	// The look-up tables contains 256 entries. Each entry is a an sRGB triplet.
	// From : https://gist.github.com/mikhailov-work/6a308c20e494d9e0ccc29036b28faa7a#file-turbo_colormap-c
	glm::vec3 GetTurboColorMapValue(float NormalizedValue);
private:
	SINGLETON_PRIVATE_PART(MeshRenderer)
};

#define MESH_RENDERER MeshRenderer::getInstance()