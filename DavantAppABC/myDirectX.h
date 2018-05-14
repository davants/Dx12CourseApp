#pragma once

#include "stdafx.h"

#include <DXGI1_4.h>
#include <d3d12.h>
#include "d3dx12.h"
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

	void FlushCommandQueue();


	void OnResize();

private:
	IDXGIFactory4 * mpDxgiFactory = nullptr;
	ID3D12Device * mpD3dDevice = nullptr;
	IDXGIAdapter1 * mpHardwareAdapter = nullptr;
	ID3D12CommandQueue * mpID3D12CommandQueue = nullptr;
	ID3D12CommandAllocator * mpID3D12CommandAllocator = nullptr;
	ID3D12GraphicsCommandList * mpID3D12GraphicsCommandList = nullptr;
	IDXGISwapChain * mpIDXGISwapChain = nullptr;
	ID3D12DescriptorHeap * mpRtvID3D12DescriptorHeap = nullptr;
	ID3D12DescriptorHeap * mpDsvID3D12DescriptorHeap = nullptr;

	HRESULT InitializeFactoryDeviceAndHardware(void);
	HRESULT GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter);
	HRESULT InitializeCommandObjects(void);
	HRESULT InitializeSwapChain(void);
	HRESULT InitializeFence(void);
	void InitializeDescriptorSizes(void);
	HRESULT InitializeRtvAndDsvDescriptorHeaps(void);


	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView()const;
	D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView()const;

	//ID3D12Resource* CurrentBackBuffer()const;

	int mRenderTargetWidth = 800;
	int mRenderTargetHeight = 600;

	UINT m4xMsaaQuality = 0;

	HWND      mhMainWnd = nullptr; // handle to the application's "window"


	// Now using the book to finish Chapter 4. 
	// Below is new components that are added and initialized.
	ID3D12Fence * mpID3D12Fence = nullptr;
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvSrvUavDescriptorSize = 0;

	int mCurrBackBuffer = 0;
	// Views/Descriptors are pointers to other GPU resources.
	// We will need 2 views of the render targets and 1 view for the depth stencil.
	ID3D12Resource * mpSwapChainBufferViewList[2];
	ID3D12Resource * mpDepthStencilBufferView;

	UINT64 mCurrentFence = 0;


};

