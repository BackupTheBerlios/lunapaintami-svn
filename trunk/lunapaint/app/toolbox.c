/****************************************************************************
*                                                                           *
* toolbox.c -- Lunapaint, http://www.sub-ether.org/lunapaint                *
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

#include "toolbox.h"

AROS_UFH3 ( void, getMenu_func,
	AROS_UFHA ( struct Hook*, h, A0 ),
	AROS_UFHA ( APTR, obj, A2 ),
	AROS_UFHA ( APTR, param, A1 )
)
{
	AROS_USERFUNC_INIT
	
	checkMenuEvents ( *( int* )param );
	
	AROS_USERFUNC_EXIT
}

/* Get the opacity mode (add/keep) */
AROS_UFH3 ( void, getOpacMode_func,
	AROS_UFHA ( struct Hook*, h, A0 ),
	AROS_UFHA ( APTR, obj, A2 ),
	AROS_UFHA ( APTR, param, A1 )
)
{
	AROS_USERFUNC_INIT
	
	brushTool.opacitymode = XGET( tbxCycColorCtrl, MUIA_Cycle_Active );
	
	AROS_USERFUNC_EXIT
}

AROS_UFH3 ( void, getFill_func, 
	AROS_UFHA ( struct Hook *, h, A0 ),
	AROS_UFHA ( APTR, obj, A2 ),
	AROS_UFHA ( APTR, param, A1 )
)
{
	AROS_USERFUNC_INIT
	
	GetDrawFillState ( );
	
	AROS_USERFUNC_EXIT
}

AROS_UFH3 ( void, brushOptions_func,
	AROS_UFHA ( struct Hook*, h, A0 ),
	AROS_UFHA ( APTR, obj, A2 ),
	AROS_UFHA ( APTR, param, A1 )
)
{
	AROS_USERFUNC_INIT
	
	int val = *( int *)param;
	
	switch ( val )
	{
		case SET_ANTIALIAS:
			set ( antiImage, MUIA_Background, MUII_FILL );
			set ( antiOffImage, MUIA_Background, MUII_ButtonBack );
			brushTool.antialias = TRUE;
			break;
		case SET_ANTIALIASOFF:
			set ( antiImage, MUIA_Background, MUII_ButtonBack );
			set ( antiOffImage, MUIA_Background, MUII_FILL );
			brushTool.antialias = FALSE;
			break;
		case SET_FEATHERED:
			set ( featherImage, MUIA_Background, MUII_FILL );
			set ( solidImage, MUIA_Background, MUII_ButtonBack );
			brushTool.feather = TRUE;
			break;
		case SET_FEATHEREDOFF:
			set ( featherImage, MUIA_Background, MUII_ButtonBack );
			set ( solidImage, MUIA_Background, MUII_FILL );
			brushTool.feather = FALSE;
			break;
		default: break;
	}
	DoMethod ( tbxAreaPalette, MUIM_AlterBrushShape );
	
	AROS_USERFUNC_EXIT
}

BOOPSI_DISPATCHER ( IPTR, tbxPreview, CLASS, self, message )
{
	switch ( message->MethodID )
	{
		case MUIM_Draw:
			return ( IPTR )tbxPaintPreview ( );
		default:
			return DoSuperMethodA ( CLASS, self, message );
	}
	return ( IPTR )NULL;
}
BOOPSI_DISPATCHER_END

void prevPaletteColor ( )
{
	currColor = ( currColor - 1 ) % 256;
	DoMethod ( tbxAreaPalette, MUIM_Draw );
	if ( globalBrushMode == 0 )
		makeToolBrush ( );
}
void nextPaletteColor ( )
{
	currColor = ( currColor + 1 ) % 256;
	DoMethod ( tbxAreaPalette, MUIM_Draw );
	if ( globalBrushMode == 0 )
		makeToolBrush ( );
}

IPTR tbxPaintPreview ( )
{
	unsigned int w = XGET ( tbxAreaPreview, MUIA_Width );
	unsigned int h = XGET ( tbxAreaPreview, MUIA_Height );
	unsigned int t = XGET ( tbxAreaPreview, MUIA_TopEdge );
	unsigned int l = XGET ( tbxAreaPreview, MUIA_LeftEdge );
	unsigned int rectsize = w * h;
	
	unsigned int bpw = h;
	
	if ( PreviewRectData != NULL )
		FreeVec ( PreviewRectData );
	PreviewRectData = AllocVec ( rectsize * 4, MEMF_ANY );
	int i = 0; for ( ; i < rectsize; i++ )
	{
		int x = i % w;
		int y = i / w;
		
		// Draw brush
		if ( x >= 0 && x < bpw && y >= 0 && y < h && brushTool.buffer != NULL )
		{
			int offx = ( ( double )x / bpw ) * brushTool.width;
			int offy = ( ( double )y / h ) * brushTool.height;
			
			rgbaData rgba = canvasColorToRGBA ( 
				brushTool.buffer[ offy * brushTool.width + offx ]
			);
			
			if ( rgba.a == 0 )
			{
				PreviewRectData[ i ] = 0x000000;
			}
			else
			{
				// Blend towards gray
				rgba.r -= ( int )( ( double )( rgba.r ) / rgba.a );			
				rgba.g -= ( int )( ( double )( rgba.g ) / rgba.a );
				rgba.b -= ( int )( ( double )( rgba.b ) / rgba.a );
				PreviewRectData[ i ] = rgba.r | ( rgba.g << 8 ) | ( rgba.b << 16 ) | 0;
			}
		}
		else
			PreviewRectData[ i ] = 0x707070;
	}
	
	// UPDATE
	WritePixelArray ( 
		PreviewRectData, 0, 0, w * 4, _rp ( tbxAreaPreview ), l, t, w, h, RECTFMT_RGBA
	);
	
	return ( IPTR )NULL;
}

