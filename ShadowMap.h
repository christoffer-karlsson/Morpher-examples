#pragma once

#include "Light.h"
#include "Graphics.h"

class ShadowMap
{
	public:

	ShadowMap();
	ShadowMap( mtGraphics *gpu, uint32 size );
	~ShadowMap();
	// Prepare the OM stage for rendering to the shadow map.
	// Null render target to disable color writes, since we only 
	// need depth values. This is faster aswell.
	void ClearDepthBuffer();
	void BindDepthBuffer();
	// Bind depth buffer and sampler.
	void BindTexture( int slot );
	// Unbind after scene is rendered.
	void UnbindTexture( int slot );
	// Set view direction for the shadow.
	void SetView( const mtVec3 &position, const mtVec3 &view_dir );
	// Set view direction for the shadow.
	void SetView( const mtVec3 &position, mtDirection dir );
	// Set projection (not necessary if using standard 90 degree perpective projection).
	void SetProjection( const XMMATRIX &projection );

	const XMMATRIX &GetView() const;
	const XMMATRIX &GetProj() const;

	private:

	mtGraphics *gpu;

	ID3D11Texture2D *texture;
	ID3D11SamplerState *shadowMapSampler;
	ID3D11ShaderResourceView *depthMapSRV;
	ID3D11DepthStencilView *depthMapDSV;
	D3D11_VIEWPORT viewport;

	XMMATRIX view;
	XMMATRIX proj;
};