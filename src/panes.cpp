#include <windows.h>
#include <windowsx.h>
#include <vector>
#include <iostream>
#include "panes.h"
#include <algorithm>
#include <math.h>

using namespace std;

extern vector<PANE> panes;

void InitPanes(){
	PANE newPane;
	newPane.hWnd = NULL;
	newPane.rect.left = 0;
	newPane.rect.right = 0.5;
	newPane.rect.bottom = 1;
	newPane.rect.top = 0;
	panes.insert(panes.end(), newPane);

	newPane.rect.left = 0.5;
	newPane.rect.right = 1;
	newPane.rect.bottom = 1;
	newPane.rect.top = 0;
	newPane.hWnd = NULL;
	panes.insert(panes.end(), newPane);
}
void AttachWindow(HWND hWnd, int paneIndex){
	if (paneIndex < 0 || paneIndex > panes.size()){
		cout << "Log: Attempted to attach " << hWnd << " no pane selected" << endl;
		return;
	}
	if(panes[paneIndex].hWnd != NULL){
		cout << "Log: Attach failed, " << hWnd << " already attached" << endl;
		return;
	}
	panes[paneIndex].hWnd = hWnd;
	cout << "Log: Attached " << hWnd << " to pane " << paneIndex << endl;
}
void DettachWindow(int paneIndex){
	if (paneIndex < 0 || paneIndex > panes.size()){
		cout << "Log: Failed to deattach window from pane " << paneIndex<< endl;
		return;
	}
	if(panes[paneIndex].hWnd == NULL){
		cout << "Log: Deattach failed, no window attached." << endl;
		return;
	}
	HWND window = panes[paneIndex].hWnd;
	panes[paneIndex].hWnd = NULL;
	RECT windowRect;
	GetWindowRect(window, &windowRect);
	//'Pop-out' window
	SetWindowPos(window, 
	HWND_TOP,
	windowRect.left, 
	windowRect.top, 
	windowRect.right - windowRect.left + 50, 
	windowRect.bottom - windowRect.top + 50, 0);
	SetForegroundWindow(window);
	cout << "Log: Dettached window from pane " << paneIndex << endl;
}

void PanesToString(){
	for(int i = 0; i < panes.size(); i++){
	 	cout << panes[i].hWnd << " | ";
	}
}

