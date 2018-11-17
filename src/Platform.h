#pragma once

#if defined(_MSC_VER)

#ifndef NDEBUG
#include <crtdbg.h>
void CPPOGL_Initialize_Platform() {
	/*_CrtSetReportMode(_CRT_ASSERT, 0);
	// ON assert, write to stderr.
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
	_CrtSetReportFile( _CRT_ASSERT, _CRTDBG_FILE_STDERR );

	// Suppress the abort message
	_set_abort_behavior(0, _WRITE_ABORT_MSG);*/
}
#else
void CPPOGL_Initialize_Platform() {
}
#endif

#else
void CPPOGL_Initialize_Platform() {
}
#endif