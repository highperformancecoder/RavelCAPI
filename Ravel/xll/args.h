// args.h - Excel add-in arguments.
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include <string>
#include <vector>
#include "utility/hash.h"
#include "oper.h"

namespace xll {

	// ???Use 'X' for handle and change when registering???
	// Friendly name from argument type.
	template<class X>
	inline typename xll::traits<X>::xcstr ArgType(const XOPER<X>& type)
	{
		ensure (type.xltype == xltypeStr);
		ensure (type.val.str[0] > 0);

		switch (type.val.str[1]) {
			case _T('A') : return _T("Boolean");
			case _T('B') : {
				if (type.val.str[0] > 1) {
					if (type.val.str[2] == XLL_DATE[1]/*'1'*/)
						return _T("Date");
					if (type.val.str[2] == XLL_HANDLE[1]/*'2'*/)
						return _T("Handle");
				}
				return _T("Number");
			}
			case _T('C') : return _T("String");
			case _T('D') : return _T("String");
			case _T('E') : return _T("Number");
			case _T('F') : return _T("String");
			case _T('G') : return _T("String");
			case _T('H') : return _T("Integer");
			case _T('I') : return _T("Short");
			case _T('J') : return _T("Long");
			case _T('K') : return _T("Array");
			case _T('L') : return _T("Boolean");
			case _T('M') : return _T("Short");
			case _T('N') : return _T("Long");
			case _T('P') : return _T("Range");
			case _T('R') : return _T("Reference");
			// not used!!!
			case _T('!') : return _T("Volatile");
			case _T('#') : return _T("Uncalced");
			case _T('$') : return _T("ThreadSafe");
			case _T('&') : return _T("ClusterSafe");
		}

		return _T("???Unknown???");
	}

	// Single argument to Excel add-in.
	template<class X>
	class XArg {
		XOPER<X> type_, name_, help_, default_;
	public:
		typedef typename xll::traits<X>::xcstr xcstr;

		XArg(xcstr type, xcstr name, xcstr help)
			: type_(type), name_(name), help_(help)
		{ }
		XArg(xcstr type, xcstr name, xcstr help, const XOPER<X>& d)
			: type_(type), name_(name), help_(help), default_(d)
		{ }
		const XOPER<X>& Type(void) const
		{
			return type_;
		}
		const XOPER<X>& Name(void) const
		{
			return name_;
		}
		const XOPER<X>& Help(void) const
		{
			return help_;
		}
		const XOPER<X>& Default(void) const
		{
			return default_;
		}
		bool isDate(void) const {
			return type_.val.str[0] > 1 && type_.val.str[1] == 'B' && type_.val.str[2] == XLL_DATE[1]/*'1'*/;
		}
		bool isHandle(void) const {
			return type_.val.str[0] > 1 && type_.val.str[1] == 'B' && type_.val.str[2] == XLL_HANDLE[1]/*'2'*/;
		}
	};

	typedef XArg<XLOPER>   Arg;
	typedef XArg<XLOPER12> Arg12;
	typedef X_(Arg)        ArgX;

	// Build up arguments for call to xlfRegister.
	template<class X>
	class XArgs {
		typedef typename xll::traits<X>::xcstr xcstr;
		typedef typename xll::traits<X>::xchar xchar;
		typedef typename xll::traits<X>::xword xword;
		typedef typename xll::traits<X>::xstring xstring;

		std::vector<XArg<X>> args_; // arg[0] is return value
		mutable XOPER<X> arg_; // to be passed to xlfRegister
		unsigned int tid_; // help file topic id
		std::vector<XOPER<X> > alias_;
		std::basic_string<typename traits<X>::xchar> doc_, related_;
		XOPER<X> sort_; // sort key for documentation
		bool auth_;