void TransformWindows(RECT client_rect){
	long width = client_rect.right - client_rect.left;
	long height = client_rect.bottom - client_rect.top;
	for(int i = 0; i < panes.size(); i++){
		if(panes[i].hWnd == NULL){
			continue;
		}
		if(!SetWindowPos(panes[i].hWnd, HWND_TOP,
		panes[i].rect.left * width + client_rect.left,
		panes[i].rect.top * height + client_rect.top,
		panes[i].rect.right * width - panes[i].rect.left * width,
		panes[i].rect.bottom * height - panes[i].rect.top * height,
		SWP_SHOWWINDOW | SWP_NOACTIVATE)){
			cout << "Log: Error changing position of " << panes[i].hWnd;
			cout << " assuming exited and freeing pane" << endl;
			panes[i].hWnd = NULL;
		}
		ShowWindow(panes[i].hWnd, SW_RESTORE);
	}
}
void BringPanesToFront(HWND hWndTopWindow){
	for(int i = 0; i < panes.size();i++){
		if(panes[i].hWnd != NULL){
			SetWindowPos(panes[i].hWnd, hWndTopWindow, 0, 0, 0, 0, 
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
}
int GetPaneFromPoint(POINT point, RECT client_rect){
	long width = client_rect.right - client_rect.left;
	long height = client_rect.bottom - client_rect.top;
	for(int i = 0; i < panes.size(); i++){
		if(point.x < panes[i].rect.left * width + client_rect.left) continue;
		if(point.x > panes[i].rect.right * width + client_rect.left) continue;
		if(point.y < panes[i].rect.top * height + client_rect.top) continue;
		if(point.y > panes[i].rect.bottom * height + client_rect.top) continue;
		return i;
	}
	return -1;
}
void SplitPane(int paneIndex, bool Vertical){
	if(paneIndex < 0 || paneIndex > panes.size()){
		cout << "Pane index " << paneIndex << " not valid" << endl;
		return;
	}
	PANE newPane;
	newPane.hWnd = NULL;
	if(Vertical){
		newPane.rect.left = panes[paneIndex].rect.left;
		newPane.rect.left += (panes[paneIndex].rect.right - panes[paneIndex].rect.left) / 2;
		newPane.rect.right = panes[paneIndex].rect.right;
		newPane.rect.top = panes[paneIndex].rect.top;
		newPane.rect.bottom = panes[paneIndex].rect.bottom;
		panes[paneIndex].rect.right = newPane.rect.left;
	}
	else{
		newPane.rect.top = panes[paneIndex].rect.top;
		newPane.rect.top += (panes[paneIndex].rect.bottom - panes[paneIndex].rect.top) / 2;
		newPane.rect.bottom = panes[paneIndex].rect.bottom;
		newPane.rect.left = panes[paneIndex].rect.left;
		newPane.rect.right = panes[paneIndex].rect.right;
		panes[paneIndex].rect.bottom = newPane.rect.top;
	}
	panes.insert(panes.end(), newPane);
}

void DeletePane(int paneIndex){
	if(paneIndex >= 0 && paneIndex < panes.size()){
		cout << "Log: Deleting pane " << paneIndex << "..." << endl;
	}
	else{
		cout << "Log: Cannot delete pane " << paneIndex << endl;
		return;
	}
	//Merge Left
	vector <int> mergeLeftIndicies;
	vector <int> mergeRightIndicies;
	vector <int> mergeTopIndicies;
	vector <int> mergeBottomIndicies;
	double mergeTopWidth = 0;
	double mergeBottomWidth = 0;
	double mergeLeftHeight = 0;
	double mergeRightHeight = 0;
	for(int i = 0; i < panes.size(); i++){
		if(i != paneIndex){
			//Merge Left
			if(panes[i].rect.left == panes[paneIndex].rect.right){
				if(panes[i].rect.top >= panes[paneIndex].rect.top){
					if(panes[i].rect.bottom <= panes[paneIndex].rect.bottom){
						mergeLeftIndicies.insert(mergeLeftIndicies.begin(), i);	
						mergeLeftHeight += panes[i].rect.bottom - panes[i].rect.top;
					}
				}
			}
			//Merge Right
			if(panes[i].rect.right == panes[paneIndex].rect.left){
				if(panes[i].rect.top >= panes[paneIndex].rect.top){
					if(panes[i].rect.bottom <= panes[paneIndex].rect.bottom){
						mergeRightIndicies.insert(mergeRightIndicies.begin(), i);	
						mergeRightHeight += panes[i].rect.bottom - panes[i].rect.top;
					}
				}
			}
			//Merge Top
			if(panes[i].rect.top == panes[paneIndex].rect.bottom){
				if(panes[i].rect.left >= panes[paneIndex].rect.left){
					if(panes[i].rect.right <= panes[paneIndex].rect.right){
						mergeTopIndicies.insert(mergeTopIndicies.begin(), i);
						mergeTopWidth += panes[i].rect.right - panes[i].rect.left;	
					}
				}
			}
			//Merge Bottom
			if(panes[i].rect.bottom == panes[paneIndex].rect.top){
				if(panes[i].rect.left >= panes[paneIndex].rect.left){
					if(panes[i].rect.right <= panes[paneIndex].rect.right){
						mergeBottomIndicies.insert(mergeBottomIndicies.begin(), i);
						mergeBottomWidth += panes[i].rect.right - panes[i].rect.left;	
					}
				}
			}
		}
	}
	if(mergeLeftHeight == panes[paneIndex].rect.bottom - panes[paneIndex].rect.top){
		for(int i = 0; i < mergeLeftIndicies.size(); i++){
			panes[mergeLeftIndicies[i]].rect.left = panes[paneIndex].rect.left;	
		}
		panes.erase(panes.begin() + paneIndex);
	}
	else if(mergeRightHeight == panes[paneIndex].rect.bottom - panes[paneIndex].rect.top){
		for(int i = 0; i < mergeRightIndicies.size(); i++){
			panes[mergeRightIndicies[i]].rect.right = panes[paneIndex].rect.right;	
		}
		panes.erase(panes.begin() + paneIndex);
	}
	else if(mergeTopWidth == panes[paneIndex].rect.right - panes[paneIndex].rect.left){
		for(int i = 0; i < mergeTopIndicies.size(); i++){
			panes[mergeTopIndicies[i]].rect.top = panes[paneIndex].rect.top;	
		}
		panes.erase(panes.begin() + paneIndex);
	}
	else if(mergeBottomWidth == panes[paneIndex].rect.right - panes[paneIndex].rect.left){
		for(int i = 0; i < mergeBottomIndicies.size(); i++){
			panes[mergeBottomIndicies[i]].rect.bottom = panes[paneIndex].rect.bottom;
		}
		panes.erase(panes.begin() + paneIndex);
	}
	return;
}
bool WindowMoved(int paneIndex){
	if(panes[paneIndex].hWnd == NULL){
		return 0;
	}
	RECT newRect;
	GetWindowRect(panes[paneIndex].hWnd, &newRect);
	int differenceCount = 0;
	if(newRect.left != panes[paneIndex].rect.left){
		differenceCount++;
	}
	if(newRect.right != panes[paneIndex].rect.right){
		differenceCount++;
	}
	if(newRect.top != panes[paneIndex].rect.top){
		differenceCount++;
	}
	if(newRect.bottom != panes[paneIndex].rect.bottom){
		differenceCount++;
	}
	return (differenceCount == 4);
}
bool WindowResized(int paneIndex){
	if(panes[paneIndex].hWnd == NULL){
		return 0;
	}
	RECT newRect;
	GetWindowRect(panes[paneIndex].hWnd, &newRect);
	int differenceCount = 0;
	if(newRect.left != panes[paneIndex].rect.left){
		differenceCount++;
	}
	if(newRect.right != panes[paneIndex].rect.right){
		differenceCount++;
	}
	if(newRect.top != panes[paneIndex].rect.top){
		differenceCount++;
	}
	if(newRect.bottom != panes[paneIndex].rect.bottom){
		differenceCount++;
	}
	return (differenceCount > 0 && differenceCount < 4);
}

void ResizePane(int paneIndex, RECTD change){
	double threshHold = 0.01;
	if(paneIndex < 0 || paneIndex >= panes.size()){
		return;
	}
	if(change.left + threshHold > panes[paneIndex].rect.right && change.left != 0){
		return;
	}
	if(change.right - threshHold < panes[paneIndex].rect.left && change.right != 0){
		return;
	}
	if(change.top + threshHold > panes[paneIndex].rect.bottom && change.top != 0){
		return;
	}
	if(change.bottom - threshHold < panes[paneIndex].rect.top && change.bottom != 0){
		return;
	}
	for(int i = 0; i < panes.size(); i++){
		if(i == paneIndex){
			continue;
		}
		if(panes[i].rect.left == panes[paneIndex].rect.left && change.left != 0){
			if(change.left + threshHold > panes[i].rect.right){
				return;	
			}
		}
		if(panes[i].rect.right == panes[paneIndex].rect.right && change.right != 0){
			if(change.right - threshHold < panes[i].rect.left){
				return;	
			}
		}
		if(panes[i].rect.top == panes[paneIndex].rect.top && change.top != 0){
			if(change.top + threshHold > panes[i].rect.bottom){
				return;	
			}
		}
		if(panes[i].rect.bottom == panes[paneIndex].rect.bottom && change.bottom != 0){
			if(change.bottom - threshHold < panes[i].rect.top){
				return;
			}
		}
		if(panes[i].rect.right == panes[paneIndex].rect.left && change.left != 0){
			if(change.left - threshHold < panes[i].rect.left){
				return;	
			}
		}
		if(panes[i].rect.left == panes[paneIndex].rect.right && change.right != 0){
			if(change.right + threshHold > panes[i].rect.right){
				return;	
			}
		}
		if(panes[i].rect.bottom == panes[paneIndex].rect.top && change.top != 0){
			if(change.top - threshHold < panes[i].rect.top){
				return;
			}
		}
		if(panes[i].rect.top == panes[paneIndex].rect.bottom && change.bottom != 0){
			if(change.bottom + threshHold > panes[i].rect.bottom){
				return;	
			}
		}
	}
	
	for(int i = 0; i < panes.size(); i++){
		if(i == paneIndex){
			continue;
		}
		if(panes[i].rect.left == panes[paneIndex].rect.left && change.left != 0){
			panes[i].rect.left = change.left;
		}
		if(panes[i].rect.right == panes[paneIndex].rect.right && change.right != 0){
			panes[i].rect.right = change.right;
		}
		if(panes[i].rect.top == panes[paneIndex].rect.top && change.top != 0){
			panes[i].rect.top = change.top;
		}
		if(panes[i].rect.bottom == panes[paneIndex].rect.bottom && change.bottom != 0){
			panes[i].rect.bottom = change.bottom;
		}
		if(panes[i].rect.right == panes[paneIndex].rect.left && change.left != 0){
			panes[i].rect.right = change.left;
		}
		if(panes[i].rect.left == panes[paneIndex].rect.right && change.right != 0){
			panes[i].rect.left = change.right;
		}
		if(panes[i].rect.bottom == panes[paneIndex].rect.top && change.top != 0){
			panes[i].rect.bottom = change.top;
		}
		if(panes[i].rect.top == panes[paneIndex].rect.bottom && change.bottom != 0){
			panes[i].rect.top = change.bottom;
		}
	}
	if(change.left != 0){
		panes[paneIndex].rect.left = change.left;
	}
	if(change.right != 0){
		panes[paneIndex].rect.right = change.right;
	}
	if(change.top != 0){
		panes[paneIndex].rect.top = change.top;
	}
	if(change.bottom != 0){
		panes[paneIndex].rect.bottom = change.bottom;
	}
	return;
}

void CalculatePaneSizeRatios(){
	long nextLeft = panes[0].rect.right;
	long nextTop = panes[0].rect.bottom;
	long nextRight = panes[0].rect.left;
	long nextBottom = panes[0].rect.top;
	long totalWidth = panes[0].rect.right - panes[0].rect.left;
	long totalHeight = panes[0].rect.bottom - panes[0].rect.top;
	long min_left = -1;
	long max_right = -1;
	long min_top = -1;
	long max_bottom = -1;
	for(int i = 0; i < panes.size(); i++){
		if(panes[i].rect.left == nextLeft){
			totalWidth += panes[i].rect.right - panes[i].rect.left;	
			nextLeft = panes[i].rect.right;
		}
		if(panes[i].rect.right == nextRight){
			totalWidth += panes[i].rect.right - panes[i].rect.left;	
			nextRight = panes[i].rect.left;
		}
		if(panes[i].rect.top == nextTop){
			totalHeight += panes[i].rect.bottom - panes[i].rect.top;
			nextTop = panes[i].rect.bottom;
		}
		if(panes[i].rect.bottom == nextBottom){
			totalHeight += panes[i].rect.bottom - panes[i].rect.top;
			nextBottom = panes[i].rect.top;
		}
		if(min_left == -1 || panes[i].rect.left < min_left){
			min_left = panes[i].rect.left;
		}
		if(max_right == -1 || panes[i].rect.right > max_right){
			max_right = panes[i].rect.right;
		}
		if(min_top == -1 || panes[i].rect.top < min_top){
			min_top = panes[i].rect.top;
		}
		if(max_bottom == -1 || panes[i].rect.bottom > max_bottom){
			max_bottom = panes[i].rect.bottom;
		}
	}

	for(int i = 0; i < panes.size(); i++){
		panes[i].x_ratio = ((double)panes[i].rect.left - (double)min_left);
		panes[i].x_ratio /= ((double)max_right - (double)min_left);
		panes[i].width_ratio = ((double)panes[i].rect.right - (double)panes[i].rect.left);
		panes[i].width_ratio /= (double)totalWidth;
		panes[i].y_ratio = ((double)panes[i].rect.top - (double)min_top);
		panes[i].y_ratio /= ((double)max_bottom - (double)min_top);
		panes[i].height_ratio = ((double)panes[i].rect.bottom - (double)panes[i].rect.top);
		panes[i].height_ratio /= (double)totalHeight;
	}
}
void NormalResizeAllPanes(RECT client_rect){
	for(int i = 0; i < panes.size(); i++){
		panes[i].rect.left = client_rect.left;
		panes[i].rect.left += panes[i].x_ratio * (client_rect.right - client_rect.left);
		panes[i].rect.right = panes[i].rect.left;
		panes[i].rect.right += panes[i].width_ratio * (client_rect.right - client_rect.left);
		panes[i].rect.top = client_rect.top;
		panes[i].rect.top += panes[i].y_ratio * (client_rect.bottom - client_rect.top);
		panes[i].rect.bottom = panes[i].rect.top;
		panes[i].rect.bottom += panes[i].height_ratio * (client_rect.bottom - client_rect.top);
	}
}
int GetPaneFromWindow(HWND hWnd){
	for(int i = 0; i < panes.size(); i++){
		if(panes[i].hWnd == hWnd){
			return i;
		}
	}
	return -1;
}
void UpdatePanes(RECT window_rect, RECT client_rect){
	for(int i = 0; i < panes.size(); i++){
		if(WindowMoved(i)){
			cout << "Log: User repositioned " << panes[i].hWnd << endl;
			cout << "Log: Detaching " << panes[i].hWnd << " from pane " << i << endl;
			panes[i].hWnd = NULL;
		}
		if(WindowResized(i)){
			cout << "Log: User resized " << panes[i].hWnd << endl;
			cout << "Log: attempting to resize other panes" << endl; 
			//ResizePane(i, window_rect, client_rect);
			break;
		}
	}
}

int GetAdjacentPane(int paneIndex, int direction, RECT window_rect){
	//{left: 37, up: 38, right: 39, down: 40}

	long minDistance = 1;
	int destinationIndex = -1;
	for(int i = 0; i < panes.size(); i++){
		//Moving Right
		if(panes[paneIndex].rect.right == panes[i].rect.left && direction == 39){
			if(abs(panes[i].rect.top - panes[i].rect.top) < minDistance){
				destinationIndex = i;
				minDistance = abs(panes[paneIndex].rect.top - panes[i].rect.top);
			}
		}
		//Moving Left
		if(panes[paneIndex].rect.left == panes[i].rect.right && direction == 37){
			if(abs(panes[paneIndex].rect.top - panes[i].rect.top) < minDistance){
				destinationIndex = i;
				minDistance = abs(panes[paneIndex].rect.top - panes[i].rect.top);
			}
		}
		//Moving Up
		if(panes[paneIndex].rect.top == panes[i].rect.bottom && direction == 38){
			if(abs(panes[paneIndex].rect.left - panes[i].rect.left) < minDistance){
				destinationIndex = i;
				minDistance = abs(panes[paneIndex].rect.left - panes[i].rect.left);
			}
		}
		//Moving Down
		if(panes[paneIndex].rect.bottom == panes[i].rect.top && direction == 40){
			if(abs(panes[paneIndex].rect.left - panes[i].rect.left) < minDistance){
				destinationIndex = i;
				minDistance = abs(panes[paneIndex].rect.left - panes[i].rect.left);
			}
		}
	}
	cout << "Log: Adjacent Destination " << destinationIndex << endl;
	return destinationIndex;
}
void ActivatePaneWindow(int paneIndex, RECT window_rect, RECT client_rect){
	if(paneIndex >= 0 && paneIndex < panes.size()){
		if(panes[paneIndex].hWnd != NULL){
			for(int i = 0; i < panes.size(); i++){
				if(i != paneIndex){
					if(panes[i].hWnd != NULL){
						SetWindowPos(panes[i].hWnd, HWND_TOPMOST, 0, 0, 0, 0, 
						SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
						SetWindowPos(panes[i].hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, 
						SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
					}
				}
			}
			SetForegroundWindow(panes[paneIndex].hWnd);	
		}
	}
}