BOOPSI_DISPATCHER ( IPTR, tbxPalette, CLASS, self, message )
{
	switch ( message->MethodID )
	{
		case MUIM_Draw:
			return ( IPTR )tbxPaletteRedraw ();
		
		case MUIM_HandleInput:
			return ( IPTR )tbxPaletteThink ( ( struct MUIP_HandleInput* )message );
			
		case MUIM_Setup:
			MUI_RequestIDCMP( self, IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY );
			return DoSuperMethodA ( CLASS, self, message );	
		
		case MUIM_OpenPaletteEditor:
			set ( paletteWindow, MUIA_Window_Open, TRUE );
			break;
		
		case MUIM_ClearActiveCanvas:
			
			// Clear active canvas and redraw...
			if ( globalActiveCanvas != NULL )
			{
				int i; for ( i = 0; i < globalActiveCanvas->height * globalActiveCanvas->width; i++ )
					globalActiveCanvas->activebuffer[ i ] = TRANSCOLOR;
				globalActiveCanvas->winHasChanged = TRUE;
				DoMethod ( globalActiveWindow->area, MUIM_Draw, NULL );
			}
			break;
		
		case MUIM_SetPaintMode:
			
			if ( 1 )
			{
				LONG num = 0; get ( tbxCycPaintMode, MUIA_Cycle_Active, &num );
				switch ( num )
				{
					case 0:
						brushTool.paintmode = LUNA_PAINTMODE_NORMAL;
						break;
					case 1:
						brushTool.paintmode = LUNA_PAINTMODE_COLOR;
						break;
					case 2:
						brushTool.paintmode = LUNA_PAINTMODE_BLUR;
						break;
					case 3:
						brushTool.paintmode = LUNA_PAINTMODE_SMUDGE;
						break;
					case 4:
						brushTool.paintmode = LUNA_PAINTMODE_LIGHTEN;
						break;
					case 5:
						brushTool.paintmode = LUNA_PAINTMODE_DARKEN;
						break;
					case 6:
						brushTool.paintmode = LUNA_PAINTMODE_COLORIZE;
						break;
					case 7:
						brushTool.paintmode = LUNA_PAINTMODE_ERASE;
						break;
					case 8:
						brushTool.paintmode = LUNA_PAINTMODE_UNERASE;
						break;
					default: return 0;
				}
				if ( globalBrushMode == 0 )
					makeToolBrush ( );
			}
			break;
		
		case MUIM_AlterBrushShape:
			{
				// Get brushsize from toolbox and set it
				int tempWidth = 0, tempHeight = 0, tempOpacity = 0, tempPower = 0;
				get ( tbxSlider_Brushdiameter, MUIA_Numeric_Value, &tempWidth );
				get ( tbxSlider_Brushdiameter, MUIA_Numeric_Value, &tempHeight );
				get ( tbxSlider_Brushopacity, MUIA_Numeric_Value, &tempOpacity );
				get ( tbxSlider_Brushincrement, MUIA_Numeric_Value, &tempPower );
				brushTool.step = XGET ( tbxSlider_Brushstep, MUIA_Numeric_Value );
				
				// Opacity and power always change
				brushTool.opacity = tempOpacity; 
				brushTool.power = tempPower;
				
				// Alter size and start making a brush
				brushTool.width = tempWidth; 
				brushTool.height = tempHeight; 
				makeToolBrush ( );
				return ( IPTR )NULL;
			}
			break;
		case MUIM_AlterBrushStrength:
			{
				// Get brushsize from toolbox and set it
				int tempOpacity = 0, tempPower = 0;
				get ( tbxSlider_Brushopacity, MUIA_Numeric_Value, &tempOpacity );
				get ( tbxSlider_Brushincrement, MUIA_Numeric_Value, &tempPower );
				brushTool.opacity = tempOpacity; 
				brushTool.power = tempPower;
				return ( IPTR )NULL;
			}
			break;
		case MUIM_AlterBrushStep:
			brushTool.step = XGET( tbxSlider_Brushstep, MUIA_Numeric_Value );	
			break;
		
		case MUIM_SetGlobalGrid:
			{
				int num = 0; 
				get ( tbxCycGrid, MUIA_Cycle_Active, &num );
				if ( num == 0 )
					globalGrid = 0;
				else
					globalGrid = 1;
			}
			break;
			
		case MUIM_SetGridSize:
			{
				int num = 0;
				get ( tbxCyc_GridSize, MUIA_Cycle_Active, &num );
				globalCurrentGrid = num + 2;
			}
			break;
			
		case MUIM_NewProject:
			{
				int width = 0, height = 0, frames = 0;
				get ( nwStringWidth, MUIA_String_Integer, &width );
				get ( nwStringHeight, MUIA_String_Integer, &height );
				get ( nwStringFrames, MUIA_String_Integer, &frames );
				
				// TODO: Add memory checks!!!
				if ( width > 0 && height > 0 && frames > 0 )
				{
					// TODO: Could projectName remain used?
					addCanvaswindow ( width, height, 1, frames, TRUE ); 
					STRPTR txtbuf = ( STRPTR )NULL;
					get (	nwStringPrjName, MUIA_String_Contents, &txtbuf );
				  	set ( globalActiveWindow->win, MUIA_Window_Title, ( IPTR )txtbuf );
					set ( globalActiveWindow->win, MUIA_Window_Open, TRUE );
					set ( nwWindow, MUIA_Window_Open, FALSE );
				}
			}
			break;
			
		case MUIM_EffectOffset:
			
			if ( globalActiveCanvas != NULL )
			{
				int x = 0, y = 0;
				get ( offsetWindowX, MUIA_String_Integer, &x );
				get ( offsetWindowY, MUIA_String_Integer, &y );
				effectOffset ( x, y, globalActiveCanvas );
				globalActiveCanvas->winHasChanged = TRUE;
				DoMethod ( globalActiveWindow->area, MUIM_Draw, FALSE );
			}
			break;
		
		case MUIM_SetColorMode:
			{
				int num = 0; get ( tbxCyc_PaletteSnap, MUIA_Cycle_Active, &num );
				if ( num == 0 )
					globalColorMode = LUNA_COLORMODE_64BIT;
				else if ( num == 1 )
					globalColorMode = LUNA_COLORMODE_SNAP;
			}
			break;
		
		default:
			return DoSuperMethodA ( CLASS, self, message );
	}
	return ( IPTR )0;
}
BOOPSI_DISPATCHER_END