		// append to arg_[2]
		void append(xcstr type)
		{
			xstring remove;
			xstring arg2(type);

			remove.resize(2);
			remove[0] = XLL_DATE[1]; remove[1] = XLL_HANDLE[1];
			// remove decorators
			xstring::size_type i = 0;
			while ((i = arg2.find_first_of(remove)) != xstring::npos)
				arg2.erase(i, 1);

			arg_[2].append(arg2.c_str());
		}
	public:
		XArgs()
			: tid_(0)
		{
		}
		// documentation
		XArgs(xcstr category)
			: arg_(1, 7), tid_(utility::hash<xchar>(category, 0, true))
		{
			arg_[5] = -1;
			arg_[6] = category;
		}
		// macro: C function and Excel name
		XArgs(xcstr procedure, xcstr function_text)
			: arg_(1, 10), tid_(0)
		{
			arg_[1] = procedure;
			arg_[3] = function_text;
			arg_[5] = 2; // macro
		}
		// user defined function: C function, C signature, Excel name and argument prompt
		XArgs(xcstr procedure, xcstr type_text, xcstr function_text, xcstr argument_text,
			xcstr category = traits<X>::null(), xcstr function_help = traits<X>::null())
				: arg_(1, 10), tid_(0)
		{
			arg_[1] = procedure;
			append(type_text);
			arg_[3] = function_text;
			arg_[4] = argument_text;
			arg_[5] = 1; // function
			if (category)
				arg_[6] = category;
			if (function_help)
				arg_[9] = function_help;

			args_.push_back(XArg<X>(type_text, function_text, function_help));
		}
		// return type, C name, Excel name
		XArgs(xcstr type_text, xcstr procedure, xcstr function_text)
			: arg_(1, 10), tid_(0)
		{
			arg_[1] = procedure;
			append(type_text);
			arg_[3] = function_text;
			arg_[4] = traits<X>::null();
			arg_[5] = 1; // function

			args_.push_back(XArg<X>(type_text, function_text, traits<X>::null()));
		}
		/*
		XArgs(const XArgs& arg)
			: args_(arg.args_), arg_(arg.arg_), doc_(arg.doc_), tid_(arg.tid_)
		{
		}
		XArgs& operator=(const XArgs& arg)
		{
			if (this != &arg) {
				args_ = arg.args_;
				arg_ = arg.arg_;
				doc_ = arg.doc_;
				tid_ = arg.tid_;
			}
		
			return *this;
		}
		~XArgs()
		{
		}
		*/
		bool isHidden(void) const
		{
			return arg_[5] == 0;
		}
	
		bool isFunction(void) const
		{
			return arg_[5] == 1;
		}
		bool isMacro(void) const
		{
			return arg_[5] == 2;
		}
		bool isDocument(void) const
		{
			return arg_[5] == -1;
		}
		bool isDocumented(void) const
		{
			return doc_.size() != 0;
		}
		unsigned int TopicId(void) const
		{
			return tid_;
		}

		// Tack on an argument
		XArgs& Arg(xcstr type, xcstr name, xcstr help)
		{
			append(type);

			if (ArgumentText())
				arg_[4].append(_T(", "));
			arg_[4].append(name);
			
			ArgumentHelp(help);

			args_.push_back(XArg<X>(type, name, help));

			return *this;
		}
		// add a default value
		template<class T>
		XArgs& Arg(xcstr type, xcstr name, xcstr help, const T& t)
		{
			append(type);

			if (ArgumentText())
				arg_[4].append(_T(", "));
			arg_[4].append(name);
			
			ArgumentHelp(help);

			args_.push_back(XArg<X>(type, name, help, XOPER<X>(t)));

			return *this;
		}
		XArgs& Volatile(void)
		{
			xchar c = XLL_VOLATILEX[0];
			arg_[2].append(&c,1);

			return *this;
		}
		XArgs& Uncalced(void)
		{
			xchar c = XLL_UNCALCEDX[0];
			arg_[2].append(&c,1);

			return *this;
		}
		XArgs& ThreadSafe(void)
		{
			if (typeid(X) == typeid(XLOPER12)) {
				xchar c = XLL_THREAD_SAFEX[0];
				arg_[2].append(&c, 1);
			}

			return *this;
		}
		XArgs& ClusterSafe(void)
		{
			if (typeid(X) == typeid(XLOPER12)) {
				xchar c = XLL_CLUSTER_SAFE[0];
				arg_[2].append(&c,1);
			}

			return *this;
		}
		XArgs& Asynchronous(void)
		{
			if (typeid(X) == typeid(XLOPER12)) {
				xchar c = XLL_ASYNCHRONOUS[0];
				arg_[2].append(&c,1);
			}

			return *this;
		}
		XArgs& Hide(void)
		{
			arg_[5] = 0;
		}
		xword ArgCount(void) const
		{
			return static_cast<xll::traits<X>::xword>(args_.size());
		}
		// 1-based individual argument
		const XArg<X>& Arg(xword i) const
		{
			ensure (i < ArgCount());

			return args_[i];
		}
		//
		// Accessors
		//

