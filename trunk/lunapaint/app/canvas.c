/****************************************************************************
*                                                                           *
* canvas.c -- Lunapaint, http://www.sub-ether.org/lunapaint                 *
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

#include "canvas.h"

//---------------------------------------------------------------------

/*
	Change project info
*/
AROS_UFH3 ( void, projFunc,
	AROS_UFHA ( struct Hook*, h, A0 ),
	AROS_UFHA ( APTR, obj, A2 ),
	AROS_UFHA ( APTR, param, A1 )
)
{
	AROS_USERFUNC_INIT

	// We allocate but do not deallocate as
	// this var is freed by Zune when it becomes part
	// of it	
	unsigned char *projectName = NULL;
	
	WindowList *lst = *( WindowList **)param;

	get ( lst->projName, MUIA_String_Contents, &projectName );
	set ( lst->win, MUIA_Window_Title, projectName );
	set ( lst->projectWin, MUIA_Window_Open, FALSE );
	
	AROS_USERFUNC_EXIT
}

/* 
	Dispatcher for our RGBitmapObject
*/
BOOPSI_DISPATCHER ( IPTR, RGBitmapDispatcher, CLASS, self, message )
{			
    switch ( message->MethodID )
	{		
		case MUIM_Draw:
			
	        // Redrawing		
            if ( globalActiveCanvas != NULL )
            {
                AskMinMaxTimes = 0;
                globalActiveWindow->prevBlit.w = 0;
                RGBitmapRedraw ( CLASS, self );
                return ( IPTR )NULL; 
            }
		    return DoSuperMethodA ( CLASS, self, message );         
		
        // Called externally      
        case MUIM_RedrawArea:
            if ( globalActiveCanvas != NULL )
            {
                globalActiveCanvas->winHasChanged = TRUE;
                if ( redrawTimes < 5 )
                    redrawTimes++;
            }
            return ( IPTR )NULL;
           
		case MUIM_HandleInput:
			return ( IPTR )RGBitmapHandleInput ( CLASS, self, ( struct MUIP_HandleInput *)message );
		
		case MUIM_Setup:
            {      
                AskMinMaxTimes = 0;
		        isScrolling = FALSE;
		        isZooming = FALSE;
		        MUI_RequestIDCMP( self, IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY );
            }      
			return DoSuperMethodA ( CLASS, self, message );
		
		case MUIM_Cleanup:
			MUI_RejectIDCMP( self, IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY );
			return ( IPTR )NULL;
		
		case MUIM_CanvasActivate:
			{
				struct RGBitmapData *data = INST_DATA ( CLASS, self );
                Update_AnimValues ( );            
                AskMinMaxTimes = 0;
				globalActiveWindow = data->window;
				globalActiveCanvas = data->window->canvas;
                
                // Set scrollbars
                int areaHeight = XGET ( self, MUIA_Height );
                int areaWidth = XGET ( self, MUIA_Width );
                set ( data->window->scrollV, MUIA_Prop_Entries, ( IPTR )globalActiveCanvas->height * data->currentzoom );
                set ( data->window->scrollH, MUIA_Prop_Entries, ( IPTR )globalActiveCanvas->width * data->currentzoom );
                set ( data->window->scrollV, MUIA_Prop_Visible, ( IPTR )areaHeight );
                set ( data->window->scrollH, MUIA_Prop_Visible, ( IPTR )areaWidth );
                set ( data->window->scrollH, MUIA_Prop_First, ( IPTR )globalActiveCanvas->offsetx );
                set ( data->window->scrollV, MUIA_Prop_First, ( IPTR )globalActiveCanvas->offsety );
                //
				data->window->isActive = FALSE;            
				DoMethod ( WidgetLayers, MUIM_Draw );
			}
			return ( IPTR )NULL;
		
		case MUIM_ScrollingNotify:
			isScrolling = TRUE;
			return ( IPTR )NULL;
		
		case MUIM_CanvasDeactivate:
			if ( globalActiveWindow != NULL )
			{
				struct RGBitmapData *data = INST_DATA ( CLASS, self );
				data->window->isActive = FALSE;
                data->window->canvas->offsetx = XGET ( data->window->scrollH, MUIA_Prop_First );
                data->window->canvas->offsety = XGET ( data->window->scrollV, MUIA_Prop_First );
			}
			return ( IPTR )NULL;
			
		case MUIM_ZoomIn:
			if ( globalActiveCanvas->zoom + 1 <= 32 && !isZooming )
			{	
				isZooming = TRUE;
                AskMinMaxTimes = 2;
				
				// Remove any previous tool preview
				removePrevToolPreview ( );
				
				// Set new zoom
				globalActiveCanvas->zoom++;
			
				// Move scrollbars
				globalActiveCanvas->offsetx += globalActiveCanvas->visibleWidth * 0.5;
				globalActiveCanvas->offsety += globalActiveCanvas->visibleHeight * 0.5;
				
				// Update GUI
				winHasChanged ( );
				isZooming = FALSE;
			}
			return ( IPTR )NULL;
		
		case MUIM_ZoomOut:
			if ( globalActiveCanvas->zoom - 1 >= 1 && !isZooming )
			{
				isZooming = TRUE;
				
				// Remove any previous tool preview
				removePrevToolPreview ( );
				
				// Set new zoom
				globalActiveCanvas->zoom--;
                AskMinMaxTimes = 2;
				
				// Move scrollbars
				globalActiveCanvas->offsetx -= globalActiveCanvas->visibleWidth * 0.5;
				globalActiveCanvas->offsety -= globalActiveCanvas->visibleHeight * 0.5;
				constrainOffset ( globalActiveCanvas );
				
				// Update GUI
				winHasChanged ( );
				isZooming = FALSE;
			}
			return ( IPTR )NULL;
		
		case MUIM_ShowAll:
			// Remove any previous tool preview
			isZooming = TRUE;
			removePrevToolPreview ( );
            AskMinMaxTimes = 2;    
			globalActiveCanvas->zoom = 1;
			globalActiveCanvas->offsetx = 0; 
			globalActiveCanvas->offsety = 0;
			winHasChanged ( );
			isZooming = FALSE;
			return ( IPTR )NULL;
		
		case MUIM_AskMinMax:
			DoSuperMethodA ( CLASS, self, message );
			return ( IPTR )CanvasAskMinMax ( CLASS, self, ( struct MUIP_AskMinMax* )message );
            
		case MUIM_CloseCanvasWin:
			removeActiveWindow ( CLASS, self );
			layerRenderBlank ( );
			return ( IPTR )NULL;
		
		default:  
            return DoSuperMethodA ( CLASS, self, message );
	}
	return ( IPTR )NULL;
}
BOOPSI_DISPATCHER_END


