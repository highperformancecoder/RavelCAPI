// xml.h - Generate XML strings parameterized on char type.
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
/*
document:  element+
element:   '<' name '/>' | stag content etag
stag:      '<' name attribute* '>'
etag:      '</' name '>'
attribute: name '=' value
value:     '"' string '"'
content:   string ( element | string )*
*/
#pragma once
#include <string>

#define XML_ELEMENT(T,N,A,C)  nxml::element<T>(nxml::tag<T>(_T(N))A, C)
#define XML_ATTRIBUTE(T,K,V)  [nxml::attribute<T>(_T(K), _T(V))]
#define XML_CONTENT(T,C)      nxml::content<T>(_T(C))
#define XML_DOCUMENT(T, B, E) nxml::document<T>(B,nxml::elements<T>(E))

namespace nxml {

	// Generate strings using operator,()
	template<class T>
	class string : public std::basic_string<T> {
	public:
		string()
			: std::basic_string<T>()
		{ }
		string(const T* t)
			: std::basic_string<T>(t)
		{ }
		string(const std::basic_string<T>& s)
			: std::basic_string<T>(s)
		{ }
		operator const T*()
		{
			return c_str();
		}
	};

	template<class T>
	struct attribute : public string<T> {
		attribute(const T* k, const T* v)
		{
			push_back(' ');
			append(k);
			push_back('=');
			push_back('\"');
			append(v);
			push_back('\"');
		}
	};

	// really stag
	template<class T>
	struct tag : public string<T> {
		const T* name;
		tag(const T* t)
			: name(t)
		{
			push_back('<');
			append(t);
		}
		tag& operator[](const attribute<T>& a)
		{
			append(a);

			return *this;
		}
	};
	
	template<class T> struct element;

	template<class T> 
	struct content : public string<T> {
		content(const T* t = 0)
		{
			if (t)
				append(t);
		}
		content& string(const T*)
		{
			if (t)
				append(t);

			return *this;
		}
		content& element(const element<T>& e)
		{
			append(e);

			return *this;
		}
		content& element(const tag<T>& s, const content<T>& c = content<T>())
		{
			return element(nxml::element<T>(s, c));
		}
	};

	template<class T>
	struct etag : public string<T> {
		etag(const T* name)
		{
			push_back('<');
			push_back('/');
			append(name);
			push_back('>');
		}
	};

	template<class T>
	struct element : public string<T> {
		element(const tag<T>& s, const content<T>& c = content<T>())
		{
			append(s);
			if (c.size()) {
				push_back('>');
				append(c);
				append(etag<T>(s.name));
			}
			else { // empty tag
				push_back('/');
				push_back('>');
			}
		}
	};

	template<class T>
	struct prolog : public string<T> {
		prolog()
		{
			push_back('<');
			push_back('?');
			push_back('x');
			push_back('m');
			push_back('l');
			push_back(' ');
			push_back('v');
			push_back('e');
			push_back('r');
			push_back('s');
			push_back('i');
			push_back('o');
			push_back('n');
			push_back('=');
			push_back('\"');
			push_back('1');
			push_back('.');
			push_back('1');
			push_back('"');
			push_back('?');
			push_back('>');
		}
	};

	// one or more elements
	template<class T>
	struct elements : public string<T> {
		elements(const element<T>& e)
		{
			append(e);
		}
		elements(const tag<T>& s, const content<T>& c = content<T>())
		{
			append(nxml::element<T>(s, c));
		}
		elements& element(const tag<T>& s, const content<T>& c = content<T>())
		{
			append(nxml::element<T>(s, c));

			return *this;
		}
	};

	template<class T>
	struct document : public string<T> {
		document(bool prolog, const elements<T>& e)
		{
			if (prolog)
				append(nxml::prolog<T>());
			append(e);
		}
	};
	

} // namespace xml


