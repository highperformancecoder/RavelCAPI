// uuid.h - Guids and Uuids using RPC interface.
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#define WINDOWS_LEAN_AND_MEAN
#include <rpc.h>
#include <cassert>
//#include <string>
#include "xll/utility/ensure.h"
#pragma comment(lib, "rpcrt4.lib")

#ifdef UNICODE
typedef RPC_WSTR RPC_TSTR;
#else
typedef RPC_CSTR  RPC_TSTR;
#endif

struct Uuid : public UUID {
	Uuid()
	{
		ensure (RPC_S_OK == UuidCreate(this));
	}
	Uuid(const RPC_TSTR uuid)
	{
		ensure (RPC_S_OK == UuidFromString(uuid, this));
	}
	~Uuid()
	{ }
	class String {
		RPC_TSTR str_;
		String(const String&);
		String& operator=(const String&);
	public:
		String(const Uuid& uid)
		{
			ensure (RPC_S_OK == UuidToString((UUID*)&uid, &str_));
		}
		~String()
		{
			ensure (RPC_S_OK == RpcStringFree(&str_));
		}
		operator const LPCTSTR() const
		{
			return reinterpret_cast<LPCTSTR>(str_);
		}
	};
};