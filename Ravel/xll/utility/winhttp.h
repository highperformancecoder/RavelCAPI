// winhttp.h - WinHttpXXX functions
// Copyright (c) 2011 KALX, LLC. All rights reserved. No warranty is made.
#pragma once
#include <string>
#include <Windows.h>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

namespace WinHttp {

	class Open {
		HINTERNET h_;
		Open(const Open&);
		Open& operator=(const Open&);
	public:
		Open(
			LPCWSTR pwszUserAgent = L"",
			DWORD dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY,
			LPCWSTR pwszProxyName = WINHTTP_NO_PROXY_NAME,
			LPCWSTR pwszProxyBypass = WINHTTP_NO_PROXY_BYPASS,
			DWORD dwFlags = 0 // or WINHTTP_FLAG_ASYNC
			)
		{
			h_ = WinHttpOpen(pwszUserAgent, dwAccessType, pwszProxyName, pwszProxyBypass, dwFlags);
		}
		~Open()
		{
			WinHttpCloseHandle(h_);
		}
		operator HINTERNET()
		{
			return h_;
		}

	};

	// specify the server
	class Connect {
		HINTERNET h_;
		Connect(const Connect&);
		Connect& operator=(const Connect&);
	public:
		Connect(
			HINTERNET hSession, 
			LPCWSTR pswzServerName,
			INTERNET_PORT nServerPort = INTERNET_DEFAULT_PORT,
			DWORD dwReserved = 0
			)
		{
			h_ = WinHttpConnect(hSession, pswzServerName, nServerPort, dwReserved);
		}
		~Connect()
		{
			WinHttpCloseHandle(h_);
		}
		operator HINTERNET()
		{
			return h_;
		}
	};

	// specify the request
	class OpenRequest {
		HINTERNET h_;
		OpenRequest(const OpenRequest&);
		OpenRequest& operator=(const OpenRequest&);
	public:
		OpenRequest(
			HINTERNET hConnect,
			LPCWSTR pwszVerb = NULL, // L"GET"
			LPCWSTR pwszObjectName = NULL,
			LPCWSTR pwszVersion = NULL, // HTTP/1.1
			LPCWSTR pwszReferrer = WINHTTP_NO_REFERER,
			LPCWSTR *ppwszAcceptTypes = WINHTTP_DEFAULT_ACCEPT_TYPES,
			DWORD dwFlags = 0
			)
		{
			h_ = WinHttpOpenRequest(hConnect, pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, ppwszAcceptTypes, dwFlags);
		}
		~OpenRequest()
		{
			WinHttpCloseHandle(h_);
		}

		operator HINTERNET()
		{
			return h_;
		}

		BOOL SendRequest(
			LPCWSTR pwszHeaders = WINHTTP_NO_ADDITIONAL_HEADERS,
			DWORD dwHeadersLength = 0,
			LPVOID lpOptional = WINHTTP_NO_REQUEST_DATA,
			DWORD dwOptionalLength = 0,
			DWORD dwTotalLength = 0,
			DWORD_PTR dwContext = 0
			)
		{
			return WinHttpSendRequest(h_, pwszHeaders, dwHeadersLength, lpOptional, dwOptionalLength, dwTotalLength, dwContext);
		}

		// must be called before ReadData or QueryHeaders
		BOOL ReceiveResponse(LPVOID lpReserved = 0)
		{
			return WinHttpReceiveResponse(h_, lpReserved);
		}

		DWORD QueryStatus(void)
		{
			DWORD status, dw = sizeof(DWORD);

			QueryHeaders(WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER, 0, &status, &dw);

			return status;
		}
	
		BOOL QueryHeaders(
			DWORD dwInfoLevel,
			LPCWSTR pwszName,
			LPVOID lpBuffer,
			LPDWORD lpdwBufferLength,
			LPDWORD lpdwIndex = WINHTTP_NO_HEADER_INDEX
			)
		{
			return WinHttpQueryHeaders(h_, dwInfoLevel, pwszName, lpBuffer, lpdwBufferLength, lpdwIndex);
		}
		std::basic_string<WCHAR> QueryHeaders(void)
		{
			DWORD dw;
			std::basic_string<WCHAR> header;

			QueryHeaders(WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, 0, &dw);
			header.resize(++dw/2);
			QueryHeaders(WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, &header[0], &dw);

			return header;
		}

		BOOL QueryDataAvailable(LPDWORD lpdwNumberOfBytesAvailable)
		{
			return WinHttpQueryDataAvailable(h_, lpdwNumberOfBytesAvailable);
		}

		BOOL ReadData(
			LPVOID lpBuffer,
			DWORD dwNumberOfBytesToRead,
			LPDWORD lpdwNumberOfBytesRead
			)
		{
			return WinHttpReadData(h_, lpBuffer, dwNumberOfBytesToRead, lpdwNumberOfBytesRead);
		}
		std::string ReadData(void)
		{
			DWORD dw;
			std::string data;
			
			while (QueryDataAvailable(&dw) && dw) {
				data.resize(data.size() + dw);
				ReadData(&data[0] + data.size() - dw, dw, &dw);
			}

			return data;
		}
	};

} // namespace win
