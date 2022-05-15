#pragma once

#define ERROR_SOCKET_OPEN       -1
#define ERROR_BIND              -2
#define ERROR_EVENT_BASE        -3
#define ERROR_LISTEN            -4
#define ERROR_EVENT_ADD         -5
#define ERROR_SOCKET_NONBLOCK   -6
#define ERROR_SET_REUSEADDR     -7
#define ERROR_CONNECT           -8
#define ERROR_DISPATCH          -9
#define ERROR_SET_WIN_THREAD	-10
#define ERROR_EB_NULLPTR		-11
#define ERROR_EVENT_NEW			-12

using NetworkError = struct {
	int code = 0;
	String str = 0;
	void SetError(int _code, String _str);
};