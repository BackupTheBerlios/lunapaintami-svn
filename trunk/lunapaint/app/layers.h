/****************************************************************************
*                                                                           *
* layers.h -- Lunapaint, http://www.sub-ether.org/lunapaint                 *
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

#ifndef _layers_h_
#define _layers_h_

#include <stdio.h>

#include <exec/types.h>
#include <libraries/mui.h>
#include <cybergraphx/cybergraphics.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/muimaster.h>
#include <proto/graphics.h>
#include <proto/cybergraphics.h>
#include <clib/alib_protos.h>
#include <string.h>

#include "canvas.h"

#include "../config.h"

Object *WindowLayers;
Object *WidgetLayers;
Object *ScrollLayers;
Object *BtnAddLayer;
Object *BtnDelLayer;
Object *BtnSwapLayer;
Object *BtnMergeLayer;
Object *BtnCopyLayer;
Object *LayerOpacity;
Object *LayerName;
Object *LayerVisible;
Object *LayerControlGroup;
Object *LayerOpacityValue;

unsigned int *LayersWidgetTmpBuf;
unsigned int layersWidgetWidth;
unsigned int layersWidgetHeight;
int layersLastScrollPos;
int layersUpdateX, layersUpdateY, layersUpdateW, layersUpdateH;

struct Hook changeOpacityHook;
struct Hook acknowledgeOpacity;
struct Hook acknowledgeLayName;
struct Hook changeVisibilityHook;
oCanvas* lastDrawnCanvas;
BOOL forceLayerRedraw;

/*
	This function initializes the layer window and the widget
*/
void Init_LayersWindow ( );

/*
	This function returns the dimensions of the layer widget
*/
IPTR layerMinMax ( Class *CLASS, Object *self, struct MUIP_AskMinMax *message );

/*
	Blank box on the layer widget
*/
void layerRenderBlank ( );

/*
	This function renders the layer widget
*/ 
BOOL layerRender ( Class *CLASS, Object *self );

/*
    Render OS friendly text to the layer preview
*/
void RenderLayerNames ( int x, int y, int w, int h );

/*
	This function handles input on the layer widget
*/
IPTR layerHandleInput ( Class *CLASS, Object *self, struct MUIP_HandleInput *msg );

/*
	Repaint the actual widget
*/
void layersRepaintWindow ( Class *CLASS, Object *self );

/*
	Clean up
*/
void Exit_LayersWindow ( );


#endif

