#pragma warning(disable: 4996)
#include "addin.h"
#include "dialog.h"
#include "error.h"

using namespace xll;

// All addins coordinate through the registry.
Reg::Object<char, DWORD> xll_alert_level(
	HKEY_CURRENT_USER, "Software\\KALX\\xll", "xll_alert_level", 
	XLL_ALERT_ERROR|XLL_ALERT_WARNING
);
Reg::Object<wchar_t, DWORD> xll_alert_level12(
	HKEY_CURRENT_USER, L"Software\\KALX\\xll", L"xll_alert_level12", 
	XLL_ALERT_ERROR|XLL_ALERT_WARNING
);

extern "C" int 
XLL_ALERT(const char* text, const char* caption, int level, UINT type, bool force)
{
	int result = 0;

	if ((xll_alert_level&level) || force) {
		result = MessageBoxA(GetForegroundWindow(), text, caption, MB_OKCANCEL|type);
		if (result == IDCANCEL)
			xll_alert_level &= ~level;
	}

	return result;
}

int 
XLL_ERROR(const char* e, bool force)
{
	return XLL_ALERT(e, "Error", XLL_ALERT_ERROR, MB_ICONERROR, force);
}
int 
XLL_WARNING(const char* e, bool force)
{
	return XLL_ALERT(e, "Warning", XLL_ALERT_WARNING, MB_ICONWARNING, force);
}
int 
XLL_INFO(const char* e, bool force)
{
	return XLL_ALERT(e, "Information", XLL_ALERT_INFO, MB_ICONINFORMATION, force);
}

static AddIn xai_alert_filter(XLL_DECORATE("xll_alert_filter", 0), "ALERT.FILTER");
extern "C" int __declspec(dllexport) WINAPI
xll_alert_filter(void)
{
	try {
		int w = 230, h = 90, b = 10;
		Dialog d("Alert Filter", 10, 10, w, h);

		d.Add(Dialog::GroupBox, "Show alerts:", b, b, w/2 - 2*b, h - 2*b);

		WORD error = d.Add(Dialog::CheckBox, "Error");
   		d.Set(error, Num(xll_alert_level&XLL_ALERT_ERROR));

		WORD warning = d.Add(Dialog::CheckBox, "Warning");
   		d.Set(warning, Num(xll_alert_level&XLL_ALERT_WARNING));

		WORD info = d.Add(Dialog::CheckBox, "Info");
   		d.Set(info, Num(xll_alert_level&XLL_ALERT_INFO));

		WORD ok = d.Add(Dialog::OKButton, "OK", w/2 + 2*b, 2*b);
		d.Add(Dialog::CancelButton, "Cancel", w/2 + 2*b, h/2 + b);

		if (ok == d.Show()) {
			if (d.Get(error))
				xll_alert_level |= XLL_ALERT_ERROR;
			else
				xll_alert_level &= ~XLL_ALERT_ERROR;

			if (d.Get(warning))
				xll_alert_level |= XLL_ALERT_WARNING;
			else
				xll_alert_level &= ~XLL_ALERT_WARNING;

			if (d.Get(info))
				xll_alert_level |= XLL_ALERT_INFO;
			else
				xll_alert_level &= ~XLL_ALERT_INFO;
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return 0;
	}
	catch (...) {
		XLL_ERROR("ALERT.FILTER: unknown exception");

		return 0;
	}

	return 1;
}
