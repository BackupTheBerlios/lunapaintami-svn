/****************************************************************************
*                                                                           *
* canvas.h -- Lunapaint, http://www.sub-ether.org/lunapaint                 *
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

#ifndef _aros_canvas_h_
#define _aros_canvas_h_

#define MUIM_ClearActiveCanvas 		    7001
#define MUIM_SetPaintMode				7004
#define MUIM_AlterBrushShape			7005
#define MUIM_AlterBrushStep			    7006
#define MUIM_AlterBrushStrength		    7007
#define MUIM_CanvasAddLayer			    7020
#define MUIM_CanvasSwapLayer			7021
#define MUIM_CanvasDeleteLayer		    7022
#define MUIM_CanvasMergeLayer			7023
#define MUIM_CanvasCopyLayer			7024
#define MUIM_ChangeOnionskin			7025
#define MUIM_ChangeScrollOffset		    7026
#define MUIM_SetGlobalGrid				7050
#define MUIM_SetGridSize				7051
#define MUIM_SetColorMode				7070
#define MUIM_NextFrame					7060
#define MUIM_PrevFrame					7061
#define MUIM_GotoFrame					7062
#define MUIM_EffectOffset				8000
#define MUIM_OpenPaletteEditor		    7600
#define MUIM_ScrollingNotify			7777
#define MUIM_RedrawArea                 43587
#define MUIM_Redraw                     43588

// Paint tool keys
#define MUIA_Key_F						1001
#define MUIA_Key_D						1002
#define MUIA_Key_V						1003
#define MUIA_Key_L						1004
#define MUIA_Key_E						1005
#define MUIA_Key_R						1006
#define MUIA_Key_O						1007
#define MUIA_Key_B						1008
// Buffer keys
#define MUIA_Key_J						1050
#define MUIA_Key_Shift_J				1051
#define MUIA_Key_X						1052
#define MUIA_Key_Y						1053
#define MUIA_Key_Up						1054
#define MUIA_Key_Left					1055
#define MUIA_Key_Right					1056
#define MUIA_Key_Down					1057
// Effect keys
#define MUIA_Key_F1						1100
#define MUIA_Key_F2						1101
#define MUIA_Key_F3						1102
#define MUIA_Key_F4						1103
#define MUIA_Key_F5						1104
#define MUIA_Key_F6						1105
#define MUIA_Key_F7						1106
#define MUIA_Key_F8						1107
#define MUIA_Key_F9						1108
// Color keys
#define MUIA_Key_Shift_Comma			1200
#define MUIA_Key_Comma					1201

#include <stdio.h>
#include <stdlib.h>

#include <exec/types.h>
#include <datatypes/datatypes.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include <cybergraphx/cybergraphics.h>

#include <math.h>
#include <devices/rawkeycodes.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/datatypes.h>
#include <proto/gadtools.h>
#include <proto/muimaster.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <clib/alib_protos.h>
#include <string.h>

#ifdef __AROS__
#define DEBUG 1
#include <aros/debug.h>
#endif

#ifndef __AROS__
#include "../aros/aros.h"
#endif

#include "parts.h"
#include "toolbox.h"
#include "layers.h"
#include "project.h"

#include "../config.h"

#include "../core/canvas.h"
#include "../core/tools.h"
#include "../math/math.h"
#include "../common.h"

BOOL isZooming;
BOOL isScrolling;
BOOL fullscreenEditing;
int AskMinMaxTimes;

/*
    For editing in fullscreen mode
*/
Object *windowFullscreen;
Object *fullscreenGroup;
Object *fsTopEdge, *fsLeftEdge, *fsRightEdge, *fsBottomEdge;

/*
    Setup keyboard shortcuts on canvas and toolbox
*/
void checkKeyboardShortcuts ( UWORD stri );

/*
    Move canvas
*/
void moveScrollbarUp ( );
void moveScrollbarDown ( );
void moveScrollbarLeft ( );
void moveScrollbarRight ( );

/*
    Redraw the area object with canvas data
*/
IPTR RGBitmapRedraw ( Class *CLASS, Object *self );

/*
    Return min default and max sizes of canvas
*/
IPTR CanvasAskMinMax ( Class *CLASS,Object *self, struct MUIP_AskMinMax *message );

/*
    Update Frame: 1/1 etc
*/
void UpdateCanvasInfo ( WindowList *win );

/*
    Scroll the active canvas
*/
IPTR ScrollCanvas ( int x, int y );

/*
    Snap offset coords to zoom (globalactivecanvas)
*/
void SnapOffsetToZoom ( oCanvas *canv );

/*
    Check if active window has changed and
    redraw if it has
*/
void winHasChanged ( );

/*
    Handle input events on area, like pressed mouse buttons, mouse movements and so on.
    Also, store the state of events in self.
*/
IPTR RGBitmapHandleInput ( Class *CLASS, Object *self, struct MUIP_HandleInput *msg );

/*
    Store mouse coordinates into the global variables
*/
void getMouseCoordinates ( struct MUIP_HandleInput *msg, struct RGBitmapData *data );

/*
    This function adds a canvas window to the canvas window list
    ( canwinlist )
*/
void addCanvaswindow ( 
    unsigned int width, unsigned int height, 
    unsigned int layers, unsigned int frames,
    BOOL generateCanvas 
);

/*
    Show/Hide the fullscreen editing window
*/
void showFullscreenWindow ( oCanvas *canvas );
void hideFullscreenWindow ( );
void scaleFullscreenWindow ( );
void centerFullscreenWindow ( );

/*
    Contstrain offset values within scope of canvas
*/
void constrainOffset ( oCanvas *canvas );

/*
    Test if we're hitting "walls"
*/
void snapToBounds ( int *x, int *y );

/*
    This function sends back a pointer to a window object
*/
Object *getCanvaswindowById ( unsigned int id );

/*
    This function sends back a pointer to the window struct
*/
WindowList *getCanvasDataById ( unsigned int id );

/*
    Import an image rawly! :-D
*/
void importImageRAW ( unsigned int w, unsigned int h, unsigned long long int *buffer );

/*
    Load an image through datatypes
*/
BOOL loadDatatypeImage ( );

/*
    Free memory
*/
void deleteCanvaswindows ( );

/*
    Remove a single canvas window and it's structure
*/
IPTR removeActiveWindow ( Class *CLASS, Object *self );

/*
    Blit over a rect to the area from a canvas
*/
void blitAreaRect ( 
    int x, int y, int w, int h, 
    oCanvas* canvas, struct RastPort *rp
);

/*
    Removes previous tool preview blit from the painting
*/
void removePrevToolPreview ( );

/*
    Calls blitAreaRect over a hovered over part of the screen
    to show the current brush or a clipbrush
*/
void callToolPreview ( );

#endif
