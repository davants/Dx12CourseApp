#pragma once

#include "stdafx.h"

#include <DXGI1_4.h>
#include <d3d12.h>
#include <string>

using namespace std;

class myDirectX
{
public:
	myDirectX();
	~myDirectX();

	IDXGIFactory4 * getDxgiFactory(void);
	ID3D12Device * getD3dDevice(void);
	IDXGIAdapter1 * getHardwareAdapter(void);
	int getRenderTargetWidth(void);
	int getRenderTargetHeight(void);

	HRESULT Initialize(HWND vhMainWnd);



private:
	IDXGIFactory4 * mpDxgiFactory = nullptr;
	ID3D12Device * mpD3dDevice = nullptr;
	IDXGIAdapter1 * mpHardwareAdapter = nullptr;
	ID3D12CommandQueue * mpID3D12CommandQueue = nullptr;
	ID3D12CommandAllocator * mpID3D12CommandAllocator = nullptr;
	ID3D12GraphicsCommandList * mpID3D12GraphicsCommandList = nullptr;
	IDXGISwapChain * mpIDXGISwapChain = nullptr;


	HRESULT InitializeFactoryDeviceAndHardware(void);
	HRESULT GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter);
	HRESULT InitializeCommandObjects(void);
	HRESULT InitializeSwapChain(void);

	int mRenderTargetWidth = 800;
	int mRenderTargetHeight = 600;

	UINT m4xMsaaQuality = 0;

	HWND      mhMainWnd = nullptr; // handle to the application's "window"

};

