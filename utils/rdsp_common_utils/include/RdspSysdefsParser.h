/*
 * Copyright (c) 2021 by Retune DSP.
 * Copyright 2022-2024 NXP
 *
 * NXP Confidential. This software is owned or controlled by NXP
 * and may only be used strictly in accordance with the applicable license terms.
 * By expressly accepting such terms or by downloading, installing,
 * activating and/or otherwise using the software, you are agreeing that you have read,
 * and that you agree to comply with and are bound by, such license terms.
 * If you do not agree to be bound by the applicable license terms,
 * then you may not retain, install, activate or otherwise use the software.
 */


#ifndef RDSP_SYSDEFS_PARSER_H
#define RDSP_SYSDEFS_PARSER_H

typedef struct RETUNE_VOICESEEKER_plugin_s RETUNE_VOICESEEKERLIGHT_plugin_t;

void load_user_sysdefs_xml(RETUNE_VOICESEEKERLIGHT_plugin_t* APluginInit, const char* Auser_sysdefs_fn);

#endif // RDSP_SYSDEFS_PARSER_H