IPTR tbxPaletteRedraw ( )
{
	int topedge = 0, leftedge = 0, w = 0, h = 0;
	get ( tbxAreaPalette, MUIA_TopEdge, &topedge );
	get ( tbxAreaPalette, MUIA_LeftEdge, &leftedge );
	get ( tbxAreaPalette, MUIA_Width, &w );
	get ( tbxAreaPalette, MUIA_Height, &h );
	
	unsigned int *tmpRect = AllocVec ( 4 * ( w * h ), MEMF_ANY );

	// Set how broad each color rect is
	float segW = ( float )w / 16;
	int segH = ( float )h / 16;
 	int x1 = segW * ( currColor % 16 );
	int y1 = segH * ( currColor / 16 );

	// Draw color rects
	int y = 0; for ( ; y < h; y++ )
	{
		int posy = y * w;
		int x = 0; for ( ; x < w; x++ )
		{
			int offset = ( ( y / segH ) * ( w / segW ) ) + ( x / segW );
			if ( 
				( x >= x1 && x < x1 + segW && ( y == y1 || y == y1 + segH - 1 ) ) || 
				( y >= y1 && y < y1 + segH && ( x == x1 || x == x1 + ( int )segW ) )
			)
			{
				int sw = ( y + x ) % 2;
				tmpRect[ posy + x ] = ( sw == 1 ) ? 0x00ffffff : 0x00000000;
			}
			else
			{
				tmpRect[ posy + x ] = *( globalPalette + offset );
			}
		}
	}

	WritePixelArray ( 
		tmpRect, 0, 0, w * 4, 
		_rp ( tbxAreaPalette ), leftedge, topedge, w, h, 
		RECTFMT_RGBA 
	);
	
	FreeVec ( tmpRect );
	
	// Set color
	globalColor = globalPalette[ currColor ];
	
	return ( IPTR )NULL;
}

IPTR tbxPaletteThink ( struct MUIP_HandleInput *msg )
{
	int topedge = 0, leftedge = 0, w = 0, h = 0;
	get ( tbxAreaPalette, MUIA_TopEdge, &topedge );
	get ( tbxAreaPalette, MUIA_LeftEdge, &leftedge );
	get ( tbxAreaPalette, MUIA_Width, &w );
	get ( tbxAreaPalette, MUIA_Height, &h );

	// Click counter for doubleclicking
	if ( mouseClickCount == 0 )
		tbxPaletteClickMode = 0;

	if ( msg->imsg )
	{
		switch ( msg->imsg->Class )
		{
			case IDCMP_MOUSEBUTTONS:
				if ( 
					( int )msg->imsg->MouseX - leftedge >= 0 &&
					( int )msg->imsg->MouseX - leftedge < w &&
					( int )msg->imsg->MouseY - topedge >= 0 &&
					( int )msg->imsg->MouseY - topedge < h
				)
				{
					// Set how broad each color rect is
					float w = ( float )XGET ( tbxAreaPalette, MUIA_Width );
					int y = msg->imsg->MouseY - topedge;
					int x = msg->imsg->MouseX - leftedge;
					float wu = w / 16;
					int segh = h / 16;
					
					currColor = ( 16 * ( y / segh ) ) + ( x / wu );
					tbxPaletteRedraw ( ); DoMethod ( paletteRect, MUIM_Draw );
					
					// If we are using normal brushes, make a new 
					// brush with the new color
					if ( globalBrushMode == 0 ) makeToolBrush ( );
						
					if ( msg->imsg->Code == SELECTDOWN )
					{
						// On singleclick, make double click possible
						if ( tbxPaletteClickMode == 0 )
						{
							tbxPaletteClickMode = 1;
							mouseClickCount = MOUSE_DoubleClickTime;
						}
						// On doubleclick, open palette editor and make single click possible
						else if ( tbxPaletteClickMode == 1 && mouseClickCount > 0 )
						{
							tbxPaletteClickMode = 0;
							mouseClickCount = 0;
							set ( paletteWindow, MUIA_Window_Open, TRUE );
						}	
					}
				}
			break;
			
			default: break;
		}
	}
	return ( IPTR )NULL;
}


