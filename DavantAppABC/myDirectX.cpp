#include "stdafx.h"
#include "myDirectX.h"


myDirectX::myDirectX()
{
}


myDirectX::~myDirectX()
{
	if (mpDxgiFactory != nullptr)
	{
		mpDxgiFactory->Release();
	}

	if (mpHardwareAdapter != nullptr)
	{
		mpHardwareAdapter->Release();
	}

	if (mpD3dDevice != nullptr)
	{
		mpD3dDevice->Release();
	}

	if (mpID3D12CommandQueue != nullptr)
	{
		mpID3D12CommandQueue->Release();
	}

	if (mpID3D12CommandAllocator != nullptr)
	{
		mpID3D12CommandAllocator->Release();
	}

	if (mpID3D12GraphicsCommandList != nullptr)
	{
		mpID3D12GraphicsCommandList->Release();
	}

	if (mpIDXGISwapChain != nullptr)
	{
		mpIDXGISwapChain->Release();
	}

	if (mpID3D12Fence != nullptr)
	{
		mpID3D12Fence->Release();
	}

	if (mpRtvID3D12DescriptorHeap != nullptr)
	{
		mpRtvID3D12DescriptorHeap->Release();
	}
	
	if (mpDsvID3D12DescriptorHeap != nullptr)
	{
		mpDsvID3D12DescriptorHeap->Release();
	}

	for (int i = 0; i < 2; i++)
	{
		if (mpSwapChainBufferViewList[i] != nullptr)
		{
			mpSwapChainBufferViewList[i]->Release();
		}
	}

	if (mpDepthStencilBufferView != nullptr)
	{
		mpDepthStencilBufferView->Release();
	}


}

IDXGIFactory4 * myDirectX::getDxgiFactory(void)
{
	return mpDxgiFactory;
}

ID3D12Device * myDirectX::getD3dDevice(void)
{
	return mpD3dDevice;
}

IDXGIAdapter1 * myDirectX::getHardwareAdapter(void)
{
	return mpHardwareAdapter;
}

int myDirectX::getRenderTargetWidth(void)
{
	return mRenderTargetWidth;
}

int myDirectX::getRenderTargetHeight(void)
{
	return mRenderTargetHeight;
}

HRESULT myDirectX::Initialize(HWND vhMainWnd)
{
	mhMainWnd = vhMainWnd;

	HRESULT hr;
	hr = InitializeFactoryDeviceAndHardware();

	if (hr == S_OK)
	{
		hr = InitializeCommandObjects();
	}

	if (hr == S_OK)
	{
		hr = InitializeSwapChain();
	}

	if (hr == S_OK)
	{
		hr = InitializeFence();
	}

	InitializeDescriptorSizes();

	if (hr == S_OK)
	{
		hr = InitializeRtvAndDsvDescriptorHeaps();
	}



	return hr;
}

void myDirectX::FlushCommandQueue()
{
	// Advance the fence number
	mCurrentFence++;

	// Give the fence, with the new number, to the command queue.
	mpID3D12CommandQueue->Signal(mpID3D12Fence, mCurrentFence);

	// We just added a command to the command queue.
	// When the GPU runs this command, it will update the mpID3D12Fence with the new number.

	// Check the number in the fence, maybe it already set it!  Probably not, but we will check.
	if (mpID3D12Fence->GetCompletedValue() < mCurrentFence)
	{
		// The signal command has not been run yet... so wait for the signal.

		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		HRESULT hr = mpID3D12Fence->SetEventOnCompletion(mCurrentFence, eventHandle);

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);

	}

}

