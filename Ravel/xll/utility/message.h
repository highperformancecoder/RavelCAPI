// message.h - error routine wrappers
// Copyright (c) 2010 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

namespace Error {

	// usage: Error::Message(GetLastError()).what()
	class Message {
		LPTSTR msg_;
		Message(const Message&);
		Message& operator=(const Message&);
	public:
		Message(DWORD dwMessageId)
			: msg_(NULL)
		{
			FormatMessage( 
			   // use system message tables to retrieve error text 
			   FORMAT_MESSAGE_FROM_SYSTEM 
			   // allocate buffer on local heap for error text 
			   |FORMAT_MESSAGE_ALLOCATE_BUFFER 
			   // Important! will fail otherwise, since we're not  
			   // (and CANNOT) pass insertion parameters 
			   |FORMAT_MESSAGE_IGNORE_INSERTS,   
			   NULL,    // unused with FORMAT_MESSAGE_FROM_SYSTEM 
			   dwMessageId, 
			   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
			   (LPTSTR)&msg_,  // output  
			   0, // minimum size for output buffer 
			   NULL);   // arguments - see note  
		}
		~Message()
		{
			LocalFree(msg_);
		}
		LPCTSTR what() const
		{
			return msg_;
		}
	};
} // namespace Error
