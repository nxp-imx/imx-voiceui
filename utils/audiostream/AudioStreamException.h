/*----------------------------------------------------------------------------
    Copyright 2020-2021 NXP
    SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/
#include <exception>

#ifndef AUDIO_STREAM_EXCEPTION_GUARD_
#define AUDIO_STREAM_EXCEPTION_GUARD_

class AudioStreamException : public std::exception
{
    public:
        AudioStreamException(const char * error_message, const char * error_source, const char * file, int line, int error_number);
        const char * what(void) const noexcept;
        const char * getFile(void) const noexcept;
        const char * getSource(void) const noexcept;
        int getLine(void) const noexcept;
        int getErrorCode(void) const noexcept;

    private:
        const char * _error_message;
        const char * _source;
        const char * _file;
        int _line;
        int _error_code;
};

#endif /* AUDIO_STREAM_EXCEPTION_GUARD_ */