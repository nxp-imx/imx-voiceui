/*----------------------------------------------------------------------------
	Copyright 2020-2021 NXP
	SPDX-License-Identifier: BSD-3-Clause
----------------------------------------------------------------------------*/

#include "SignalProcessor_NotifyTrigger.h"

namespace SignalProcessor {

	int32_t SignalProcessor_notifyTrigger(bool& notified, const char* command, int32_t iteration, int32_t& last_notification) {
		if (!notified) {
			if (iteration > last_notification + 20) {
				notified = !notified;
				last_notification = iteration;
				return system(command);
			}
		}
		return -1;
	}
}
