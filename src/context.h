#include <windows.h>
#include <windowsx.h>
#include <string>
#include <iostream>
#include <vector>
#include "panes.h"

using namespace std;

BOOL CALLBACK GetWindowListProc(HWND hWnd, LPARAM lParam);
void UpdateContextMenu(HMENU &hPopupMenu, vector<PANE> current_panes, POINT pos, HWND hWnd);
HWND GetContextWindow(int index);
void GetWindowList();

#ifndef APP_H
#define APP_H
struct APP{
	HWND hWnd;
	char name[255];
};
#endif
