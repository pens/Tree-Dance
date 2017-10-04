#include "ImageConvert.h"
#include <memory>
#include <atlbase.h>
#include <wincodec.h>
using namespace std;

void ConvertImage(string filename, Texture* texture, bool bw) {
        CComPtr<IWICImagingFactory> wicFac;
        CComPtr<IWICBitmapDecoder> decoder;
        CComPtr<IWICBitmapFrameDecode> frame;
        CComPtr<IWICFormatConverter> converter;
        CoInitialize(0);
        CoCreateInstance(CLSID_WICImagingFactory,
            0,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&wicFac));
    
        //WIC requires wchar_t* (LPCWSTR) for the filename
        size_t size = strlen(filename.c_str()) + 1;
        size_t buffSize = 0;
        unique_ptr<wchar_t> wsource(new wchar_t[size]);
        mbstowcs_s(&buffSize, wsource.get(), size, filename.c_str(), size);
        wicFac->CreateDecoderFromFilename(wsource.get(),
            0,
            GENERIC_READ,
            WICDecodeMetadataCacheOnDemand,
            &decoder);
        decoder->GetFrame(0, &frame);
        wicFac->CreateFormatConverter(&converter);
        converter->Initialize(frame,
            !bw ? GUID_WICPixelFormat32bppRGBA : GUID_WICPixelFormat8bppGray,
            WICBitmapDitherTypeNone,
            0, 0,
            WICBitmapPaletteTypeMedianCut);
        UINT width, height;
        frame->GetSize(&width, &height);
        int bpp = bw ? 1 : 4;
        vector<char> data(width * bpp * height);
        converter->CopyPixels(0, width * bpp, width * bpp * height, 
                              (BYTE*)data.data());
    
        //Assume that width and height are the same
        texture->height = height;
        texture->width = width;
        texture->depth = !bw ? 4 : 1;
        texture->data = data;
}