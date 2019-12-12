#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <string>
#include <iostream>
#include <vector>
#include "panes.h"
#include "context.h"

using namespace std;

vector <APP> windows;
vector<PANE> contextPanes; //local copy for CALLBACK use
HWND hWndContextParent; //local copy for CALLBACK use

BOOL CALLBACK GetWindowListProc(HWND hWnd, LPARAM lParam){
	APP newWindow;
	newWindow.hWnd = hWnd;
	char name[255];
	GetWindowTextA(hWnd, newWindow.name, 100);
	if(hWnd == hWndContextParent){
		return true;
	}
	if(!IsWindowVisible(hWnd)){
		return true;
	}
	if(newWindow.name == NULL){
		return true;
	}
	for(int i = 0; i < contextPanes.size(); i++){
		if(contextPanes[i].hWnd == hWnd){
		 	return true;
		}
	}
	windows.insert(windows.begin(), newWindow);
	return true;
}
void GetWindowList(){
	windows.clear();
	EnumWindows(GetWindowListProc, 0);
}
HWND GetContextWindow(int index){
	if(index >= 0 && index <= windows.size()){
		return windows[index].hWnd;	
	}
	return 0;
}
void UpdateContextMenu(HMENU &hPopupMenu, vector<PANE> current_panes, POINT pos, HWND hWnd){
	hPopupMenu = CreatePopupMenu();
	MENUINFO mi;
	memset(&mi, 0, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.fMask = MIM_STYLE;
	mi.dwStyle = MNS_NOTIFYBYPOS;
	SetMenuInfo(hPopupMenu, &mi);
	contextPanes = current_panes;
	hWndContextParent = hWnd;
	GetWindowList();

	vector<APP>::iterator window;
	for(window = windows.begin(); window != windows.end();){
		string w = window->name;
		bool remove = false;
		if(w.compare("Program Manager") == 0){
			remove = true;
		}
		if(w.compare("") == 0){
			remove = true;
		}
		if(w.compare("Start") == 0){
			remove = true;
		}
		if(remove){
			window = windows.erase(window);
		}else{
			window++;
		}
	}
	for(int i = 0; i < windows.size(); i++){
		string w = windows[i].name;
		w = "Attach: " + w;
		wstring w2(w.length(), L' ');
		copy(w.begin(), w.end(), w2.begin());
		InsertMenu(hPopupMenu, i, MF_BYPOSITION | MF_STRING, 0, w2.c_str());
	}
	InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 0, L"Split Pane (Vertical)");
	InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 0, L"Split Pane (Horizontal)");
	InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 0, L"Delete Pane");
	InsertMenu(hPopupMenu, 0, MF_BYPOSITION | MF_STRING, 0, L"Dettach Window");
	TrackPopupMenu(hPopupMenu, TPM_RIGHTBUTTON, pos.x, pos.y, 0, hWnd, NULL);
	cout << "Log: Context Menu triggered" << endl;
	cout << "Log: Menu pos is x:" << pos.x << ", ";
	cout << "y:" << pos.y << endl;
}
