#include "pch.h"
#include <UniDx/D3DManager.h>

#include <windows.h>
#include <UniDx/ConstantBuffer.h>

namespace UniDx{

// Direct3Dを初期化し、使用できるようにする
bool D3DManager::Initialize(HWND hWnd, int width, int height)
{
	// ----------------------------------------------------------
	// ファクトリー作成(ビデオ グラフィックの設定の列挙や指定に使用されるオブジェクト)
	// ----------------------------------------------------------
	ComPtr<IDXGIFactory> factory;

	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory))))
	{
		return false;
	}

	// ----------------------------------------------------------
	// デバイス生成(主にリソース作成時に使用するオブジェクト)
	// ----------------------------------------------------------
	UINT creationFlags = 0;

#ifdef _DEBUG
	// DEBUGビルド時はDirect3Dのデバッグを有効にする
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,	// Direct3D 11.1  ShaderModel 5
		D3D_FEATURE_LEVEL_11_0,	// Direct3D 11    ShaderModel 5
		D3D_FEATURE_LEVEL_10_1,	// Direct3D 10.1  ShaderModel 4
		D3D_FEATURE_LEVEL_10_0,	// Direct3D 10.0  ShaderModel 4
		D3D_FEATURE_LEVEL_9_3,	// Direct3D 9.3   ShaderModel 3
		D3D_FEATURE_LEVEL_9_2,	// Direct3D 9.2   ShaderModel 3
		D3D_FEATURE_LEVEL_9_1,	// Direct3D 9.1   ShaderModel 3
	};

	// デバイスとでデバイスコンテキストを作成
	D3D_FEATURE_LEVEL futureLevel;
	if (FAILED(D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		creationFlags,
		featureLevels,
		_countof(featureLevels),
		D3D11_SDK_VERSION,
		&m_device,
		&futureLevel,
		&m_context)))
	{
		return false;
	}

	// ----------------------------------------------------------
	// スワップチェイン作成(フロントバッファに表示可能なバックバッファを持つもの)
	// ----------------------------------------------------------
	DXGI_SWAP_CHAIN_DESC scDesc = {};		// スワップチェーンの設定データ
	scDesc.BufferDesc.Width = width;						// 画面の幅
	scDesc.BufferDesc.Height = height;						// 画面の高さ
	scDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// バッファの形式
	scDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	scDesc.BufferDesc.RefreshRate.Numerator = 0;
	scDesc.BufferDesc.RefreshRate.Denominator = 1;
	scDesc.SampleDesc.Count = 1;							// MSAAは使用しない
	scDesc.SampleDesc.Quality = 0;							// MSAAは使用しない
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// バッファの使用方法
	scDesc.BufferCount = 2;									// バッファの数
	scDesc.OutputWindow = hWnd;
	scDesc.Windowed = TRUE;									// ウィンドウモード
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// スワップチェインの作成
	if (FAILED(factory->CreateSwapChain(m_device.Get(), &scDesc, &m_swapChain)))
	{
		return false;
	}

	// スワップチェインからバックバッファリソース取得
	ComPtr<ID3D11Texture2D> pBackBuffer;
	if (FAILED(m_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
	{
		return false;
	}

	// バックバッファリソース用のRTVを作成
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = scDesc.BufferDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	if (FAILED(m_device->CreateRenderTargetView(pBackBuffer.Get(), &rtvDesc, &m_renderTarget)))
	{
		return false;
	}

	// ----------------------------------------------------------
	// 深度ステンシルバッファとビューの作成
	// ----------------------------------------------------------
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	if (FAILED(m_device->CreateTexture2D(&depthDesc, nullptr, &m_depthStencilBuffer)))
	{
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = depthDesc.Format;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;

	if (FAILED(m_device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, &m_depthStencilView)))
	{
		return false;
	}

	// ----------------------------------------------------------
	// 深度ステンシルステートの作成
	// ----------------------------------------------------------
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE; // 深度テスト有効
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS; // 小さい値が手前

	dsDesc.StencilEnable = FALSE;

	if (FAILED(m_device->CreateDepthStencilState(&dsDesc, &m_depthStencilState)))
	{
		return false;
	}

	// ----------------------------------------------------------
	// デバイスコンテキストに描画に関する設定を行っておく
	// ----------------------------------------------------------

	// バックバッファをRTとしてセット
	m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), m_depthStencilView.Get());

	// ビューポートの設定
	D3D11_VIEWPORT vp = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	m_context->RSSetViewports(1, &vp);

	// 深度ステンシルステートのセット
	m_context->OMSetDepthStencilState(m_depthStencilState.Get(), 1);

	screenSize.x = float(width);
	screenSize.y = float(height);
	currentRenderingMode = RenderingMode_Opaque;

	return true;
}


// バックバッファレンダーターゲットをクリア
void D3DManager::Clear(float r, float g, float b, float a)
{
	const float color[4] = { r, g, b, a };
	m_context->ClearRenderTargetView(m_renderTarget.Get(), color);
//	m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), nullptr); // 深度ステンシル未使用
	m_context->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	m_context->OMSetRenderTargets(1, m_renderTarget.GetAddressOf(), m_depthStencilView.Get());
}

D3DManager::~D3DManager()
{
	m_context->ClearState();
	m_context->Flush();
	m_context.Reset();
	m_renderTarget.Reset();
}


} // UniDx