void myDirectX::OnResize()
{
	if (mpD3dDevice == nullptr)
	{
		return;
	}
	if (mpIDXGISwapChain == nullptr)
	{
		return;
	}
	if (mpID3D12CommandAllocator == nullptr)
	{
		return;
	}

	// Flush before changing any resources.
	FlushCommandQueue();

	mpID3D12GraphicsCommandList->Reset(mpID3D12CommandAllocator, nullptr);

	// Release the previous resources we will be recreating.
	for (int i = 0; i < 2; ++i)
	{
		mpSwapChainBufferViewList[i]->Release();
	}

	mpDepthStencilBufferView->Release();

	// Resize the swap chain.
	HRESULT hr = mpIDXGISwapChain->ResizeBuffers(
		2, // front and back buffer
		mRenderTargetWidth, mRenderTargetHeight,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	mCurrBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mpRtvID3D12DescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (UINT i = 0; i < 2; i++)
	{
		mpIDXGISwapChain->GetBuffer(i, IID_PPV_ARGS(&mpSwapChainBufferViewList[i]));

		mpD3dDevice->CreateRenderTargetView(mpSwapChainBufferViewList[i], nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}


	// Now for the Depth Stencil resource.

	// Fill out a struct that will tell the GPU what we want it to create.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mRenderTargetWidth;
	depthStencilDesc.Height = mRenderTargetHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1; // book has m4xMsaaQuality ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;



}

HRESULT myDirectX::InitializeFactoryDeviceAndHardware(void)
{
	HRESULT vResult;

	vResult = CreateDXGIFactory1(IID_PPV_ARGS(&mpDxgiFactory));

	vResult = GetHardwareAdapter(mpDxgiFactory, &mpHardwareAdapter);

	if (vResult == S_OK)
	{
		vResult = D3D12CreateDevice(
			mpHardwareAdapter,
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&mpD3dDevice)
		);

		if (vResult == S_OK)
		{
			// Let's see how much memory this video card GPU has...
			DXGI_ADAPTER_DESC1 vDXGI_ADAPTER_DESC1;
			mpHardwareAdapter->GetDesc1(&vDXGI_ADAPTER_DESC1);

			//wstring vDedicatedVideoMemory = std::to_wstring(vDXGI_ADAPTER_DESC1.DedicatedVideoMemory);
			//wstring vOutputMessage = L"Success - ID3D12Device was created and the video card memory is " + vDedicatedVideoMemory;
			//MessageBox(nullptr, vOutputMessage.c_str(), L"Working!!", MB_OK);


		

			// Check 4X MSAA quality support for our back buffer format.
			// All Direct3D 11 capable devices support 4X MSAA for all render 
			// target formats, so we only need to check quality support.

			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
			msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			msQualityLevels.SampleCount = 1;
			msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
			msQualityLevels.NumQualityLevels = 0;
			HRESULT vFeatureResult = mpD3dDevice->CheckFeatureSupport(
				D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
				&msQualityLevels,
				sizeof(msQualityLevels));

			m4xMsaaQuality = msQualityLevels.NumQualityLevels;
			
			// https://msdn.microsoft.com/en-us/library/windows/desktop/bb173072(v=vs.85).aspx
			// "The valid range is between zero and one less than the level returned by ..."
			if (m4xMsaaQuality > 0)
			{
				m4xMsaaQuality--;
			}

		}
		else
		{
			MessageBox(nullptr, L"FAIL, adapter was created but ID3D12Device was not!", L"Working, but we have a problem!", MB_OK);
		}
	}
	else
	{
		MessageBox(nullptr, L"FAIL to create the adapter.", L"Working, but we have a problem!", MB_OK);
	}

	
	return vResult;
}


HRESULT myDirectX::GetHardwareAdapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter)
{
	*ppAdapter = nullptr;
	SIZE_T vMemory = 0;
	IDXGIAdapter1* pAdapterTEMP = nullptr;
	HRESULT vResult = E_FAIL;

	for (UINT adapterIndex = 0; true; ++adapterIndex)
	{
		if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapterTEMP))
		{
			// No more adapters to enumerate.
			break;
		}

		// Output info on the video card
		// Create a struct to hold the adapter's information and call GetDesc1 to fill it with data.
		DXGI_ADAPTER_DESC1 vDXGI_ADAPTER_DESC1;
		pAdapterTEMP->GetDesc1(&vDXGI_ADAPTER_DESC1);
		wstring sStart = L"\n\nVideo Card Info and memory: ";
		wstring sAdapterDescription = vDXGI_ADAPTER_DESC1.Description;
		wstring sSpace = L" ";
		wstring sAdapterMemory = std::to_wstring(vDXGI_ADAPTER_DESC1.DedicatedVideoMemory);
		wstring sEnd = L"\n";
		wstring sCombined = sStart + sAdapterDescription + sSpace + sAdapterMemory + sEnd;
		OutputDebugString(sCombined.c_str());



		// Check to see if the adapter supports Direct3D 12, but don't create the
		// actual device yet.
		if (SUCCEEDED(D3D12CreateDevice(pAdapterTEMP, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			// If here then the card meets the feature levels we need.
			wstring s = L"   Found One! This video card meets the Level 11_0 features!";
			OutputDebugString(s.c_str());

			

			// Another video card may have passed the DX feature requirement, so check if this one has more memory.
			// If it does have more memory then call Release() of the smaller one we don't want anymore.
			if (vDXGI_ADAPTER_DESC1.DedicatedVideoMemory > vMemory)
			{
				// This "if" checks to see if we already chose an adapter.
				if ((*ppAdapter) != nullptr)
				{
					(*ppAdapter)->Release();
					*ppAdapter = nullptr;
				}

				// Update the video memory of our local variable vMemory so we can check against other cards.
				vMemory = vDXGI_ADAPTER_DESC1.DedicatedVideoMemory;

				
				// Set the adapter parameter, dereference it so we are updating the client's variable.
				*ppAdapter = pAdapterTEMP;




			







				vResult = S_OK;
			}
			else
			{
				// This card has the needed features, but does not have as much memory as another card so call Release()
				// because we won't use it.
				pAdapterTEMP->Release();
			}
		}
		else
		{
			pAdapterTEMP->Release();
		}
	}

	return vResult;
}

