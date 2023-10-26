#include "WindowVideoDestination.h"
#include "Error.h"
#include "Frame.h"
#include <opencv2/core/directx.hpp>
#include <opencv2/imgproc.hpp>

using namespace OpenVCR;

WindowVideoDestination::WindowVideoDestination()
{
	this->windowHandle = nullptr;
    this->windowWidth = 0;
    this->windowHeight = 0;
    this->device = nullptr;
    this->swapChain = nullptr;
    this->deviceContext = nullptr;
    this->backBufferTexture = nullptr;
    this->frameTexture = nullptr;
    this->renderTargetView = nullptr;
    this->openCLContext = new cv::ocl::Context();
}

/*virtual*/ WindowVideoDestination::~WindowVideoDestination()
{
    delete this->openCLContext;
}

void WindowVideoDestination::SetWindowHandle(HWND windowHandle)
{
	this->windowHandle = windowHandle;
}

HWND WindowVideoDestination::GetWindowHandle()
{
	return this->windowHandle;
}

/*virtual*/ bool WindowVideoDestination::PowerOn(Error& error)
{
    RECT clientRect;
    if (!::GetClientRect(this->windowHandle, &clientRect))
    {
        error.Add("Could not get client area dimensions.");
        return false;
    }

    this->windowWidth = clientRect.right - clientRect.left;
    this->windowHeight = clientRect.bottom - clientRect.top;

    DXGI_SWAP_CHAIN_DESC swapChainDesc;

    ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.Width = this->windowWidth;
    swapChainDesc.BufferDesc.Height = this->windowHeight;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = this->windowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    HRESULT result = ::D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &this->swapChain, &this->device, nullptr, &this->deviceContext);
    if (result != S_OK)
    {
        error.Add(std::format("D3D11CreateDeviceAndSwapChain failed with error code: {}", result));
        return false;
    }

    result = this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&this->backBufferTexture);
    if (result != S_OK)
    {
        error.Add(std::format("GetBuffer on swap chain failed with error code: {}", result));
        return false;
    }

    result = this->device->CreateRenderTargetView(this->backBufferTexture, nullptr, &this->renderTargetView);
    if (result != S_OK)
    {
        error.Add(std::format("CreateRenderTargetView failed with error code: {}", result));
        return false;
    }

    this->deviceContext->OMSetRenderTargets(1, &this->renderTargetView, nullptr);

    D3D11_VIEWPORT viewport;
    ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));
    viewport.Width = float(this->windowWidth);
    viewport.Height = float(this->windowHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 0.0f;

    this->deviceContext->RSSetViewports(1, &viewport);

    D3D11_TEXTURE2D_DESC textureDesc;

    textureDesc.Width = this->windowWidth;
    textureDesc.Height = this->windowHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    textureDesc.MiscFlags = 0;

    result = this->device->CreateTexture2D(&textureDesc, nullptr, &this->frameTexture);
    if (result != S_OK)
    {
        error.Add(std::format("CreateTexture2D failed with error code: {}", result));
        return false;
    }

    /*if (cv::ocl::haveOpenCL())        Do we really need this?
    {
        *this->openCLContext = cv::directx::ocl::initializeContextFromD3D11Device(this->device);
    }*/

    return true;
}

/*virtual*/ bool WindowVideoDestination::PowerOff(Error& error)
{
    if (this->frameTexture)
    {
        this->frameTexture->Release();
        this->frameTexture = nullptr;
    }

    if (this->device)
    {
        this->device->Release();
        this->device = nullptr;
    }

    if (this->deviceContext)
    {
        this->deviceContext->Release();
        this->deviceContext = nullptr;
    }

    if (this->swapChain)
    {
        this->swapChain->Release();
        this->swapChain = nullptr;
    }

    this->backBufferTexture = nullptr;
    this->windowWidth = 0;
    this->windowHeight = 0;

	return true;
}

/*virtual*/ bool WindowVideoDestination::AddFrame(Frame& frame, Error& error)
{
    // We need the frame in a format where we can copy it directly into the direct-X texture resource.
    cv::Mat frameRGBA;
    cv::cvtColor(*frame.data, frameRGBA, cv::COLOR_BGR2RGBA);

    // Map the dirct-X texture into memory.
    UINT subResource = ::D3D11CalcSubresource(0, 0, 1);
    D3D11_MAPPED_SUBRESOURCE mappedTextureResource;
    HRESULT result = this->deviceContext->Map(this->frameTexture, subResource, D3D11_MAP_WRITE_DISCARD, 0, &mappedTextureResource);
    if (result != S_OK)
    {
        error.Add(std::format("Failed to map texture resource into memory with error code: {}", result));
        return false;
    }

    // Now do the copy and unmap.
    cv::Mat mappedTexture(this->windowHeight, this->windowWidth, CV_8UC4, mappedTextureResource.pData, mappedTextureResource.RowPitch);
    frameRGBA.copyTo(mappedTexture);
    this->deviceContext->Unmap(this->frameTexture, subResource);

    // Lastly, just blit the frame texture into the back-buffer and present.
    this->deviceContext->CopyResource(this->backBufferTexture, this->frameTexture);
    result = this->swapChain->Present(0, 0);
    if (result != S_OK)
    {
        error.Add(std::format("Swap-chain present failed with error code: {}", result));
        return false;
    }

	return true;
}