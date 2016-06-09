// document.cpp - Generate files for Sandcastle documentation.
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
//
// Load the add-in in Excel and run XLL.DOC to create documentation.
//
#include <fstream>
#include <map>
#include "../xll.h"
#include "../utility/env.h"
#include "../utility/file.h"
#include "../utility/message.h"
#include "../utility/registry.h"
#include "../maml/document.h"

#define ENSURE(x) if (!(x)) throw std::runtime_error(Error::Message(GetLastError()).what())
#define XML_PRE _T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");
#define MAX_BUF 2048
#define DIR(x, y) std::basic_string<TCHAR>(x).append(_T("\\")).append(y).c_str()
#define STR(x) xll::to_string<XLOPERX>(x).c_str()
#define PSTR(x) x.val.str + 1, x.val.str[0]

using namespace std;
using namespace xll;
using namespace xml;

// category -> function -> Args map
typedef	map<OPERX, const ArgsX*> fun_map;
typedef fun_map::const_iterator fun_citer;
typedef	map<OPERX, fun_map> cat_fun_map;
typedef cat_fun_map::const_iterator cat_fun_citer;

#pragma warning(push)
#pragma warning(disable: 4706)

// return base file name from full path
inline std::basic_string<TCHAR> 
module_name(LPCTSTR path)
{
	LPCTSTR b, e;

	ensure (b = _tcsrchr(path, _T('\\')));
	ensure (e = _tcsrchr(path, _T('.')));
	ensure (b < e);

	return std::basic_string<TCHAR>(b + 1, e);
}
#pragma warning(pop)
inline std::basic_string<TCHAR>
project_dir(LPCTSTR path)
{
	std::basic_string<TCHAR> dir(path);

	dir.resize(dir.find_last_of(_T('\\')));
	dir.resize(dir.find_last_of(_T('\\')));

	return dir;
}

void
write_xmlcomp(const ArgsX& aia)
{
	std::basic_string<TCHAR> file = _T("xmlcomp\\");
	file += TUID(aia.TopicId());
	file += _T(".cmp.xml");

	File::Create cmp(file.c_str(), GENERIC_WRITE, CREATE_ALWAYS);

	cmp << XML_PRE;
	cmp << 
	element(_T("metadata"))
	.attribute(_T("fileAssetGuid"), TUID(aia.TopicId()))
	.attribute(_T("assetTypeId"), _T("CompanionFile"))
	.content(
		element(_T("topic"))
		.attribute(_T("id"), TUID(aia.TopicId()))
		.content(
			element(_T("title"))
			.content(STR((aia.isFunction() || aia.isMacro()) ? aia.FunctionText() : aia.Category()))
		)
	);
}

void
write_aml(const ArgsX& aia)
{
	try {
		if (aia.isDocument()) {
			LPCTSTR doc = aia.Documentation();
			if (0 == _tcsnicmp(_T("<developerConceptualDocument"), doc, 28)) {
				Topic dc(TUID(aia.TopicId()), Content().content(doc));
			}
			else {
				Topic dc(TUID(aia.TopicId()), Conceptual(_T("")).content(doc).relatedTopics(aia.Related()));
			}
		}
		else {
			// function argument table
			element table(_T("table"));
			for (unsigned short i = 1; i < aia.ArgCount(); ++i) {
				table.content(
					element(_T("row"))
					.content(element(_T("entry")).content(ArgType(aia.Arg(i).Type())))
					.content(element(_T("entry")).content(element(_T("legacyBold")).content(STR(aia.Arg(i).Name()))))
					.content(element(_T("entry")).content(STR(aia.ArgumentHelp(i))))
				);
			}

			OPERX syntax;
			if (aia.isFunction())
				syntax = ExcelX(xlfConcatenate, aia.FunctionText(), StrX(_T("(")), aia.ArgumentText(), StrX(_T(")")));
			else
				syntax = aia.FunctionText(); // macro
//			LPCTSTR doc = aia.Documentation();
			Topic dc(TUID(aia.TopicId()),
				Conceptual(
					STR(aia.FunctionHelp()))
				.section(_T("Syntax"),
					xll::to_string<XLOPERX>(syntax).c_str())
				.section(_T(""), table)
				/*
				.section(doc[0] == _T('<') 
					? _T("") // free form
					: _T("Remarks"), aia.Documentation())
				*/
				.section(_T("Remarks"), aia.Documentation())
				.relatedTopics(aia.Related())
			);
		}
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());
	}
}

