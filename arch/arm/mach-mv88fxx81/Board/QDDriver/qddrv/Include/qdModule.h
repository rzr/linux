/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
* qdModule.h
*
* DESCRIPTION:
*		Defines the entry point for the QD driver
*
* DEPENDENCIES:   Platform.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef _QD_MODULE_H_
#define _QD_MODULE_H_
#include "msApiDefs.h"

int qdEntryPoint(void);
void qdExitPoint(void);
int qdModuleStart(void);

#endif /* _QD_MODULE_H_ */
