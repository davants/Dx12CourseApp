#include "stdafx.h"

#include <DXGI1_4.h>
#include <d3d12.h>
#include <wrl.h>
#include <string>
#include <iostream>
#include <Windows.h>


#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3D12.lib")

using namespace Microsoft::WRL;
using namespace std;


#include "myDirectX.h"

myDirectX gMyDirectX;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	HRESULT hr;
	hr = gMyDirectX.Initialize();

	return 0;
}

