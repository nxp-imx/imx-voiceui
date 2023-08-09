/*----------------------------------------------------------------------------
    Copyright 2020-2021 NXP
    SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/
#include <stdint.h>
#include <iostream>

#ifndef __SignalProcessor_NotifyTrigger_h__
#define __SignalProcessor_NotifyTrigger_h__

namespace SignalProcessor {

    //Inform upon a trigger event
    int32_t SignalProcessor_notifyTrigger(bool& notified, const char* command, int32_t iteration, int32_t& last_notification);
}

#endif
