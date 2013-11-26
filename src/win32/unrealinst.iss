; UnrealIRCd Win32 Installation Script for My Inno Setup Extensions
; Requires Inno Setup 4.1.6 and ISX 3.0.4 to work

;#define USE_SSL
; Uncomment the above line to package an SSL build
#define USE_CURL
; Uncomment the above line to package with libcurl support


[Setup]
AppName=UnrealIRCd
AppVerName=UnrealIRCd3.4-dev
AppPublisher=UnrealIRCd Team
AppPublisherURL=http://www.unrealircd.com
AppSupportURL=http://www.unrealircd.com
AppUpdatesURL=http://www.unrealircd.com
AppMutex=UnrealMutex,Global\UnrealMutex
DefaultDirName={pf}\Unreal3.2
DefaultGroupName=UnrealIRCd
AllowNoIcons=yes
#ifndef USE_SSL
LicenseFile=.\gpl.rtf
#else
LicenseFile=.\gplplusssl.rtf
#endif
Compression=lzma
SolidCompression=true
MinVersion=5.0
OutputDir=../../

; !!! Make sure to update SSL validation (WizardForm.TasksList.Checked[9]) if tasks are added/removed !!!
[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"; Flags: unchecked
Name: "installservice"; Description: "Install as a &service (not for beginners)"; GroupDescription: "Service support:"; Flags: unchecked; MinVersion: 0,4.0
Name: "installservice/startboot"; Description: "S&tart UnrealIRCd when Windows starts"; GroupDescription: "Service support:"; MinVersion: 0,4.0; Flags: exclusive unchecked
Name: "installservice/startdemand"; Description: "Start UnrealIRCd on &request"; GroupDescription: "Service support:"; MinVersion: 0,4.0; Flags: exclusive unchecked
Name: "installservice/crashrestart"; Description: "Restart UnrealIRCd if it &crashes"; GroupDescription: "Service support:"; Flags: unchecked; MinVersion: 0,5.0;
#ifdef USE_SSL
Name: "makecert"; Description: "&Create certificate"; GroupDescription: "SSL options:";
Name: "enccert"; Description: "&Encrypt certificate"; GroupDescription: "SSL options:"; Flags: unchecked;
#endif
Name: "fixperm"; Description: "Make Unreal folder writable by current user";

[Files]
Source: "..\..\wircd.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\WIRCD.pdb"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\.CHANGES.NEW"; DestDir: "{app}"; DestName: "CHANGES.NEW.txt";Flags: ignoreversion
Source: "..\..\.CONFIG.RANT"; DestDir: "{app}"; DestName: "CONFIG.RANT.txt"; Flags: ignoreversion
Source: "..\..\.RELEASE.NOTES"; DestDir: "{app}"; DestName: "RELEASE.NOTES.txt"; Flags: ignoreversion
Source: "..\..\.SICI"; DestDir: "{app}"; DestName: "SICI.txt"; Flags: ignoreversion
Source: "..\..\badwords.channel.conf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\badwords.message.conf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\badwords.quit.conf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\spamfilter.conf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\dccallow.conf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\Changes"; DestDir: "{app}"; DestName: "Changes.txt"; Flags: ignoreversion
Source: "..\..\Changes.old"; DestDir: "{app}"; DestName: "Changes.old.txt"; Flags: ignoreversion
Source: "..\..\Donation"; DestDir: "{app}"; DestName: "Donation.txt"; Flags: ignoreversion
Source: "..\..\help.conf"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\LICENSE"; DestDir: "{app}"; DestName: "LICENSE.txt"; Flags: ignoreversion
Source: "..\..\Unreal.nfo"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\doc\*.*"; DestDir: "{app}\doc"; Flags: ignoreversion
Source: "..\..\doc\technical\*.*"; DestDir: "{app}\doc\technical"; Flags: ignoreversion
Source: "..\..\aliases\*"; DestDir: "{app}\aliases"; Flags: ignoreversion
Source: "..\..\unreal.exe"; DestDir: "{app}"; Flags: ignoreversion; MinVersion: 0,4.0
Source: "..\modules\*.dll"; DestDir: "{app}\modules"; Flags: ignoreversion
Source: "c:\dev\tre\win32\release\tre.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "C:\dev\c-ares\msvc90\cares\dll-release\cares.dll"; DestDir: "{app}"; Flags: ignoreversion
#ifdef USE_SSL
Source: "c:\openssl\bin\openssl.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "c:\openssl\bin\ssleay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "c:\openssl\bin\libeay32.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "c:\dev\setacl.exe"; DestDir: "{app}\tmp"; Flags: ignoreversion
Source: ".\makecert.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: ".\encpem.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\ssl.cnf"; DestDir: "{app}"; Flags: ignoreversion
#endif
#ifdef USE_SSL
#ifdef USE_CURL
; curl with ssl support
Source: "C:\dev\curl-ssl\builds\libcurl-release-dll-sspi-spnego\bin\libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\curl-ca-bundle.crt"; DestDir: "{app}"; Flags: ignoreversion
#endif
#else
#ifdef USE_CURL
; curl without ssl support
Source: "C:\dev\curl\builds\libcurl-release-dll-sspi-spnego\bin\libcurl.dll"; DestDir: "{app}"; Flags: ignoreversion
#endif
#endif
Source: isxdl.dll; DestDir: {tmp}; Flags: dontcopy
Source: "..\..\..\dbghelp.dll"; DestDir: "{app}"; Flags: ignoreversion

[Dirs]
Name: "{app}\tmp"

[UninstallDelete]
Type: files; Name: "{app}\DbgHelp.Dll"

[Code]
procedure isxdl_AddFile(URL, Filename: PChar);
external 'isxdl_AddFile@files:isxdl.dll stdcall';
function isxdl_DownloadFiles(hWnd: Integer): Integer;
external 'isxdl_DownloadFiles@files:isxdl.dll stdcall';
function isxdl_SetOption(Option, Value: PChar): Integer;
external 'isxdl_SetOption@files:isxdl.dll stdcall';

var
	MSVSRedistPath: string;
	downloadNeeded: boolean;
	MSVSNeeded: boolean;
	memoDependenciesNeeded: string;
  uninstaller: String;
  ErrorCode: Integer;

const
	MSVSRedistURL = 'http://download.microsoft.com/download/1/1/1/1116b75a-9ec3-481a-a3c8-1777b5381140/vcredist_x86.exe';

//*********************************************************************************
// This is where all starts.
//*********************************************************************************
function InitializeSetup(): Boolean;

begin

	Result := true;
	MSVSNeeded := false;
	
  //************************************************************************************
	// Check for the existance of the Visual C++ 2008 Redist. Package on client machine before installing
	//************************************************************************************
	// ids: supposedly VC++ 2008 runtime x86, and one from another source, SP1 version, SP1 with ATL security update, similar but MFC upd.
    if ((not RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{3C3D696B-0DB7-3C6D-A356-3DB8CE541918}'))
         and (not RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{FF66E9F6-83E7-3A3E-AF14-8DE9A809A6A4}'))
         and (not RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{9A25302D-30C0-39D9-BD6F-21E6EC160475}'))
         and (not RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{1F1C2DFC-2D24-3E06-BCB8-725134ADF989}'))
         and (not RegKeyExists(HKLM, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{9BE518E6-ECC6-35A9-88E4-87755C07200F}'))
         ) then
		begin
			MSVSNeeded := true;
				

			if (not IsAdminLoggedOn()) then
				begin
					MsgBox('Application needs the Microsoft Visual C++ Redistributable Package to be installed by an Administrator', mbInformation, MB_OK);
					Result := false;
				end
			else
				begin
					memoDependenciesNeeded := memoDependenciesNeeded + '      Microsoft Visual C++ Redist. Package' #13;
					MSVSRedistPath := ExpandConstant('{src}\vcredist_x86.exe');
					if not FileExists(MSVSRedistPath) then
						begin
							MSVSRedistPath := ExpandConstant('{tmp}\vcredist_x86.exe');
							if not FileExists(MSVSRedistPath) then
								begin
									isxdl_AddFile(MSVSRedistURL, MSVSRedistPath);
									downloadNeeded := true;
								end
						end

					//SetIniString('install', 'MSVSRedist', MSVSRedistPath, ExpandConstant('{tmp}\dep.ini'));
					// wth is that?
				end
		end;

end;

function NextButtonClick(CurPage: Integer): Boolean;

var
  hWnd: Integer;
  ResultCode: Integer;
  ResultXP: boolean;
  Result2003: boolean;
  Res: Integer;
begin

  Result := true;
  ResultXP := true;
  Result2003 := true;

  // Prevent the user from selecting both 'Install as service' and 'Encrypt SSL certificate'
  if CurPage = wpSelectTasks then
  begin
    if IsTaskSelected('enccert') and IsTaskSelected('installservice') then
    begin
      MsgBox('When running UnrealIRCd as a service there is no way to enter the password for an encrypted SSL certificate, therefore you cannot combine the two. Please deselect one of the options.', mbError, MB_OK);
      Result := False
    end
  end;

  //*********************************************************************************
  // Only run this at the "Ready To Install" wizard page.
  //*********************************************************************************
  if CurPage = wpReady then
	begin

		hWnd := StrToInt(ExpandConstant('{wizardhwnd}'));

		// don't try to init isxdl if it's not needed because it will error on < ie 3

		//********************************************************************************************************
		// Download the Visual C++ 2008 Redist. Package. Can change the MS link to application development site to avoid dead link
		//*********************************************************************************************************
		if downloadNeeded and (MSVSNeeded = true) then
			begin
				isxdl_SetOption('label', 'Downloading Microsoft Visual C++ Redist. Package');
				isxdl_SetOption('description', 'This app needs to install the Microsoft Visual C++ Redist. Package. Please wait while Setup is downloading extra files to your computer.');
				if isxdl_DownloadFiles(hWnd) = 0 then Result := false;
			end;

		//***********************************************************************************
		// Run the install file...
		//***********************************************************************************
      if (MSVSNeeded = true) then
			begin

				if Exec(ExpandConstant(MSVSRedistPath), '', '', SW_SHOW, ewWaitUntilTerminated, ResultCode) then
					begin

						// handle success if necessary; ResultCode contains the exit code
						if not (ResultCode = 0) then
							begin

	 						  Res := MsgBox('UnrealIRCd requires the Microsoft Visual C++ 2008 Redistributable Package, and tried to download & install it for you. However, this failed. This could be for a number of reasons, such as it already being installed, or you are using a 64 bit Operating System. If it is not installed yet, or you do not know, then you are suggested to manually download and install the "Microsoft Visual C++ 2008 Redistributable package" from www.microsoft.com. Do you want to continue installing UnrealIRCd anyway?', mbConfirmation, MB_YESNO);
  							if (Res = IDNO) then
  							begin
                  Result := false;
                end

							end
					end
					else
						begin

							// handle failure if necessary; ResultCode contains the error code
							Res := MsgBox('UnrealIRCd requires the Microsoft Visual C++ 2008 Redistributable Package, and tried to download & install it for you. However, this failed. This could be for a number of reasons, such as it already being installed, or you are using a 64 bit Operating System. If it is not installed yet, or you do not know, then you are suggested to manually download and install the "Microsoft Visual C++ 2008 Redistributable package" from www.microsoft.com. Do you want to continue installing UnrealIRCd anyway?', mbConfirmation, MB_YESNO);
							if (Res = IDNO) then
							begin
                  Result := false;
              end
						end
			end;


end;
end;

//*********************************************************************************
// Updates the memo box shown right before the install actuall starts.
//*********************************************************************************
function UpdateReadyMemo(Space, NewLine, MemoUserInfoInfo, MemoDirInfo, MemoTypeInfo, MemoComponentsInfo, MemoGroupInfo, MemoTasksInfo: String): String;
var
  s: string;

begin

  if memoDependenciesNeeded <> '' then s := s + 'Dependencies that will be automatically downloaded And installed:' + NewLine + memoDependenciesNeeded + NewLine;
  s := s + MemoDirInfo + NewLine + NewLine;

  Result := s

end;

procedure CurStepChanged(CurStep: TSetupStep);

var
  hWnd: Integer;
  ResultCode: Integer;
  ResultXP: boolean;
  Result2003: boolean;
  Res: Integer;
  s: String;
  d: String;
begin
if CurStep = ssPostInstall then
	begin
     d := ExpandConstant('{app}');
	   if IsTaskSelected('fixperm') then
	   begin
	     // This fixes the permissions in the Unreal3.2 folder by granting full access to the user
	     // running the install.
	     s := '-on "'+d+'" -ot file -actn ace -ace "n:'+GetUserNameString()+';p:full;m:set';
	     Exec(d+'\tmp\setacl.exe', s, d, SW_HIDE, ewWaitUntilTerminated, Res);
	   end
	   else
	   begin
	     MsgBox('You have chosen to not have the installer automatically set write access. Please ensure that the user running the IRCd can write to '+d+', otherwise the IRCd will fail to load.',mbConfirmation, MB_OK);
	   end
  end;
end;

//*********************************************************************************
// Checks if ssl cert file exists
//*********************************************************************************

#ifdef USE_SSL
procedure CurPageChanged(CurPage: Integer);
begin
  if (CurPage = wpSelectTasks)then
  begin
     if FileExists(ExpandConstant('{app}\server.cert.pem')) then
     begin
        WizardForm.TasksList.Checked[9]:=false;
     end
     else
     begin
        WizardForm.TasksList.Checked[9]:=true;
     end
  end
end;
#endif

[Icons]
Name: "{group}\UnrealIRCd"; Filename: "{app}\wircd.exe"; WorkingDir: "{app}"
Name: "{group}\Uninstall UnrealIRCd"; Filename: "{uninstallexe}"; WorkingDir: "{app}"
#ifdef USE_SSL
Name: "{group}\Make Certificate"; Filename: "{app}\makecert.bat"; WorkingDir: "{app}"
Name: "{group}\Encrypt Certificate"; Filename: "{app}\encpem.bat"; WorkingDir: "{app}"
#endif
Name: "{group}\Documentation"; Filename: "{app}\doc\unreal32docs.html"; WorkingDir: "{app}"
Name: "{userdesktop}\UnrealIRCd"; Filename: "{app}\wircd.exe"; WorkingDir: "{app}"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\UnrealIRCd"; Filename: "{app}\wircd.exe"; WorkingDir: "{app}"; Tasks: quicklaunchicon

[Run]
Filename: "notepad"; Description: "View example.conf"; Parameters: "{app}\doc\example.conf"; Flags: postinstall skipifsilent shellexec runmaximized
Filename: "{app}\doc\unreal32docs.html"; Description: "View UnrealIRCd documentation"; Parameters: ""; Flags: postinstall skipifsilent shellexec runmaximized
Filename: "notepad"; Description: "View Release Notes"; Parameters: "{app}\RELEASE.NOTES.txt"; Flags: postinstall skipifsilent shellexec runmaximized
Filename: "notepad"; Description: "View Changes"; Parameters: "{app}\Changes.txt"; Flags: postinstall skipifsilent shellexec runmaximized
Filename: "{app}\unreal.exe"; Parameters: "install"; Flags: runminimized nowait; Tasks: installservice
Filename: "{app}\unreal.exe"; Parameters: "config startup manual"; Flags: runminimized nowait; Tasks: installservice/startdemand
Filename: "{app}\unreal.exe"; Parameters: "config startup auto"; Flags: runminimized nowait; Tasks: installservice/startboot
Filename: "{app}\unreal.exe"; Parameters: "config crashrestart 2"; Flags: runminimized nowait; Tasks: installservice/crashrestart
#ifdef USE_SSL
Filename: "{app}\makecert.bat"; Tasks: makecert; Flags: postinstall;
Filename: "{app}\encpem.bat"; WorkingDir: "{app}"; Tasks: enccert; Flags: postinstall;
#endif

[UninstallRun]
Filename: "{app}\unreal.exe"; Parameters: "uninstall"; Flags: runminimized; RunOnceID: "DelService"; Tasks: installservice