// manifest.xml
void
write_manifest(const cat_fun_map& cf)
{
	File::Create manifest(_T("manifest.xml"), GENERIC_WRITE, CREATE_ALWAYS);

	manifest << XML_PRE;
	element topics(_T("topics"));
	for (cat_fun_citer i = cf.begin(); i != cf.end(); ++i) {
		topics.content(_T("\n"));
		for (fun_citer j = i->second.begin(); j != i->second.end(); ++j) {
			topics.content(
				element(_T("topic"))
				.attribute(_T("id"), TUID(j->second->TopicId()))
			);
		}
	}
	manifest << topics;
}

// table of contents
void
write_toc(const cat_fun_map& cf)
{
	File::Create toc(_T("toc.xml"), GENERIC_WRITE, CREATE_ALWAYS);

	toc << XML_PRE;

	element topics(_T("topics"));
	topics.content(_T("\n"));
	for (cat_fun_citer i = cf.begin(); i != cf.end(); ++i) {
		element topic;
		for (fun_citer j = i->second.begin(); j != i->second.end(); ++j) {
			if (j->second->isFunction() || j->second->isMacro()) {
				topic.content(
					element(_T("topic"))
					.attribute(_T("id"), TUID(j->second->TopicId()))
					.attribute(_T("file"), TUID(j->second->TopicId()))
				).nl();
			}
		}
		// GUID based on category name
		LPCTSTR tuid = TUID(utility::hash(PSTR(i->first), true));
		topics.content(
			element(_T("topic"))
			.attribute(_T("id"), tuid)
			.attribute(_T("file"), tuid)
//			.content(_T("\n"))
			.content(topic)
		);
	}

	toc << topics;
}

inline OPERX
idh_safename(const OPERX& o)
{
	OPERX x(o);

	for (int i = 1; i < x.val.str[0]; ++i)
		if (!isalnum(x.val.str[i]))
			x.val.str[i] = '_';

	return x;
}

// ALIAS file
void
write_alias(const cat_fun_map& cf)
{
	File::Create alias(_T("Alias.h"), GENERIC_WRITE, CREATE_ALWAYS);

	for (cat_fun_citer i = cf.begin(); i != cf.end(); ++i) {
		for (fun_citer j = i->second.begin(); j != i->second.end(); ++j) {
			if (j->second->isFunction() || j->second->isMacro()) {
				alias << _T("IDH_");
				OPERX xProc = idh_safename(j->second->FunctionText());
				// skip first '?' char
				alias.Write(xProc.val.str + 1, xProc.val.str[0]);
				alias << _T("=html\\");
				alias << TUID(j->second->TopicId());
				alias << _T(".htm\n");
			}
		}
	}
}

// MAP file
void
write_map(const cat_fun_map& cf)
{
	File::Create map(_T("Map.h"), GENERIC_WRITE, CREATE_ALWAYS);

	for (cat_fun_citer i = cf.begin(); i != cf.end(); ++i) {
		for (fun_citer j = i->second.begin(); j != i->second.end(); ++j) {
			if (j->second->isFunction() || j->second->isMacro()) {
				map << _T("#define IDH_");
				OPERX xProc = idh_safename(j->second->FunctionText());
				// skip first '?' char from C++ mangled name
				map.Write(xProc.val.str + 1, xProc.val.str[0]);
				map << _T(" ");
				TCHAR buf[32];
				_ultot_s(j->second->TopicId(), buf, 32, 10);
				map << buf;
				map << _T("\n");
			}
		}
	}
}

