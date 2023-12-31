#include "WindowVideoDestination.h"
#include "Machine.h"
#include "Error.h"
#include <opencv2/core/directx.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace OpenVCR;

WindowVideoDestination::WindowVideoDestination(const std::string& givenName) : VideoDevice(givenName)
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
    this->renderMode = RenderMode::MAINTAIN_ASPECT_RATIO;
}

/*virtual*/ WindowVideoDestination::~WindowVideoDestination()
{
}

/*static*/ WindowVideoDestination* WindowVideoDestination::Create(const std::string& name)
{
    return new WindowVideoDestination(name);		// Allocate class in this DLL's heap!
}

void WindowVideoDestination::SetWindowHandle(HWND windowHandle)
{
	this->windowHandle = windowHandle;
}

HWND WindowVideoDestination::GetWindowHandle()
{
	return this->windowHandle;
}

void WindowVideoDestination::SetRenderMode(RenderMode renderMode)
{
    this->renderMode = renderMode;
}

WindowVideoDestination::RenderMode WindowVideoDestination::GetRenderMode()
{
    return this->renderMode;
}

/*virtual*/ bool WindowVideoDestination::PowerOn(Machine* machine, Error& error)
{
    if (this->GetNumSourceNames() != 1)
    {
        error.Add("Window video destination expected exactly one source.");
        return false;
    }

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

    if (!this->SetupRenderTargetViewAndViewport(error))
        return false;

    if (!this->CreateFrameTexture(error))
        return false;

    this->poweredOn = true;
    return true;
}

/*virtual*/ bool WindowVideoDestination::PowerOff(Machine* machine, Error& error)
{
    if (this->frameTexture)
    {
        this->frameTexture->Release();
        this->frameTexture = nullptr;
    }

    if (this->renderTargetView)
    {
        this->renderTargetView->Release();
        this->renderTargetView = nullptr;
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

    this->poweredOn = false;
	return true;
}

bool WindowVideoDestination::SetupRenderTargetViewAndViewport(Error& error)
{
    if (this->backBufferTexture)
    {
        error.Add("Back-buffer texture already set!");
        return false;
    }

    HRESULT result = this->swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&this->backBufferTexture);
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

    return true;
}

bool WindowVideoDestination::WindowSizeChanged(Error& error)
{
    RECT clientRect;
    if (!::GetClientRect(this->windowHandle, &clientRect))
    {
        error.Add("Could not get client area dimensions.");
        return false;
    }

    int newWidth = clientRect.right - clientRect.left;
    int newHeight = clientRect.bottom - clientRect.top;

    if (newWidth == this->windowWidth && newHeight == this->windowHeight)
        return true;

    if (newWidth == 0 || newHeight == 0)
    {
        error.Add(std::format("Ignoring degenerate size: {} x {}", newWidth, newHeight));
        return false;
    }

    this->windowWidth = newWidth;
    this->windowHeight = newHeight;

    // Before we can resize the swap-chain, we have to let go of any references to it.
    this->backBufferTexture->Release();
    this->backBufferTexture = nullptr;
    this->deviceContext->OMSetRenderTargets(0, nullptr, nullptr);       // Unbind the render target.
    this->renderTargetView->Release();
    this->renderTargetView = nullptr;

    // Let go of our frame texture too.  It's not referencing the swap-chain, but now is a good time to do it.
    this->frameTexture->Release();
    this->frameTexture = nullptr;
    
    // A flush is needed to make all the releases final, I think.  Or, to make sure the GPU is done with them too, I suppose.
    this->deviceContext->Flush();

    // Now it should be safe to resize the swap-chain.
    HRESULT result = this->swapChain->ResizeBuffers(0, this->windowWidth, this->windowHeight, DXGI_FORMAT_UNKNOWN, 0);
    if (result != S_OK)
    {
        error.Add(std::format("Failed to resize swapchain with error code: {}", result));
        return false;
    }

    // Now I think we need to recreate our render target view and rebind it.
    if (!this->SetupRenderTargetViewAndViewport(error))
        return false;

    // Re-create our frame-texture.
    if (!this->CreateFrameTexture(error))
        return false;

    return true;
}

