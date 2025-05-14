#pragma once

struct ID3D11Device;

namespace menu
{
	void draw();
	void drawInternal();
	inline ID3D11Device* g_pd3dDevice = nullptr;
}