void
write_metadata(const cat_fun_map& cf)
{
	element md(_T("metadata"));
	md.attribute(_T("fileAssetGuid"), TUID(123));
	md.attribute(_T("assetTypeId"), _T("ContentMetadata"));

	for (cat_fun_citer i = cf.begin(); i != cf.end(); ++i) {
		for (fun_citer j = i->second.begin(); j != i->second.end(); ++j) {
			md.content(
				element(_T("topic"))
				.attribute(_T("id"), TUID(j->second->TopicId()))
				.attribute(_T("revisionNumber"), _T("1"))
				.attribute(_T("author"), _T(""))
			);
		}
	}

	CreateDirectory(_T("extractedfiles"), 0);
	File::Create meta(_T("extractedfiles\\contentmetadata.xml"), GENERIC_WRITE, CREATE_ALWAYS);
	meta << XML_PRE;
	meta << md;
}

bool
exists(LPCTSTR dir)
{
	return !CreateDirectory(dir, 0) && ERROR_ALREADY_EXISTS == GetLastError();
}

bool
dotdot(LPCTSTR dir)
{
	return (dir[0] == _T('.') && dir[1] == 0)
		|| (dir[0] == _T('.') && dir[1] == _T('.') && dir[2] == 0);
}

// rmdir dir /s /q
bool
rmdir_(LPCTSTR dir)
{
	WIN32_FIND_DATA fd;
	ZeroMemory(&fd, sizeof(fd));

	HANDLE h = FindFirstFile(std::basic_string<TCHAR>(dir).append(_T("\\*")).c_str(), &fd);
	if (INVALID_HANDLE_VALUE != h) {
		do {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (!dotdot(fd.cFileName))
					ENSURE (rmdir_(DIR(dir, fd.cFileName)));
			}
			else {
				ENSURE (DeleteFile(DIR(dir, fd.cFileName)));
			}
		} while (FindNextFile(h, &fd));

		FindClose(h);
	}

	ENSURE (RemoveDirectory(dir));

	return true;
}

// copy all file matching from into folder to
BOOL
copy(LPCTSTR from, LPCTSTR to, LPCTSTR spec = _T("*"))
{
	WIN32_FIND_DATA fd;
	ZeroMemory(&fd, sizeof(fd));

	HANDLE h = FindFirstFile(DIR(from, spec), &fd);
	if (INVALID_HANDLE_VALUE != h) {
		do {
			if (fd.cFileName[0] != _T('.') && !(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				ENSURE (CopyFile(DIR(from, fd.cFileName), DIR(to, fd.cFileName), FALSE));
			}
		} while (FindNextFile(h, &fd));

		FindClose(h);
	}
	else {
		return false;
	}

	return true;
}

BOOL
run(LPCTSTR cmd, LPCTSTR arg, LPCTSTR log = 0)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(pi));

	// CreateProcess wants to modify its second argument.
	TCHAR args[MAX_BUF];
	_tcsncpy_s(args, MAX_BUF, arg, _tcslen(arg));
	HANDLE hPipeOutputRead(0), hPipeOutputWrite(0);
	if (log) { // redirect to log file
		 SECURITY_ATTRIBUTES sa;
		 sa.nLength = sizeof(SECURITY_ATTRIBUTES);
		 sa.bInheritHandle = TRUE;
		 sa.lpSecurityDescriptor = 0;
		 
		 // Create the anonymous pipe and store read/write handles    HANDLE hPipeInputRead, hPipeOutputWrite;    
		 CreatePipe(&hPipeOutputRead, &hPipeOutputWrite, &sa, 0);
		 
		 si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
		 si.wShowWindow = SW_HIDE;
		 si.hStdError = hPipeOutputWrite;
		 si.hStdOutput = hPipeOutputWrite;
	}

