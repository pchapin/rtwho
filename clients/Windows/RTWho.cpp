/****************************************************************************
FILE          : rtwho.cpp
LAST REVISION : 2007-03-25
SUBJECT       : Real-Time Who
PROGRAMMER    : (C) Copyright 2000-2007 by The VTC Computer Club

This program was originally written by Christopher Deguise while a member of VTC3.

This program will give a listing of everyone logged in to an Novell network and give their
location. Since VTC is now using a Windows network, this program needs to be updated. The Novell
specific material has been commented out for now.

LICENSE

This program is free software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANT- ABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program; if
not, write to the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA

Please send comments or bug reports to

     VTC Computer Club
     c/o Peter C. Chapin
     Vermont Technical College
     Williston, VT 05495
     PChapin@vtc.vsc.edu
****************************************************************************/

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <conio.h>
#include <iostream>
#include <vector>

using namespace std;

#include <netstream.hpp>
#include "config.h"

// Windows
#include <windows.h>
#include <commctrl.h>

// RTWho
#include "RTWho.h"
#include "RTWho.rh"

// Declaration of some Functions to Handle the TaskBar
BOOL MyTaskBarAddIcon(HWND hwnd, UINT uID, HICON hicon, LPSTR lpszTip);
BOOL MyTaskBarDeleteIcon(HWND hwnd, UINT uID);

