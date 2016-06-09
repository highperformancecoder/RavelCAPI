// env.h -  Windows environment variables
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include <cassert>
#include <string>
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include "ensure.h"

namespace Environment {
	template<class T>
	class Variable {
		std::basic_string<T> name_, value_;
	public:
		Variable(const T* name)
			: name_(name)
		{
			value_.resize(1024);
			while (GetEnvironmentVariable(name, &value_[0], static_cast<DWORD>(value_.size())) > static_cast<DWORD>(value_.length()))
				value_.resize(value_.size() + 1024);
		}
		operator const T*() const
		{
			return value_.c_str();
		}
		Variable& operator=(const T* value)
		{
			ensure (SetEnvironmentVariable(name_.c_str(), value));

			return *this;
		}
	};
} // namespace Environment

namespace Current {

	template<class T>
	class Directory {
		std::basic_string<T> cwd_;
	public:
		Directory()
		{
			DWORD size;
			ensure (0 < (size = GetCurrentDirectory(0, 0)));
			cwd_.resize(size);
			ensure (size - 1 == GetCurrentDirectory(size, &cwd_[0]));
		}
		operator const T*() const
		{
			return cwd_.c_str();
		}
		const T* Get() const
		{
			return cwd_.c_str();
		}
		Directory& Set(LPCTSTR cwd = 0)
		{
			ensure (cwd ? SetCurrentDirectory(cwd) : SetCurrentDirectory(cwd_.c_str()));

			return *this;
		}
		// move up n directories, but do not set
		Directory& Up(DWORD n)
		{
			while (n--) {
				size_t i;
				ensure (std::basic_string<T>::npos != (i = cwd_.find_last_of(_T('\\'))));
				cwd_.erase(i);
			}

			return *this;
		}
		// append to cwd, but do not set
		Directory Cd(LPCTSTR dir)
		{
			if (dir && *dir) {
				if (cwd_.back() == _T('\\')) {
					while (*dir++ == _T('\\'))
						; // skip initial backslashes
					cwd_.append(dir);
				}
				else {
					if (*dir != _T('\\'))
						cwd_.push_back(_T('\\'));
					cwd_.append(dir);
				}
			}

			return *this;
		}
	};
} // namespace Environment

