#include "ShadowMap.h"

ShadowMap::ShadowMap()
{
	this->gpu = nullptr;
	this->texture = 0;
	this->depthMapDSV = 0;
	this->depthMapSRV = 0;
	this->viewport = {};
	this->view = XMMatrixIdentity();
	this->proj = XMMatrixIdentity();
}

ShadowMap::ShadowMap( mtGraphics *gpu, uint32 size )
{
	this->gpu = gpu;
	this->texture = 0;
	this->depthMapDSV = 0;
	this->depthMapSRV = 0;
	this->view = XMMatrixIdentity();
	this->proj = XMMatrixPerspectiveFovLH( PI32DIV2, 1.0f, 0.5f, 70.0f );

	HRESULT hr;

	// Viewport that matches the shadow map dimensions.
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)size;
	viewport.Height = (float)size;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	// The buffer.
	D3D11_TEXTURE2D_DESC texdesc = {};
	texdesc.Width = size;
	texdesc.Height = size;
	texdesc.MipLevels = 1;
	texdesc.ArraySize = 1;
	texdesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texdesc.SampleDesc.Count = 1;
	texdesc.SampleDesc.Quality = 0;
	texdesc.Usage = D3D11_USAGE_DEFAULT;
	texdesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texdesc.CPUAccessFlags = 0;
	texdesc.MiscFlags = 0;

	hr = gpu->Device()->CreateTexture2D( &texdesc, 0, &texture );
	D3D_ERROR_CHECK( hr );

	// Buffer viewed as depth data (Depth Stencil View).
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvdesc = {};
	dsvdesc.Flags = 0;
	dsvdesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsvdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvdesc.Texture2D.MipSlice = 0;

	hr = gpu->Device()->CreateDepthStencilView( texture, &dsvdesc, &depthMapDSV );
	D3D_ERROR_CHECK( hr );

	// Buffer viewed as texture (Shader Resource View).
	D3D11_SHADER_RESOURCE_VIEW_DESC srvdesc = {};
	srvdesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvdesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvdesc.Texture2D.MipLevels = texdesc.MipLevels;
	srvdesc.Texture2D.MostDetailedMip = 0;

	hr = gpu->Device()->CreateShaderResourceView( texture, &srvdesc, &depthMapSRV );
	D3D_ERROR_CHECK( hr );

	// Sampler for shadow mapping.
	// TODO: Using linear filtering for PCF. For now, sampling from individual
	// textures, which is bad with filtering, since sampling from texture
	// edges is a problem, producing small gaps in shadows when looking close.
	D3D11_SAMPLER_DESC desc = {};
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.BorderColor[0] = 1.0f;
	desc.BorderColor[1] = 1.0f;
	desc.BorderColor[2] = 1.0f;
	desc.BorderColor[3] = 1.0f;
	desc.MinLOD = 0.0f;
	desc.MaxLOD = D3D11_FLOAT32_MAX;
	desc.MipLODBias = 0.0f;
	desc.MaxAnisotropy = 0;
	desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;

	hr = gpu->Device()->CreateSamplerState( &desc, &shadowMapSampler );
	D3D_ERROR_CHECK( hr );
}

ShadowMap::~ShadowMap()
{
	mtGraphics::SafeRelease( texture );
	mtGraphics::SafeRelease( depthMapSRV );
	mtGraphics::SafeRelease( depthMapDSV );
}

void ShadowMap::ClearDepthBuffer()
{
	gpu->Context()->ClearDepthStencilView( depthMapDSV, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0 );
}

void ShadowMap::BindDepthBuffer()
{
	gpu->Context()->RSSetViewports( 1, &viewport );
	gpu->Context()->OMSetRenderTargets( 0, nullptr, depthMapDSV );
}

void ShadowMap::BindTexture( int slot )
{
	gpu->Context()->PSSetSamplers( slot, 1, &shadowMapSampler );
	gpu->Context()->PSSetShaderResources( slot, 1, &depthMapSRV );
}

void ShadowMap::UnbindTexture( int slot )
{
	ID3D11ShaderResourceView *nullSRV = nullptr;
	gpu->Context()->PSSetShaderResources( slot, 1, &nullSRV );
}

void ShadowMap::SetView( const mtVec3 &position, const mtVec3 &viewdir )
{
	static const XMVECTOR updir = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );

	XMVECTOR pos = XMLoadFloat3( &position );
	XMVECTOR dir = XMLoadFloat3( &viewdir );
	XMVECTOR lookdir = XMVectorAdd( pos, dir );

	view = XMMatrixLookAtLH( pos, lookdir, updir );
}

void ShadowMap::SetView( const mtVec3 &position, mtDirection dir )
{
	static bool init;
	static XMVECTOR viewdir[6];
	static XMVECTOR updir[6];

	if( !init )
	{
		init = true;

		viewdir[(int)mtDirection::Forward] = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
		viewdir[(int)mtDirection::Backward] = XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
		viewdir[(int)mtDirection::Left] = XMVectorSet( -1.0f, 0.0f, 0.0f, 0.0f );
		viewdir[(int)mtDirection::Right] = XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f );
		viewdir[(int)mtDirection::Up] = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
		viewdir[(int)mtDirection::Down] = XMVectorSet( 0.0f, -1.0f, 0.0f, 0.0f );

		updir[(int)mtDirection::Forward] = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
		updir[(int)mtDirection::Backward] = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
		updir[(int)mtDirection::Left] = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
		updir[(int)mtDirection::Right] = XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f );
		updir[(int)mtDirection::Up] = XMVectorSet( 0.0f, 0.0f, -1.0f, 0.0f );
		updir[(int)mtDirection::Down] = XMVectorSet( 0.0f, 0.0f, 1.0f, 0.0f );
	}

	XMVECTOR pos = XMLoadFloat3( &position );
	XMVECTOR lookdir = XMVectorAdd( pos, viewdir[(int)dir] );

	view = XMMatrixLookAtLH( pos, lookdir, updir[(int)dir] );
}

void ShadowMap::SetProjection( const XMMATRIX &projection )
{
	this->proj = projection;
}

const XMMATRIX &ShadowMap::GetView() const
{
	return view;
}

const XMMATRIX &ShadowMap::GetProj() const
{
	return proj;
}
