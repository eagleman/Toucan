/////////////////////////////////////////////////////////////////////////////////
// Author:      Steven Lamerton
// Copyright:   Copyright (C) 2008 - 2010 Steven Lamerton
// License:     GNU GPL 2 http://www.gnu.org/licenses/gpl-2.0.html
/////////////////////////////////////////////////////////////////////////////////

#include "settings.h"
#include <wx/fileconf.h>
#include <wx/font.h>
#include <wx/string.h>
#include <wx/settings.h>
#include <wx/intl.h>

Settings::Settings(wxString path){
	//Set the default values
	m_LanguageCode = wxT("en");
	m_Font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetNativeFontInfoDesc();
	m_Height = m_Width = m_Y = m_X = -1;
    m_ProgressHeight = m_ProgressWidth = m_ProgressY = m_ProgressX = -1;
	m_RememberSync = m_RememberBackup = m_RememberSecure = false;
	m_EnableTooltips = false;
	m_SmallBorders = false;
	m_DisableStream = false;
	m_DisableLog = false;
	config = new wxFileConfig( wxT(""), wxT(""), path);
}

Settings::~Settings(){
	delete config;
}

bool Settings::TransferToFile(){
	config->Write(wxT("General/Tabs"), ToEn(m_TabStyle));
	config->Write(wxT("General/Position"), ToEn(m_Position));
	config->Write(wxT("General/LanguageCode"), m_LanguageCode);
	config->Write(wxT("General/Font"), m_Font);
	config->Write(wxT("General/Height"), m_Height);
	config->Write(wxT("General/Width"), m_Width);
	config->Write(wxT("General/X"), m_X);
	config->Write(wxT("General/Y"), m_Y);
	config->Write(wxT("General/ProgressHeight"), m_ProgressHeight);
	config->Write(wxT("General/ProgressWidth"), m_ProgressWidth);
	config->Write(wxT("General/ProgressX"), m_ProgressX);
	config->Write(wxT("General/ProgressY"), m_ProgressY);
	config->Write(wxT("General/RememberSync"), m_RememberSync);
	config->Write(wxT("General/RememberBackup"), m_RememberBackup);
	config->Write(wxT("General/RememberSecure"), m_RememberSecure);
	config->Write(wxT("General/EnableTooltips"), m_EnableTooltips);
	config->Write(wxT("General/SmallBorders"), m_SmallBorders);
	config->Write(wxT("Sync/DisableStream"), m_DisableStream);
	config->Write(wxT("CommandLine/DisableLog"), m_DisableLog);
    config->Flush();
	return true;
}

bool Settings::TransferFromFile(){
	wxString stemp;
	
	//Read from the settings file
	if(!config->Read(wxT("General/Tabs"), &stemp)){
		m_TabStyle = ToLang("Icons and Text");
	}	
	else{
		m_TabStyle = ToLang(stemp);
	}
	if(!config->Read(wxT("General/Position"), &stemp)){
		m_Position = ToLang("Sync");
	}
	else{
		m_Position = ToLang(stemp);
	}
	config->Read(wxT("General/LanguageCode"), &m_LanguageCode);
	config->Read(wxT("General/Font"), &m_Font);
	config->Read(wxT("General/Height"), &m_Height);
	config->Read(wxT("General/Width"), &m_Width);
	config->Read(wxT("General/X"), &m_X);
	config->Read(wxT("General/Y"), &m_Y);
	config->Read(wxT("General/ProgressHeight"), &m_ProgressHeight);
	config->Read(wxT("General/ProgressWidth"), &m_ProgressWidth);
	config->Read(wxT("General/ProgressX"), &m_ProgressX);
	config->Read(wxT("General/ProgressY"), &m_ProgressY);
	config->Read(wxT("General/RememberSync"), &m_RememberSync);
	config->Read(wxT("General/RememberBackup"), &m_RememberBackup);
	config->Read(wxT("General/RememberSecure"), &m_RememberSecure);
	config->Read(wxT("General/EnableTooltips"), &m_EnableTooltips);
	config->Read(wxT("General/SmallBorders"), &m_SmallBorders);
	config->Read(wxT("Sync/DisableStream"), &m_DisableStream);
	config->Read(wxT("CommandLine/DisableLog"), &m_DisableLog);
	return true;
}
