
#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <string>
#include <wingdi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <future>
#include "panes.h"
#include "context.h"

using namespace std;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
HWND hWndMain;
HINSTANCE hInstanceMain;
HMENU hPopupMenu;

string WindowName(HWND hWnd);
void GetApplicationRect();
void Redraw();
int contextMenuSelectedPane = 0;

int lastActivePane = -1;
BOOL CALLBACK GetLastActivePaneProc(HWND hWnd, LPARAM lParam);
void GetLastActivePane();

vector<PANE> panes;
vector<HWND> devWindows;
RECT client_rect;
RECT window_rect;
POINT old_mouse_point;
POINT mouse_point;

MOUSEHOOKSTRUCT mouseHook;
bool mouseDrag = false;
bool mouseDown = false;
KBDLLHOOKSTRUCT prevKeyCode;
KBDLLHOOKSTRUCT nextKeyCode;
WPARAM prevKeyDown;
WPARAM nextKeyDown;
HWND hWndAction;
bool actionTrigger = false;
bool forceTransformWindows = false;
bool MouseOnPaneEdge();
int selectedPane = -1;
int focusHoverSelectedPane = -1;
int hoverRectMargin = 10;
int resizePaneIndex = -1;
bool resizingPanes = false;
string resizeSide = "";

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam){
	bool process = true;
	mouseHook = *(MOUSEHOOKSTRUCT*)lParam;
	if(wParam == WM_LBUTTONDOWN) mouseDrag = true;
	if(wParam == WM_LBUTTONUP) mouseDrag = false;
	if(!mouseDrag) process = false; 
	if(GetForegroundWindow() == hWndMain) process = false;
	if(mouseHook.pt.x <= client_rect.left) process = false; 
	if(mouseHook.pt.y <= client_rect.top) process = false; 
	if(mouseHook.pt.x >= client_rect.right) process = false; 
	if(mouseHook.pt.y >= client_rect.bottom) process = false; 
	
	if(mouseDrag){
		HWND hWndActive = GetForegroundWindow();
		RECT rectActive;
		GetWindowRect(hWndActive, &rectActive);
		bool titleBar = true;
		if(mouseHook.pt.x < rectActive.left) titleBar = false;
		if(mouseHook.pt.x > rectActive.right) titleBar = false;
		if(mouseHook.pt.y < rectActive.top) titleBar = false;
		//TODO: FIND REAL TITLEBAR HEIGHT
		if(mouseHook.pt.y > rectActive.top + 30) titleBar = false;
		if(hWndActive != hWndMain && titleBar){
			for(int i = 0; i < panes.size(); i++){
				if(panes[i].hWnd == hWndActive){
					DettachWindow(i);
					break;
				}
			}
		}
	}
	int oldHoverSelectedPane = selectedPane;	
	if(wParam == WM_LBUTTONUP && selectedPane != -1){
		HWND hWndActive = GetForegroundWindow();
		RECT rectActive;
		GetWindowRect(hWndActive, &rectActive);
		bool titleBar = true;
		if(mouseHook.pt.x < rectActive.left) titleBar = false;
		if(mouseHook.pt.x > rectActive.right) titleBar = false;
		if(mouseHook.pt.y < rectActive.top) titleBar = false;
		//TODO: FIND REAL TITLEBAR HEIGHT
		if(mouseHook.pt.y > rectActive.top + 30) titleBar = false;
		if(titleBar){
			for(int i = 0; i < panes.size(); i++){
				if(panes[i].hWnd == hWndActive){
					panes[i].hWnd = NULL;
				}
			}
			if(hWndActive != hWndMain){
				if(GetPaneFromPoint(mouseHook.pt, client_rect) == selectedPane){
					AttachWindow(hWndActive, selectedPane);
					forceTransformWindows = true;
				}
			}
		}
	}
	if(!process && selectedPane != -1){
		//selectedPane = -1;
		//cout << "Log: Disabled hover on " << oldHoverSelectedPane << endl;
		RedrawWindow(hWndMain, 0, 0, RDW_INVALIDATE);
	}
	else if(process){
		selectedPane = GetPaneFromPoint(mouseHook.pt, client_rect);
		if(oldHoverSelectedPane != selectedPane){
			cout << "Log: Hover detected on pane " << selectedPane << endl;
			if(panes[selectedPane].hWnd != NULL){
				selectedPane = -1;
				cout << "Log: Pane is already filled, disabling hover" << endl;
			}
			else{
				Redraw();
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}
bool MouseOnPaneEdge(){
	resizePaneIndex = -1;
	resizeSide = "";	
	double threshHold = 0.01;
	double width = client_rect.right - client_rect.left;
	double height = client_rect.bottom - client_rect.top;
	POINTD mouse_point_client;
	mouse_point_client.x = (mouse_point.x - client_rect.left) / width;
	mouse_point_client.y = (mouse_point.y - client_rect.top) / height;
	for(int i = 0; i < panes.size(); i++){
		if(mouse_point_client.x >= panes[i].rect.left  - threshHold){
			if(mouse_point_client.x <= panes[i].rect.left + threshHold){
				if(panes[i].rect.left != 0){
					resizePaneIndex = i;
					resizeSide = "left";
					return true;
				}
			}
		}
		if(mouse_point_client.x >= panes[i].rect.right - threshHold){
			if(mouse_point_client.x <= panes[i].rect.right + threshHold){
				if(panes[i].rect.right != 1){
					resizePaneIndex = i;
					resizeSide = "right";
					return true;
				}
			}
		}
		if(mouse_point_client.y >= panes[i].rect.top - threshHold){
			if(mouse_point_client.y <= panes[i].rect.top + threshHold){
				if(panes[i].rect.top != 0){
					resizePaneIndex = i;
					resizeSide = "top";
					return true;
				}
			}
		}
		if(mouse_point_client.y >= panes[i].rect.bottom - threshHold){
			if(mouse_point_client.y <= panes[i].rect.bottom + threshHold){
				if(panes[i].rect.bottom != 1){
					resizePaneIndex = i;
					resizeSide = "bottom";
					return true;
				}
			}
		}
	}
	return false;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam){
	if(wParam == WM_KEYDOWN){
		prevKeyCode = nextKeyCode;
		nextKeyCode = *(KBDLLHOOKSTRUCT*)lParam;
		prevKeyDown = nextKeyDown;
		nextKeyDown = wParam  == WM_KEYDOWN;
		if(nextKeyCode.vkCode == 220 && nextKeyDown && actionTrigger){ //|
			hWndAction = GetForegroundWindow();
			SplitPane(GetPaneFromWindow(hWndAction), true);
			TransformWindows(client_rect);
			Redraw();
			cout << "Split Vertical" << endl;
		}
		if(nextKeyCode.vkCode == 189 && nextKeyDown && actionTrigger){ //-
			hWndAction = GetForegroundWindow();
			SplitPane(GetPaneFromWindow(hWndAction), false);
			TransformWindows(client_rect);
			Redraw();
			cout << "Split Horizontal" << endl;
		}
		actionTrigger = false;
		if(prevKeyCode.vkCode == 162 && prevKeyDown){ //ctrl
			if(nextKeyCode.vkCode == 65 && nextKeyDown){ //a
				actionTrigger = true;
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}


BOOL CALLBACK GetLastActivePaneProc(HWND hWnd, LPARAM lParam){
	for(int i = 0; i < panes.size(); i++){
		if(hWnd == panes[i].hWnd){
			selectedPane = i;
			return false;
		}
	}
	return true;
}
void GetLastActivePane(){
	EnumWindows(GetLastActivePaneProc, 0);
}
string WindowName(HWND hWnd){
	char buffer[255];
	GetWindowTextA(hWnd, (LPSTR)buffer, 100);
	return buffer;
}
void GetApplicationRect(){
	GetWindowRect(hWndMain, &window_rect);
	POINT p;
	p.x = 0;
	p.y = 0;
	ClientToScreen(hWndMain, &p);	
	client_rect.left = p.x;
	client_rect.top = p.y;
	RECT client;
	GetClientRect(hWndMain, &client);
	client_rect.right = client_rect.left + client.right;
	client_rect.bottom = client_rect.top + client.bottom;
	return;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR pCmdLine, int nCmdShow){

	const wchar_t CLASS_NAME[] = L"WindowPane";

	WNDCLASS wc = {};

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	HWND hWnd = CreateWindowEx(0, CLASS_NAME, L"WindowPane", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	if (hWnd == NULL) {return 0;}
	hWndMain = hWnd;
	hInstanceMain = hInstance;
	SetWindowLong(hWndMain, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hWnd, 0, 220, LWA_ALPHA);
	
	GetApplicationRect();
	InitPanes();
	CalculatePaneSizeRatios();
	ShowWindow(hWnd, nCmdShow);
	TransformWindows(client_rect);
	MSG msg = {};
	//HHOOK kHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
	//cout << "Log: Keyboard Hook Handle: " << kHook << endl;
	//HHOOK mHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0);
	//cout << "Log: Mouse Hook Handle: " << mHook << endl;
	while (GetMessage(&msg, NULL, 0, 0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	//UnhookWindowsHookEx(kHook);
	//UnhookWindowsHookEx(mHook);
	return 0;

}

void Redraw(){
	RedrawWindow(hWndMain, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
}
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam){
	if(forceTransformWindows){
		TransformWindows(client_rect);
		forceTransformWindows = false;
	}
	switch (uMsg){
		case WM_KEYDOWN:{
				if(lParam >> 30 == 0){ // not held down
					if(wParam == 37 || wParam == 38 || wParam == 39 || wParam == 40){
						cout << "Log: Left Key Pressed" << endl;	
						int newSelectedPane = GetAdjacentPane(selectedPane, wParam, window_rect);
						if(newSelectedPane != -1){
							selectedPane = newSelectedPane;
						}
						cout << "Log: Selected Pane is " << selectedPane << endl;
						Redraw();
					}
					if(wParam == 13){
						ActivatePaneWindow(selectedPane, window_rect, client_rect);
						Redraw();
					}
					if(wParam == 220){
						SplitPane(selectedPane, true);
						TransformWindows(client_rect);
						Redraw();
					}
					if(wParam == 189){
						SplitPane(selectedPane, false);
						TransformWindows(client_rect);
						Redraw();
					}
					if(wParam == 68){
						DeletePane(selectedPane);
						selectedPane = 0;
						TransformWindows(client_rect);
						Redraw();
					}
				}
			break;
		}
		case WM_LBUTTONDOWN:{
			cout << "Log: Mouse left Button Down detected" << endl;
			GetCursorPos(&mouse_point);
			mouseDown = true;
			int paneIndex = GetPaneFromPoint(mouse_point, client_rect);
			resizingPanes = false;
			if(!MouseOnPaneEdge()){
				selectedPane = paneIndex;
				mouseDown = false;
				ActivatePaneWindow(paneIndex, window_rect, client_rect);
			}else{
				resizingPanes = true;	
			}
			Redraw();
			break;
		}
		case WM_LBUTTONUP:{
			mouseDown = false;
			resizingPanes = false;
			TransformWindows(client_rect);
		}
		case WM_MOUSEMOVE:{
			old_mouse_point = mouse_point;	
			GetCursorPos(&mouse_point);
			if(!mouseDown || !resizingPanes){
				if(MouseOnPaneEdge()){
					if(resizeSide == "left" || resizeSide == "right"){
						cout << "Log: Hover over pane edge detected" << endl;
						SetCursor(LoadCursor(NULL, IDC_SIZEWE));
					}
					if(resizeSide == "top" || resizeSide == "bottom"){
						cout << "Log: Hover over pane edge detected" << endl;
						SetCursor(LoadCursor(NULL, IDC_SIZENS));
					}
				}
				else{
					SetCursor(LoadCursor(NULL, IDC_ARROW));
				}
			}
			if(mouseDown && resizingPanes){
				double width = (double)client_rect.right - (double)client_rect.left;
				double height = (double)client_rect.bottom - (double)client_rect.top;
				RECTD change;
				change.left = 0.0;
				change.right = 0.0;
				change.top = 0.0;
				change.bottom = 0.0;
				POINTD mouse_point_client;
				mouse_point_client.x = (double)mouse_point.x - (double)client_rect.left;
				mouse_point_client.x /= width;
				mouse_point_client.y = (double)mouse_point.y - (double)client_rect.top;
				mouse_point_client.y /= height;
				if(resizeSide == "left"){
					change.left = mouse_point_client.x;
				}
				if(resizeSide == "right"){
					change.right = mouse_point_client.x;
				}
				if(resizeSide == "top"){
					change.top = mouse_point_client.y; 
				}
				if(resizeSide == "bottom"){
					change.bottom = mouse_point_client.y; 
				}
				ResizePane(resizePaneIndex, change);
				Redraw();
			}
			break;
		}
		case WM_DESTROY:{
			PostQuitMessage(0);
			return 0;
		}
		case WM_MENUCOMMAND:{
			cout << "Context menu option " << wParam << " selected" << endl;
			if(wParam == 0){
				DettachWindow(contextMenuSelectedPane);
			}
			else if(wParam == 1){
				DeletePane(contextMenuSelectedPane);
				Redraw();
			}
			else if(wParam == 2){ 
				SplitPane(contextMenuSelectedPane, false);
				TransformWindows(client_rect);
				Redraw();
			}
			else if(wParam == 3){
				SplitPane(contextMenuSelectedPane, true);
				TransformWindows(client_rect);
				Redraw();
			}
			else{
				AttachWindow(GetContextWindow(wParam - 4), contextMenuSelectedPane);
				TransformWindows(client_rect);
				BringPanesToFront(hWndMain);
			}
			break;
		}
		case WM_PAINT:{
			InvalidateRect(hWnd, NULL, true);
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			RECT r;
			HBRUSH hFrameBrush = CreateSolidBrush(RGB(0,0,0));
			FillRect(hdc, &ps.rcPaint, CreateSolidBrush(RGB(52, 73, 80)));
			double width = client_rect.right - client_rect.left;
			double height = client_rect.bottom - client_rect.top;
			if(selectedPane != -1){
				RECT frameRect;
				SetRect(&frameRect,
				panes[selectedPane].rect.left * width + hoverRectMargin,
				panes[selectedPane].rect.top * height + hoverRectMargin, 
				panes[selectedPane].rect.right * width - hoverRectMargin,
				panes[selectedPane].rect.bottom * height - hoverRectMargin);
				FrameRect(hdc, &frameRect, CreateSolidBrush(RGB(41, 128, 185)));
			}
			for(int i = 0; i < panes.size(); i++){
				RECT frameRect;
				SetRect(&frameRect,
				panes[i].rect.left * width,
				panes[i].rect.top * height,
				panes[i].rect.right * width,
				panes[i].rect.bottom * height);
				FrameRect(hdc, &frameRect, hFrameBrush);
			}
			EndPaint(hWnd, &ps);
			break;	
		}
		case WM_ACTIVATEAPP:{
			if(wParam){
				GetLastActivePane();
				BringPanesToFront(hWndMain);
				cout << "Log: Last Active pane " << selectedPane << endl;
			}
			TransformWindows(client_rect);
			RedrawWindow(hWndMain, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		}
		case WM_SIZE:{
			GetApplicationRect();
			TransformWindows(client_rect);
			Redraw();
			break;
		}
		case WM_MOVE:{
			long x = (long)LOWORD(lParam);
			long y = (long)HIWORD(lParam);
			GetApplicationRect();
			TransformWindows(client_rect);
			break;
		}
		case WM_CONTEXTMENU:{
			GetCursorPos(&mouse_point);
			if(GET_X_LPARAM(lParam) == -1){
				 //Probably used F10
				 contextMenuSelectedPane = selectedPane;
			}
			else{
				contextMenuSelectedPane = GetPaneFromPoint(mouse_point, client_rect);
			}
			cout << "Log: Context menu selection is " << contextMenuSelectedPane << endl;
			UpdateContextMenu(hPopupMenu, panes, mouse_point, hWndMain);
			break;
		}
		GetApplicationRect();
		return 0;
	}
	
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
