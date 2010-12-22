/////////////////////////////////////////////////////////////////////////////////
// Author:      Steven Lamerton
// Copyright:   Copyright (C) 2007 - 2010 Steven Lamerton
// License:     GNU GPL 2 http://www.gnu.org/licenses/gpl-2.0.html
/////////////////////////////////////////////////////////////////////////////////

#include "frmmain.h"
#include "frmprogress.h"
#include "../luamanager.h"
#include "../toucan.h"
#include "../settings.h"
#include "../basicfunctions.h"
#include "../controls/loglistctrl.h"

#include <boost/interprocess/ipc/message_queue.hpp>

using namespace boost::interprocess;

#include <wx/stdpaths.h>
#include <wx/listctrl.h>
#include <wx/textfile.h>
#include <wx/datetime.h>
#include <wx/tglbtn.h>
#include <wx/wx.h>

#if defined(__WXMSW__) && !defined(__MINGW32__)
	#include <shobjidl.h>
	#undef Yield
#endif

//frmProgress event table
BEGIN_EVENT_TABLE(frmProgress, wxDialog)
	EVT_BUTTON(wxID_OK, frmProgress::OnOkClick)
	EVT_BUTTON(wxID_CANCEL, frmProgress::OnCancelClick)
	EVT_BUTTON(wxID_SAVE, frmProgress::OnSaveClick)
	EVT_CLOSE(frmProgress::OnClose)
    EVT_SIZE(frmProgress::OnSize)
    EVT_IDLE(frmProgress::OnIdle)
END_EVENT_TABLE()

//Constructor
frmProgress::frmProgress(wxWindow* parent, wxWindowID id, const wxString& caption){
    Init();

    wxGetApp().m_Settings->TransferFromFile();

    //Set up the size and position
    int height, width;
    wxClientDisplayRect(NULL, NULL, &width, &height);

    wxPoint position((int)(wxGetApp().m_Settings->GetProgressX() * width), (int)(wxGetApp().m_Settings->GetProgressY() * height));
    if(position.x < 0 || position.y < 0 || position.x > width || position.y > height){
        position = wxDefaultPosition;
    }

    wxSize size((int)(wxGetApp().m_Settings->GetProgressWidth() * width), (int)(wxGetApp().m_Settings->GetProgressHeight() * height));
    if(size.GetX() < 0 || size.GetY() < 0)
        size = wxSize(640, 480);

    long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER;

    wxDialog::Create(parent, id, caption, position, size, style);
    CreateControls();
}

//frmProgress initialisation
void frmProgress::Init(){
	m_List = NULL;
	m_OK = NULL;
	m_Cancel = NULL;
	m_Save = NULL;
	m_Gauge = NULL;
}