// uncomment to pause console output
//	AllocConsole();
	BOOL b = CreateProcess(cmd, args, 0, 0, TRUE, CREATE_NO_WINDOW, 0, 0, &si, &pi); 
	if (b) 
		if (log) {
			File::Create file(log);
			DWORD dwRead, dwAvail;
			TCHAR chBuf[256];        
			BOOL pipe_data = true;        
			BOOL process_finished;        
			do {
				process_finished = (WaitForSingleObject (pi.hProcess, 0) == WAIT_OBJECT_0);
				PeekNamedPipe(hPipeOutputRead, chBuf, 255, &dwRead, &dwAvail,NULL);
				ZeroMemory(chBuf, sizeof(chBuf));
				if( dwAvail > 0 ) {
					pipe_data = ReadFile(hPipeOutputRead, chBuf, 255, &dwRead, NULL);
					file.Write(chBuf, dwRead);
				}        
			}        
			while( dwAvail > 0 || !process_finished );

			DWORD dwExitCode;
			ensure (GetExitCodeProcess(pi.hProcess, &dwExitCode));
			DWORD err;
			if (dwExitCode != EXIT_SUCCESS)
				if (0 != (err = GetLastError()))
					MessageBox(0, Error::Message(err).what(), _T("Error"), MB_OK);
				
			CloseHandle(pi.hProcess);        
			CloseHandle(pi.hThread);  
			CloseHandle(hPipeOutputRead);        
			CloseHandle(hPipeOutputWrite);
		}
		else {
			ENSURE (WAIT_FAILED != WaitForSingleObject(pi.hProcess, INFINITE));
		}

	// uncomment to pause console output
//	FreeConsole();

	return b;
}

void
build_conceptual(LPCTSTR module)
{
	// !!! lookup InstallDir in registry!!!
	std::basic_string<TCHAR> pfiles = Environment::Variable<TCHAR>(_T("ProgramFiles(x86)")); //!!!"ProgramFiles(x86)"
	std::basic_string<TCHAR> install = pfiles + _T("\\KALX\\xll");
	std::basic_string<TCHAR> dxroot = install + _T("\\Sandcastle");
	// use the line below for test project
//	std::basic_string<TCHAR> dxroot = Current::Directory<TCHAR>().Up(3).Cd(_T("\\Sandcastle"));
	Environment::Variable<TCHAR>(_T("DXROOT")) = dxroot.c_str();

	// equivalent of copyOutput.bat
	CreateDirectory(_T("output"), 0);
	CreateDirectory(_T("output\\html"), 0);
	CreateDirectory(_T("output\\icons"), 0);
	CreateDirectory(_T("output\\scripts"), 0);
	CreateDirectory(_T("output\\styles"), 0);
	CreateDirectory(_T("output\\media"), 0);
	copy(DIR(dxroot, _T("Presentation\\vs2005\\icons")), _T("output\\icons"));
	copy(DIR(dxroot, _T("Presentation\\vs2005\\scripts")), _T("output\\scripts"));
	copy(DIR(dxroot, _T("Presentation\\vs2005\\styles")), _T("output\\styles"));
	CreateDirectory(_T("Intellisense"), 0);

	// override files
	copy(DIR(install, _T("config")), _T("."), _T("*.xml"));

	ENSURE (run(DIR(dxroot, _T("ProductionTools\\BuildAssembler.exe")), 
		std::basic_string<TCHAR>(" /config:\"").append(install).append(_T("\\config\\conceptual.config\"  manifest.xml")).c_str(),
		_T("BuildAssembler.log")));

	CreateDirectory(_T("chm"), 0);
	CreateDirectory(_T("chm\\html"), 0);
	CreateDirectory(_T("chm\\icons"), 0);
	CreateDirectory(_T("chm\\scripts"), 0);
	CreateDirectory(_T("chm\\styles"), 0);

	copy(_T("output\\icons"), _T("chm\\icons"));
	copy(_T("output\\scripts"), _T("chm\\scripts"));
	copy(_T("output\\styles"), _T("chm\\styles"));

	std::basic_string<TCHAR> arg(_T(" /project:"));
	arg.append(module);
	arg.append(_T(" /html:output\\html /lcid:1033 /toc:Toc.xml /out:Chm"));
	ENSURE (run(DIR(dxroot, _T("ProductionTools\\ChmBuilder.exe")), arg.c_str(), 
		_T("ChmBuilder.log")));

	ENSURE (run(DIR(dxroot, _T("ProductionTools\\DBCSFix.exe")), _T(" /d:Chm /l:1033"))); 

	ENSURE (run(DIR(pfiles, _T("HTML Help Workshop\\hhc.exe")), 
		std::basic_string<TCHAR>(_T(" Chm\\")).append(module).append(_T(".hhp")).c_str(), 
		_T("HelpCompiler.log")));

	MoveFile(std::basic_string<TCHAR>(_T("Chm\\")).append(module).append(_T(".chm")).c_str(),
		     std::basic_string<TCHAR>(_T("..\\")).append(module).append(_T(".chm")).c_str());
}

