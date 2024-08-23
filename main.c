/* Compile:
gcc main.c resources.coff -o Ternaria.exe -l gdi32 -l msimg32
*/

#include "imports.h"
#include "input.c"

//Flag para indicar se a thread está rodando
int Mode = 0;
int ThreadRunning = 0;
HANDLE Thread;

//Função da thread principal
DWORD WINAPI MainThread(LPVOID lpParam)
{
        
    mapax = 0;
    mapay = 800;
    srand(time(NULL));
    //Se o jogo for reiniciado depois do game over, esta flag é falsa e o mapa não é gerado
    HWND hwnd = *((HWND *)lpParam);//Recupera-se o Handle para a janela do parâmetro da thread

    DArrayCreate(&Map, 200);
    if(Mode == 0)
    {
        Mode = 1;
        ThreadRunning = 0;
    	ReadArchive(&Map);
        HDC hdc = GetDC(hwnd);
        RenderBkgd(hdc);
        RenderMap(&Map, hdc);
        RenderLogo(hdc, 0);
        RenderMenu(Mode);
        ReleaseDC(hwnd, hdc);
        return 0;
    }
    if(Mode == 1)
    {
        CreateArchive();
    	ReadArchive(&Map);
    }else
	if(Mode == 2)
	{
        ReadArchive(&Map);
	}else
	if(Mode == 3)
	{
    	ReadSenai(&Map); 
	}
    int gameover = 0, count = 0;
    character player;

    zombie zombie;
    SpawnPlayer(&player, &zombie);
    zombie.baseLife = 10;
    SpawnZombie(&zombie, &player);
    
    /*
    Normalmente, se usa um sleep para esperar uma determinada quantidade de tempo. As funções de sleep são
    imprecisas, pois liberam a thread para o sistema operacional, que pode te devolver depois do prazo.
    Os timers do Windows travam a thread, mas não a liberam, tornando-os muito precisos e eficientes.
    */

    HANDLE Timer = CreateWaitableTimer(NULL, 0, NULL);
    LARGE_INTEGER DueTime;
    DueTime.QuadPart = -333333;
    SetWaitableTimer(Timer, &DueTime, 33, NULL, NULL, 0);//Timer com intervalo de 33ms (30 fps)
    
    HDC hdc = GetDC(hwnd);//Handle com as informações necessárias para se desenhar na tela
    RECT R;
    GetClientRect(hwnd, &R);
    RenderBkgd(hdc);
    RenderMap(&Map, hdc);
    RenderLife(hdc, player.life);
    RenderInv(hdc, &player);
    ReleaseDC(hwnd, hdc);

    RECT hud = {10, 10, 317, 82};
    int renderHud = 0;
    while(player.life > 0)
    {
        HDC hdc = GetDC(hwnd);
        RECT R;
        GetClientRect(hwnd, &R);
        CenterPlayer(&player, &zombie);
        /*
        Para se desenhar na tela, se cria um handle HDC compatível com a tela na memória (TempDC).
        Selecionamos um bitmap neste HDC para definir o tamanho e as cores.
        Desenhamos tudo neste DC da memória primeiro, depois fazemos uma única transferência para a tela
        usando o BitBlt
        */
        HDC TempDC = CreateCompatibleDC(hdc);
        HBITMAP Bitmap = CreateCompatibleBitmap(hdc, (R.right-R.left), (R.bottom - R.top));
        SelectObject(TempDC, Bitmap);
        BitBlt(TempDC, 0, 0, R.right-R.left, R.bottom-R.top, hdc, 0, 0, SRCCOPY);

        //Isto redesenha o hud caso o jogador ou o zumbi passem na frente dele
        if(Collision(&player.hitbox, &hud) || Collision(&zombie.hitbox, &hud))
        {
            renderHud = 1;
        }else
        {
            renderHud = 0;
        }

        EraseRect(TempDC, &player.hitbox);
        EraseRect(TempDC, &zombie.hitbox);
        int antigox = mapax;
        int antigoy = mapay;
        Input(TempDC, &player, &zombie, &Map);

        MoveZombie(&player, &zombie);
        
        DefineMap(&Map);
        
        zombie.hitbox.left += antigox - mapax;
        zombie.hitbox.right += antigox - mapax;
        zombie.hitbox.top -= antigoy - mapay;
        zombie.hitbox.bottom -= antigoy - mapay;

        RenderBkgd(TempDC);
        RenderMap(&Map, TempDC);
        RenderInv(TempDC, &player);
        RenderLife(TempDC, player.life);

        if(zombie.life > 0)
        {
            RenderZombie(&zombie, TempDC);
        }
        
        RenderPlayer(&player, TempDC);
        RenderTool(&player, TempDC);

        count += 1;
        if(count == 300)
        {
            Regeneration(&player);
            RenderLife(TempDC, player.life);
            count = 0;
        }

        if(count % 30 == 0)
        {
            DArrayDestroy(&Map);
            DArrayCreate(&Map, 200);
            ReadArchive(&Map);
        }

        if(player.hitbox.top > 1000)
        {
            player.life = 0;
        }
        if(zombie.hitbox.top > 1000)
        {
            if(!zombie.respawn)
            {
                DamageZombie(&zombie, 200);
            }
        }
        printf("%d, %d\n", mapax, mapay);

        BitBlt(hdc, 0, 0, R.right-R.left, R.bottom-R.top, TempDC, 0, 0, SRCCOPY);
        DeleteDC(TempDC);
        DeleteObject(Bitmap);
        ReleaseDC(hwnd, hdc);

        WaitForSingleObject(Timer, INFINITE);//Aqui se espera até o timer terminar
    }

    hdc = GetDC(hwnd);
    RenderLogo(hdc, 1);
    RenderMenu(Mode);
    ReleaseDC(hwnd, hdc);
    DArrayDestroy(&Map);
    ThreadRunning = 0;
}

