/****************************************************************************
*                                                                           *
* easyevents.h -- Lunapaint, http://www.sub-ether.org/lunapaint             *
* Copyright (C) 2006, 2007, Hogne Titlestad <hogga@sub-ether.org>           *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License for more details.                              *
*                                                                           *
* You should have received a copy of the GNU General Public License         *
* along with this program; if not, write to the Free Software Foundation,   *
* Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.            *
*                                                                           *
****************************************************************************/

#ifndef _events_h_
#define _events_h_
#define DEBUG 1
#include <stdio.h>
#include <stdlib.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <devices/rawkeycodes.h>
#include <aros/debug.h>
#include <exec/exec.h>
#include <proto/exec.h>

#include "canvas.h"

#define eventMouseButtonLeft        0x00000001
#define eventMouseButtonRight       0x00000002
#define eventMouseX                 0x00000010
#define eventMouseY                 0x00000011

struct IOStdReq *gameInput;
struct MsgPort *gameInputPort;
struct Interrupt *gameInputInterrupt;

struct Hook DisableKeyboard_hook;
struct Hook EnableKeyboard_hook;

BOOL evalMouseButtonL;
BOOL evalMouseButtonR;
LONG evalMouseX;
LONG evalMouseY;
ULONG evalRawKey;
ULONG releaseKey;

BOOL InitEvents ( );
void ShutdownEvents ( );
IPTR CheckEvent ( ULONG event );

#endif