void checkKeyboardShortcuts ( UWORD valu )
{
	switch ( valu )
	{
        case RAWKEY_A:
            if ( brushTool.antialias == FALSE )
            {
                DoMethod ( antiOffImage, MUIM_CallHook, &brushOptions_hook, SET_ANTIALIAS );
            }
            else 
            {
                DoMethod ( antiOffImage, MUIM_CallHook, &brushOptions_hook, SET_ANTIALIASOFF );
            }
            makeToolBrush ( );
            break;      
        case RAWKEY_SPACE:
            {
                int VAL = ( int )XGET( tbxCyc_PaletteSnap, MUIA_Cycle_Active );
                if ( VAL == 0 )
                    set ( tbxCyc_PaletteSnap, MUIA_Cycle_Active, 1 );
                else set ( tbxCyc_PaletteSnap, MUIA_Cycle_Active, 0 );
            } 
            break;
		case RAWKEY_F:
			globalCurrentTool = LUNA_TOOL_FILL;
			setToolbarActive ( );
			break;
		case RAWKEY_D:
			DoMethod ( paletteRect, MUIM_SetTool_Draw );
			break;
		case RAWKEY_L:
			DoMethod ( paletteRect, MUIM_SetTool_Line );
			break;
		case RAWKEY_E:
			DoMethod ( paletteRect, MUIM_SetTool_Circle );
			break;
		case RAWKEY_R:
			DoMethod ( paletteRect, MUIM_SetTool_Rectangle );
			break;
		case RAWKEY_O:
			DoMethod ( paletteRect, MUIM_SetTool_Colorpicker );
			break;
		case RAWKEY_B:
			DoMethod ( paletteRect, MUIM_SetTool_ClipBrush );
			break;
        case RAWKEY_G:
            {
                ULONG curr = XGET ( tbxCycGrid, MUIA_Cycle_Active );
                if ( curr == 0 )
                    set ( tbxCycGrid, MUIA_Cycle_Active, 1 );
                else
                    set ( tbxCycGrid, MUIA_Cycle_Active, 0 );
            }
            break;         
		case RAWKEY_J:
			swapCanvasBuffers ( globalActiveCanvas );
			globalActiveWindow->rRectW = 0;
			globalActiveWindow->rRectX = 0;
			globalActiveWindow->rRectH = 0;
			globalActiveWindow->rRectY = 0;
			globalActiveCanvas->winHasChanged = TRUE;
            globalActiveWindow->layersChg = TRUE;         
			DoMethod ( globalActiveWindow->area, MUIM_Draw );
			break;
		case RAWKEY_J | RAWKEY_RSHIFT:
			copyToSwap ( globalActiveCanvas );
			break;
		case RAWKEY_Y:
			flipBrushVert ( );
			break;
		case RAWKEY_X:
			flipBrushHoriz ( );
			break;
        case RAWKEY_Z:
            rotateBrush90 ( );
            break;         
		case RAWKEY_PERIOD:
			nextPaletteColor ( );
			break;
		case RAWKEY_COMMA:
			prevPaletteColor ( );
			break;
		case RAWKEY_F1: set ( tbxCycPaintMode, MUIA_Cycle_Active, 0 ); break;
		case RAWKEY_F2: set ( tbxCycPaintMode, MUIA_Cycle_Active, 1 ); break;
		case RAWKEY_F3: set ( tbxCycPaintMode, MUIA_Cycle_Active, 6 ); break;
		case RAWKEY_F4: set ( tbxCycPaintMode, MUIA_Cycle_Active, 4 ); break;
		case RAWKEY_F5: set ( tbxCycPaintMode, MUIA_Cycle_Active, 5 ); break;
		case RAWKEY_F6: set ( tbxCycPaintMode, MUIA_Cycle_Active, 7 ); break;
		case RAWKEY_F7: set ( tbxCycPaintMode, MUIA_Cycle_Active, 8 ); break;
		case RAWKEY_F8: set ( tbxCycPaintMode, MUIA_Cycle_Active, 2 ); break;
		case RAWKEY_F9: set ( tbxCycPaintMode, MUIA_Cycle_Active, 3 ); break;
		case RAWKEY_UP: moveScrollbarUp (  ); break;
		case RAWKEY_DOWN: moveScrollbarDown (  ); break;
		case RAWKEY_LEFT: moveScrollbarLeft (  ); break;
		case RAWKEY_RIGHT: moveScrollbarRight (  ); break;
		
		// Opacity values
		case RAWKEY_KP_1: set ( tbxSlider_Brushopacity, MUIA_Numeric_Value, 5 ); break;
		case RAWKEY_KP_2: set ( tbxSlider_Brushopacity, MUIA_Numeric_Value, 10 ); break;
		case RAWKEY_KP_3: set ( tbxSlider_Brushopacity, MUIA_Numeric_Value, 30 ); break;
		case RAWKEY_KP_4: set ( tbxSlider_Brushopacity, MUIA_Numeric_Value, 60 ); break;
		case RAWKEY_KP_5: set ( tbxSlider_Brushopacity, MUIA_Numeric_Value, 128 ); break;
		case RAWKEY_KP_6: set ( tbxSlider_Brushopacity, MUIA_Numeric_Value, 255 ); break;
		// Brush sizes
		case RAWKEY_KP_PLUS: 
			{
				int bsz = XGET ( tbxSlider_Brushdiameter, MUIA_Numeric_Value );
				set ( tbxSlider_Brushdiameter, MUIA_Numeric_Value, bsz + 2 ); 
				break;
			}
		case RAWKEY_MINUS: 
			{
				int bsz = XGET ( tbxSlider_Brushdiameter, MUIA_Numeric_Value );
				set ( tbxSlider_Brushdiameter, MUIA_Numeric_Value, bsz - 2 ); 
				break;
			}
		case RAWKEY_KP_DECIMAL: set ( tbxSlider_Brushdiameter, MUIA_Numeric_Value, 1 ); break;
		case RAWKEY_KP_ENTER: set ( tbxSlider_Brushdiameter, MUIA_Numeric_Value, 100 ); break;
		
		default: break;
	}
	lastKeyPressed = valu;
}