//Função para o tratamanto de mensagens
LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
    switch(Msg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT PS;
            HDC hdc = BeginPaint(hwnd, &PS);
            FillRect(hdc, &PS.rcPaint, CreateSolidBrush(RGB(255,255,255)));
            EndPaint(hwnd, &PS);
        }
        break;
        case WM_CLOSE:
            DArrayDestroy(&Map);
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        case WM_KEYDOWN:
        {
			if(ThreadRunning == 0)
			{
				if(GetAsyncKeyState(VK_DOWN) & 0x8000)
				{
					++Mode;
                    if(Mode > 3){Mode = 1;}
                    RenderMenu(Mode);
				}else
				if(GetAsyncKeyState(VK_UP) & 0x8000)
				{
					--Mode;
                    if(Mode < 1){Mode = 3;}
                    RenderMenu(Mode);
				}else
                if(GetAsyncKeyState(VK_RETURN) & 0x8000 && Mode != 0)
                {
                    Thread = CreateThread(NULL, 0, MainThread, &hwnd, 0, NULL);
                    ThreadRunning = 1;
                }
			}
        }
        default:
            return DefWindowProc(hwnd, Msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    const wchar_t WClassName[] = L"Ternaria";
    MSG Msg;
    WNDCLASS Window = {};
    Window.lpfnWndProc = WndProc;
    Window.hInstance = hInstance;
    GInstance = hInstance;
    Window.lpszClassName = WClassName;
    Window.hIcon = (HICON)LoadImage(GetModuleHandle(NULL), L"TernariaIcon", IMAGE_ICON, 0, 0, 0);
    Window.hCursor = (HCURSOR)LoadImage(GetModuleHandle(NULL), L"Hand", IMAGE_ICON, 0, 0, 0);

    if(!RegisterClass(&Window))
    {
        MessageBox(NULL, L"Window Class Registration Failed", L"Error", MB_ICONERROR | MB_OK);
        return 0;
    }

    HWND hwnd = CreateWindowEx
    (
        0,
        WClassName,
        L"Ternaria",
        WS_OVERLAPPEDWINDOW,
        0, 0, 1920, 1080,
        NULL, NULL, hInstance, NULL
    );

    if(hwnd == NULL)
    {
        MessageBox(NULL, L"Window Creation Failed", L"Error", MB_ICONERROR | MB_OK);
        return 0;
    }
    Ghwnd = hwnd;
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    Thread = CreateThread(NULL, 0, MainThread, &hwnd, 0, NULL);
    FreeConsole();

    //Loop para receber as mensagens da WINAPI
    while(GetMessage(&Msg, NULL, 0, 0))
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return 0;
}