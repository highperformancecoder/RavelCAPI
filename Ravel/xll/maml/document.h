// document.h - Document an Excel add-in using Sandcastle.
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include <map>
#include <string>
#include <vector>
//#define WINDOWS_LEAN_AND_MEAN
//#include <windows.h>
//#include <tchar.h>
#include <direct.h>
#include "../utility/file.h"
#include "../utility/hash.h"
//#include "../addin.h"

//#define EXCEL12

// create Sandcastle documentation
#ifndef _LIB
#ifdef _DEBUG
#ifdef _M_X64 
#pragma comment(linker, "/include:xll_make_doc")
#else
#pragma comment(linker, "/include:_xll_make_doc@0")
#endif
#endif // _DEBUG
#endif // _LIB

namespace xml {

	// phony guid using topic id
	inline LPCTSTR 
	TUID(unsigned int tid)
	{
		static TCHAR guid[64];

		_stprintf_s(guid, sizeof(guid)/sizeof(TCHAR), _T("%04x%04x-0000-0000-0000-000000000000"), tid>>16, tid&0xFFFF);
		
		return guid;
	}

	// XML element <tag attribute*>content</tag>
	class element {
		std::basic_string<TCHAR> e_;
		bool min_;
	public:
		typedef std::basic_string<TCHAR> string;
		typedef std::basic_string<TCHAR>::iterator iterator;

		element(LPCTSTR tag = _T(""))
			: e_(tag), min_(true)
		{
			if (e_.size()) {
				e_.insert(0, (_T("<")));
				e_.append(_T(" />"));
			}
		}
		element(const element& e)
			: e_(e.e_)
		{
		}
		element& operator=(const element& e)
		{
			if (this != &e) {
				e_ = e.e_;
			}

			return *this;
		}
		virtual ~element()
		{
		}

		bool operator==(const std::basic_string<TCHAR>& s) const
		{
			return e_ == s;
		}

		string tag(void) const
		{
			string t;

			if (e_.length()) {
				size_t n;
				size_t n0 = e_.find(' ');
				size_t n1 = e_.find('>');
				if (n1 < n0)
					n = n1;
				else
					n = n0;
				ensure (n != string::npos);

				t = e_.substr(1, n - 1);
			}

			return t;
		}
		operator LPCTSTR() const
		{
			return e_.c_str();
		}
/*		operator std::basic_string<TCHAR>() const
		{
			return e_;
		}
*/		element& nl()
		{
			e_.append(_T("\n"));

			return *this;
		}
		element& attribute(LPCTSTR key, LPCTSTR value)
		{
			string attr(_T(" "));
			attr.append(key);
			attr.append(_T("=\""));
			attr.append(value);
			attr.append(_T("\""));

			if (min_) {
				size_t n = e_.find(_T(" /"));
				ensure (n != string::npos);
				e_.insert(n, attr);

			}
			else {
				size_t n = e_.find('>');
				ensure (n != string::npos);
				e_.insert(n, attr);
			}

			return *this;
		}
		element& content(LPCTSTR data)
		{
			if (e_.length() == 0) {
				e_ = data;
			}
			else if (min_ && e_.back() == '\n') {
				e_.append(data);
			}
			else if (min_) {
				size_t n = e_.find(_T(" /"));
				ensure (n != string::npos);
				e_.erase(n, 2);

				e_.append(data);
				e_.append(_T("</"));
				e_.append(tag());
				e_.append(_T(">\n"));

				min_ = false;
			}
			else {
				size_t n = min_ ? 0 : e_.find_last_of('<');
				ensure (n != string::npos)
				e_.insert(n, data);
			}

			return *this;
		}
/*
		element& content(const std::basic_string<TCHAR>& data)
		{
			return content(data.c_str());
		}
*/		static std::basic_string<TCHAR> escape(LPCTSTR s)
		{
			std::basic_string<TCHAR> sx;

			sx = replace(_T('&'), _T("\a"), s);
			sx = replace(_T('\''), _T("&apos;"), sx.c_str());
			sx = replace(_T('\"'), _T("&quot;"), sx.c_str());
			sx = replace(_T('<'), _T("&lt;"), sx.c_str());
			sx = replace(_T('>'), _T("&gt;"), sx.c_str());
			sx = replace(_T('\a'), _T("&amp;"), sx.c_str());

			return sx;
		}
	private:
		// replace c by with in s
		static std::basic_string<TCHAR>
		replace(TCHAR c, LPCTSTR with, LPCTSTR s)
		{
			std::basic_string<TCHAR> sx(s);

			size_t pos = 0;
			for (pos = sx.find(c, pos); pos != std::basic_string<TCHAR>::npos; pos = sx.find(c, pos))
				sx.replace(pos, 1, with);

			return sx;
		}
	};
 
