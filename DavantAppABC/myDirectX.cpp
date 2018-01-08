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

HRESULT myDirectX::Initialize(void)
{
	HRESULT hr;
	hr = InitializeFactoryDeviceAndHardware();

	if (hr == S_OK)
	{
		hr = InitializeCommandObjects();
	}

	return hr;
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
	
	if (vResult == S_OK)
	{
		wstring vOutputMessage = L"Success - ID3D12CommandQueue was created";
		MessageBox(nullptr, vOutputMessage.c_str(), L"Working!!", MB_OK);
	}
	else
	{
		MessageBox(nullptr, L"FAIL, ID3D12CommandQueue was not created!", L"Working, but we have a problem!", MB_OK);
	}


	return vResult;
}