void moveScrollbarUp ( )
{
	if ( globalActiveWindow == NULL ) return;
	int pos = XGET ( globalActiveWindow->scrollV, MUIA_Prop_First );
    int dist = globalActiveCanvas->zoom * 10;
	set ( globalActiveWindow->scrollV, MUIA_Prop_First, pos - dist );
}
void moveScrollbarDown ( )
{
	if ( globalActiveWindow == NULL ) return;
	int pos = XGET ( globalActiveWindow->scrollV, MUIA_Prop_First );
    int dist = globalActiveCanvas->zoom * 10;   
	set ( globalActiveWindow->scrollV, MUIA_Prop_First, pos + dist );
}
void moveScrollbarLeft ( )
{
	if ( globalActiveWindow == NULL ) return;
	int pos = XGET ( globalActiveWindow->scrollH, MUIA_Prop_First );
    int dist = globalActiveCanvas->zoom * 10;   
	set ( globalActiveWindow->scrollH, MUIA_Prop_First, pos - dist );
}
void moveScrollbarRight ( )
{
	if ( globalActiveWindow == NULL ) return;
	int pos = XGET ( globalActiveWindow->scrollH, MUIA_Prop_First );
    int dist = globalActiveCanvas->zoom * 10;   
	set ( globalActiveWindow->scrollH, MUIA_Prop_First, pos + dist );
}

void constrainOffset ( oCanvas *canvas )
{
	int zoom = canvas->zoom;
	int vWidth = ( double )canvas->visibleWidth / zoom;
	int vHeight = ( double )canvas->visibleHeight / zoom;
	int cWidth = canvas->width * zoom;
	int cHeight = canvas->height * zoom;
	
	if ( canvas->offsetx < 0 ) canvas->offsetx = 0;
	else if ( canvas->offsetx + vWidth >= cWidth  )
		canvas->offsetx = cWidth - vWidth;
	if ( canvas->offsety < 0 ) canvas->offsety = 0;
	else if ( canvas->offsety + vHeight >= cHeight )
		canvas->offsety = cHeight - vHeight;
		
	// Snap to zoom
	SnapOffsetToZoom ( canvas );
}

void winHasChanged ( )
{	
	globalActiveCanvas->winHasChanged = TRUE;
  	DoMethod ( globalActiveWindow->container, MUIM_Group_InitChange );
	DoMethod ( globalActiveWindow->container, MUIM_Group_ExitChange );
}

IPTR CanvasAskMinMax ( Class *CLASS,Object *self, struct MUIP_AskMinMax *message )
{   
	struct RGBitmapData *data = INST_DATA ( CLASS, self );
	
	int calcwidth = data->window->canvas->width * data->window->canvas->zoom;
	int calcheight = data->window->canvas->height * data->window->canvas->zoom;
	
	int tWidth = 0, minWidth = 0, tHeight = 0, minHeight = 0;
	tWidth = XGET ( globalActiveWindow->container, MUIA_Width );
	tHeight = XGET ( globalActiveWindow->container, MUIA_Height );
	minWidth = globalActiveCanvas->width * globalActiveCanvas->zoom;
	minHeight = globalActiveCanvas->height * globalActiveCanvas->zoom;
	if ( minWidth > tWidth ) minWidth = tWidth;
	if ( minHeight > tHeight ) minHeight = tHeight;
    if ( AskMinMaxTimes > 0 )   
	{
        message->MinMaxInfo->MinWidth += minWidth;
	    message->MinMaxInfo->MinHeight += minHeight;
        AskMinMaxTimes--;   
    }
    else
    {
        message->MinMaxInfo->MinWidth += 0;
        message->MinMaxInfo->MinHeight += 0;
    }
    message->MinMaxInfo->DefWidth += calcwidth;
    message->MinMaxInfo->DefHeight += calcheight;
    message->MinMaxInfo->MaxWidth += calcwidth;
    message->MinMaxInfo->MaxHeight += calcheight;      

	return ( IPTR )NULL;
}

IPTR RGBitmapRedraw ( Class *CLASS, Object *self )
{	    
	// Get stored data
	struct RGBitmapData *data = INST_DATA ( CLASS, self );

	// Get image dimensions multiplied by zoom
	unsigned int imageWidth = ( unsigned int )( data->window->canvas->width );
	unsigned int imageHeight = ( unsigned int )( data->window->canvas->height );
	imageWidth = ( int )( imageWidth * data->window->canvas->zoom );
	imageHeight = ( int )( imageHeight * data->window->canvas->zoom );
	unsigned int visWidth = data->window->canvas->visibleWidth;
	unsigned int visHeight = data->window->canvas->visibleHeight;
	
	// Get area dimensions
	ULONG areaWidth = XGET ( data->window->area, MUIA_Width );
	ULONG areaHeight = XGET ( data->window->area, MUIA_Height );
	if ( areaWidth > imageWidth ) areaWidth = imageWidth;
	if ( areaHeight > imageHeight ) areaHeight = imageHeight;
	int areatop = XGET ( self, MUIA_TopEdge );
	int arealeft = XGET ( self, MUIA_LeftEdge );

	// Update known container size
	unsigned int cvisWidth = XGET ( data->window->container, MUIA_Width );
	unsigned int cvisHeight = XGET ( data->window->container, MUIA_Height );
	data->window->contWidth = cvisWidth;
	data->window->contHeight = cvisHeight;

	// Update window edge coordinates
	data->window->canvas->winEdgeWidth = data->arealeft;
	data->window->canvas->winEdgeHeight = data->areatop;
	if ( areaWidth != visWidth || areaHeight != visHeight )
		data->window->canvas->winHasChanged = TRUE;
	data->window->canvas->visibleWidth = areaWidth;
	data->window->canvas->visibleHeight = areaHeight;
	
	// Update scrollbars if we're changing zoom
	if ( 
		globalActiveCanvas->zoom != data->currentzoom ||
		areaWidth != data->currentareawidth || 
		areaHeight != data->currentareaheight
	)
	{	
		// Snap the coordinates to zoom level
		SnapOffsetToZoom ( NULL );
		
		// Set scrollbars
		set ( data->window->scrollH, MUIA_Prop_First, ( IPTR )globalActiveCanvas->offsetx );
		set ( data->window->scrollV, MUIA_Prop_First, ( IPTR )globalActiveCanvas->offsety );
		set ( data->window->scrollV, MUIA_Prop_Visible, ( IPTR )areaHeight );
		set ( data->window->scrollV, MUIA_Prop_Entries, ( IPTR )imageHeight );
		set ( data->window->scrollH, MUIA_Prop_Visible, ( IPTR )areaWidth );
		set ( data->window->scrollH, MUIA_Prop_Entries, ( IPTR )imageWidth );
        
		// Sometimes the scrollbar will refuse a coordinate, always sync with
		// the scrollbar!!!
		globalActiveCanvas->offsetx = XGET ( data->window->scrollH, MUIA_Prop_First );
		globalActiveCanvas->offsety = XGET ( data->window->scrollV, MUIA_Prop_First );
        
		// Snap the coordinates to zoom level
		SnapOffsetToZoom ( NULL );
		
		data->currentzoom = globalActiveCanvas->zoom;
		data->currentareawidth = areaWidth;
		data->currentareaheight = areaHeight;
	}
	
	// Update the screen buffer
	if ( data->window->canvas->winHasChanged )
	{
		int Tool = globalCurrentTool;      
		globalCurrentTool = -1;
		
		// Accelerated redraw
		if ( data->window->rRectW || data->window->rRectH )
		{   
			oCanvas *canvas = data->window->canvas;
			int offsetx 	= ( canvas->offsetx + data->window->rRectX ) / canvas->zoom;
			int offsety 	= ( canvas->offsety + data->window->rRectY ) / canvas->zoom;
			int width 		= ( double )data->window->rRectW / canvas->zoom + 1;
			int height 		= ( double )data->window->rRectH / canvas->zoom + 1;
			
			if ( redrawScreenbufferRect ( canvas, offsetx, offsety, width, height, FALSE ) )
			{ 
                blitAreaRect ( 
				    offsetx, offsety, 
				    width, height, 
				    canvas, _rp ( data->window->area )
			    );   
                // Reset accelerator parameters
			    data->window->rRectX = 0;
			    data->window->rRectY = 0;
			    data->window->rRectW = 0;
			    data->window->rRectH = 0;
            }               
		}
		// Slow redraw (with zoom etc)
		else
		{
			if ( redrawScreenbuffer ( data->window->canvas ) )         
            {         
			    // Write all data to the rastport!
			    WritePixelArray ( 
				    data->window->canvas->screenbuffer,
				    0, 0, data->window->canvas->visibleWidth * 4, _rp ( data->window->area ),
				    arealeft, areatop, areaWidth, areaHeight, RECTFMT_RGBA
			    );
            }         
            UpdateCanvasInfo ( data->window );         
            Update_AnimValues ( );
		}   
		data->window->canvas->winHasChanged = FALSE;
		globalCurrentTool = Tool;
	}
	else
	{
		if ( data->window->canvas->screenstorage != NULL )
		{
			// Write all data to the rastport!
			WritePixelArray ( 
				data->window->canvas->screenstorage,
				0, 0, data->window->canvas->scrStorageWidth * 4, _rp ( data->window->area ),
				arealeft, areatop, areaWidth, areaHeight, RECTFMT_RGBA
			);
		}
	}
    if ( globalActiveWindow->layersChg )      
        DoMethod ( WidgetLayers, MUIM_Draw );  
    
	return ( IPTR )NULL;
}