HRESULT myDirectX::InitializeCommandObjects(void)
{
	HRESULT vResult = E_FAIL;

	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	vResult = mpD3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mpID3D12CommandQueue));
	
	if(vResult != S_OK)
	{
		MessageBox(nullptr, L"FAIL, ID3D12CommandQueue was not created!", L"Working, but we have a problem!", MB_OK);
		return vResult;
	}
	
	// Now the command allocator
	vResult = mpD3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&mpID3D12CommandAllocator));

	if (vResult != S_OK)
	{
		MessageBox(nullptr, L"FAIL, Command Allocator was not created!", L"Working, but we have a problem!", MB_OK);
		return vResult;
	}

	// Now the command list
	vResult = mpD3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, mpID3D12CommandAllocator, nullptr, 
		IID_PPV_ARGS(&mpID3D12GraphicsCommandList));
		
	if (vResult != S_OK)
	{
		MessageBox(nullptr, L"FAIL, Command List was not created!", L"Working, but we have a problem!", MB_OK);
		return vResult;
	}

	mpID3D12GraphicsCommandList->Close();	

	//if (vResult == S_OK)
	//{
	//	wstring vOutputMessage = L"Success - Command Objects were created";
	//	MessageBox(nullptr, vOutputMessage.c_str(), L"Working!!", MB_OK);
	//}

	return vResult;
}

HRESULT myDirectX::InitializeSwapChain(void)
{
	DXGI_SWAP_CHAIN_DESC sd;

	// Render target width and height. The is the size of our screen which should match the window we created.
	// We will set them our new member variables. The window also uses these same values so both will
	// be the same.
	sd.BufferDesc.Width = mRenderTargetWidth;
	sd.BufferDesc.Height = mRenderTargetHeight;

	// This is the refresh rate of the images we want on the screen. 	
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;

	// The swap chain will create the buffers and we need to specify what type of image format we want.
	// We will use standard Red, Green, Blue, Alpha. (Alpha can be used for transparency).
	// Notice the "R8", "G8", "B8". This means each pixel will hold an 8 bits (one byte) for each color plus alpha.
	// This means that each pixel will be 4 bytes or 32 bits.
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	// The next 2 we will not cover, but they need to be set.
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Sampling is covered later. Basically, it says to look at other pixels around each pixel and do a blend 
	// to make things smoother.
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = m4xMsaaQuality; // This was already reduced by one.

											// What are these buffers used for?  Render targets.
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	// We want 2 buffers; front and back.
	sd.BufferCount = 2;

	// Set the Window's window to draw to.
	sd.OutputWindow = mhMainWnd;

	// Not full screen.
	sd.Windowed = true;

	// After we have used a buffer, throw away the data.
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// Misc setting.
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hr;

	// Now create the swap chain using the struct we just filled out.
	hr = mpDxgiFactory->CreateSwapChain(
		mpID3D12CommandQueue,
		&sd,
		&mpIDXGISwapChain);

	if (hr != S_OK)
	{
		throw(new exception("error creating swap chain"));
	}

	//if (hr == S_OK)
	//{
	//	wstring vOutputMessage = L"Success - The Swap Chain was Created!";
	//	MessageBox(nullptr, vOutputMessage.c_str(), L"Working!!", MB_OK);
	//}


	return hr;

}

HRESULT myDirectX::InitializeFence(void)
{
	HRESULT vResult = mpD3dDevice->CreateFence(
		0,
		D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&mpID3D12Fence));

	return vResult;
}

void myDirectX::InitializeDescriptorSizes(void)
{
	mRtvDescriptorSize = mpD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = mpD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvSrvUavDescriptorSize = mpD3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	
}

HRESULT myDirectX::InitializeRtvAndDsvDescriptorHeaps(void)
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	HRESULT vResult = mpD3dDevice->CreateDescriptorHeap(
		&rtvHeapDesc, IID_PPV_ARGS(&mpRtvID3D12DescriptorHeap));


	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	vResult = mpD3dDevice->CreateDescriptorHeap(
		&dsvHeapDesc, IID_PPV_ARGS( &mpDsvID3D12DescriptorHeap));


	if (vResult != S_OK)
	{
		throw(new exception("error creating descriptor heaps"));
	}

	if (vResult == S_OK)
	{
		wstring vOutputMessage = L"Success - The descriptor heaps were Created!";
		MessageBox(nullptr, vOutputMessage.c_str(), L"Working!!", MB_OK);
	}


	return vResult;
}

D3D12_CPU_DESCRIPTOR_HANDLE myDirectX::CurrentBackBufferView() const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		mpRtvID3D12DescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		mCurrBackBuffer,
		mRtvDescriptorSize);
}

D3D12_CPU_DESCRIPTOR_HANDLE myDirectX::DepthStencilView() const
{
	return mpDsvID3D12DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}


//ID3D12Resource* myDirectX::CurrentBackBuffer()const
//{
//	return mpSwapChainBufferList[mCurrBackBuffer];
//	
//}
