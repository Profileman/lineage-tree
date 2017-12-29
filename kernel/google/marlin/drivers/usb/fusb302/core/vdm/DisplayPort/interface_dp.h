/*
 * fusb302 usb phy driver for type-c and PD
 *
 * Copyright (C) 2015, 2016 Fairchild Semiconductor Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. Seee the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef _FSC_INTERFACE_DP_H
#define _FSC_INTERFACE_DP_H

#include "../../platform.h"
#include "dp_types.h"

/* vvvvv Functions in the core to be called by the system vvvvv */
/* Initiate status request - called by 'system' to get status of port partner */
void requestDpStatus(void);
/* Initiate config request - called by 'system' to configure port partner */
void requestDpConfig(DisplayPortConfig_t in);
/* Configure DP */
void configDp(FSC_BOOL enabled, FSC_U32 status);
void configAutoDpModeEntry(FSC_BOOL enabled, FSC_U32 mask, FSC_U32 value);
void WriteDpControls(FSC_U8* data);
void ReadDpControls(FSC_U8* data);
void ReadDpStatus(FSC_U8* data);
/* ^^^^^ Functions in the core to be called by the system ^^^^^ */

/* vvvvv functions for the system to implement vvvvv */
void informStatus(DisplayPortStatus_t stat); 
void updateStatusData(void);
void informConfigResult (FSC_BOOL success);
FSC_BOOL DpReconfigure(DisplayPortConfig_t config);

/* Automatic DisplayPort features! */
/* Automatic Mode Entry */
/* Uses a value/mask scheme */
/* 1 in mask = don't care. 0 in mask = compare to specified value */
extern FSC_BOOL DpEnabled;
extern FSC_BOOL DpAutoModeEntryEnabled;
extern DisplayPortCaps_t DpModeEntryMask;
extern DisplayPortCaps_t DpModeEntryValue;

/* DisplayPort Status/Config objects */
extern DisplayPortCaps_t DpCaps;
extern DisplayPortStatus_t DpStatus;
extern DisplayPortConfig_t DpConfig;

/* Port Partner Status/Config objects */
extern DisplayPortStatus_t DpPpStatus;
extern DisplayPortConfig_t DpPpRequestedConfig;
extern DisplayPortConfig_t DpPpConfig;
extern FSC_U32 DpModeEntered;

#endif /* #ifndef _FSC_INTERFACE_DP_H */