void SnapOffsetToZoom ( oCanvas *canv )
{
	if ( canv == NULL ) canv = globalActiveCanvas;
	// Contstrain offset to zoom
	int	*ofx = &canv->offsetx, *ofy = &canv->offsety, zoom = canv->zoom;
	*ofx = ( int )( ( double )*ofx / zoom ) * zoom;
	*ofy = ( int )( ( double )*ofy / zoom ) * zoom;
}

void UpdateCanvasInfo ( WindowList *win )
{
	unsigned char frameText[ 32 ];
    unsigned char layerText[ 32 ];
    int frame = ( int )( win->canvas->currentFrame + 1 ); 
    int frams = ( int )win->canvas->totalFrames;
    int layer = ( int )( win->canvas->currentLayer + 1 );
    int layrs = ( int )win->canvas->totalLayers;
    sprintf ( ( unsigned char *)&frameText, "Frame: %d/%d ", frame, frams );
	set ( win->txtFrameInf, MUIA_Text_Contents, ( STRPTR )&frameText );
	sprintf ( ( unsigned char *)&layerText, "Layer: %d/%d ", layer, layrs );
	set ( win->txtLayerInf, MUIA_Text_Contents, ( STRPTR )&layerText );
}

IPTR ScrollCanvas ( int x, int y )
{	
	// Remove any prevous toolpreview
	int currTool = globalCurrentTool;
	globalCurrentTool = -1;
	removePrevToolPreview ( );
	globalCurrentTool = currTool;
	
	// Get coordinates
	ULONG areaWidth = XGET ( globalActiveWindow->area, MUIA_Width );
	ULONG areaHeight = XGET ( globalActiveWindow->area, MUIA_Height );
	ULONG areaLeft = XGET ( globalActiveWindow->area, MUIA_LeftEdge );
	ULONG areaTop = XGET ( globalActiveWindow->area, MUIA_TopEdge );
			
	// find diff and align to zoom
	int diffx = x -  ( int )globalActiveCanvas->offsetx;
	int diffy = y - ( int )globalActiveCanvas->offsety;
	int hght = areaHeight - abs ( diffy );
	int wdth = areaWidth - abs ( diffx );
	
	if ( abs ( diffy ) > 0 )
	{
		// If scrolling inside viewable area
		if ( hght > 0 )
		{
			int dsty = ( diffy < 0 ) ? abs ( diffy ) : 0;
			int srcy = ( diffy < 0 ) ? 0 : diffy;
			
			MovePixelArray ( 
				areaLeft,
				areaTop + srcy,
				_rp ( globalActiveWindow->area ),
				areaLeft,
				areaTop + dsty,
				areaWidth,
				hght
			);
			
			globalActiveWindow->rRectW = areaWidth;
			globalActiveWindow->rRectX = 0;
			globalActiveWindow->rRectH = abs ( diffy ) ? abs ( diffy ) : areaHeight;
			globalActiveWindow->rRectY = ( diffy > 0 ) ? ( areaHeight - diffy ) : 0;
		}
		// If jumping a page
		else
		{
			globalActiveWindow->rRectW = 0;
			globalActiveWindow->rRectX = 0;
			globalActiveWindow->rRectH = 0;
			globalActiveWindow->rRectY = 0;
		}
		globalActiveCanvas->offsety = y;
        globalActiveCanvas->winHasChanged = TRUE;      
		DoMethod ( globalActiveWindow->area, MUIM_Draw );
	}
	if ( abs ( diffx ) > 0 )
	{
		// If scrolling inside viewable area
		if ( wdth > 0 )
		{
			int dstx = ( diffx < 0 ) ? abs ( diffx ) : 0;
			int srcx = ( diffx < 0 ) ? 0 : diffx;
			
			MovePixelArray ( 
				areaLeft + srcx,
				areaTop,
				_rp ( globalActiveWindow->area ),
				areaLeft + dstx,
				areaTop,
				wdth,
				areaHeight
			);
			
			globalActiveWindow->rRectW = abs ( diffx ) ? abs ( diffx ) : areaWidth;
			globalActiveWindow->rRectX = ( diffx > 0 ) ? ( areaWidth - diffx ) : 0;
			globalActiveWindow->rRectH = areaHeight;
			globalActiveWindow->rRectY = 0;
		}
		// Jumping a page
		else
		{
			globalActiveWindow->rRectW = 0;
			globalActiveWindow->rRectX = 0;
			globalActiveWindow->rRectH = 0;
			globalActiveWindow->rRectY = 0;
		}
		globalActiveCanvas->offsetx = x;
        globalActiveCanvas->winHasChanged = TRUE;
		DoMethod ( globalActiveWindow->area, MUIM_Draw );
	}
	return ( IPTR )NULL;
}

