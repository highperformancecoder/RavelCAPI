// cred.h - CredUI wrapper for the Credential Manager
#pragma once
#include <functional>
#include <windows.h>
#include <tchar.h>
#include <wincred.h>

#pragma comment(lib, "credui.lib")

/*****************************************************************************

Use generic credentials and 3rd party authentication function.

Cred cred(TARGET_NAME, authorize);

if (!cred) {
	// failed to read credentials
	cred.Prompt(_T("Caption"), _T("Message"));
}

if (cred)
	; // success
else
	; // failure

*****************************************************************************/

class Cred {
	LPCTSTR TargetName_;
	bool authorized;
	std::function<bool(LPCTSTR,LPCTSTR)> authorize_;
	PCREDENTIAL pcred;
	Cred(const Cred&);
	Cred& operator=(const Cred&);
public:
	TCHAR user[CREDUI_MAX_USERNAME_LENGTH + 1];
	TCHAR pass[CREDUI_MAX_PASSWORD_LENGTH + 1];

	Cred(LPCTSTR TargetName, const std::function<bool(LPCTSTR,LPCTSTR)>& authorize)
		: TargetName_(TargetName), authorized(false), authorize_(authorize), pcred(0)
	{
		SecureZeroMemory(user, sizeof(user));
		SecureZeroMemory(pass, sizeof(pass));

		if (CredRead(TargetName, CRED_TYPE_GENERIC, 0, &pcred)) {
			_tcsncpy_s(user, CREDUI_MAX_USERNAME_LENGTH + 1, pcred->UserName, CREDUI_MAX_USERNAME_LENGTH);
			size_t i;
			for (i = 0; i < pcred->CredentialBlobSize/2; ++i)
				pass[i] = pcred->CredentialBlob[2*i]; // yep, 2*i
			pass[i] = 0;

			authorized = authorize_(user, pass);

			if (!authorized)
				CredDelete(TargetName_, CRED_TYPE_GENERIC, 0);
		}
	}
	~Cred()
	{
		SecureZeroMemory(user, sizeof(user));
		SecureZeroMemory(pass, sizeof(pass));

		if (pcred)
			CredFree(pcred);
	}
	operator bool() const
	{
		return authorized;
	}
	bool Prompt(HWND Parent = GetForegroundWindow(), LPCTSTR CaptionText = _T("Credentials"), LPCTSTR MessageText = _T("Please enter your credentials"), HBITMAP Banner = 0)
	{
		static BOOL save(TRUE);
		static DWORD flags = CREDUI_FLAGS_GENERIC_CREDENTIALS|
			                 CREDUI_FLAGS_SHOW_SAVE_CHECK_BOX|
							 CREDUI_FLAGS_INCORRECT_PASSWORD|
							 CREDUI_FLAGS_EXPECT_CONFIRMATION;

		static CREDUI_INFO info;
		info.cbSize = sizeof(CREDUI_INFO);
		info.hbmBanner = Banner;
		info.hwndParent = Parent;
		info.pszCaptionText = CaptionText;
		info.pszMessageText = MessageText;
		SecureZeroMemory(user, sizeof(user));
reprompt:
		SecureZeroMemory(pass, sizeof(pass));

		DWORD result = CredUIPromptForCredentials(&info, 
			TargetName_, 0, 0, 
			user, CREDUI_MAX_USERNAME_LENGTH + 1, 
			pass, CREDUI_MAX_PASSWORD_LENGTH + 1, 
			&save, flags);

		if (result == ERROR_CANCELLED) {
			return true;
		}

		if (result == NO_ERROR) {
			authorized = authorize_(user, pass);
			// cleans up if not authorized
			CredUIConfirmCredentials(TargetName_, authorized);

			if (!authorized)
				goto reprompt;

			//CredWrite(pcred, 0);
		}

		return false;
	}
};