		// C function name
		const XOPER<X>& Procedure() const
		{
			return arg_[1];
		}
		XArgs& Procedure(xcstr procedure)
		{
			arg_[1] = procedure;

			return *this;
		}
		XArgs& Procedure(const X& procedure)
		{
			arg_[1] = procedure;

			return *this;
		}

		// C signature
		const XOPER<X>& TypeText() const
		{
			return arg_[2];
		}
		XArgs& TypeText(xcstr type_text)
		{
			append(type_text);

			return *this;
		
		}
		XArgs& TypeText(const X& type_text)
		{
			append(type_text);

			return *this;
		
		}

		// Excel function name
		const XOPER<X>& FunctionText() const
		{
			return arg_[3];
		}
		XArgs& FunctionText(xcstr function_text)
		{
			arg_[3] = function_text;

			return *this;
		}
		XArgs& FunctionText(const X& function_text)
		{
			arg_[3] = function_text;

			return *this;
		}

		// Excel Control-Shift-A argument prompt
		const XOPER<X>& ArgumentText() const
		{
			return arg_[4];
		}
		XArgs& ArgumentText(xcstr argument_text)
		{
			arg_[4] = argument_text;

			return *this;
		}
		XArgs& ArgumentText(const X& argument_text)
		{
			arg_[4] = argument_text;

			return *this;
		}

		// 0 - hidden function, 1 - function, 2 - macro
		const XOPER<X>& MacroType() const
		{
			return arg_[5];
		}
		XArgs& MacroType(int macro_type)
		{
			arg_[5] = macro_type;

			return *this;
		}
		XArgs& MacroType(xcstr macro_type)
		{
			arg_[5] = macro_type;

			return *this;
		}
		XArgs& MacroType(const X& macro_type)
		{
			arg_[5] = macro_type;

			return *this;
		}

		// Function Wizard category
		const XOPER<X>& Category() const
		{
			return arg_[6];
		}
		XArgs& Category(xcstr category)
		{
			arg_[6] = category;

			return *this;
		}

		// CONTROL+SHIFT+shorcut_text single character macro shortcut
		const XOPER<X>& ShortcutText() const
		{
			return arg_[7];
		}
		XArgs& ShortcutText(xcstr shortcut_text)
		{
			arg_[7] = shortcut_text;

			return *this;
		}
		XArgs& ShortcutText(const X& shortcut_text)
		{
			arg_[7] = shortcut_text;

			return *this;
		}

		// path\file.chm!help_context_id
		const XOPER<X>& HelpTopic() const
		{
			return arg_[8];
		}
		XArgs& HelpTopic(xcstr help_topic)
		{
			arg_[8] = help_topic;

			return *this;
		}
		XArgs& HelpTopic(const X& help_topic)
		{
			arg_[8] = help_topic;

			return *this;
		}

		// Function description in Function Wizard
		const XOPER<X>& FunctionHelp() const
		{
			return arg_[9];
		}
		XArgs& FunctionHelp(xcstr function_help)
		{
			arg_[9] = function_help;

			return *this;
		}
		XArgs& FunctionHelp(const X& function_help)
		{
			arg_[9] = function_help;

			return *this;
		}

		// Individual help in the Function Wizard.

