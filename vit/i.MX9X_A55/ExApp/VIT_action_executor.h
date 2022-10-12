/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ACTION_EXECUTOR_H
#define ACTION_EXECUTOR_H
#include "VIT_helper.h"

// #define RUN_ON_OTHER_THREAD 1

int VIT_run_thread();
int VIT_stop_thread();
int VIT_send_cmd(int cmd);

#endif