void Init_ToolboxWindow ( )
{	
	// Palette widget
	struct MUI_CustomClass *mcc = MUI_CreateCustomClass ( 
		NULL, MUIC_Rectangle, NULL, 0, tbxPalette
	);
	
	// Brush preview widget
	struct MUI_CustomClass *mcc_preview = MUI_CreateCustomClass (
		NULL, MUIC_Rectangle, NULL, 0, tbxPreview
	);
	
	static const char *arr_fillmode[] = { "Draw only", "Fill shapes", "Draw/Fill", NULL };
	static const char *arr_colorctrl[] = { "Add alpha", "Keep alpha", NULL };
	static const char *arr_grid[] = { "Grid off", "Grid on", NULL };
	static const char *arr_grdz[] = { "2", "3", "4", "5", "6", "7", "8", "9", "10", NULL };
	static const char *arr_drawmodes[] = { 
		"Normal", "Color", "Blur", "Smudge", "Lighten", "Darken", "Colorize", "Erase", "UnErase", NULL 
	};
	static const char *arr_pltsnap[] = { "64-bit paint mode", "Snap to palette", NULL };
	
	offsetWindow = WindowObject,
		MUIA_Window_Title, ( IPTR )"Set layer offset",
		MUIA_Window_ScreenTitle, ( IPTR )"Set layer offset",
		MUIA_Window_Screen, ( IPTR )lunaPubScreen,
		WindowContents, ( IPTR )VGroup,
			Child, ( IPTR )GroupObject,
				MUIA_Frame, MUIV_Frame_Group,
				Child, ( IPTR )TextObject,
					MUIA_Text_Contents, ( IPTR )"Please write in the coodinates\nby which you want to skew the\nactive layer.",
				End,
				Child, ( IPTR )HGroup,
					Child, ( IPTR )TextObject,
						MUIA_Text_Contents, ( IPTR )"X Offset:",
						MUIA_Weight, 50,
					End,
					Child, ( IPTR )( offsetWindowX = StringObject,
						MUIA_Weight, 60,
						MUIA_Frame, MUIV_Frame_String,
						MUIA_String_Format, MUIV_String_Format_Right,
						MUIA_String_MaxLen, 4,
						MUIA_CycleChain, 1,
					End ),
				End,
				Child, ( IPTR )HGroup,
					Child, ( IPTR )TextObject,
						MUIA_Text_Contents, ( IPTR )"Y Offset:",
						MUIA_Weight, 50,
					End,
					Child, ( IPTR )( offsetWindowY = StringObject,
						MUIA_Weight, 60,
						MUIA_Frame, MUIV_Frame_String,
						MUIA_String_Format, MUIV_String_Format_Right,
						MUIA_String_MaxLen, 4,
						MUIA_CycleChain, 1,
					End ),
				End,
			End,
			Child, ( IPTR )HGroup,
				Child, ( IPTR )( offsetWindowOk = SimpleButton ( ( IPTR )"Ok" ) ),
				Child, ( IPTR )( offsetWindowCancel = SimpleButton ( ( IPTR )"Cancel" ) ),
			End,
		End,
	End;
	
	// The toolbox
	toolbox = WindowObject,
		MUIA_Window_Title, ( IPTR )"Toolbox",
		MUIA_Window_ScreenTitle, ( IPTR )LUNA_SCREEN_TITLE,
		MUIA_Window_CloseGadget, FALSE,
		MUIA_Window_SizeGadget, FALSE,
		MUIA_Window_Width, 150,
		MUIA_Window_Screen, ( IPTR )lunaPubScreen,
		MUIA_Window_LeftEdge, 0,
		MUIA_Window_TopEdge, ( lunaPubScreen->BarHeight + 1 ),
		WindowContents, ( IPTR )VGroup,
			Child, ( IPTR )VGroup,
				Child, ( IPTR )( toolboxTopGroup = GroupObject,
					Child, ( IPTR )GroupObject,
						MUIA_FrameTitle, ( IPTR )"Tool palette:",
						MUIA_Frame, MUIV_Frame_Group,
						Child, ( IPTR )VGroup,
							MUIA_Group_HorizSpacing, 0,
							MUIA_Group_VertSpacing, 0,
							Child, ( IPTR )HGroup,
								MUIA_Group_HorizSpacing, 0,
								Child, ( IPTR )( tbxBtn_draw = 
									ImageButton ( "", ( IPTR )"Lunapaint:data/draw.png" ) ),
								Child, ( IPTR )( tbxBtn_line = 
									ImageButton ( "", ( IPTR )"Lunapaint:data/line.png" ) ),
								Child, ( IPTR )( tbxBtn_rectangle = 
									ImageButton ( "", ( IPTR )"Lunapaint:data/rectangle.png" ) ),
								Child, ( IPTR )( tbxBtn_circle = 
									ImageButton ( "", ( IPTR )"Lunapaint:data/circle.png" ) ),
							End,
							Child, ( IPTR )HGroup,
								MUIA_Group_HorizSpacing, 0,
								Child, ( IPTR )( tbxBtn_polygon = 
									ImageButton ( "", ( IPTR )"Lunapaint:data/polygon.png" ) ),
								Child, ( IPTR )( tbxBtn_fill = 
									ImageButton ( "", ( IPTR )"Lunapaint:data/fill.png" ) ),
								Child, ( IPTR )( tbxBtn_getbrush =
									ImageButton ( "", ( IPTR )"Lunapaint:data/getbrush.png" ) ),
								Child, ( IPTR )( tbxBtn_pickcolor = 
									ImageButton ( "", ( IPTR )"Lunapaint:data/pick_color.png" ) ),
							End,
						End,
					End,
					Child, ( IPTR )GroupObject,
						MUIA_Frame, MUIV_Frame_Group,
						MUIA_FrameTitle, ( IPTR )"Brush options:",
						Child, ( IPTR )HGroup,
							MUIA_Frame, MUIV_Frame_None,
							Child, RectangleObject,
								MUIA_Frame, MUIV_Frame_None, 
								MUIA_Weight, 2,
							End,
							Child, ( IPTR )VGroup,
								MUIA_Weight, 32,
								Child, ( IPTR )TextObject, MUIA_Text_Contents, ( IPTR )"Size:", End,
								Child, ( IPTR )( tbxSlider_Brushdiameter = NumericbuttonObject,
									MUIA_Numeric_Min, 1, 
									MUIA_Numeric_Max, 100,
									MUIA_Numeric_Value, 1,
								End ),
							End,
							Child, ( IPTR )VGroup,
								MUIA_Weight, 32,
								Child, ( IPTR )TextObject, MUIA_Text_Contents, ( IPTR )"Opacity:", End,
								Child, ( IPTR )( tbxSlider_Brushopacity = NumericbuttonObject, 
									MUIA_Numeric_Min, 1,
									MUIA_Numeric_Max, 255,
									MUIA_Numeric_Value, 255,
								End ),
							End,
							Child, ( IPTR )VGroup,
								MUIA_Weight, 32,
								Child, ( IPTR )TextObject, MUIA_Text_Contents, ( IPTR )"Power:", End,
								Child, ( IPTR )( tbxSlider_Brushincrement = NumericbuttonObject, 
									MUIA_Numeric_Min, 1,
									MUIA_Numeric_Max, 100,
									MUIA_Numeric_Value, 100,
								End ),
							End,
							Child, ( IPTR )VGroup,
								MUIA_Weight, 32,
								Child, ( IPTR )TextObject, MUIA_Text_Contents, ( IPTR )"Step:", End,
								Child, ( IPTR )( tbxSlider_Brushstep = NumericbuttonObject, 
									MUIA_Numeric_Min, 1,
									MUIA_Numeric_Max, 50,
									MUIA_Numeric_Value, 1,
								End ),
							End,
							Child, RectangleObject,
								MUIA_Frame, MUIV_Frame_None, 
								MUIA_Weight, 2,
							End,
						End,
						Child, ( IPTR )( brushOpGroup = GroupObject,
							MUIA_Group_Horiz, TRUE,
							MUIA_Group_HorizSpacing, 0,
							MUIA_Frame, MUIV_Frame_None,
							InnerSpacing( 0, 0 ),
							MUIA_Group_SameSize, TRUE,
							Child, ( IPTR )GroupObject,
								MUIA_Frame, MUIV_Frame_Button,
								Child, ( IPTR )( tbxAreaPreview = NewObject (
									mcc_preview->mcc_Class, NULL,
									InnerSpacing( 0, 0 ),
									MUIA_FixHeight, 24,
									MUIA_FixWidth, 24,
									TAG_DONE
								) ),
							End,
							Child, ( IPTR )( 
								antiImage = ImageButton ( "", ( IPTR )"Lunapaint:data/antialias.png" ) 
							),
							Child, ( IPTR )( 
								antiOffImage = ImageButton ( "", ( IPTR )"Lunapaint:data/antialias_off.png" ) 
							),
							Child, ( IPTR )( 
								featherImage = ImageButton ( "", ( IPTR )"Lunapaint:data/feathered.png" ) 
							),
							Child, ( IPTR )( 
								solidImage = ImageButton ( "", ( IPTR )"Lunapaint:data/solid.png" ) 
							),
						End ),
						Child, ( IPTR )GroupObject,
							MUIA_Frame, MUIV_Frame_None,						
							MUIA_Group_Horiz, TRUE,
							Child, ( IPTR )VGroup,
								Child, ( IPTR )HGroup,
									Child, ( IPTR )TextObject, 
										MUIA_Text_Contents, ( IPTR )"Paintmode:", 
										MUIA_Weight, 60,
									End,
									Child, ( IPTR )( tbxCycPaintMode = CycleObject,
										MUIA_Cycle_Entries, &arr_drawmodes,
										MUIA_Weight, 40,
									End ),
								End,
								Child, ( IPTR )HGroup,
									Child, ( IPTR )( tbxCycFillmode = CycleObject,
										MUIA_Cycle_Entries, ( IPTR )arr_fillmode,
										MUIA_Cycle_Active, 0,
										MUIA_Weight, 50,
									End ),
									Child, ( IPTR )( tbxCycColorCtrl = CycleObject,
										MUIA_Cycle_Entries, ( IPTR )arr_colorctrl,
										MUIA_Cycle_Active, 0,
										MUIA_Weight, 50,
									End ),
								End,
							End,
						End,
						Child, ( IPTR )HGroup,
							MUIA_Frame, MUIV_Frame_None,
							Child, ( IPTR )( tbxCycGrid = CycleObject,
								MUIA_Cycle_Entries, &arr_grid,
								MUIA_Weight, 60,
							End ),
							Child, ( IPTR )( tbxCyc_GridSize = CycleObject,
								MUIA_Cycle_Entries, &arr_grdz,
								MUIA_Weight, 40,
							End ),
						End,
					End,
				End ),
				Child, ( IPTR )GroupObject,
					MUIA_Frame, MUIV_Frame_Group,
					MUIA_FrameTitle, ( IPTR )"Palette:",
					Child, ( IPTR )HGroup,
						InnerSpacing(0,0),
						MUIA_Frame, MUIV_Frame_Group,
						Child, ( IPTR )( tbxAreaPalette = NewObject (
							mcc->mcc_Class, NULL,
							MUIA_FixHeight, 96,
							TAG_DONE
						) ),
					End,
					Child, ( IPTR )( tbxCyc_PaletteSnap = CycleObject,
						MUIA_Cycle_Entries, &arr_pltsnap,
					End ),
				End,
			End,
		End,
	End;

	// Move to left edge of screen (todo: improve this!)
	int toolboxWidth = 0; get ( toolbox, MUIA_Window_Width, &toolboxWidth );
	set ( toolbox, MUIA_Window_LeftEdge, ( lunaPubScreen->Width - toolboxWidth ) );
	
	// set buttons
	set ( antiImage, MUIA_Background, MUII_FILL );
	set ( featherImage, MUIA_Background, MUII_FILL );
	
	// Set initial gridsize
	set ( tbxCyc_GridSize, MUIA_Cycle_Active, 8 );
	globalCurrentGrid = 10;
}