bool WindowVideoDestination::CreateFrameTexture(Error& error)
{
    if (this->frameTexture)
    {
        error.Add("Frame texture already created.");
        return false;
    }

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

    HRESULT result = this->device->CreateTexture2D(&textureDesc, nullptr, &this->frameTexture);
    if (result != S_OK)
    {
        error.Add(std::format("CreateTexture2D failed with error code: {}", result));
        return false;
    }

    return true;
}

/*virtual*/ bool WindowVideoDestination::MoveData(Machine* machine, Error& error)
{
    VideoDevice* videoDevice = machine->FindIODevice<VideoDevice>(this->GetSourceName(0));
    if (!videoDevice)
    {
        error.Add(std::format("Window video destination failed to find video device with name \"{}\".", this->GetSourceName(0).c_str()));
        return false;
    }

    // We can't do anything until our video source is complete.
    if (!videoDevice->IsComplete())
        return true;

    cv::Mat* sourceFrame = videoDevice->GetFrameData();
    if (!sourceFrame)
    {
        error.Add("Video source did not have any frame data for us.");
        return false;
    }

    // We need the frame in a format where we can copy it directly into the direct-X texture resource.
    cv::Mat frameRGBA;
    cv::cvtColor(*sourceFrame, frameRGBA, cv::COLOR_BGR2RGBA);

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
    this->RenderFrameIntoTexture(frameRGBA, mappedTexture);
    this->deviceContext->Unmap(this->frameTexture, subResource);

    // Lastly, just blit the frame texture into the back-buffer and present.
    this->deviceContext->CopyResource(this->backBufferTexture, this->frameTexture);
    result = this->swapChain->Present(0, 0);
    if (result != S_OK)
    {
        error.Add(std::format("Swap-chain present failed with error code: {}", result));
        return false;
    }

    this->complete = true;
	return true;
}

void WindowVideoDestination::RenderFrameIntoTexture(cv::Mat& frame, cv::Mat& texture)
{
    switch (this->renderMode)
    {
        case RenderMode::STRETCH:
        {
            cv::resize(frame, texture, texture.size());
            break;
        }
        case RenderMode::MAINTAIN_ASPECT_RATIO:
        {
            // Find a sub-rectangle of the frame that is centered and as large as possible and having the same aspect ratio as the texture.

            cv::Point2i upperLeftCorner;
            cv::Point2i lowerRightCorner;

            double desiredAspectRatio = double(texture.size().width) / double(texture.size().height);
            double frameAspectRatio = double(frame.size().width) / double(frame.size().height);

            if (frameAspectRatio > desiredAspectRatio)
            {
                int delta = (int)::floor(0.5 * (double(frame.size().width) - double(texture.size().width * frame.size().height) / double(texture.size().height)));
                upperLeftCorner.x = delta;
                upperLeftCorner.y = 0;
                lowerRightCorner.x = frame.size().width - delta;
                lowerRightCorner.y = frame.size().height;
            }
            else if (frameAspectRatio < desiredAspectRatio)
            {
                int delta = (int)::floor(0.5 * (double(frame.size().height) - double(texture.size().height * frame.size().width) / double(texture.size().width)));
                upperLeftCorner.x = 0;
                upperLeftCorner.y = delta;
                lowerRightCorner.x = frame.size().width;
                lowerRightCorner.y = frame.size().height - delta;
            }
            else
            {
                upperLeftCorner.x = 0;
                upperLeftCorner.y = 0;
                lowerRightCorner.x = frame.size().width;
                lowerRightCorner.y = frame.size().height;
            }

            cv::Mat subFrame = frame(cv::Rect(upperLeftCorner, lowerRightCorner));
            cv::resize(subFrame, texture, texture.size());

            break;
        }
    }
}