// Declaration callback functions.
LRESULT CALLBACK window_procedure(HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK IntroBoxProc    (HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK ConfigBoxProc   (HWND, UINT, WPARAM, LPARAM);
BOOL    CALLBACK SendMsgBoxProc  (HWND, UINT, WPARAM, LPARAM);

// Used for popup menu
HINSTANCE window_instance;

// Holds the timer interval set by the user in the Config Box, Default = 10min
UINT timer_interval = 1;

// Holds the user to search for that will alarm user when they log in
string search_user("AUSER");

// Have already alerted the user that someone has logged in.
BOOL found = FALSE;

//
// get_configuration
//
// The following function locates and reads the configuration files.
//
static void get_configuration()
{
  const int PATH_BUFFER_SIZE = 256;

  TCHAR buffer[PATH_BUFFER_SIZE];
  LPSTR file_part;
  DWORD result;

  result = SearchPath(NULL, "RTWconfig.cfg", NULL, PATH_BUFFER_SIZE, buffer, &file_part);
  if (result == 0 || result >= PATH_BUFFER_SIZE) return;
  spica::read_config_files(buffer);
}

//
// configure_ListView
//
// The following function sets up the main ListView control. This is where the columns are
// configured, etc. This function is only called once.
//
static void configure_ListView(HWND list_handle)
{
  struct column_info {
    int    fmt;
    int    cx;
    LPTSTR text;
  };
  static struct column_info columns[] = {
    { LVCFMT_LEFT, 100, "UserID"       },
    { LVCFMT_LEFT, 200, "Real name"    },
    { LVCFMT_LEFT, 200, "Host"         },
    { LVCFMT_LEFT, 200, "Location"     },
    { LVCFMT_LEFT, 100, "Type"         }
  };
  static const int column_count = sizeof(columns)/sizeof(struct column_info);

  LV_COLUMN col;
  col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT;

  for (int i = 0; i < column_count; ++i) {
    col.fmt     = columns[i].fmt;
    col.cx      = columns[i].cx;
    col.pszText = columns[i].text;
    ListView_InsertColumn(list_handle, i, &col);
  }
}

//
// populate_ListView
//
// The following function loads the ListView with information from the user_info vector. It
// basically maps the user_info vector into the Windows specific data structures needed for the
// display.
//
void populate_ListView(HWND list_handle, vector<UserData> *user_info)
{
  typedef string UserData::*string_pointer;
  string_pointer field_names[] = {
    &UserData::real_name,
    &UserData::host_name,
    &UserData::location,
    &UserData::object_type
  };
  static const int field_count = sizeof(field_names)/sizeof(string_pointer);
  LV_ITEM item;

  for (vector<UserData>::size_type i = 0 ; i < user_info->size(); ++i) {
    item.mask     = LVIF_TEXT;
    item.iItem    = i;

    item.pszText  = const_cast<LPSTR>((*user_info)[i].login_name.c_str());
    item.iSubItem = 0;
    ListView_InsertItem(list_handle, &item);

    for (int j = 0; j < field_count; ++j) {
      item.pszText  = const_cast<LPSTR>( ( (*user_info)[i] .* (field_names[j]) ).c_str() );
      item.iSubItem = j + 1;
      ListView_SetItem(list_handle, &item);
    }
  }
}

//
// Main Program
//
int WINAPI WinMain(
  HINSTANCE instance, HINSTANCE, LPSTR /* command_line */, int command_show
  )
  {
    static char class_name[] = "RTWHO";
    HWND        window_handle;
    MSG         message;
    WNDCLASS    the_class;
    netstream::Init network_initializer;

    get_configuration();

    // Register this window class.
    the_class.style         = CS_HREDRAW | CS_VREDRAW;
    the_class.lpfnWndProc   = window_procedure;
    the_class.cbClsExtra    = 0;
    the_class.cbWndExtra    = 0;
    the_class.hInstance     = instance;
    the_class.hIcon         = LoadIcon(instance, MAKEINTRESOURCE(RTW_ICON));
    the_class.hCursor       = LoadCursor(0, IDC_ARROW);
    the_class.hbrBackground = static_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
    the_class.lpszMenuName  = MAKEINTRESOURCE(MAIN_MENU);
    the_class.lpszClassName = class_name;
    RegisterClass(&the_class);

    // Make a copy of this windows instance global.
    window_instance = instance;

    InitCommonControls();

    char version_buffer[128];
    sprintf(version_buffer, "%s (%s)", MBOX_CAPTION, __DATE__);

    // Get the main window going.
    window_handle = CreateWindow(
      class_name,
      version_buffer,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      CW_USEDEFAULT,
      0,
      0,
      instance,
      0
    );
    ShowWindow(window_handle, command_show);
    UpdateWindow(window_handle);

    // Set up the timer for the update function.
    if (!SetTimer(window_handle, ID_TIMER, timer_interval * 60 * 1000, NULL))
       MessageBox(window_handle,
        "Too many clocks or Timers.\nAutomatic update will not execute.", MBOX_CAPTION, MB_OK|MB_ICONEXCLAMATION);

    // Add Icon to task bar
    MyTaskBarAddIcon(window_handle, RTW_ICON, the_class.hIcon, MBOX_CAPTION);

    while (GetMessage(&message, 0, 0, 0)) {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
    return message.wParam;
  }


//
// window_procedure
//
LRESULT CALLBACK window_procedure(HWND window_handle, UINT message, WPARAM wParam, LPARAM lParam)
  {
    static int     user_selected = 0;
    static vector<UserData> *user_info;
    HDC            context_handle;
    PAINTSTRUCT    paint_info;
    RECT           the_rectangle;
    HCURSOR        cursor;
    static HMENU   instant_menu;
    static HWND    list_handle;

    // Obtain the client area information about the parent window about to be drawn. This data
    // will not be available to all case statments in switch and winproc.
    //
    context_handle = GetDC(window_handle);
    GetClientRect(window_handle, &the_rectangle);
    ReleaseDC(window_handle, context_handle);

    switch (message) {
      case WM_CREATE:
        if (location_read() == 1) {
          MessageBox(NULL, "Unable to read card map file", MBOX_CAPTION, MB_OK|MB_ICONEXCLAMATION);
        }

        user_info = get_users();
        list_handle = CreateWindowEx(
          0, WC_LISTVIEW, "", WS_CHILD|WS_VISIBLE|LVS_REPORT, 0, 0, 0, 0, window_handle, (HMENU)1, window_instance, NULL
        );
        SetWindowPos(list_handle, HWND_TOP, 0, 0, the_rectangle.right, the_rectangle.bottom, SWP_SHOWWINDOW);
        configure_ListView(list_handle);
        populate_ListView(list_handle, user_info);

        if (found == FALSE) {
          for(vector<UserData>::size_type i = 0; i < user_info->size(); ++i) {
            if((*user_info)[i].login_name == search_user)
              MessageBox(window_handle, "Is now logged in", search_user.c_str(), MB_OK);
          }
          found = TRUE;
        }
        return 0;

      case WM_PAINT:
        //You MUST call BeginPaint() at the start and EndPaint() at the end.
        context_handle = BeginPaint(window_handle, &paint_info);
        EndPaint(window_handle, &paint_info);
        return 0;

      case WM_CONTEXTMENU: {
        LV_HITTESTINFO hi;
        hi.pt.x = LOWORD(lParam);
        hi.pt.y = HIWORD(lParam);
        ScreenToClient(list_handle, &hi.pt);
        //Setup and create the popup menu (right mouse menu).
        if (ListView_HitTest(list_handle, &hi) > -1) {
          user_selected = hi.iItem;
          instant_menu = LoadMenu(window_instance, MAKEINTRESOURCE(POPUP_MENU));
          instant_menu = GetSubMenu(instant_menu, 0);
          TrackPopupMenu(instant_menu,
            TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
            LOWORD(lParam), HIWORD(lParam), 0, window_handle, NULL);
          DestroyMenu(instant_menu);
        }
      }
        break;

      case WM_TIMER:
        SendMessage(window_handle, WM_COMMAND, FILE_UPDATE, 0);
        return 0;

      case WM_COMMAND:
        switch (wParam) {
          case FILE_UPDATE:
            cursor = LoadCursor(NULL, IDC_WAIT);
            SetCursor(cursor);
            user_info = get_users();
            cursor = LoadCursor(NULL, IDC_APPSTARTING);
            SetCursor(cursor);

            ListView_DeleteAllItems(list_handle);
            populate_ListView(list_handle, user_info);
            UpdateWindow(window_handle);
            // SetScrollRange(window_handle, SB_VERT, 0, user_info->size() - Number_Of_Lines, NULL);

            if(found == FALSE) {
              for (vector<UserData>::size_type i = 0; i < user_info->size(); ++i) {
                if ((*user_info)[i].login_name == search_user)
                  MessageBox(window_handle, "Is now logged in", search_user.c_str(), MB_OK);
              }
              found = TRUE;
            }
            break;

          case FILE_EXIT:
            // Send Winproc a close message
            SendMessage(window_handle, WM_CLOSE, 0, 0);
            break;

          case FILE_CONFIG:
            // Launch the configuration dialog box.
            DialogBox(window_instance, "CONFIGBOX1", window_handle, ConfigBoxProc);
            // Set the timer to the new time set by the user
            SetTimer(window_handle, ID_TIMER, timer_interval * 1000 * 60, NULL);
            return 0;

          case HLP_ABOUT:
            MessageBox(window_handle,
              "RTWho Version 2.0\nVTC Computer Club\nSpring 2007", MBOX_CAPTION, MB_OK|MB_APPLMODAL|MB_ICONASTERISK);
            break;

          case UP_CHAT:
            char command_line[40];  // Hold the command line being constructed.
            char error[40];         // Hold the error string being constructed.
            int  error_num;         // The error returned by WinExec().

            sprintf(command_line, "P:\\BIN\\Chat98 %s %i", (*user_info)[user_selected].login_name);
            MessageBox(window_handle, command_line, command_line, MB_OK);
            error_num = WinExec(command_line, SW_SHOWNORMAL);
            sprintf(error,"%i",error_num);
            if (error_num == ERROR_FILE_NOT_FOUND)
              MessageBox(window_handle, error, "CHAT 98->ERROR FILE NOT FOUND", MB_OK|MB_ICONEXCLAMATION);
            if (error_num == ERROR_PATH_NOT_FOUND)
              MessageBox(window_handle, error, "CHAT98->ERROR_PATH_NOT_FOUND", MB_OK|MB_ICONEXCLAMATION);
            break;
        }
        return 0;

      case WM_SIZE:
         // Get the client area cordinates
         // cyClient = HIWORD (lParam);
         // cxClient = LOWORD (lParam);
         // Set the size of the child window to the size of the parent window
         SetWindowPos(list_handle, HWND_TOP, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_SHOWWINDOW);
         switch (wParam) {
           case SIZE_MINIMIZED:
             ShowWindow(window_handle, SW_HIDE);
             break;
         }
         return 0;

       case MYWM_NOTIFYICON:
         switch (lParam) {
            // Was our icon double clicked on? If yes show window.
           case WM_LBUTTONDBLCLK:
             ShowWindow(window_handle,SW_SHOWNORMAL);
             break;
         }
         return 0;

      case WM_DESTROY:
        MyTaskBarDeleteIcon(window_handle, RTW_ICON);
        KillTimer(window_handle, ID_TIMER);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(window_handle, message, wParam, lParam);
 }

// The function in the following example demonstrates how to add an icon to the taskbar.
// MyTaskBarAddIcon - adds an icon to the taskbar notification area. Returns TRUE if successful
// or FALSE otherwise.
//
// hwnd - handle of the window to receive callback messages
// uID - identifier of the icon
// hicon - handle of the icon to add
// lpszTip - tooltip text
//
BOOL MyTaskBarAddIcon(HWND hwnd, UINT uID, HICON hicon, LPSTR lpszTip)
{
  BOOL res;
  NOTIFYICONDATA tnid;
  tnid.cbSize = sizeof(NOTIFYICONDATA);
  tnid.hWnd = hwnd;
  tnid.uID = uID;
  tnid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  tnid.uCallbackMessage = MYWM_NOTIFYICON;
  tnid.hIcon = hicon;
  if (lpszTip)
    lstrcpyn(tnid.szTip, lpszTip, sizeof(tnid.szTip));
  else
    tnid.szTip[0] = '\0';
  res = Shell_NotifyIcon(NIM_ADD, &tnid);
  if (hicon)
    DestroyIcon(hicon);
  return res;
}

// MyTaskBarDeleteIcon - deletes an icon from the taskbar notification area. Returns TRUE if
// successful or FALSE otherwise.
//
// hwnd - handle of the window that added the icon
// uID - identifier of the icon to delete
//
BOOL MyTaskBarDeleteIcon(HWND hwnd, UINT uID)
{
  BOOL res;
  NOTIFYICONDATA tnid;
  tnid.cbSize = sizeof(NOTIFYICONDATA);
  tnid.hWnd = hwnd;
  tnid.uID = uID;
  res = Shell_NotifyIcon(NIM_DELETE, &tnid);
  return res;
}

BOOL CALLBACK ConfigBoxProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM /* lParam */)
 {
   char text[50];
   switch(iMsg) {
     case WM_INITDIALOG:
       //Set up the initial setting for the text controls
       sprintf(text, "%i", timer_interval);
       SetDlgItemText(hDlg, IDC_EDIT1, text);
       SetDlgItemText(hDlg, IDC_EDIT2, search_user.c_str());
       return TRUE;

     case WM_COMMAND:
       switch (LOWORD(wParam)) {
         char buffer[128+1];

         case IDOK:
           // if user clicked ok retrive current text control settings
           GetDlgItemText(hDlg, IDC_EDIT1, text, 10);
           timer_interval = atoi(text);
           if(timer_interval <1) timer_interval = 1;
           GetDlgItemText(hDlg, IDC_EDIT2, buffer, 128);
           search_user = buffer;
           // Send an update messsage to the main window
           SendMessage(GetParent(hDlg), WM_COMMAND, FILE_UPDATE, 0);
           EndDialog(hDlg, 0);
           found = FALSE;
           return TRUE;

         case IDCANCEL:
           EndDialog(hDlg,0);
           return TRUE;
       }
       break;
   }
   return FALSE;
 }

BOOL CALLBACK IntroBoxProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM /* lParam */)
 {
   switch(iMsg) {
     case WM_INITDIALOG:
       return TRUE;

     case WM_COMMAND:
       switch (LOWORD(wParam)) {
         case IDOK:
           EndDialog(hDlg,0);
           return TRUE;
       }
       break;
   }
   return FALSE;
 }