void Exit_ToolboxWindow ( )
{
	if ( PreviewRectData != NULL )
		FreeVec ( PreviewRectData );
}

void Init_ToolboxMethods ( )
{
	// Set up our hooks
	getMenu_hook.h_Entry = ( HOOKFUNC )getMenu_func;
	brushOptions_hook.h_Entry = ( HOOKFUNC )brushOptions_func;
	getFill_hook.h_Entry = ( HOOKFUNC )getFill_func;
	getOpacMode_hook.h_Entry = ( HOOKFUNC )getOpacMode_func;
	
	// Methods
	DoMethod ( 
		antiImage, MUIM_Notify, MUIA_Pressed, TRUE,
		antiImage, 3, MUIM_CallHook, &brushOptions_hook, SET_ANTIALIAS
	);
	DoMethod (
		antiOffImage, MUIM_Notify, MUIA_Pressed, TRUE,
		antiOffImage, 3, MUIM_CallHook, &brushOptions_hook, SET_ANTIALIASOFF
	);
	DoMethod ( 
		featherImage, MUIM_Notify, MUIA_Pressed, FALSE,
		featherImage, 3, MUIM_CallHook, &brushOptions_hook, SET_FEATHERED
	);
	DoMethod ( 
		solidImage, MUIM_Notify, MUIA_Pressed, FALSE,
		solidImage, 3, MUIM_CallHook, &brushOptions_hook, SET_FEATHEREDOFF
	);
	DoMethod ( 
		tbxCycColorCtrl, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		tbxCycColorCtrl, 2, MUIM_CallHook, &getOpacMode_hook
	);	
	
	/*
		Draw tools and bubble help
	*/
	DoMethod ( 
		tbxBtn_draw, MUIM_Notify, 
		MUIA_Pressed, FALSE,
		paletteRect, 1, MUIM_SetTool_Draw
	);
	DoMethod ( tbxBtn_draw, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Draw" );
	
	DoMethod ( 
		tbxBtn_fill, MUIM_Notify, 
		MUIA_Pressed, FALSE,
		paletteRect, 1, MUIM_SetTool_Fill
	);
	DoMethod ( tbxBtn_fill, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Flood fill" );
	
	DoMethod (
		tbxBtn_line, MUIM_Notify,
		MUIA_Pressed, FALSE,
		paletteRect, 1, MUIM_SetTool_Line
	);
	DoMethod ( tbxBtn_line, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Line" );
	
	DoMethod ( 
		tbxBtn_rectangle, MUIM_Notify, 
		MUIA_Pressed, FALSE,
		paletteRect, 1, MUIM_SetTool_Rectangle
	);
	DoMethod ( tbxBtn_rectangle, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Rectangle" );
	
	DoMethod ( 
		tbxBtn_circle, MUIM_Notify, 
		MUIA_Pressed, FALSE,
		paletteRect, 1, MUIM_SetTool_Circle
	);
	DoMethod ( tbxBtn_circle, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Circle" );
	
	DoMethod ( 
		tbxBtn_getbrush, MUIM_Notify, 
		MUIA_Pressed, FALSE,
		paletteRect, 1, MUIM_SetTool_ClipBrush
	);
	DoMethod ( tbxBtn_getbrush, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Cut out brush" );
	
	DoMethod (
		tbxBtn_pickcolor, MUIM_Notify,
		MUIA_Pressed, FALSE,
		paletteRect, 1, MUIM_SetTool_Colorpicker
	);
	DoMethod ( tbxBtn_pickcolor, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Pick color" );
	
	/*
		Other
	*/
	DoMethod ( 
		tbxSlider_Brushdiameter, MUIM_Notify,
		MUIA_Numeric_Value, MUIV_EveryTime,
		tbxAreaPalette, 1, MUIM_AlterBrushShape
	);
	DoMethod ( 
		tbxSlider_Brushopacity, MUIM_Notify,
		MUIA_Numeric_Value, MUIV_EveryTime,
		tbxAreaPalette, 1, MUIM_AlterBrushStrength
	);
	DoMethod ( 
		tbxSlider_Brushincrement, MUIM_Notify,
		MUIA_Numeric_Value, MUIV_EveryTime,
		tbxAreaPalette, 1, MUIM_AlterBrushStrength
	);
	DoMethod ( 
		tbxSlider_Brushstep, MUIM_Notify,
		MUIA_Numeric_Value, MUIV_EveryTime,
		tbxAreaPalette, 1, MUIM_AlterBrushStep
	);
	
	DoMethod (
		offsetWindowOk, MUIM_Notify, MUIA_Pressed, FALSE,
		tbxAreaPalette, 1, MUIM_EffectOffset
	);
	
	DoMethod (
		tbxCyc_PaletteSnap, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		tbxAreaPalette, 1, MUIM_SetColorMode
	);
	
	// When you access the menu
	DoMethod (
		PaintApp, MUIM_Notify, MUIA_Application_MenuAction, MUIV_EveryTime,
		( IPTR )toolbox, 3, MUIM_CallHook, ( IPTR )&getMenu_hook, MUIV_EveryTime
	);
	DoMethod (
		tbxCycPaintMode, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		tbxAreaPalette, 1, MUIM_SetPaintMode
	);
	DoMethod (
		tbxCycGrid, MUIM_Notify, 
		MUIA_Cycle_Active, MUIV_EveryTime,
		tbxAreaPalette, 1, MUIM_SetGlobalGrid
	);
	DoMethod (
		tbxCyc_GridSize, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		tbxAreaPalette, 1, MUIM_SetGridSize
	);
	
	// Offset window
	DoMethod (
		offsetWindowCancel, MUIM_Notify, MUIA_Pressed, FALSE,
		offsetWindow, 3, MUIM_Set, MUIA_Window_Open, FALSE
	);
	DoMethod ( 
		offsetWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		offsetWindow, 3, MUIM_Set, MUIA_Window_Open, FALSE
	);
	
	// Fill state
	DoMethod ( 
		tbxCycFillmode, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
		tbxCycFillmode, 2, MUIM_CallHook, &getFill_hook
	);
	
	
	// Set default brush options and update the brush
	brushTool.antialias = 1;
	brushTool.feather = 1;
	brushTool.width = 1;
	brushTool.height = 1;
	brushTool.opacity = 255;
	brushTool.step = 1;
	brushTool.power = 100;
}

void checkMenuEvents ( int udata )
{
	switch ( udata )
	{
		case 597:
			// Load
			if ( 1 )
			{
				char *filename = getFilename ( );
				if ( filename != NULL ) 
					LoadProject ( filename, FALSE );
				if ( filename != NULL ) FreeVec ( filename );
			}
			break;
	
		case 594:
			// Load datatype image
			if ( loadDatatypeImage ( ) )
			{
				setActiveBuffer ( globalActiveCanvas );
				globalActiveCanvas->winHasChanged = TRUE;
				set ( globalActiveWindow->win, MUIA_Window_Open, TRUE );
				DoMethod ( globalActiveWindow->area, MUIM_Draw, NULL );
			}
			break;
	
		case 595:
			// Revert
			if ( globalActiveCanvas != NULL && globalActiveWindow != NULL )
			{
				int currentFrame = globalActiveCanvas->currentFrame;
				int currentLayer = globalActiveCanvas->currentLayer;
				
				if ( globalActiveWindow->filename == NULL )
					globalActiveWindow->filename = getFilename ( );
				if ( globalActiveWindow->filename != NULL )
					LoadProject ( globalActiveWindow->filename, TRUE );
				else 
					return;
					
				if ( globalActiveCanvas->totalFrames <= currentFrame ) 
					currentFrame = globalActiveCanvas->totalFrames - 1;
				globalActiveCanvas->currentFrame = currentFrame;
				if ( globalActiveCanvas->totalLayers <= currentLayer ) 
					currentLayer = globalActiveCanvas->totalLayers - 1;
				globalActiveCanvas->currentLayer = currentLayer;
				
				setActiveBuffer ( globalActiveCanvas );
				globalActiveCanvas->winHasChanged = TRUE;
				
				DoMethod ( globalActiveWindow->area, MUIM_Draw );
			}
			break;
			
		case 598:
			// Save
			if ( globalActiveWindow != NULL )
			{
				if ( globalActiveWindow->filename == NULL )
					globalActiveWindow->filename = getFilename ( );
				if ( globalActiveWindow->filename != NULL )
					SaveProject ( globalActiveWindow );
			}
			break;
	
		case 596:
			// Save AS
			if ( globalActiveWindow != NULL )
			{
				char *newFilename = getFilename ( );
				if ( newFilename != NULL )
				{
					if ( globalActiveWindow->filename != NULL )
						FreeVec ( globalActiveWindow->filename );
					globalActiveWindow->filename = newFilename;
				}
				checkMenuEvents ( 598 );
			}
			break;
			
		case 599:
			set ( nwWindow, MUIA_Window_Open, TRUE );
			break;
			
		case 600:
			if ( globalActiveCanvas != NULL )
				set ( importWindow, MUIA_Window_Open, TRUE );
			break;
		
		/* Export */
		case 601:
			if ( globalActiveCanvas != NULL )
			{
				set ( exportWindow, MUIA_Window_Open, TRUE );
			}
			break;
			
		case 602:
			set ( aboutWindow, MUIA_Window_Open, TRUE );
			break;
			
		case 604:
			if ( globalActiveWindow != NULL )
			{
				set ( globalActiveWindow->projectWin, MUIA_Window_Open, TRUE );
			}
			break;
			
		case 700:
			DoMethod (
				PaintApp, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit
			);
			break;
			
		case 730:
			DoMethod ( globalActiveWindow->area, MUIM_ZoomIn );
			break;
			
		case 731:
			DoMethod ( globalActiveWindow->area, MUIM_ZoomOut );
			break;
		
		case 732:
			DoMethod ( globalActiveWindow->area, MUIM_ShowAll );
			break;
		
		case 740:
			if ( globalActiveCanvas != NULL )
			{
				copyToSwap ( globalActiveCanvas );
			}
			break;
			
		case 741:
			if ( globalActiveCanvas != NULL )
			{
				swapCanvasBuffers ( globalActiveCanvas );
				globalActiveWindow->rRectW = 0;
				globalActiveWindow->rRectX = 0;
				globalActiveWindow->rRectH = 0;
				globalActiveWindow->rRectY = 0;
				globalActiveCanvas->winHasChanged = TRUE;
				DoMethod ( globalActiveWindow->area, MUIM_Draw );
			}
			break;
			
		case 742:
			if ( globalActiveWindow != NULL )
			{
				set ( offsetWindow, MUIA_Window_Open, TRUE );
			}
			break;
			
		case 743:
			if ( globalActiveCanvas != NULL )
			{
				effectFlipVert ( globalActiveCanvas );
				globalActiveCanvas->winHasChanged = TRUE;
				DoMethod ( globalActiveWindow->area, MUIM_Draw );
			}
			break;
			
		case 744:
			if ( globalActiveCanvas != NULL )
			{
				effectFlipHoriz ( globalActiveCanvas );
				globalActiveCanvas->winHasChanged = TRUE;
				DoMethod ( globalActiveWindow->area, MUIM_Draw );
			}
			break;
		
		case 745:
			if ( globalActiveCanvas != NULL )
			{
				DoMethod ( tbxAreaPalette, MUIM_ClearActiveCanvas );
			}
			break;
	
		// next palette color
		case 746:
			nextPaletteColor ( );
			break;
		
		// prev palette color
		case 747:
			prevPaletteColor ( );
			break;
			
		// Flip brush vert
		case 748:
			flipBrushVert ( );
			break;
			
		// Flip brush vert
        case 749:
            flipBrushHoriz ( );
            break;
        
        // Rotate brush 90 degrees
        case 751:
            rotateBrush90 ( );
            break;
	
		// Palette editor
		case 750:
			set ( paletteWindow, MUIA_Window_Open, TRUE );
			break;
	
		// The text to brush window 
		case 755:
			set ( textToBrushWindow, MUIA_Window_Open, TRUE );
			break;
	
		// Paint modes 
	
		case 800:
			set ( tbxCycPaintMode, MUIA_Cycle_Active, 0 );
			break;
		
		case 801:
			set ( tbxCycPaintMode, MUIA_Cycle_Active, 6 );
			break;
		
		case 802:
			set ( tbxCycPaintMode, MUIA_Cycle_Active, 7 );
			break;
		
		case 807:
			set ( tbxCycPaintMode, MUIA_Cycle_Active, 8 );
			break;
			
		case 803:
			set ( tbxCycPaintMode, MUIA_Cycle_Active, 3 );
			break;
			
		case 804:
			set ( tbxCycPaintMode, MUIA_Cycle_Active, 4 );
			break;
		
		case 805:
			set ( tbxCycPaintMode, MUIA_Cycle_Active, 5 );
			break;
			
		case 806:
			set ( tbxCycPaintMode, MUIA_Cycle_Active, 2 );
			break;
		
		case 808:
			set ( tbxCycPaintMode, MUIA_Cycle_Active, 1 );
			break;
			
		/* Paint tools */
			
		case 850:
			DoMethod ( paletteRect, MUIM_SetTool_Draw );
			break;
		
		case 851:
			DoMethod ( paletteRect, MUIM_SetTool_Line );
			break;
			
		case 852:
			DoMethod ( paletteRect, MUIM_SetTool_Fill );
			break;
		
		case 853:
			DoMethod ( paletteRect, MUIM_SetTool_Rectangle );
			break;
		
		case 855:
			DoMethod ( paletteRect, MUIM_SetTool_Circle );
			break;
		
		case 854:
			DoMethod ( paletteRect, MUIM_SetTool_ClipBrush );
			break;
		
		case 856:
			DoMethod ( paletteRect, MUIM_SetTool_Colorpicker );
			break;
		
		/* Animation */
			
		case 820:
			DoMethod ( WidgetLayers, MUIM_NextFrame );
			break;
		
		case 821:
			DoMethod ( WidgetLayers, MUIM_PrevFrame );
			break;
			
		case 825:
			{
				int num = 0;
				get ( CycleOnionSkin, MUIA_Cycle_Active, &num );
				set (	CycleOnionSkin, MUIA_Cycle_Active, ( num == 0 ) ? 1 : 0 );
			}
			break;
			
		/* Windows */
		
		case 400:
			{
				BOOL opened = FALSE;
				get ( WindowAnimation, MUIA_Window_Open, &opened );
				if ( opened )
					set ( WindowAnimation, MUIA_Window_Open, FALSE);
				else
					set ( WindowAnimation, MUIA_Window_Open, TRUE );
			}
			break;
			
		case 401:
			{
				BOOL opened = FALSE;
				get ( WindowLayers, MUIA_Window_Open, &opened );
				if ( opened )
					set ( WindowLayers, MUIA_Window_Open, FALSE );
				else
					set ( WindowLayers, MUIA_Window_Open, TRUE );
			}
			break;
			
		// Program preferences
		case 900:
			set ( PrefsWindow, MUIA_Window_Open, TRUE );
			break;
		
		default:
			break;
	}
}

void setToolbarActive ( )
{
	// Draw:
	if ( globalCurrentTool == LUNA_TOOL_BRUSH )
		set ( tbxBtn_draw, MUIA_Background, MUII_FILL ); 
	else
		set ( tbxBtn_draw, MUIA_Background, MUII_ButtonBack ); 
	
	if ( globalCurrentTool == LUNA_TOOL_FILL )
		set ( tbxBtn_fill, MUIA_Background, MUII_FILL ); 
	else
		set ( tbxBtn_fill, MUIA_Background, MUII_ButtonBack ); 
	
	if ( globalCurrentTool == LUNA_TOOL_LINE )
		set ( tbxBtn_line, MUIA_Background, MUII_FILL ); 
	else
		set ( tbxBtn_line, MUIA_Background, MUII_ButtonBack ); 
		
	if ( globalCurrentTool == LUNA_TOOL_RECTANGLE )
		set ( tbxBtn_rectangle, MUIA_Background, MUII_FILL ); 
	else
		set ( tbxBtn_rectangle, MUIA_Background, MUII_ButtonBack ); 
		
	if ( globalCurrentTool == LUNA_TOOL_CIRCLE )
		set ( tbxBtn_circle, MUIA_Background, MUII_FILL ); 
	else
		set ( tbxBtn_circle, MUIA_Background, MUII_ButtonBack ); 
		
	if ( globalCurrentTool == LUNA_TOOL_CLIPBRUSH )
		set ( tbxBtn_getbrush, MUIA_Background, MUII_FILL ); 
	else
		set ( tbxBtn_getbrush, MUIA_Background, MUII_ButtonBack ); 
	
	if ( globalCurrentTool == LUNA_TOOL_COLORPICKER )
		set ( tbxBtn_pickcolor, MUIA_Background, MUII_FILL ); 
	else
		set ( tbxBtn_pickcolor, MUIA_Background, MUII_ButtonBack ); 
}

void GetDrawFillState ( )
{
	filledDrawing = XGET ( tbxCycFillmode, MUIA_Cycle_Active );
}
