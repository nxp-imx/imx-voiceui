// Copyright 2020-2021 NXP
// SPDX-License-Identifier: BSD-3-Clause
#include "AudioStreamException.h"

AudioStreamException::AudioStreamException(
    const char * error_message,
    const char * error_source,
    const char * file,
    int line,
    int error_code) : std::exception(),
    _error_message(error_message),
    _source(error_source),
    _file(file),
    _line(line),
    _error_code(error_code)
{
}

const char * AudioStreamException::what(void) const noexcept
{
    return this->_error_message;
}

const char * AudioStreamException::getSource(void) const noexcept
{
    return this->_source;
}

const char * AudioStreamException::getFile(void) const noexcept
{
    return this->_file;
}

int AudioStreamException::getLine(void) const noexcept
{
    return this->_line;
}

int AudioStreamException::getErrorCode(void) const noexcept
{
    return this->_error_code;
}