		const XOPER<X>& ArgumentHelp(xword i) const
		{
			ensure (i > 0);
			ensure (9 + i < arg_.size());

			return arg_[9 + i];
		}
		XArgs& ArgumentHelp(xcstr argument_help, xword i = 0)
		{
			if (i) {
				if (10 + i >= arg_.size()) {
					arg_.resize(1, 11 + i);
				}
				arg_[10 + i] = argument_help;
			}
			else {
				arg_.push_back(XOPER<X>(argument_help));
			}

			return *this;
		}
		XArgs& ArgumentHelp(const X& argument_help, xword i = 0)
		{
			if (i) {
				if (10 + i >= arg_.size()) {
					arg_.resize(1, 11 + i);
				}
				arg_[10 + i] = argument_help;
			}
			else {
				arg_.push_back(argument_help);
			}

			return *this;
		}

		// Additional documentation for help file.
		xcstr Documentation(void) const
		{
			return doc_.c_str();
		}
		xcstr Related(void) const
		{
			return related_.c_str();
		}
		// Only generate documentation if this is called.
		XArgs& Documentation(xcstr doc = 0, xcstr see_also = 0)
		{
#ifdef _DEBUG
			if (doc)
				doc_ = doc;
			if (see_also)
				related_ = see_also;
#else // C4100
			doc = doc;
			see_also = see_also; 
#endif
			if (!tid_) {
				traits<X>::xcstr ft = FunctionText().val.str;
				tid_ = utility::hash(ft + 1, ft[0], true);
			}
			
			return *this;
		}
		XArgs& Alias(xcstr alias)
		{
			alias_.push_back(XOPER<X>(alias));

			return *this;
		}
		const std::vector<XOPER<X>>& Alias(void) const
		{
			return alias_;
		}
		// create registry entry and reg file controlling access
		XArgs& Authorize(void)
		{
			auth_ = true;
//			LPCSTR module;
//			module = XAddIn<X>::GetName();
			// weak encription
//			unsigned int value = hash(

			return *this;
		}

		/*
		XArgs& Sort(xcstr sort)
		{
			sort_ = sort;

			return *this;
		}
		*/
		XArgs& Sort(const XOPER<X>& key1, const XOPER<X>& key2 = XOPER<X>())
		{
			sort_.resize(1, 2);
			
			sort_[0] = key1;
			sort_[1] = key2;

			return *this;
		}
		const XOPER<X>& Sort(void) const
		{
			return sort_.size() > 1 ? sort_ : FunctionText();
		}

		// arguments xlfRegister needs
		int size() const
		{
			return static_cast<int>(arg_.size());
		}
		typename const traits<X>::pointer_type* pointers() const
		{
			ensure (size() < limits<X>::maxargs);
			static traits<X>::pointer_type p[limits<X>::maxargs]; // It's not as bad as it looks.
			
			p[0] = &GetName(); // have to wait for xlAutoOpen to be called
			if (tid_) {
				XOPER<X> topic(GetName());
				// replace xll by chm
				XOPER<X> search = Excel<X>(xlfSearch, traits<X>::dotll(), topic);
				ensure (search.xltype == xltypeNum);
				topic = Excel<X>(xlfConcatenate, Excel<X>(xlfLeft, topic, search), traits<X>::chmbang(), XOPER<X>(tid_));
				arg_[8] = topic;
			}
			for (int i = 1; i < size(); ++i)
				p[i] = arg_.val.array.lparray + i;

			return p;
		}
		static X& GetName(void) {
			static LXOPER<X> o_(Excel<X>(xlGetName));

			return o_;
		}
	private:
		bool CheckArgs()
		{
			int arity = arg_size() - 10;
			// parse type_text!!!
		}
	};

	typedef XArgs<XLOPER>   Args;
	typedef XArgs<XLOPER12> Args12;
	typedef X_(Args)        ArgsX;

	typedef XArgs<XLOPER>   Function;
	typedef XArgs<XLOPER12> Function12;
	typedef X_(Args)        FunctionX;

	typedef XArgs<XLOPER>   Macro;
	typedef XArgs<XLOPER12> Macro12;
	typedef X_(Args)        MacroX;

	typedef XArgs<XLOPER>   Document;
	typedef XArgs<XLOPER12> Document12;
	typedef X_(Args)        DocumentX;

} // namespace xll