	class externalLink : public element {
		std::basic_string<TCHAR> text_, uri_, alt_;
		bool self_;
	public:
		externalLink(LPCTSTR text, LPCTSTR uri, LPCTSTR alt = 0, bool self = false)
			: element(_T("externalLink"))
		{
			content(element(_T("linkText")).content(text));
			content(element(_T("linkUri")).content(element::escape(uri).c_str()));
			if (alt)
				content(element(_T("linkAlternateText")).content(alt));
			else
				content(element(_T("linkAlternateText")).content(text));
			if (self)
				content(element(_T("linkTarget")).content(_T("_self")));
		}
		~externalLink()
		{ }
	};

	// cross link using FunctionText
	struct xlink : public element {
		xlink(LPCTSTR text, bool minimized = false)
			: element(_T("link"))
		{
			attribute(_T("xlink:href"), TUID(utility::hash<TCHAR>(text, 0, true)));
			if (!minimized)
				content(text);
		}
		~xlink()
		{ }
	};

	// topic file type
	class Content : public element {
		std::basic_string<TCHAR> type_;
	public:
		Content(LPCTSTR type = _T(""))
			: type_(type)
		{ }
		LPCTSTR type(void) const
		{
			return type_.c_str();
		}

		Content& content(LPCTSTR e)
		{
			element::content(e);

			return *this;
		}
		virtual ~Content()
		{ }
	};

	// Also need ErrorMessage, Glossary, ...
	class Conceptual : public Content {
	public:
		Conceptual(LPCTSTR introduction, LPCTSTR summary = 0)
			: Content(_T("developerConceptualDocument"))
		{
			if (summary && *summary)
				content(element(_T("summary")).content(element(_T("para")).content(summary)));
			content(element(_T("introduction")).content(element(_T("para")).content(introduction)));
		}

		Conceptual& content(LPCTSTR e)
		{
			Content::content(e);

			return *this;
		}

		Conceptual& section(LPCTSTR title, LPCTSTR content_)
		{
			element e(_T("section"));
			if (title && *title)
				e.content(
					element(_T("title"))
					.content(title)
				);
			e.content(
				element(_T("content"))
				.content(
					element(_T("para"))
					.content(content_)
				)
			);
			
			content(e);

			return *this;
		}

		Conceptual& relatedTopics(LPCTSTR topic)
		{
			if (topic && *topic)
				content(element(_T("relatedTopics")).content(topic));

			return *this;
		}
	}; 

