/****************************************************************************
*                                                                           *
* layers.c -- Lunapaint, http://www.sub-ether.org/lunapaint                 *
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

#include "layers.h"

#define LAYERTHUMBSIZE 84
#define LAYERIMAGESIZE 80

AROS_UFH3 ( void, changeOpacityFunc,
	AROS_UFHA ( struct Hook*, h, A0 ),
	AROS_UFHA ( APTR, obj, A2 ),
	AROS_UFHA ( APTR, param, A1 )
)
{
	AROS_USERFUNC_INIT
	
	// Fix it
	if ( globalActiveCanvas != NULL )
	{
		gfxbuffer *buf = getActiveGfxbuffer ( globalActiveCanvas );
		buf->opacity = XGET ( LayerOpacity, MUIA_Numeric_Value );
        set ( LayerOpacityValue, MUIA_String_Integer, ( IPTR )buf->opacity );
        globalActiveWindow->layersChg = TRUE;
		DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
	}
	
	AROS_USERFUNC_EXIT
}

AROS_UFH3 ( void, changeVisibilityFunc,
    AROS_UFHA ( struct Hook*, h, A0 ),
    AROS_UFHA ( APTR, obj, A2 ),
    AROS_UFHA ( APTR, param, A1 )
)
{
    AROS_USERFUNC_INIT
    
    // Fix it
    if ( globalActiveCanvas != NULL )
    {
        gfxbuffer *buf = getActiveGfxbuffer ( globalActiveCanvas );
        if ( buf->visible )
        {
            buf->visible = FALSE;
            set ( LayerVisible, MUIA_Text_Contents, ( STRPTR )"Hidden" );
        }
        else
        {
            buf->visible = TRUE;
            set ( LayerVisible, MUIA_Text_Contents, ( STRPTR )"Shown" );
        }
        globalActiveWindow->layersChg = TRUE;
        DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
    }
    
    AROS_USERFUNC_EXIT
}

AROS_UFH3 ( void, acknowledgeOpacityFunc,
    AROS_UFHA ( struct Hook*, h, A0 ),
    AROS_UFHA ( APTR, obj, A2 ),
    AROS_UFHA ( APTR, param, A1 )
)
{
    AROS_USERFUNC_INIT
    
    // Fix it
    if ( globalActiveCanvas != NULL )
    {
        int integer = ( int )XGET ( LayerOpacityValue, MUIA_String_Integer );
        if ( integer < 0 ) integer = 0;
        if ( integer > 100 ) integer = 100;
        set ( LayerOpacity, MUIA_Numeric_Value, integer );
    }
    
    AROS_USERFUNC_EXIT
}

AROS_UFH3 ( void, acknowledgeLayNameFunc,
    AROS_UFHA ( struct Hook*, h, A0 ),
    AROS_UFHA ( APTR, obj, A2 ),
    AROS_UFHA ( APTR, param, A1 )
)
{
    AROS_USERFUNC_INIT
    
    // Fix it
    if ( globalActiveCanvas != NULL )
    {
        gfxbuffer *buf = getActiveGfxbuffer ( globalActiveCanvas );
        if ( buf->name != NULL && strlen ( buf->name ) > 0 ) FreeVec ( buf->name );
        unsigned char *nm = ( unsigned char *)XGET ( LayerName, MUIA_String_Contents );
        buf->name = AllocVec ( strlen ( nm ), MEMF_ANY|MEMF_CLEAR );
        strcpy ( buf->name, nm );
        forceLayerRedraw = TRUE;
        DoMethod ( WidgetLayers, MUIM_Draw );
    }
    
    AROS_USERFUNC_EXIT
}


BOOPSI_DISPATCHER ( IPTR, LayersClass, CLASS, self, message )
{
	switch ( message->MethodID )
	{
		case MUIM_Setup:
			MUI_RequestIDCMP( self, IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY );
			return DoSuperMethodA ( CLASS, self, message );	
	
		case MUIM_Draw:
			
			// Some rules for when to redraw
			if ( globalActiveWindow == NULL || globalActiveCanvas == NULL )
			{
				layerRenderBlank ( );
                set ( LayerName, MUIA_String_Contents, ( IPTR )"" );            
				return ( IPTR )NULL;
			}
			else
			{
                // Make sure the layer window shows correct opacity
                gfxbuffer *buf = getActiveGfxbuffer ( globalActiveCanvas );
                int opa = XGET( LayerOpacity, MUIA_Numeric_Value );
                if ( opa != buf->opacity )
                {
                    set ( LayerOpacity, MUIA_Numeric_Value, ( IPTR )buf->opacity );
                    return ( IPTR )NULL;
                }
                         
				int testWidth = XGET ( WidgetLayers, MUIA_Width );
				unsigned int posV = 0; get ( ScrollLayers, MUIA_Prop_First, &posV );
				if ( posV != globalActiveCanvas->layerScrollPosV )
					posV = 1;
				else posV = 0;
                
				// If we changed active window
				if ( lastDrawnCanvas != globalActiveCanvas )
				{
					lastDrawnCanvas = globalActiveCanvas;
					layerRender ( CLASS, self );
                    layersRepaintWindow ( CLASS, self );
                    set ( LayerName, MUIA_String_Contents, ( STRPTR )buf->name );
                    if ( buf->visible )
                        set ( LayerVisible, MUIA_Text_Contents, ( STRPTR )"Shown" );
                    else set ( LayerVisible, MUIA_Text_Contents, ( STRPTR )"Hidden" );
                    UpdateCanvasInfo ( globalActiveWindow );
				}
				// If we changed anything important
				else if 
                ( 
				    posV == 1 || 
                    LayersWidgetTmpBuf == NULL || 
				    globalActiveWindow->layersChg || 
                    layersWidgetWidth != testWidth || 
                    forceLayerRedraw
				)
				{   
                    forceLayerRedraw = FALSE;            
					layerRender ( CLASS, self );
                    layersRepaintWindow ( CLASS, self );
                    set ( LayerName, MUIA_String_Contents, ( STRPTR )buf->name );
                    if ( buf->visible )
                        set ( LayerVisible, MUIA_Text_Contents, ( STRPTR )"Shown" );
                    else set ( LayerVisible, MUIA_Text_Contents, ( STRPTR )"Hidden" );
                    UpdateCanvasInfo ( globalActiveWindow );
				}
                globalActiveWindow->layersChg = FALSE;
				return ( IPTR )NULL;
			}
		
		case MUIM_HandleInput:
			return ( IPTR )layerHandleInput ( CLASS, self, ( struct MUIP_HandleInput * )message );
			
		case MUIM_AskMinMax:
			DoSuperMethodA ( CLASS, self, message );
			return ( IPTR )layerMinMax ( CLASS, self, ( struct MUIP_AskMinMax * )message );

		case MUIM_PrevFrame:
			if ( globalActiveCanvas != NULL && !IgnoreFramechange )
			{
				PrevFrame ( globalActiveCanvas );
				set ( AnimationSlider, MUIA_Numeric_Value, ( globalActiveCanvas->currentFrame + 1 ) );
				UpdateCanvasInfo ( globalActiveWindow );
                globalActiveWindow->layersChg = TRUE;            
                DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
			}
			break;
		
		case MUIM_NextFrame:
			if ( globalActiveCanvas != NULL && !IgnoreFramechange )
			{
				NextFrame ( globalActiveCanvas );
				set ( AnimationSlider, MUIA_Numeric_Value, ( globalActiveCanvas->currentFrame + 1 ) );
				UpdateCanvasInfo ( globalActiveWindow );
                globalActiveWindow->layersChg = TRUE;            
                DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
			}
			break;

		case MUIM_GotoFrame:
			if ( globalActiveCanvas != NULL && !IgnoreFramechange )
			{
				int num = XGET ( AnimationSlider, MUIA_Numeric_Value );
				num = ( num - 1 < 0 ) ? 0 : ( num - 1 );
				globalActiveCanvas->currentFrame = num;
				setActiveBuffer ( globalActiveCanvas );
				UpdateCanvasInfo ( globalActiveWindow );
                globalActiveWindow->layersChg = TRUE;
                DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
			}
			break;

		case MUIM_CanvasAddLayer:
			if ( globalActiveCanvas != NULL )
			{
				addLayer ( globalActiveCanvas );
                globalActiveWindow->layersChg = TRUE;            
				DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
				return 0;
			}
			break;
			
		case MUIM_CanvasSwapLayer:
			if ( 
				globalActiveCanvas != NULL && 
				globalActiveCanvas->currentLayer != globalActiveCanvas->previousLayer 
			)
			{
				swapLayers ( globalActiveCanvas );
                globalActiveWindow->layersChg = TRUE;            
				DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
				return 0;
			}
			break;

		case MUIM_CanvasDeleteLayer:
			if ( globalActiveCanvas != NULL )
			{
				deleteLayer ( globalActiveCanvas );
				setActiveBuffer ( globalActiveCanvas );
                globalActiveWindow->layersChg = TRUE;            
				DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
				return 0;
			}
			break;
			
		case MUIM_CanvasMergeLayer:
			if ( globalActiveCanvas != NULL )
			{
				mergeLayers ( globalActiveCanvas );
				setActiveBuffer ( globalActiveCanvas );
                globalActiveWindow->layersChg = TRUE;            
				DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
				return 0;
			}
			break;
		
		case MUIM_CanvasCopyLayer:
			if ( globalActiveCanvas != NULL )
			{
				copyLayers ( globalActiveCanvas );
                globalActiveWindow->layersChg = TRUE;
				DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
				return 0;
			}
			break;

		case MUIM_ChangeOnionskin:
			if ( globalActiveCanvas != NULL )
			{
				int num = 0;
				get ( CycleOnionSkin, MUIA_Cycle_Active, &num );
				globalActiveCanvas->onion = ( char )num;
                globalActiveWindow->layersChg = TRUE;            
				DoMethod ( globalActiveWindow->area, MUIM_RedrawArea );
			}
			break;

		default:
			return DoSuperMethodA ( CLASS, self, message );
	}
	return ( IPTR )0;
}
BOOPSI_DISPATCHER_END

void Init_LayersWindow ( )
{
	lastDrawnCanvas = NULL;
	LayersWidgetTmpBuf = NULL;

	struct MUI_CustomClass *lay = MUI_CreateCustomClass (
		NULL, MUIC_Area, NULL, 0, LayersClass
	);

	WindowLayers = WindowObject,
		MUIA_Window_Title, ( IPTR )"Layers",
		MUIA_Window_ScreenTitle, ( IPTR )LUNA_SCREEN_TITLE,
		MUIA_Window_CloseGadget, TRUE,
		MUIA_Window_Screen, ( IPTR )lunaPubScreen,
		MUIA_Window_SizeGadget, TRUE,
		MUIA_Window_LeftEdge, 0,
		MUIA_Window_TopEdge, ( lunaPubScreen->BarHeight + 1 ),
		WindowContents, ( IPTR )VGroup,
			MUIA_Group_HorizSpacing, 0,
			Child, ( IPTR )GroupObject,
				Child, ( IPTR )HGroup,
					MUIA_Group_VertSpacing, 0,
					MUIA_Group_HorizSpacing, 0,
					Child, ( IPTR )GroupObject,
						MUIA_Frame, MUIV_Frame_Group,
						InnerSpacing( 0, 0 ),
						MUIA_Group_VertSpacing, 0,
						MUIA_Group_HorizSpacing, 0,
						Child, ( IPTR )( WidgetLayers = NewObject (
							lay->mcc_Class, NULL, 
							TAG_DONE
						) ),
					End,
					Child, ( IPTR )( ScrollLayers = ScrollbarObject, 
						MUIA_Prop_UseWinBorder, MUIV_Prop_UseWinBorder_None,
					End ),
				End,
			End,
			Child, ( IPTR )( LayerControlGroup = VGroup,
				Child, ( IPTR )( LayerName = StringObject,
					MUIA_String_Contents, ( IPTR )"",
					MUIA_Frame, MUIV_Frame_String,
				End ),
				Child, ( IPTR )HGroup,
					Child, ( IPTR )( LayerOpacity = SliderObject,
                        MUIA_Weight, 50,               
						MUIA_Numeric_Min, 0,
						MUIA_Numeric_Max, 100,
						MUIA_Numeric_Value, 100,
						MUIA_Prop_Slider, TRUE,
						MUIA_Prop_Horiz, TRUE,
						MUIA_Slider_Horiz, TRUE,
					End ),
                    Child, ( IPTR )( LayerOpacityValue = StringObject,
                        MUIA_Weight, 25,
                        MUIA_String_Integer, 100,
                        MUIA_String_Accept, ( IPTR )"0123456789",
                        MUIA_String_MaxLen, 4,
                        MUIA_String_Format, MUIV_String_Format_Right,
                        MUIA_Frame, MUIV_Frame_String,
                    End ),
                    Child, ( IPTR )VGroup,
                        MUIA_Weight, 25,
                        InnerSpacing ( 0, 0 ),
					    Child, ( IPTR )( LayerVisible = SimpleButton ( "Shown" ) ),
                    End,               
				End,
			End ),
			Child, ( IPTR )HGroup,
				Child, ( IPTR )( BtnAddLayer = ImageButton ( "", "Lunapaint:data/btn_add.png" ) ),
				Child, ( IPTR )( BtnDelLayer = ImageButton ( "", "Lunapaint:data/btn_sub.png" ) ),
				Child, ( IPTR )( BtnSwapLayer = ImageButton ( "", "Lunapaint:data/btn_swap.png" ) ),
				Child, ( IPTR )( BtnCopyLayer = ImageButton ( "", "Lunapaint:data/btn_copy.png" ) ),
				Child, ( IPTR )( BtnMergeLayer = ImageButton ( "", "Lunapaint:data/btn_merge.png" ) ),
			End,
		End,
	End;
	
	/* Bubble help */
	DoMethod ( BtnAddLayer, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Add layer" );
	DoMethod ( BtnDelLayer, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Delete layer" );
	DoMethod ( BtnSwapLayer, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Swap layer" );
	DoMethod ( BtnCopyLayer, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Copy layer" );
	DoMethod ( BtnMergeLayer, MUIM_Set, MUIA_ShortHelp, (STRPTR)"Merge layer" );
	
	/*
		Layers
	*/
	DoMethod ( 
		BtnAddLayer, MUIM_Notify,
		MUIA_Pressed, FALSE,
		( IPTR )WidgetLayers, 1, MUIM_CanvasAddLayer
	);
	DoMethod (
		BtnSwapLayer, MUIM_Notify,
		MUIA_Pressed, FALSE,
		( IPTR )WidgetLayers, 1, MUIM_CanvasSwapLayer
	);
	DoMethod (
		BtnDelLayer, MUIM_Notify,
		MUIA_Pressed, FALSE,
		( IPTR )WidgetLayers, 1, MUIM_CanvasDeleteLayer
	);
	DoMethod (
		BtnMergeLayer, MUIM_Notify,
		MUIA_Pressed, FALSE,
		( IPTR )WidgetLayers, 1, MUIM_CanvasMergeLayer
	);
	DoMethod (
		BtnCopyLayer, MUIM_Notify,
		MUIA_Pressed, FALSE,
		( IPTR )WidgetLayers, 1, MUIM_CanvasCopyLayer
	);
	DoMethod ( 
		ScrollLayers, MUIM_Notify, MUIA_Prop_First, MUIV_EveryTime,
		( IPTR )WidgetLayers, 1, MUIM_Draw
	);
	DoMethod ( 
		WindowLayers, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
		( IPTR )WindowLayers, 3, MUIM_Set, MUIA_Window_Open, FALSE
	);   
	
    // Layer opacity   
	changeOpacityHook.h_Entry = ( HOOKFUNC )changeOpacityFunc;
    changeVisibilityHook.h_Entry = ( HOOKFUNC )changeVisibilityFunc;  
    DoMethod (
	    LayerOpacity, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime,
	    ( IPTR )LayerOpacity, 2, MUIM_CallHook, &changeOpacityHook
    );
    acknowledgeOpacity.h_Entry = ( HOOKFUNC )acknowledgeOpacityFunc;
    DoMethod (
        LayerOpacityValue, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
        ( IPTR )LayerOpacityValue, 2, MUIM_CallHook, &acknowledgeOpacity
    );
    acknowledgeLayName.h_Entry = ( HOOKFUNC )acknowledgeLayNameFunc;
    DoMethod (
        LayerName, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime,
        ( IPTR )LayerName, 2, MUIM_CallHook, &acknowledgeLayName
    );
    DoMethod (
        LayerVisible, MUIM_Notify, MUIA_Pressed, FALSE,
        ( IPTR )LayerVisible, 2, MUIM_CallHook, &changeVisibilityHook
    );
       
}

IPTR layerMinMax ( Class *CLASS, Object *self, struct MUIP_AskMinMax *message )
{
	message->MinMaxInfo->MinWidth 	+= 140;
	message->MinMaxInfo->MinHeight 	+= 30;
	message->MinMaxInfo->DefWidth 	+= 150;
	message->MinMaxInfo->DefHeight 	+= 200;
	message->MinMaxInfo->MaxWidth   += 300;
	message->MinMaxInfo->MaxHeight	+= 4096;
	return 0;
}

IPTR layerHandleInput ( Class *CLASS, Object *self, struct MUIP_HandleInput *msg )
{
	if ( globalActiveCanvas == NULL || globalActiveWindow == NULL )
		return 0;
		
	int topEdge = 0, leftEdge = 0, areaWidth = 0, areaHeight = 0, propFirst = 0;
	get ( WidgetLayers, MUIA_Width, &areaWidth );
	get ( WidgetLayers, MUIA_Height, &areaHeight );
	get ( WidgetLayers, MUIA_TopEdge, &topEdge );
	get ( WidgetLayers, MUIA_LeftEdge, &leftEdge );
	get ( ScrollLayers, MUIA_Prop_First, &propFirst );

	if ( msg->imsg )
	{
		switch ( msg->imsg->Class )
		{
			case IDCMP_MOUSEBUTTONS:
				if ( 
					( int )msg->imsg->MouseX - leftEdge >= 0 &&
					( int )msg->imsg->MouseX - leftEdge < areaWidth &&
					( int )msg->imsg->MouseY - topEdge >= 0 &&
					( int )msg->imsg->MouseY - topEdge < areaHeight
				)
				{
					// TODO: Subtract vertical scroll offset...
					int wantedLayer = ( ( double )( ( int )msg->imsg->MouseY - topEdge + propFirst ) / LAYERTHUMBSIZE );
					if ( wantedLayer < 0 ) wantedLayer = 0;
					if ( wantedLayer >= globalActiveCanvas->totalLayers )
						wantedLayer = globalActiveCanvas->totalLayers - 1;
					if ( wantedLayer != globalActiveCanvas->currentLayer )
					{
						globalActiveCanvas->previousLayer = globalActiveCanvas->currentLayer;
						globalActiveCanvas->currentLayer = wantedLayer;
						gfxbuffer *buf = getActiveGfxbuffer ( globalActiveCanvas );
						set ( LayerOpacity, MUIA_Numeric_Value, ( IPTR )buf->opacity );
                        set ( LayerName, MUIA_String_Contents, ( STRPTR )buf->name );                  
						setActiveBuffer ( globalActiveCanvas );
					}				
                    forceLayerRedraw = TRUE;
                    DoMethod ( WidgetLayers, MUIM_Draw );
				}
			break;
			
			default: break;
		}
	}	
	return 0;
}

void layerRenderBlank ( )
{
	BOOL winopen = XGET ( WindowLayers, MUIA_Window_Open );
	if ( winopen )
	{
		unsigned int areaWidth = XGET ( WidgetLayers, MUIA_Width );
		unsigned int areaHeight = XGET ( WidgetLayers, MUIA_Height );
		unsigned int areaTop = XGET ( WidgetLayers, MUIA_TopEdge );
		unsigned int areaLeft = XGET ( WidgetLayers, MUIA_LeftEdge );
		int size = areaWidth * areaHeight;
		unsigned int *buf = AllocVec ( size * 4, MEMF_ANY );
		int i = 0; for ( ; i < size; i++ ) buf[ i ] = 0x777777;
		WritePixelArray ( 
			buf, 0, 0, areaWidth * 4, 
			_rp ( WidgetLayers ), areaLeft, areaTop, areaWidth, areaHeight, RECTFMT_RGBA
		);
		FreeVec ( buf );
	}
}

BOOL layerRender ( Class *CLASS, Object *self )
{
	BOOL opened = FALSE;
	get ( WindowLayers, MUIA_Window_Open, &opened );
	if ( !opened || globalActiveCanvas == NULL || globalActiveWindow == NULL )
		return FALSE;
		
	unsigned int areaWidth = 0, areaHeight = 0;
	unsigned int topEdge = 0, leftEdge = 0;
	int propFirst = 0;
	
	get ( WidgetLayers, MUIA_Width, &areaWidth );
	get ( WidgetLayers, MUIA_Height, &areaHeight );
	get ( WidgetLayers, MUIA_TopEdge, &topEdge );
	get ( WidgetLayers, MUIA_LeftEdge, &leftEdge );
	get ( ScrollLayers, MUIA_Prop_First, &propFirst );
	
	layersWidgetWidth = areaWidth; //<- remember width
       
    int thumbHeight = LAYERTHUMBSIZE, thumbWidth = LAYERTHUMBSIZE; // <- frame
    int imageHeight = LAYERIMAGESIZE, imageWidth = LAYERIMAGESIZE; // <- cropped/centered image
    if ( globalActiveCanvas->width > globalActiveCanvas->height )
        imageHeight = ( double )globalActiveCanvas->height / globalActiveCanvas->width * LAYERIMAGESIZE;
	else
        imageWidth = ( double )globalActiveCanvas->width / globalActiveCanvas->height * LAYERIMAGESIZE;
    int HSpace = ( double )( thumbWidth - imageWidth ) / 2;
    int VSpace = ( double )( thumbHeight - imageHeight ) / 2;
    
	if ( LayersWidgetTmpBuf != NULL ) FreeVec ( LayersWidgetTmpBuf );
	LayersWidgetTmpBuf = AllocVec ( areaWidth * areaHeight * 4, MEMF_ANY|MEMF_CLEAR );
	
	gfxbuffer *buf = globalActiveCanvas->buffer;
	
	int f = 0; for ( ; f < globalActiveCanvas->totalFrames; f++ )
	{
		int l = 0; for ( ; l < globalActiveCanvas->totalLayers; l++ )
		{
			if ( f == globalActiveCanvas->currentFrame )
			{
				int lOffsetY = ( LAYERTHUMBSIZE * l ) - propFirst;
				
				int y = 0; for ( ; y < LAYERTHUMBSIZE; y ++ )
				{
					int py = ( double )( y - VSpace ) / imageHeight * globalActiveCanvas->height;
					
					int x = 0; for ( ; x < areaWidth; x++ )
					{
						int px = ( double )( x - HSpace ) / imageWidth * globalActiveCanvas->width;
                        
						// Draw only what's in view in the widget
						if ( 
							x >= 0 && x < ( int )areaWidth && ( y + lOffsetY ) >= 0 && 
							( y + lOffsetY ) < ( int )areaHeight 
 						)
						{
                            // offset to plot in tmpBuf                  
                            int off = ( ( y + lOffsetY ) * areaWidth ) + x;
                            
                            // within the frame                  
                            if ( x >= HSpace && x < imageWidth + HSpace && y >= VSpace && y < imageHeight + VSpace )
                            {            
							    // TODO: Draw alpha
							    // Show current layer normal
							
                                // Frame                     
			                    if ( x == HSpace || x == HSpace + imageWidth - 1 || y == VSpace || y == VSpace + imageHeight - 1 )
                                {
                                    if ( l == globalActiveCanvas->currentLayer )
                                        LayersWidgetTmpBuf[ off ] = 0xffffff;
                                    else LayersWidgetTmpBuf[ off ] = 0xaaaaaa;
                                }
                                // Image
                                else
                                {
                                    rgbaData rgba = canvasColorToRGBA ( 
									    buf->buf[ py * globalActiveCanvas->width + px ]
								    );
                                    // Checks                        
                                    unsigned int colr = *( unsigned int *)&( ( rgba32 ){ 128, 128, 128, 255 } );
                                    int checkCol = ( ( x % 16 ) + ( int )( y / 8 ) * 8 ) % 16;
                                    if ( checkCol >= 8 ) 
                                        colr = *( unsigned int *)&( ( rgba32 ){ 90, 90, 90, 255 } );
                                    // Blend
                                    if ( rgba.a > 0 )
								    {       
                                        rgba32 blend = *( rgba32 *)&colr;
                                        double balph = ( double )rgba.a / 255;
                                        // Convert to safe mode
                                        int r = ( int )blend.r, g = ( int )blend.g, b = ( int )blend.b;
                                        int r2 =  (int )rgba.r, g2 = ( int )rgba.g, b2 =  (int )rgba.b;
                                        // Blend
                                        r -= ( r - r2 ) * balph;
                                        g -= ( g - g2 ) * balph;
                                        b -= ( b - b2 ) * balph;
                                        // write
									    LayersWidgetTmpBuf[ off ] = r | ( g << 8 ) | ( b << 16 ) | 0;
								    }
                                    else LayersWidgetTmpBuf[ off ] = colr;
                                }
                            }
                            else
                            {
                                 // Switch background
                                if ( l % 2 == 0 )                        
                                    LayersWidgetTmpBuf[ off ] = 0x222222;
                                else                           
                                    LayersWidgetTmpBuf[ off ] = 0x444444;
                            }                     
						}
					}
				}          
			}
			buf = buf->nextbuf;
		}
	}
	return TRUE;
}

void RenderLayerNames ( )
{
    oCanvas *canv = globalActiveCanvas;
    gfxbuffer *buf = canv->buffer;
    int size = canv->totalLayers * canv->totalFrames;
    Object *obj = WidgetLayers;
    int areaHeight = ( int )XGET( WidgetLayers, MUIA_Height );
    int areaWidth = ( int )XGET( WidgetLayers, MUIA_Width );
    int areaLeft =  ( int )XGET( WidgetLayers, MUIA_LeftEdge );
    int areaTop = ( int )XGET( WidgetLayers, MUIA_TopEdge );
    APTR clip = MUI_AddClipping ( 
        muiRenderInfo( obj ), 
        areaLeft, areaTop, 
        areaWidth, areaHeight 
    );
    SetAPen( _rp( obj ), _pens( obj )[MPEN_SHINE] );
    int ioff = ( int )XGET ( ScrollLayers, MUIA_Prop_First );
    int yoffset = 20;
    int xoffset = 4;
    struct TextFont *fnt = _font ( obj );
    int i = 0; for ( ; i < size; i++ )
    {
        int l = i % canv->totalLayers;
        int f = i / canv->totalLayers;
        if ( f == canv->currentFrame )
        {
            int y = l * LAYERTHUMBSIZE - ioff;
            // Layer name:
            Move ( _rp ( obj ), areaLeft + LAYERTHUMBSIZE + xoffset, y + areaTop + yoffset );
            Text ( _rp ( obj ), buf->name, strlen ( buf->name ) );
            // Layer opacity
            Move ( _rp ( obj ), areaLeft + LAYERTHUMBSIZE + xoffset, y + areaTop + yoffset + fnt->tf_YSize + 4 );
            STRPTR str = AllocVec ( 255, MEMF_ANY|MEMF_CLEAR ); sprintf ( str, "Opacity: %d", buf->opacity );
            Text ( _rp ( obj ), str, strlen ( str ) );
            FreeVec ( str ); 
            // Visibility
            Move ( _rp ( obj ), areaLeft + LAYERTHUMBSIZE + xoffset, y + areaTop + yoffset + ( ( fnt->tf_YSize + 4 ) * 2 ) );
            if ( buf->visible )
                Text ( _rp ( obj ), "Shown", 7 );
            else Text ( _rp ( obj ), "Hidden", 6 );
        }
        buf = buf->nextbuf;
    }
    MUI_RemoveClipping( muiRenderInfo( obj ), clip );   
}

void layersRepaintWindow ( Class *CLASS, Object *self )
{
    BOOL opened = FALSE;
	get ( WindowLayers, MUIA_Window_Open, &opened );
	if ( !opened ) return;
	
	unsigned int areaWidth = 0, areaHeight = 0;
	unsigned int topEdge = 0, leftEdge = 0;
	
	get ( WidgetLayers, MUIA_Width, &areaWidth );
	get ( WidgetLayers, MUIA_Height, &areaHeight );
	get ( WidgetLayers, MUIA_TopEdge, &topEdge );
	get ( WidgetLayers, MUIA_LeftEdge, &leftEdge );

    // Write the layer data
	WritePixelArray ( 
		LayersWidgetTmpBuf, 0, 0, areaWidth * 4, 
		_rp ( WidgetLayers ), leftEdge, topEdge, areaWidth, areaHeight, RECTFMT_RGBA 
	);
    RenderLayerNames ( );
    UpdateCanvasInfo ( globalActiveWindow );

	// Update layers scrollbar
	if ( 
       globalActiveCanvas->winHasChanged || globalActiveWindow->layersChg || 
       areaWidth != layersWidgetWidth || areaHeight != layersWidgetHeight 
    )
	{
		globalActiveWindow->layersChg = FALSE;
		set ( ScrollLayers, MUIA_Prop_Visible, areaHeight );
		set ( ScrollLayers, MUIA_Prop_Entries, ( LAYERTHUMBSIZE * globalActiveCanvas->totalLayers ) );
        layersWidgetHeight = areaHeight;
        layersWidgetWidth = areaWidth;
	}   
}

void Exit_LayersWindow ( )
{
	if ( LayersWidgetTmpBuf != NULL )
		FreeVec ( LayersWidgetTmpBuf );
}