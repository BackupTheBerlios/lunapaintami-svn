/****************************************************************************
*                                                                           *
* project.h -- Lunapaint, http://www.sub-ether.org/lunapaint                *
* Copyright (C) 2006, 2007, Hogne Titlestad <hogga@sub-ether.org>           *
* Copyright (C) 2009 LunaPaint Development Team                             *
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
#ifndef _project_h_
#define _project_h_

#include <stdio.h>
#include <stdlib.h>

#ifndef __AROS__
#include "aros/aros.h"
#endif

#include <exec/types.h>
#include <libraries/mui.h>
#include <libraries/asl.h>
#include <cybergraphx/cybergraphics.h>

#include <datatypes/datatypes.h>
#include <datatypes/pictureclass.h>

#include <dos/filehandler.h>

#include <proto/asl.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/muimaster.h>
#include <proto/cybergraphics.h>
#include <proto/datatypes.h>
#include <clib/alib_protos.h>
#include <string.h>
#include <mui/TextEditor_mcc.h>

#define DEBUG 1
#include <aros/debug.h>

#include "gui.h"
#include "parts.h"
#include "config.h"
#include "core/canvas.h"
#include "core/definitions.h"

Object *projectWindow;

/*
    Creates a project window object and adds it to the PaintApp object
*/
void CreateProjectWindow ( WindowList *lst );
/*
    Destroys the project window object and removes it from the PaintApp
*/
void DestroyProjectWindow ( WindowList *lst );

/*
    Save a project to disk
*/
void SaveProject ( WindowList *lst );

/*
    Load project from disk
*/
void LoadProject ( unsigned char *filename, BOOL useCurrentCanvas );

#endif