	class Topic {
		element doc_;
		std::basic_string<TCHAR> guid_;
		Content content_;
	public:
		Topic(LPCTSTR guid, const Content& content)
			: doc_(), guid_(guid), content_(content)
		{
			doc_.content(_T("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
			to_file(std::basic_string<TCHAR>(_T("dduexml\\")).append(guid).append(_T(".xml")).c_str());
		}
	
		operator LPCTSTR()
		{
			return to_string();
		}
		operator std::basic_string<TCHAR>()
		{
			return std::basic_string<TCHAR>(to_string());
		}
		LPCTSTR to_string()
		{
			if (*content_.type()) {
				doc_.content(
					element(_T("topic"))
					.attribute(_T("id"), guid_.c_str())
					.attribute(_T("revisionNumber"), _T("1"))
					.content(
						element(content_.type())
						.attribute(_T("xmlns"), 
							_T("http://ddue.schemas.microsoft.com/authoring/2003/5"))
						.attribute(_T("xmlns:xlink"), 
							_T("http://www.w3.org/1999/xlink"))
						.content(content_)
					)
				);
			}
			else {
				doc_.content(
					element(_T("topic"))
					.attribute(_T("id"), guid_.c_str())
					.attribute(_T("revisionNumber"), _T("1"))
					.content(content_)
				);
			}

			return doc_;
		}
	private:
		void to_file(LPCTSTR file)
		{
			File::Create ofs(file, GENERIC_WRITE, CREATE_ALWAYS);
			const std::basic_string<TCHAR>& str = to_string();
			ofs.Write(str.c_str(), static_cast<DWORD>(str.length()));
		}
	};

	// return file as string
	inline LPCTSTR File(LPCTSTR file)
	{
		static std::basic_string<TCHAR> str;
		
		try {
			File::Create ifs(file, GENERIC_READ, OPEN_EXISTING);
			ifs.Read(str);
		}
		catch (...) {
			str = _T("File not found: ");
			str.append(file);

			MessageBox(0, str.c_str(), _T("Error"), MB_OK);
		}

		return str.c_str();
	}

	// common tags
#define TAG_HASH(x) #x
#define TAG_STRZ(x) TAG_HASH(x)
#define TAG(tag) 	struct tag : public element { tag(LPCTSTR str = 0) : element(_T(TAG_STRZ(tag))) { if (str) content(str); } };
#define TAG2(alias, tag) 	struct alias : public element { alias(LPCTSTR str = 0) : element(_T(TAG_STRZ(tag))) { if (str) content(str); } };

	// block
	TAG(code)
	TAG(para)
	TAG(quote)

	// inline
	TAG(application)
	TAG(codeInline)
	TAG(command)
	TAG(legacyBold)
	TAG2(bold, legacyBold)
	TAG(legacyItalic)
	TAG2(italic, legacyItalic)
	TAG2(var, legacyItalic)
	TAG(legacyUnderline)
	TAG2(underline, legacyUnderline)
	TAG(literal)
	TAG(math)
	TAG(markup)
	TAG(newTerm)
	TAG(quoteInline)
	TAG(subscript)
	TAG2(sub, subscript)
	TAG(superscript)
	TAG2(sup, superscript)
	TAG(userInput)

	// special tags
	struct alert : public element {
		alert() : element(_T("alert"))
		{ }
		alert& note() { attribute(_T("class"), _T("note")); return *this; }
		alert& caution() { attribute(_T("class"), _T("caution")); return *this; }
		alert& security() { attribute(_T("class"), _T("security")); return *this; }
		alert& cs() { attribute(_T("class"), _T("cs")); return *this; }
	};

	struct definitionTable : public element {
		definitionTable() : element(_T("definitionTable")) 
		{ }
		definitionTable& term(LPCTSTR key, LPCTSTR val)
		{
			content(element(_T("definedTerm")).content(key));
			content(element(_T("definition")).content(para(val)));

			return *this;
		}
	};

	struct list : public element {
		list() : element(_T("list")) 
		{ }
		list& bullet() { attribute(_T("class"), _T("bullet")); return *this; }
		list& ordered() { attribute(_T("class"), _T("ordered")); return *this; }
		list& nobullet() { attribute(_T("class"), _T("nobullet")); return *this; }

		list& item(LPCTSTR str)
		{
			content(element(_T("listItem")).content(para(str)));

			return *this;
		}
	};

	struct tableRow : public element {
		tableRow() : element(_T("row"))
		{ }
		tableRow& entry(LPCTSTR str = 0)
		{
			if (str)
				element(para(str));
			else
				element(para(_T("&#160;")));

			return *this;
		}
	};

	struct tableHeader : public element {
		tableHeader() : element(_T("tableHeader"))
		{ }
		tableHeader& entry(LPCTSTR str = 0)
		{
			if (str)
				element(para(str));
			else
				element(para(_T("&#160;")));

			return *this;
		}

	};

	//!!!Not working!!!
	// table("title")
	// .header().entry("Foo").entry("Bar")
	// .row().entry("foo").entry("bar")
	// .row(...)
	struct table : public element {
		table(LPCTSTR title = 0) : element(_T("table"))
		{ 
			if (title)
				content(element(_T("title")).content(title));
		}
	};

	// slip in entities for MAML from Excel help
	inline std::basic_string<TCHAR> encode(LPCTSTR s)
	{
		std::basic_string<TCHAR> t(s);

		// < -> &lt;
		// <= -> ENT_le
		// ...

		return t;
	}
	inline std::basic_string<TCHAR> markdown(LPCTSTR s)
	{
		std::basic_string<TCHAR> t(s);

		return t;
	}
} // namespace xml