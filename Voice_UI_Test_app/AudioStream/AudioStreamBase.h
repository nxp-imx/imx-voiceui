/*----------------------------------------------------------------------------
    Copyright 2020-2021 NXP
    SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/
#ifndef AUDIO_STREAM_BASE_GUARD_
#define AUDIO_STREAM_BASE_GUARD_

#include "AudioStreamException.h"

namespace AudioStreamWrapper
{

#define NOT_IMPLEMENTED_ERROR   -1

class AudioStreamBase
{
public:
    virtual
    ~AudioStreamBase(void) {};

    virtual void
    start(void) = 0;

    virtual void
    stop(bool force) = 0;

    virtual void
    close(void) = 0;

    virtual int
    recover(int err);

    virtual void
    printConfig(void);
};

} // namespace AudioStreamWrapper

#endif /* AUDIO_STREAM_BASE_GUARD_ */