IPTR RGBitmapHandleInput ( Class *CLASS, Object *self, struct MUIP_HandleInput *msg )
{		
	if ( CLASS == NULL || isZooming || globalActiveCanvas == NULL ) 
		return ( IPTR )NULL;
		
	struct RGBitmapData *data = ( struct RGBitmapData *)INST_DATA ( CLASS, self );
	char movement = 0;
	
	// Immediately set this window as active window!
	if ( globalActiveCanvas != data->window->canvas )
	{
		globalActiveCanvas = data->window->canvas;
		globalActiveWindow = data->window;
	}
	
	// Get current zoom in a simpler to write variable
	int zoom = globalActiveCanvas->zoom;
	
	// Get correct offset coordinates
	// and align them to zoom "grid"
	int ox = XGET ( globalActiveWindow->scrollH, MUIA_Prop_First );
	int oy = XGET ( globalActiveWindow->scrollV, MUIA_Prop_First );
	ox = ( int )( ( double )ox / zoom ) * zoom;
	oy = ( int )( ( double )oy / zoom ) * zoom;
	
	// If the offset has changed (with a window resize etc) or we got a scroll event
	// notification
	if ( 
		( ox != globalActiveCanvas->offsetx || oy != globalActiveCanvas->offsety ) &&
		isScrolling
	)
	{
		ScrollCanvas ( ox, oy );       
		isScrolling = FALSE;
	}
	
	// Get dimensions
	ULONG areaWidth = 0; get ( data->window->area, MUIA_Width,  &areaWidth );
	ULONG areaHeight = 0; get ( data->window->area, MUIA_Height, &areaHeight );
	ULONG areaLeft = 0; get ( data->window->area, MUIA_LeftEdge, &areaLeft );
	ULONG areaTop = 0; get ( data->window->area, MUIA_TopEdge, &areaTop );
	
	// Make sure we have a message
	if ( msg->imsg )
	{
		switch ( msg->imsg->Class )
		{		
			case IDCMP_MOUSEMOVE:
				
				// Signalize that we have movement
				movement = 1;
				MouseHasMoved = TRUE;
				
				// Get the mose coordinates relative to top and left widget edges
				data->arealeft = 0; data->areatop = 0;
				get ( self, MUIA_LeftEdge, &data->arealeft );
				get ( self, MUIA_TopEdge, &data->areatop );
				
				// Update window edge coordinates
				data->window->canvas->winEdgeWidth = data->arealeft;
				data->window->canvas->winEdgeHeight = data->areatop;
				
				if ( !data->window->isActive )
				{
					data->window->isActive = TRUE;
					getMouseCoordinates ( msg, data, ox, oy );
					dMouseX = cMouseX, dMouseY = cMouseY;
					break;
				}
				
			case IDCMP_MOUSEBUTTONS:
				
				// Signalize that we have movement
				movement = 1;
				
				// And that the window has changed
				data->window->canvas->winHasChanged = TRUE;
					
				if ( msg->imsg->Code == SELECTDOWN )
				{	
                    if ( cMouseX >= ox || cMouseX < ox + areaWidth || cMouseY >= oy || cMouseY < oy + areaWidth )
                    {            
					    // Register the click!
					    if ( !MouseButtonL ) MouseHasMoved = TRUE;
					    MouseButtonL = TRUE;
					    // Paintbrush tool filled
					    if ( globalCurrentTool == LUNA_TOOL_BRUSH ) brushTool.RecordContour = TRUE;
                    }               
				}
				// Inactivizers happen everywhere
				if ( msg->imsg->Code == SELECTUP )
				{				
                    MouseHasMoved = TRUE;
					MouseButtonL = FALSE;
					if ( globalCurrentTool == LUNA_TOOL_BRUSH ) brushTool.RecordContour = FALSE;
                    globalActiveWindow->layersChg = TRUE;               
					DoMethod ( WidgetLayers, MUIM_Draw );             
				}
				break;
				
			case IDCMP_RAWKEY:
				
				checkKeyboardShortcuts ( msg->imsg->Code );
				
				break;
		}
		
		// Update mouse coordiates
		if ( movement == 1 )
		{
			getMouseCoordinates ( msg, data, ox, oy );
			if ( !MouseButtonL && !MouseButtonR ) 
				dMouseX = cMouseX, dMouseY = cMouseY;
		}
	}
	return 0;
}

void getMouseCoordinates ( struct MUIP_HandleInput *msg, struct RGBitmapData *data, int ox, int oy )
{
	eMouseX = ( int )msg->imsg->MouseX - data->arealeft;
	eMouseY = ( int )msg->imsg->MouseY - data->areatop;
	cMouseX = ( eMouseX + ox ) / globalActiveCanvas->zoom;
	cMouseY = ( eMouseY + oy ) / globalActiveCanvas->zoom;
	
	if ( globalGrid )
	{
		cMouseX = ( int )( cMouseX / globalCurrentGrid ) * globalCurrentGrid;
		cMouseY = ( int )( cMouseY / globalCurrentGrid ) * globalCurrentGrid;
	}
	
	// Unantialiased coords..
	if ( !brushTool.antialias )
	{
		cMouseX = ( int )cMouseX;
		cMouseY = ( int )cMouseY;
	}
}