#ifdef _DEBUG
// Generate the various files Sandcastle needs.
extern "C" int __declspec(dllexport) WINAPI
xll_make_doc(void)
{
	std::basic_string<TCHAR> cwd;

	try {
		cwd = Current::Directory<TCHAR>();

		// put files for C:\<path>\module.xll into C:\<path>\module\.
		std::basic_string<TCHAR> dir = to_string<XLOPERX>(ArgsX::GetName());
		std::basic_string<TCHAR> module = module_name(dir.c_str());

		// directory to use for docs
		dir.erase(dir.find_last_of(_T('.')));
		// remove previous output
		if (exists(dir.c_str()))
			rmdir_(dir.c_str());
		CreateDirectory(dir.c_str(), 0);
		SetCurrentDirectory(dir.c_str());
	
		CreateDirectory(_T("xmlcomp"), 0);
		CreateDirectory(_T("dduexml"), 0);

		// remove old help file
		DeleteFile((dir + _T(".chm")).c_str());

		// create the Sandcastle menagerie of files
		AddInX::addin_list& l(AddInX::List());

		// category -> function_text -> Args pointer
		cat_fun_map cf_map;
		for (AddInX::addin_iter i = l.begin(); i != l.end(); ++i) {
			if ((*i)->Args().TopicId()) {
				ensure ((*i)->Args().Category().xltype == xltypeStr);
				cf_map[(*i)->Args().Category()][(*i)->Args().Sort()] = &((*i)->Args());

				write_aml((*i)->Args());
				write_xmlcomp((*i)->Args()); // companion file
			}
		}

		ensure (cf_map.size() > 0);

		write_manifest(cf_map);
		write_toc(cf_map);
		write_alias(cf_map);
		write_map(cf_map);
		write_metadata(cf_map);

		build_conceptual(module.c_str());
	}
	catch (const std::exception& ex) {
		XLL_ERROR(ex.what());

		return 0;
	}

	Current::Directory<TCHAR>().Set(cwd.c_str());

	return 1;
}

#ifdef _M_X64
static Args mda("xll_make_doc", "XLL.DOC");
#else
static Args mda("_xll_make_doc@0", "XLL.DOC");
#endif
int
xai_make_doc(void)
{
	Register<XLOPER>(mda);

	return 1;
}
static Auto<Open> xao_make_doc(xai_make_doc);
int
xai_unmake_doc(void)
{
	Unregister<XLOPER>(mda);

	return 1;
}
static Auto<Remove> xao_unmake_doc(xai_unmake_doc);

#endif // _DEBUG