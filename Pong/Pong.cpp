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
// definisce la classe della pallina

// contiene le dimensioni della finestra
float winX=0, winY=0;

//definisce l'arka

class arka {
public:
    SYSTEMTIME tempo = { 0 };
    D2D1_RECT_F aspetto{ 0,0,0,0 };
    int velX = 5;
    int velY = 5;
    int score = 0;

    int muovi(int direzione) {
        // movimento se la direzione è verso l'alto
        if (direzione > 0) {
            aspetto.bottom -= velX;
            aspetto.top -= velX;
            // controllo che non sia arrivato al bordo
            float bordoYa = winY / 25;
            if (aspetto.top < bordoYa) {
                aspetto.bottom += velX;
                aspetto.top += velX;
            }
        }
        else {
            // movimento se la direzione è verso il basso
            aspetto.bottom += velX;
            aspetto.top += velX;
            // controllo che non sia arrivato al bordo
            float bordoYb = winY * 94 / 100;
            if (aspetto.bottom > bordoYb) {
                aspetto.bottom -= velX;
                aspetto.top -= velX;
            }
        }
        return 1;
    }

} arka1;

// definisce la struttura della palla
class Palla {
public:
    float diam=5;
    ULONGLONG tempo = { 0};
    D2D1_ELLIPSE p = { 0,0,0,0 };
    D2D1_POINT_2F c = { 0,0 };
    int velX = 0; // da settare a 5
    int velY=0;

    int muovi() {
        ULONGLONG time;
        time = GetTickCount64();

        if (tempo == 0)
        {
            tempo = time;
        }
        else {
            // calcola il delta tempo
            FLOAT delta = float ((time - tempo)/10);
            //  muove la pallina in X
            c.x += delta * velX;
            float bordoXd = winX-diam;
            float bordoXs = winX/40 + diam;
            if (c.x > bordoXd) {
                c.x = 2 * bordoXd - c.x;
                velX = -velX;
            }
            if (c.x < bordoXs) {
                // controllo che arka sia in posizione
                if ((c.y >= arka1.aspetto.top) && (c.y <= arka1.aspetto.bottom)) {
                    c.x = 2 * bordoXs - c.x;
                    velX = -velX;
                }
                else {
                    // arka in posizione sbagliata
                    arka1.score -= 100;
                    return -1;
                }
            }

            // muove la pallina in Y
            c.y += delta * velY;
            float bordoYb = winY * 94 / 100 - diam;
            float bordoYa = winY / 25+diam;
            if (c.y > bordoYb) {
                c.y = 2 * bordoYb - c.y;
                velY = -velY;
            }
            if (c.y < bordoYa) {
                c.y = 2 * bordoYa - c.y;
                velY = -velY;
            }
            tempo = time;
        }
        return 0;
    }
} palla1;

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
    HANDLE  m_hTimer=nullptr; // handle del timer


    D2D1_RECT_F highBorder={0,0,0,0}, lowBorder = { 0,0,0,0 }, userBar = { 0,0,0,0 };

    void    CalculateLayout();
    HRESULT CreateGraphicsResources();
    void    DiscardGraphicsResources();
    void    OnPaint();
    void    Resize(); 
    BOOL    InitializeTimer();

public:

    MainWindow() : pFactory(NULL), pWriteFactory(NULL), pRenderTarget(NULL), pBrush(NULL), pTextF(NULL)
    {
    }

    void    WaitTimer();

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
        arka1.aspetto = D2D1::RectF(winX/100, winY * 9/20, winX/40, winY*11/20);
        palla1.c.x = winX / 40 + palla1.diam;
        palla1.c.y = winY * 10 / 20;
        palla1.p = D2D1::Ellipse(palla1.c, palla1.diam, palla1.diam);
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
        // nuove la palla
        if (palla1.velX == 0) {
            lungh = swprintf_s(sc_testo, 100, L"PREMI Q PER INIZIARE");
            pRenderTarget->DrawTextW(sc_testo, lungh, pTextF, D2D1::RectF(0, 0, winX, 100), pBrush);

        }
        else {
            if (palla1.muovi() >= 0) {
                palla1.p = D2D1::Ellipse(palla1.c, palla1.diam, palla1.diam);

                // imposta lo sfondo nero
                pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black));
                // disegna la barra superiore
                pRenderTarget->FillRectangle(highBorder, pBrush);
                // disegna la barra inferiore
                pRenderTarget->FillRectangle(lowBorder, pBrush);
                // disegna la palla
                pRenderTarget->FillEllipse(palla1.p, pBrush);
                // disegna arka
                pRenderTarget->FillRectangle(arka1.aspetto, pBrush);
                // scrive il testo sulla barra superiore
                lungh = swprintf_s(sc_testo, 100, L"Cord X %0.f - Cord Y  %0.f - Score %d", winX, winY, arka1.score);
                pRenderTarget->DrawTextW(sc_testo, lungh, pTextF, D2D1::RectF(0, 0, winX, 100), pBrush);
            }
            else {
                lungh = swprintf_s(sc_testo, 100, L"Cord X %0.f - Cord Y  %0.f - Score %d  GAME OVER", winX, winY, arka1.score);
                pRenderTarget->DrawTextW(sc_testo, lungh, pTextF, D2D1::RectF(0, 0, winX, 100), pBrush);
                palla1.velX = 0;
                palla1.velY = 0;
            }
        }

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
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }
        win.WaitTimer();
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
        if (!InitializeTimer())
        {
            return -1;
        }
        return 0;

    case WM_DESTROY:
        DiscardGraphicsResources();
        CloseHandle(m_hTimer);
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
        OnPaint();
        return 0;

    case WM_CHAR:
        if (wParam == 'w')
            arka1.muovi(1);
        if (wParam == 's')
            arka1.muovi(-1);
        if (wParam == 'q') {
            palla1.velX = 5;
            palla1.velY = 5;

        }

        return 0;

    case WM_SIZE:
        Resize();
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}

BOOL MainWindow::InitializeTimer()
{
    m_hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
    if (m_hTimer == NULL)
    {
        return FALSE;
    }

    LARGE_INTEGER li = { 0 };

    if (!SetWaitableTimer(m_hTimer, &li, (1000/80), NULL, NULL, FALSE))
    {
        CloseHandle(m_hTimer);
        m_hTimer = NULL;
        return FALSE;
    }

    return TRUE;
}

void MainWindow::WaitTimer()
{
    // Wait until the timer expires or any message is posted.
    if (MsgWaitForMultipleObjects(1, &m_hTimer, FALSE, INFINITE, QS_ALLINPUT)
        == WAIT_OBJECT_0)
    {
        InvalidateRect(m_hwnd, NULL, FALSE);
    }
}