void addCanvaswindow ( 
	unsigned int width, unsigned int height, 
	unsigned int layers, unsigned int frames,
	BOOL generateCanvas 
)
{
	WindowList *temp = canvases; // Put current in temp
	canvases = AllocVec ( sizeof ( WindowList ), MEMF_ANY );

	struct MUI_CustomClass *mcc = 
		MUI_CreateCustomClass ( 
			NULL, MUIC_Area, NULL, 
			sizeof ( struct RGBitmapData ), 
			RGBitmapDispatcher 
		);
        
    canvases->win = MUI_NewObject ( MUIC_Window,
		MUIA_Window_Title, ( IPTR )"Unnamed image",
		MUIA_Window_SizeGadget, TRUE,
		MUIA_Window_Screen, ( IPTR )lunaPubScreen,
		MUIA_Window_MouseObject, ( IPTR )canvases->mouse,
		MUIA_Window_ScreenTitle, ( IPTR )LUNA_SCREEN_TITLE,
        MUIA_Window_UseRightBorderScroller, TRUE,
        MUIA_Window_UseBottomBorderScroller, TRUE,      
        MUIA_Window_IsSubWindow, TRUE,
		WindowContents, ( IPTR )VGroup,
			InnerSpacing( 0, 0 ),
			MUIA_Group_HorizSpacing, 0,
			MUIA_Group_VertSpacing, 0,
			Child, ( IPTR )VGroup,
				InnerSpacing( 0, 0 ),
				MUIA_Group_HorizSpacing, 0,
				MUIA_Group_VertSpacing, 0,
				Child, ( IPTR )HGroup,
					MUIA_Group_HorizSpacing, 0,
					MUIA_Group_VertSpacing, 0,
					InnerSpacing( 0, 0 ),
					Child, ( IPTR )GroupObject,
						MUIA_Group_HorizSpacing, 0,
                        MUIA_Group_VertSpacing, 0,
                        InnerSpacing( 0, 0 ),
						MUIA_Frame, MUIV_Frame_None,
						MUIA_Group_Rows, 3,
						MUIA_Group_SameSize, FALSE,
						MUIA_Group_Child, ( IPTR )RectangleObject,
							MUIA_Frame, MUIV_Frame_None,
							InnerSpacing( 0, 0 ),
							MUIA_Weight, 1,
						End,
						Child, ( IPTR )HGroup,	
							MUIA_Group_HorizSpacing, 0,
                            MUIA_Group_VertSpacing, 0,
                            InnerSpacing( 0, 0 ),
							MUIA_Frame, MUIV_Frame_None,
							MUIA_Group_Columns, 3,
							MUIA_Group_SameSize, FALSE,
							Child, ( IPTR )RectangleObject,
								MUIA_Frame, MUIV_Frame_None,
								InnerSpacing( 0, 0 ),
								MUIA_Weight, 1,
							End,
							Child, ( IPTR )( canvases->container = GroupObject, 	
								MUIA_Group_HorizSpacing, 0,
                                MUIA_Group_VertSpacing, 0,
                                InnerSpacing( 0, 0 ),
								MUIA_Frame, MUIV_Frame_Group,
								MUIA_FillArea, FALSE,
								Child, ( IPTR )( canvases->area = NewObject(
									mcc->mcc_Class, NULL,
									MUIA_FillArea, FALSE,
									MUIA_Frame, MUIV_Frame_None,
									MUIA_Weight, 20000,
									TAG_DONE
								) ),
							End ),
							Child, ( IPTR )RectangleObject,
								MUIA_Frame, MUIV_Frame_None,
								InnerSpacing( 0, 0 ),
								MUIA_Weight, 1,
							End,
						End,
						Child, ( IPTR )RectangleObject,
							MUIA_Frame, MUIV_Frame_None,
							InnerSpacing( 0, 0 ),
							MUIA_Weight, 1,
						End,
					End,
				End,        
				Child, ( IPTR )HGroup,
					MUIA_Group_SameHeight, TRUE,
					MUIA_Frame, MUIV_Frame_Group,
					InnerSpacing ( 4, 4 ),
					MUIA_Background, MUII_FILL,
					MUIA_Group_Child, ( IPTR )HGroup,
						MUIA_Frame, MUIV_Frame_None,
						InnerSpacing ( 3, 3 ),
						MUIA_Weight, 100,
						Child, ( IPTR )( canvases->txtLayerInf = TextObject,
							MUIA_Text_Contents, "Layer: 0/0  ",
						End ),
						Child, ( IPTR )( canvases->txtFrameInf = TextObject,
							MUIA_Text_Contents, "Frame: 0/0  ",
						End ),
						Child, ( IPTR )RectangleObject, MUIA_Weight, 20, End,
					End,
					Child, ( IPTR )HGroup,
						MUIA_InnerTop, 0,
						MUIA_InnerLeft, 0,
						MUIA_InnerRight, 0,
						MUIA_InnerBottom, 0,
						MUIA_Weight, 40,
						Child, ( IPTR )( canvases->btnZoom = SimpleButton ( ( IPTR )"+" ) ),
						Child, ( IPTR )( canvases->btnUnZoom = SimpleButton ( ( IPTR )"-" ) ),
						Child, ( IPTR )( canvases->btnShowAll = SimpleButton ( ( IPTR )"1:1" ) ),
					End,
				End,
			End,
            Child, ( IPTR )( canvases->scrollH = ScrollbarObject, 
                MUIA_Prop_UseWinBorder, MUIV_Prop_UseWinBorder_Bottom,
            End ),
            Child, ( IPTR )( canvases->scrollV = ScrollbarObject, 
                MUIA_Prop_UseWinBorder, MUIV_Prop_UseWinBorder_Right,
            End ),             
		End,
	TAG_END );
	
	// Some values in the area object
	struct RGBitmapData *areaData = INST_DATA ( mcc->mcc_Class, canvases->area );
	areaData->window				= canvases;
	areaData->mousepressed 	    	= FALSE;
	canvases->id 				   	= globalWindowIncrement;
	canvases->layersChg 		   	= TRUE; // initially, say yes layers changed
	canvases->prevBlit 				= ( RectStruct ){ 0, 0, 0, 0 };
	canvases->filename 				= NULL; // set the filename to null *obviously*
	canvases->rRectX 				= 0;
	canvases->rRectY 				= 0;
	canvases->rRectW 				= 0;
	canvases->rRectH 				= 0;
	canvases->contWidth				= 0;
	canvases->contHeight			= 0;
	
	// Add project func to hook
	canvases->projHook.h_Entry = ( HOOKFUNC )projFunc;
	
	// Setup canvas
	
	canvases->canvas = Init_Canvas ( width, height, layers, frames, generateCanvas );
	canvases->nextwin = temp;
	
	if ( canvases->canvas->buffer != NULL )
		setActiveBuffer ( canvases->canvas );
	
	// Add the canvas window to the application gui
	DoMethod ( PaintApp, OM_ADDMEMBER, ( IPTR )canvases->win );
	
	// Generate window object and add it to the application gui
	CreateProjectWindow ( canvases );

	// Add some methods
	DoMethod ( 
		canvases->btnZoom, MUIM_Notify, MUIA_Pressed, FALSE,
		( IPTR )canvases->area, 1, MUIM_ZoomIn
	);
	DoMethod (
		canvases->btnUnZoom, MUIM_Notify, MUIA_Pressed, FALSE,
		( IPTR )canvases->area, 1, MUIM_ZoomOut
	);
	DoMethod (
		canvases->btnShowAll, MUIM_Notify, MUIA_Pressed, FALSE,
		( IPTR )canvases->area, 1, MUIM_ShowAll
	);
	DoMethod (
		canvases->win, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( IPTR )canvases->area, 1, MUIM_CloseCanvasWin
	);
	
	DoMethod ( 
		canvases->win, MUIM_Notify, MUIA_Window_Activate, TRUE,
		( IPTR )canvases->area, 1, MUIM_CanvasActivate
	);
	
	DoMethod (
		canvases->win, MUIM_Notify, MUIA_Window_Activate, FALSE,
		( IPTR )canvases->area, 1, MUIM_CanvasDeactivate
	);

	DoMethod (
		canvases->scrollV, MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
		( IPTR )canvases->area, 1, MUIM_ScrollingNotify
	);
	
	DoMethod (
		canvases->scrollH, MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
		( IPTR )canvases->area, 1, MUIM_ScrollingNotify
	);

	// Set global thingies
	globalActiveWindow = canvases;
	globalActiveCanvas = canvases->canvas;
	
	globalWindowIncrement ++;
}

