#pragma once

#include <AL/al.h>
#include "AL/alc.h"

// checks current openal error code
#ifdef _DEBUG
    #define alCheckError()\
    {\
        ALenum errcode = ::alGetError();\
        if (errcode != AL_NO_ERROR)\
        {\
            gConsole.LogMessage(eLogMessage_Error, "OpenAL error detected in %s, code 0x%04X", __FUNCTION__, errcode);\
            debug_assert(false); \
        }\
    }
#else
    #define alCheckError()
#endif

// resets current openal error code
inline void alClearError()
{
    for (ALenum alErrorCode = ::alGetError(); alErrorCode != AL_NO_ERROR; alErrorCode = ::alGetError()) 
    {
    }
}