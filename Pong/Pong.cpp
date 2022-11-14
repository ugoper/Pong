// Pong.cpp : Definisce il punto di ingresso dell'applicazione.
//


#include "pch.h" // intestazione di base Visual Studio
#include <windows.h> //  dichiarazioni in C/C++ per le funzioni delle Windows API
#include <d2d1.h> // header per Direct 2D
#pragma comment(lib, "d2d1") // inserisce record di ricerca  per d2d1. Se tolgo dà errore LNK2019 e LNK1120 esterni non risolti
#include <dwrite.h>
#pragma comment(lib, "dwrite") // inserisce record di ricerca  per dwrite. Se tolgo dà errore LNK2019 e LNK1120 esterni non risolti

#include <iostream>
#include <string>
#include <stdio.h>
using std::string;

#include "basewin.h" // include la classe BaseWindow
#include "framework.h"
#include "Pong.h"

// contiene le dimensioni della finestra
float winX=0, winY=0;

template <class T> void SafeRelease(T** ppT) // template per rilasciare puntatori dalla heap
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

class MainWindow : public BaseWindow<MainWindow>  // definisce class MainWindow sulla base di BaseWindow con funzini WindowProc e Create
{
    ID2D1Factory* pFactory; // Interfaccia ID2D1Factory (d2d1.h), crea risorse Direct2D
    IDWriteFactory* pWriteFactory; //Interfaccia DWrite.h
    ID2D1HwndRenderTarget* pRenderTarget; // Interfaccia ID2D1HwndRenderTarget (d2d1.h), Esegue il rendering delle istruzioni di disegno in una finestra
    ID2D1SolidColorBrush* pBrush; // Interfaccia ID2D1SolidColorBrush (d2d1.h), Disegna un'area con un colore a tinta unita
    IDWriteTextFormat* pTextF;

    D2D1_RECT_F highBorder={0,0,0,0}, lowBorder = { 0,0,0,0 }, userBar = { 0,0,0,0 };

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize();

public:

    MainWindow() : pFactory(NULL), pWriteFactory(NULL), pRenderTarget(NULL), pBrush(NULL), pTextF(NULL)
    {
    }

    PCWSTR  ClassName() const { return L"Circle Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// Recalculate drawing layout when the size of the window changes.

void MainWindow::CalculateLayout() // draw the screen
{
    if (pRenderTarget != NULL)
    {
        highBorder = D2D1::RectF(0, winY/25, winX, winY/20);
        lowBorder = D2D1::RectF(0, winY *94 / 100, winX, winY);
    }
}

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);
        // creazione del formato testo
        hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(pWriteFactory), reinterpret_cast<IUnknown**>(&pWriteFactory));
        if (SUCCEEDED(hr)) {
            hr = pWriteFactory->CreateTextFormat(L"Verdana", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 24, L"", &pTextF);
        }
        // creazione del pennello
        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);
        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(1.0f, 1.0f, 1.0f);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);
            if (SUCCEEDED(hr))
            {
                CalculateLayout();
            }
        }
    }
    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        WCHAR sc_testo[100] = L"Score";
        int lungh = 0;
       
        BeginPaint(m_hwnd, &ps);

        pRenderTarget->BeginDraw();

        // imposta lo sfondo nero
        pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
        // disegna la barra superiore
        pRenderTarget->FillRectangle(highBorder, pBrush);
        // disegna la barra inferiore
        pRenderTarget->FillRectangle(lowBorder, pBrush);
        // scrive il testo sulla barra superiore
        lungh = swprintf_s(sc_testo, 100, L"Cord X %0.f - Cord Y  %0.f - Score", winX, winY);
        pRenderTarget->DrawTextW(sc_testo, lungh,pTextF, D2D1::RectF(0, 0, winX, 100),pBrush);

        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }
        EndPaint(m_hwnd, &ps);
    }
}

void MainWindow::Resize()
{
    if (pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        CalculateLayout();
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ PWSTR, _In_ int nCmdShow)
{
    MainWindow win;
    winX = static_cast <float>(GetSystemMetrics(SM_CXSCREEN));
    winY = static_cast<float>(GetSystemMetrics(SM_CYSCREEN)-200);
    // WS_OVERLAPPEDWINDOW
    if (!win.Create(L"Circle", WS_CAPTION | WS_SYSMENU | SWP_NOSIZE,0,0,0,static_cast<int>(winX), static_cast<int>(winY)))
    {
        return 0;
    }

    ShowWindow(win.Window(), nCmdShow);

    // Run the message loop.

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;  // Fail CreateWindowEx.
        }
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        OnPaint();
        return 0;



    case WM_SIZE:
        Resize();
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}