//Create controls
void frmProgress::CreateControls(){
	frmProgress* Frame = this;

	wxPanel* Panel = new wxPanel(Frame, ID_PANEL_PROGRESS, wxDefaultPosition, wxDefaultSize, wxSUNKEN_BORDER|wxTAB_TRAVERSAL );

	wxBoxSizer* TopSizer = new wxBoxSizer(wxVERTICAL);
	Panel->SetSizer(TopSizer);

	wxBoxSizer* MiddleSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(MiddleSizer, 1, wxGROW|wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

    wxBoxSizer* ProgressSizer = new wxBoxSizer(wxVERTICAL);
    MiddleSizer->Add(ProgressSizer, 1, wxGROW);

	m_Gauge = new wxGauge(Panel, ID_PROGRESS_GAUGE, 100, wxDefaultPosition, wxDefaultSize, wxGA_SMOOTH|wxGA_HORIZONTAL);
	ProgressSizer->Add(m_Gauge, 0, wxALIGN_CENTER_HORIZONTAL|wxALL|wxEXPAND, 5);

	m_List = new wxListCtrl(Panel, ID_PROGRESS_LIST, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_SINGLE_SEL|wxLC_VRULES);
	ProgressSizer->Add(m_List, 1, wxGROW|wxALL, 5);

	wxBoxSizer* SmallButtonSizer = new wxBoxSizer(wxVERTICAL);
	MiddleSizer->Add(SmallButtonSizer, 0, wxALIGN_CENTER_VERTICAL|wxALL, 0);

	m_Save = new wxBitmapButton(Panel, wxID_SAVE, GetBitmapResource(wxT("save.png")));
	SmallButtonSizer->Add(m_Save, 0, wxALL, 5);

	m_Autoscroll = new wxBitmapToggleButton(Panel, ID_PROGRESS_AUTOSCROLL, GetBitmapResource(wxT("autoscroll.png")));
    m_Autoscroll->SetValue(true);
	SmallButtonSizer->Add(m_Autoscroll, 0, wxALL, 5);

	wxBoxSizer* ButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	TopSizer->Add(ButtonSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxALL, 5);

	m_OK = new wxButton(Panel, wxID_OK, _("OK"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonSizer->Add(m_OK, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);

	m_Cancel = new wxButton(Panel, wxID_CANCEL, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
	ButtonSizer->Add(m_Cancel, 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	
	//Add columns
	m_List->InsertColumn(0, _("Time"));
	m_List->InsertColumn(1, _("Status"));
	
	//Set the form icon
	wxString strPath = wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + wxFILE_SEP_PATH;
	this->SetIcon(wxIcon(strPath + wxT("Toucan.ico"), wxBITMAP_TYPE_ICO));
}

//Get bitmap resources
wxBitmap frmProgress::GetBitmapResource(const wxString& name){
	wxString path = wxPathOnly(wxStandardPaths::Get().GetExecutablePath()) + wxFILE_SEP_PATH + wxT("bitmaps") + wxFILE_SEP_PATH;
	wxBitmap bitmap(path + name, wxBITMAP_TYPE_PNG);
	if(bitmap.IsOk()){
		return bitmap;
	}
	return wxNullBitmap;
}

void frmProgress::OnClose(wxCloseEvent& WXUNUSED(event)){
    if(!wxGetApp().IsReadOnly()){
        int height, width, x, y;
        wxClientDisplayRect(&x, &y, &width, &height);
	
        wxGetApp().m_Settings->SetProgressHeight((double)(this->GetSize().GetHeight())/(height));
        wxGetApp().m_Settings->SetProgressWidth((double)(this->GetSize().GetWidth())/(width));
        wxGetApp().m_Settings->SetProgressX((double)(this->GetScreenPosition().x)/(width));
        wxGetApp().m_Settings->SetProgressY((double)(this->GetScreenPosition().y)/(height));

        wxGetApp().m_Settings->TransferToFile();
    }
    //Clear the taskbar when we close
#if defined(__WXMSW__) && !defined(__MINGW32__)
	if(m_Taskbar){
		m_Taskbar->SetProgressState(static_cast<HWND>(wxGetApp().MainWindow->GetHandle()), TBPF_NOPROGRESS);
	}
#endif
	wxGetApp().m_LuaManager->NullWindow();
	Destroy();
}

void frmProgress::OnOkClick(wxCommandEvent& WXUNUSED(event)){
	Close();
}

void frmProgress::OnCancelClick(wxCommandEvent& WXUNUSED(event)){
	wxGetApp().SetAbort(true);
}

void frmProgress::OnSaveClick(wxCommandEvent& WXUNUSED(event)){
	wxString strCaption = _("Save");
	wxString strWildcard;
	strWildcard = wxT("Text Files (*.txt)|*.txt");
	wxString defaultFilename = wxEmptyString;
	wxString defaultDir = wxT("/");
	wxFileDialog dialog(this, strCaption, defaultDir, defaultFilename, strWildcard, wxFD_SAVE);
	if(dialog.ShowModal() == wxID_OK){
		wxTextFile file(dialog.GetPath());
		if(wxFileExists(dialog.GetPath())){
			file.Open();
			file.Clear();
			file.Write();
		}
		else{
			file.Create();
		}
		for(int i = 0; i < m_List->GetItemCount() - 1; i++){
			wxListItem itemcol1, itemcol2;

			itemcol1.m_itemId = i;
			itemcol1.m_col = 0;
			itemcol1.m_mask = wxLIST_MASK_TEXT;
			m_List->GetItem(itemcol1);
			itemcol2.m_itemId = i;
			itemcol2.m_col = 1;
			itemcol2.m_mask = wxLIST_MASK_TEXT;
			m_List->GetItem(itemcol2);
			file.AddLine(itemcol1.m_text + wxT("\t") + itemcol2.m_text);
		}
	file.Write();
	}
}

void frmProgress::IncrementGauge(){
	m_Gauge->SetValue(m_Gauge->GetValue() + 1);
#if defined(__WXMSW__) && !defined(__MINGW32__)
	if(m_Taskbar){
		m_Taskbar->SetProgressValue(static_cast<HWND>(wxGetApp().MainWindow->GetHandle()), m_Gauge->GetValue() + 1, m_Gauge->GetRange());
	}
#endif

}

void frmProgress::FinishGauge(){
	m_Gauge->SetValue(m_Gauge->GetRange());
#if defined(__WXMSW__) && !defined(__MINGW32__)
	if(m_Taskbar){
		m_Taskbar->SetProgressValue(static_cast<HWND>(wxGetApp().MainWindow->GetHandle()), m_Gauge->GetRange(), m_Gauge->GetRange());
	}
#endif
}

void frmProgress::OnSize(wxSizeEvent &event){
    m_List->SetColumnWidth(1, -1);
    event.Skip();
}

void frmProgress::OnIdle(wxIdleEvent &event){
    try{
//        permissions access;
 //       access.set_unrestricted();
        message_queue mq(open_or_create, "progress", 100, 10000/*, access*/);

        std::string message;
        message.resize(10000);
        size_t size;
        unsigned int priority;

        if(mq.try_receive(&message[0], message.size(), size, priority)){
            message.resize(size);
            wxString wxmessage(message.c_str(), wxConvUTF8);

        	long index = m_List->InsertItem(m_List->GetItemCount(), wxEmptyString);
			m_List->SetItem(index, 1, wxmessage);

            if(priority == Error){
				m_List->SetItemTextColour(index, wxColour(wxT("Red")));
			}
			if(priority == Error || priority == StartingLine || priority == FinishingLine){
				m_List->SetItem(index, 0, wxDateTime::Now().FormatISOTime());
			}
            if(priority == Message || priority == Error){
                IncrementGauge();
            }

            if(m_Autoscroll->GetValue()){
			    m_List->EnsureVisible(index);
			    Update();
            }

            m_List->SetColumnWidth(1, -1);

            if(priority == StartingLine){
                StartProgress();
            }
            else if(priority == FinishingLine){
                FinishProgress();
                
            }
            else{
                event.RequestMore();
            }
        }
    }
    catch(std::exception &ex){
        wxLogError("%s", ex.what());
    }
}

void frmProgress::RequestUserAttention(){
    //We actaully need the main frame to flash as we are just a modal dialog
#ifdef __WXMSW__
	FLASHWINFO info;
	info.uCount = 3;
	info.dwTimeout = 0;
	info.hwnd = static_cast<HWND>(wxGetApp().MainWindow->GetHandle());
	info.cbSize = sizeof(info);
	if(wxWindow::FindFocus() == this){
        info.dwFlags = FLASHW_ALL|FLASHW_TIMERNOFG;			
	}
	else{
		info.dwFlags = FLASHW_ALL;					
	}
	FlashWindowEx(&info);
#else
    wxGetApp().MainWindow->RequestUserAttention();
#endif
}

void frmProgress::StartProgress(){
    //Send all errors to this window
    LogListCtrl* logList = new LogListCtrl(m_List);
    delete wxLog::SetActiveTarget(logList);

    m_OK->Enable(false);
    m_Save->Enable(false);
    m_Cancel->Enable(true);
}

void frmProgress::FinishProgress(){
    //Send all errors to the standard gui
	delete wxLog::SetActiveTarget(new wxLogGui);
		        
    //Enable the buttons
    m_OK->Enable(true);
    m_Save->Enable(true);
    m_Cancel->Enable(false);

    //Let the user know we have finished
    FinishGauge();
    RequestUserAttention();

    //Refresh the gui to show any changes
	wxCommandEvent event;
	wxGetApp().MainWindow->OnSyncRefresh(event);
	wxGetApp().MainWindow->OnBackupRefresh(event);
	wxGetApp().MainWindow->OnSecureRefresh(event);
}