Object *getCanvaswindowById ( unsigned int id )
{
	WindowList *tmp = canvases;
	
	while ( tmp != NULL )
	{
		if ( tmp->id == id )
			return tmp->win;
		tmp = tmp->nextwin;
	}	
	return NULL;
}

WindowList *getCanvasDataById ( unsigned int id )
{
	WindowList *tmp = canvases;
	
	while ( tmp != NULL )
	{
		if ( tmp->id == id )
			return tmp;
		tmp = tmp->nextwin;
	}
	return NULL;
}

void deleteCanvaswindows ( )
{
	WindowList *tmp = NULL;
	
	while ( canvases != NULL )
	{
		tmp = canvases->nextwin;
		set ( canvases->win, MUIA_Window_Open, FALSE );
		
		DoMethod ( PaintApp, OM_REMMEMBER, ( IPTR )canvases->win );
		if ( canvases->win != NULL ) DoMethod ( PaintApp, OM_REMMEMBER, ( IPTR )canvases->win );
		
		Destroy_Canvas ( canvases->canvas );
		DestroyProjectWindow ( canvases );
		
		if ( canvases->filename != NULL )
			FreeVec ( canvases->filename );
		if ( canvases != NULL )
			FreeVec ( canvases );
		
		canvases = tmp;
	}
}

IPTR removeActiveWindow ( Class *CLASS, Object *self )
{ 
	struct RGBitmapData *data = INST_DATA ( CLASS, self );
	set ( data->window->win, MUIA_Window_Open, FALSE );
    DoMethod ( PaintApp, OM_REMMEMBER, ( IPTR )data->window->win );

	WindowList *tmp = NULL;
	WindowList *ptr = canvases;
	WindowList *prev = NULL;
	
	while ( ptr != NULL )
	{
		tmp = ptr->nextwin;
		
		if ( ptr == globalActiveWindow )
		{	
			// Free memory
			Destroy_Canvas ( ptr->canvas );
			DestroyProjectWindow ( ptr );
			if ( ptr->filename != NULL )
				FreeVec ( ptr->filename );
			if ( ptr != NULL )
				FreeVec ( ptr );
			
			// Disconnect ptr from canvases chain
			if ( prev == NULL )
			{
				canvases = tmp;
				break;
			}
			prev->nextwin = tmp;
			ptr = tmp;
		}
		prev = ptr;
		ptr = tmp;
	}
	
	globalActiveWindow = canvases;
	globalActiveCanvas = NULL;
	
	//TODO: Lets reuse hidden windows in the future   
	data->window->win = NULL;
	return ( IPTR )NULL;
}

void blitAreaRect ( 
	int x, int y, int w, int h, 
	oCanvas* canvas, struct RastPort *rp
)
{
	// Get zoom - bork bork
	int zoom = canvas->zoom;
	
	// Look for conflict with pixel coordinates and canvas dimensions
	// was 0 instead of offset on the two next lines
	
	int testofx = canvas->offsetx / zoom;
	int testofy = canvas->offsety / zoom;
	if ( x < testofx ){ w -= testofx - x; x = testofx; }
	if ( y < testofy ){ h -= testofy - y; y = testofy; }
	if ( x >= ( int )canvas->width ) w = 0; // terminates func
	if ( y >= ( int )canvas->height ) h = 0; // terminates func
	if ( x + w >= ( int )canvas->width ) w = ( int )canvas->width - x;
	if ( y + h >= ( int )canvas->height ) h = ( int )canvas->height - y;
	
	// Look for conflict with onscreen coordinates and widget
	// dimentions
	
	double sx = ( x * zoom ) - ( ( canvas->offsetx / zoom ) * zoom );
	double sy = ( y * zoom ) - ( ( canvas->offsety / zoom ) * zoom );
	double sw = w * zoom;
	double sh = h * zoom;
	if ( sx < 0 ){ sw += sx; sx = 0; }
	if ( sy < 0 ){ sh += sy; sy = 0; }
	if ( sx >= canvas->visibleWidth ) sw = 0;
	if ( sy >= canvas->visibleHeight ) sh = 0;
	if ( sx + sw >= canvas->visibleWidth ) sw = canvas->visibleWidth - sx;
	if ( sy + sh >= canvas->visibleHeight ) sh = canvas->visibleHeight - sy;
	
	// TODO: Check for tool preview size and add to update area
	// dimensions!
	
	if ( sw > 0 && sh > 0 )
	{	
		// Update screenbuffer
		
		redrawScreenbufferRect ( canvas, x, y, w, h, FALSE );
		
		// Write the pixels to rastport
		WritePixelArray ( 
			canvas->screenbuffer,	0, 0, canvas->visibleWidth * 4, rp,
			( int )sx + canvas->winEdgeWidth,
			( int )sy + canvas->winEdgeHeight,
			( int )sw, ( int )sh, RECTFMT_RGBA
		);
	}
}

void importImageRAW ( unsigned int w, unsigned int h, unsigned long long int *buffer )
{
	// We have a filename!!
	char *filename = getFilename ( );
	if ( filename != NULL  )
	{
		if ( getFilesize ( filename ) == 8 * w * h )
		{
			// The aros way!
			BPTR myfile;
			if ( ( myfile = Open ( filename, MODE_OLDFILE ) ) != NULL )
			{
				Read ( myfile, buffer, 8 * w * h );
				Close ( myfile );
				DoMethod ( globalActiveWindow->area, MUIM_Draw );
			}
		}
		FreeVec ( filename );
	}
}

BOOL loadDatatypeImage ( )
{
	char *filename = getFilename ( );
	BOOL result = FALSE;
	
	if ( filename != NULL )
	{
		unsigned int size = getFilesize ( filename );
	
		if ( size > 0 )
		{
			
			struct Library *DatatypeLib = OpenLibrary ( "datatypes.library", 0 );
			
			Object *myDtObj = NewDTObject ( 
				( APTR )filename, 
				DTA_GroupID, GID_PICTURE, 
				PDTA_Remap, TRUE, 
				PDTA_DestMode, PMODE_V43,
				PDTA_Screen, ( IPTR )lunaPubScreen, 
				TAG_END
			);
			
			if ( myDtObj != NULL )
			{
				
				struct BitMapHeader *bHead = NULL;
				struct pdtBlitPixelArray message;
				
				GetDTAttrs ( myDtObj, PDTA_BitMapHeader, &bHead, TAG_END );
				
				// Do some memtests before you commence
				BOOL proceed = TRUE;
				
				unsigned long long int *memtest = 
					AllocVec ( bHead->bmh_Width * bHead->bmh_Height * 8 * 3, MEMF_ANY );
				if ( memtest == NULL )
					proceed = FALSE;
				else FreeVec ( memtest );
			
				unsigned int *bitmap = AllocVec ( bHead->bmh_Width * bHead->bmh_Height * 4, MEMF_ANY );
				if ( bitmap == NULL )
					proceed = FALSE;
				
				// We are proceeding!
				if ( proceed )
				{
					message.MethodID = PDTM_READPIXELARRAY;
					message.pbpa_PixelData = bitmap;
					message.pbpa_PixelFormat = PBPAFMT_RGBA;
					message.pbpa_PixelArrayMod = bHead->bmh_Width * 4;
					message.pbpa_Left = 0;
					message.pbpa_Top = 0;
					message.pbpa_Width = bHead->bmh_Width;
					message.pbpa_Height = bHead->bmh_Height;
					
					DoMethodA ( myDtObj, ( Msg )&message );
					
					addCanvaswindow ( 
						bHead->bmh_Width, bHead->bmh_Height, 1, 1, TRUE
					);
					
					int y = 0; for ( ; y < bHead->bmh_Height; y++ )
					{
						int pixy = y * bHead->bmh_Width;
						int x = 0; for ( ; x < bHead->bmh_Width; x++ )
						{
							unsigned long long int r = 0ULL, g = 0ULL, b = 0ULL, a = 0ULL; 
							
							unsigned int p = bitmap[ pixy + x ];
							
							r = ( ( p << 24 ) >> 24 ) 	* 256;
							g = ( ( p << 16 ) >> 24 ) 	* 256;
							b = ( ( p << 8 ) >> 24 ) * 256;
							if ( bHead->bmh_Depth == 24 )
								a = 65535;
							else
								a = ( p >> 24 ) * 256;
						
							globalActiveCanvas->activebuffer[ pixy + x ] = 
								( r << 48 ) | ( g << 32 ) | ( b << 16 ) | a;
						}
					}
					
					FreeVec ( bitmap );
					DisposeDTObject ( myDtObj );
	                set ( globalActiveWindow->win, MUIA_Window_Title, ( STRPTR )filename );
                    set ( globalActiveWindow->projName, MUIA_String_Contents, ( STRPTR )filename );				
                    result = TRUE;
				}
				else printf ( "Not enough memory.\n" );
			}
			
			CloseLibrary ( DatatypeLib );
		}
		FreeVec ( filename );
	}		
	return result;
}

void removePrevToolPreview ( )
{
	if ( globalActiveWindow->prevBlit.w > 0 && globalActiveWindow->prevBlit.h > 0 )
	{
		// Blit over prev pos
		blitAreaRect (
			globalActiveWindow->prevBlit.x, globalActiveWindow->prevBlit.y,
			globalActiveWindow->prevBlit.w, globalActiveWindow->prevBlit.h,
			globalActiveCanvas, _rp ( globalActiveWindow->area )
		);
		// Clear prevblit coords
		globalActiveWindow->prevBlit.x = 0;
		globalActiveWindow->prevBlit.y = 0;
		globalActiveWindow->prevBlit.w = 0;
		globalActiveWindow->prevBlit.h = 0;
	}
}

void callToolPreview ( )
{	
	if ( globalActiveWindow == NULL )
		return;
	
	// Some vars to be used
	int x = 0, y = 0, w = 1, h = 1; double offx = 0.0, offy = 0.0;
	
	switch ( globalCurrentTool )
	{
		case LUNA_TOOL_FILL:
		case LUNA_TOOL_COLORPICKER:
		
			x = cMouseX;
			y = cMouseY;
			w = 1;
			h = 1;
		
			break;
		
		case LUNA_TOOL_BRUSH:
		
			offx = ( double )brushTool.width / 2;
			offy = ( double )brushTool.height / 2;
			x = cMouseX - offx;
			y = cMouseY - offy;
			w = brushTool.width; 
			h = brushTool.height;
			break;
			
		case LUNA_TOOL_LINE:
		
			if ( lineTool.mode == 0 )
			{
				offx = ( double )brushTool.width / 2;
				offy = ( double )brushTool.height / 2;
				x = cMouseX - offx;
				y = cMouseY - offy;
				w = brushTool.width; 
				h = brushTool.height;
			}
			else if ( lineTool.mode == 1 )
			{
				x = lineTool.ox;
				y = lineTool.oy;
				w = lineTool.w;
				h = lineTool.h;	
			}
		 	break;
		 	
		case LUNA_TOOL_RECTANGLE:
		
			if ( rectangleTool.mode == 0 )
			{
				offx = ( double )brushTool.width / 2;
				offy = ( double )brushTool.height / 2;
				x = cMouseX - offx;
				y = cMouseY - offy;
				w = brushTool.width; 
				h = brushTool.height;
			}
			else if ( rectangleTool.mode == 1 )
			{
				x = rectangleTool.ox;
				y = rectangleTool.oy;
				w = rectangleTool.w;
				h = rectangleTool.h;	
			}
		 	break;
		 
		 case LUNA_TOOL_CIRCLE:
		
			if ( circleTool.mode == 0 )
			{
				offx = ( double )brushTool.width / 2;
				offy = ( double )brushTool.height / 2;
				x = cMouseX - offx;
				y = cMouseY - offy;
				w = brushTool.width; 
				h = brushTool.height;
			}
			else if ( circleTool.mode == 1 )
			{
				x = circleTool.ox;
				y = circleTool.oy;
				w = circleTool.bufwidth;
				h = circleTool.bufheight;	
			}
		 	break;
		 
		 case LUNA_TOOL_CLIPBRUSH:
		
			if ( clipbrushTool.mode == 0 )
			{
				x = cMouseX;
				y = cMouseY;
				w = 1; 
				h = 1;
			}
			else if ( clipbrushTool.mode == 1 )
			{
				x = clipbrushTool.ox;
				y = clipbrushTool.oy;
				w = clipbrushTool.w;
				h = clipbrushTool.h;	
			}
		 	break;
			
		default: break;
	}
	
	if ( !w || !h ) return;
	removePrevToolPreview ( );
	blitAreaRect ( x - 1, y - 1, w + 2, h + 2, globalActiveCanvas, _rp ( globalActiveWindow->area ) );
	globalActiveWindow->prevBlit = ( RectStruct ){ x - 1, y - 1, w + 2, h + 2 };
}
