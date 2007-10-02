/****************************************************************************
*                                                                           *
* gui.c -- Lunapaint, http://www.sub-ether.org/lunapaint                    *
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

#include "gui.h"

void Init_Application ( )
{	
    static struct NewMenu MyMenu[] = {
        {NM_TITLE, "Project"																    },
            {NM_ITEM, "New project",						"N",			2, 0L, (APTR)599    },	
            {NM_ITEM, "Load project",						"O",			2, 0L, (APTR)597	},
            {NM_ITEM, "Load datatype image",				"L",			2, 0L, (APTR)594	},
            {NM_ITEM, "Save project",						"S",			2, 0L, (APTR)598	},
            {NM_ITEM, "Save as project",					"A",			2, 0L, (APTR)596	},
            {NM_ITEM, "Revert",								"R",			2, 0L, (APTR)595	},
            {NM_ITEM, "Import image",						"I",			2, 0L, (APTR)600	},
            {NM_ITEM, "Export project",					    "E",			2, 0L, (APTR)601	},
            {NM_ITEM, "Project Information",				"I",			2, 0L, (APTR)604    },
            {NM_ITEM, NM_BARLABEL, 							0 , 			0, 0L, (APTR)0 	    },
            {NM_ITEM, "About LunaPaint",					NULL,			2, 0L, (APTR)602    },
            {NM_ITEM, "Quit", 								"Q",			2, 0L, (APTR)700	},
        {NM_TITLE, "Edit"																		},
            {NM_ITEM, "Zoom in",							"+",			2, 0L, (APTR)730    },
            {NM_ITEM, "Zoom out",							"-",			2, 0L, (APTR)731    },
            {NM_ITEM, "Show all",							"=",			2, 0L, (APTR)732    },
            {NM_ITEM, NM_BARLABEL, 							0 , 			0, 0L, (APTR)0 	    },
            {NM_ITEM, "Copy to swap",						"K",			2, 0L, (APTR)740    },
            {NM_ITEM, "Swap buffers",						"J",			2, 0L, (APTR)741    },
            {NM_ITEM, "Flip vertically",					NULL,			2, 0L, (APTR)743	},
            {NM_ITEM, "Flip horizontally",				    NULL,			2, 0L, (APTR)744	},
            {NM_ITEM, "Clear active layer",				    "Y",			2, 0L, (APTR)745	},
            {NM_ITEM, "Next palette color",				    "X",			2, 0L, (APTR)746	},
            {NM_ITEM, "Previous palette color",			    "Z",			2, 0L, (APTR)747	},
            {NM_ITEM, "Palette editor",					    NULL,			2, 0L, (APTR)750	},
            {NM_ITEM, NM_BARLABEL, 							0 , 			0, 0L, (APTR)0 	    },
            {NM_ITEM, "Set layer offset",					NULL,			2, 0L, (APTR)742	},
            {NM_ITEM, "Render text to brush",			    NULL,			2, 0L, (APTR)755	},
            {NM_ITEM, NM_BARLABEL, 							0 , 			0, 0L, (APTR)0 	    },
            {NM_ITEM, "Flip brush vertically",			    NULL,			2, 0L, (APTR)748	},
            {NM_ITEM, "Flip brush horizontally",            NULL,           2, 0L, (APTR)749    },
            {NM_ITEM, "Rotate brush 90 degrees",         NULL,              2, 0L, (APTR)751    },         
        {NM_TITLE, "Tools"																		},
            {NM_ITEM, "Draw", 								"D",			2, 0L, (APTR)850	},
            {NM_ITEM, "Line",								"V",			2, 0L, (APTR)851	},
            {NM_ITEM, "Rectangle",							"G",			2, 0L, (APTR)853	},
            {NM_ITEM, "Circle",								"C",			2, 0L, (APTR)855	},
            {NM_ITEM, "Get brush",							"B",			2, 0L, (APTR)854	},
            {NM_ITEM, "Fill",								"F",			2, 0L, (APTR)852	},
            {NM_ITEM, "Color picker",						"H",			2, 0L, (APTR)856	},
        {NM_TITLE, "Paint modes"																},
            {NM_ITEM, "Normal", 							"1",			2, 0L, (APTR)800 	},	
            {NM_ITEM, "Color", 								"2",			2, 0L, (APTR)808 	},
            {NM_ITEM, "Colorize", 							"3",			2, 0L, (APTR)801 	},
            {NM_ITEM, "Lighten",							"4",			2, 0L, (APTR)804 	},
            {NM_ITEM, "Darken",								"5",			2, 0L, (APTR)805 	},
            {NM_ITEM, "Blur", 								"6",			2, 0L, (APTR)806 	},
            {NM_ITEM, "Smudge",								"7",			2, 0L, (APTR)803 	},
            {NM_ITEM, "Erase", 								"8",			2, 0L, (APTR)802 	},
            {NM_ITEM, "UnErase", 							"9",			2, 0L, (APTR)807 	},
        {NM_TITLE, "Animation"																	},
            {NM_ITEM, "Next frame", 						".",			2, 0L, (APTR)820 	},
            {NM_ITEM, "Previous frame",					    ",",			2, 0L, (APTR)821 	},
            {NM_ITEM, "Onion skin on/off",				    "'",			2, 0L, (APTR)825 	},
            {NM_ITEM, NM_BARLABEL, 							0 , 			0, 0L, (APTR)0 	    },
            {NM_ITEM, "Toggle animation window",		    NULL,			2, 0L, (APTR)400    },
        {NM_TITLE, "Windows"																	},	
            {NM_ITEM, "Toggle layers window",			    NULL,			2, 0L, (APTR)401    },
            {NM_ITEM, "Toggle fullscreen editing",		    NULL,			2, 0L, (APTR)402	},
        {NM_TITLE, "Filters"																	},
            {NM_ITEM, "Soon...",							NULL,			2, 0L, (APTR)999	},
        {NM_TITLE, "Preferences"																},
            {NM_ITEM, "Program preferences",				"P",			2, 0L, (APTR)900	},
        {NM_END}
    };
    mainPulldownMenu = MUI_MakeObject ( 
        MUIO_MenustripNM, ( IPTR )MyMenu, (IPTR)NULL 
    );
    
    loadPreferences ( );
    
    InitLunaScreen ( );
    
    // Initialize the events listener
    InitEvents ( );
    
    // Let's initialize the used palette!
    initPalette ( );
    
    // Init toolbox and related windows
    Init_AboutWindow ( );
    Init_ToolboxWindow ( );
    Init_LayersWindow ( );
    Init_AnimationWindow ( );
    Init_PaletteWindow ( );
    Init_PrefsWindow ( );
    Init_AlertWindow ( );
    Init_TextToBrushWindow ( );
    Init_Backdrop ( );

    // Initialize the gui for making a new project
    nwNewWindow ( ); 
    // Initialize the export window
    makeExportWindow ( );
    // Initialize the import window
    makeImportWindow ( );

    PaintApp = ApplicationObject,
        MUIA_Application_Menustrip, (IPTR)mainPulldownMenu,
        MUIA_Application_Title, ( IPTR )LUNA_SCREEN_TITLE,
        MUIA_Application_Base, ( IPTR )"Lunapaint",
        MUIA_Application_Version, ( IPTR )LUNA_VERSION,
        MUIA_Application_Author, ( IPTR )"Hogne Titlestad",
        MUIA_Application_Copyright, ( IPTR )"Hogne Titlestad",
        MUIA_Application_Description, ( IPTR )"A 64-bit paint application for AROS.",
        MUIA_Application_SingleTask, TRUE,
        SubWindow, ( IPTR )toolbox,
        SubWindow, ( IPTR )WindowLayers,
        SubWindow, ( IPTR )WindowAnimation,
        SubWindow, ( IPTR )paletteWindow,
        SubWindow, ( IPTR )nwWindow,
        SubWindow, ( IPTR )aboutWindow,
        SubWindow, ( IPTR )offsetWindow,
        SubWindow, ( IPTR )exportWindow,
        SubWindow, ( IPTR )importWindow,
        SubWindow, ( IPTR )PrefsWindow,
        SubWindow, ( IPTR )AlertWindow,
        SubWindow, ( IPTR )textToBrushWindow,
        SubWindow, ( IPTR )LunaBackdrop,
    End;	
    
    if ( PaintApp == NULL )
    {
        printf ( "Couldn't generate application object..\n" );
        Exit_Application ( );
        return;
    }
    
    Init_ToolboxMethods ( ); 
    Init_PaletteMethods ( ); 
    Init_NewProjectMethods ( );
    Init_TextToBrushMethods ( );

    // Add the first window so we have something to use
    canvases = NULL; // tell init code that canvases is uninitialized
    globalActiveCanvas = NULL;
    globalActiveWindow = NULL;
    globalWindowIncrement = 1;
    globalCurrentTool = LUNA_TOOL_BRUSH;
    globalColorMode = LUNA_COLORMODE_64BIT;
    brushTool.paintmode = LUNA_PAINTMODE_NORMAL;
    brushTool.RecordContour = FALSE;
    brushTool.ContourBuffer = NULL;
    brushTool.opacitymode = LUNA_OPACITYMODE_ADD;
    globalBrushMode = 0; // normal
    MouseHasMoved = FALSE;
    redrawTimes = 0;
    fullscreenEditing = FALSE;
    
    // Open the toolbox and declare the app running!
    if ( GlobalPrefs.ScreenmodeType != 0 )
        set ( LunaBackdrop, MUIA_Window_Open, TRUE );
    set ( toolbox, MUIA_Window_Open, TRUE );
    setToolbarActive ( );
    makeToolBrush ( );
    GetDrawFillState ( );
}

void Exit_Application ( )
{
    // Delete canvases (free allocated memory)
    deleteCanvaswindows ( canvases );

    if ( canvases != NULL ) 
        FreeVec ( canvases );
    
    // Free up layers window buffers etc
    Exit_LayersWindow ( );
    
    LockPubScreen ( ( STRPTR )"Lunapaint" );
    
    // Dispose of the paint app
    if ( PaintApp != NULL )
        MUI_DisposeObject ( PaintApp );
    
    UnlockPubScreen ( ( STRPTR )"Lunapaint", lunaPubScreen );
    
    DOSBase = ( struct DosLibrary *)OpenLibrary ( "dos.library", 36 );
    IntuitionBase = ( struct IntuitionBase *)OpenLibrary ( "intuition.library", 36 );
    
    if ( lunaPubScreen != NULL )
        CloseScreen ( lunaPubScreen );
    
    if ( IntuitionBase != NULL )
        CloseLibrary ( ( struct Library *)IntuitionBase );
    if ( DOSBase != NULL )
        CloseLibrary ( ( struct Library *)DOSBase );
    
    // Shutdown events
    ShutdownEvents ( );
    
    
    // Free all tool buffers
    if ( brushTool.buffer != NULL )
        FreeVec ( brushTool.buffer );
    if ( brushTool.ContourBuffer != NULL )
        freeValueList ( &brushTool.ContourBuffer );
    if ( lineTool.buffer != NULL )
        FreeVec ( lineTool.buffer );
    if ( circleTool.buffer != NULL )
        FreeVec ( circleTool.buffer );
    if ( rectangleTool.buffer != NULL )
        FreeVec ( rectangleTool.buffer );
    if ( clipbrushTool.buffer != NULL )
        FreeVec ( clipbrushTool.buffer );
    
    // Free up palette
    if ( globalPalette != NULL )
        FreeVec ( globalPalette );
    
    // Final cleanup with toolbox related things
    Exit_ToolboxWindow ( );
    
    // Clean up fonts etc
    Exit_TextToBrushWindow ( );
}

int checkSignalBreak ( ULONG *sigs )
{
    if ( *sigs )
    {
        *sigs = Wait ( *sigs | SIGBREAKF_CTRL_C );
        if ( *sigs & SIGBREAKF_CTRL_C )
            return 1;
    }	
    return 0;
}

int getSignals ( ULONG *sigs )
{
    LONG result = DoMethod ( PaintApp, MUIM_Application_NewInput, ( IPTR )sigs );
    
    if ( result	!= MUIV_Application_ReturnID_Quit )
    {
        return 1;
    }
    return 0;
}

void InitLunaScreen ( )
{
    // Open some libraries for our pubscreen
    DOSBase = ( struct DosLibrary *)OpenLibrary ( "dos.library", 36 );
    IntuitionBase = ( struct IntuitionBase *)OpenLibrary ( "intuition.library", 36 );
    
    if ( lunaPubScreen != NULL )
    {
        UnlockPubScreen ( ( STRPTR )"Lunapaint", lunaPubScreen );
        CloseScreen ( lunaPubScreen );
    }
    
    // Open a clone screen
    if ( GlobalPrefs.ScreenmodeType == 1 )
    {
        lunaPubScreen = OpenScreenTags ( 
            lunaScreen,
            SA_Title, ( IPTR )LUNA_SCREEN_TITLE,
            SA_PubName, ( IPTR )"Lunapaint",
            SA_ShowTitle, TRUE,
            SA_SysFont, 1,
            SA_SharePens, TRUE,
            SA_LikeWorkbench, TRUE,
            TAG_DONE 
        );
        PubScreenStatus( lunaPubScreen, 0 );
    }
    // Open a specified screen
    else if ( 0 )
    {
        /*
        ULONG idok;
        idok = BestCModeIDTags ( 
            CYBRBIDTG_NominalWidth, scrWidth,
            CYBRBIDTG_NominalHeight, scrHeight,
            CYBRBIDTG_Depth, scrDepth,
            TAG_DONE
        );
        lunaPubScreen = OpenScreenTags ( 
            lunaScreen,
            SA_Width, scrWidth, 
            SA_Height, scrHeight, 
            SA_Depth, scrDepth, 
            SA_DisplayID, idok,
            SA_Title, ( IPTR )LUNA_SCREEN_TITLE,
            SA_PubName, ( IPTR )"Lunapaint",
            SA_ShowTitle, TRUE,
            SA_SysFont, 1,
            SA_SharePens, TRUE,
            SA_LikeWorkbench, TRUE,
            TAG_DONE 
        );
        PubScreenStatus( lunaPubScreen, 0 );
        */
    }
    // Open on Workbench/Wanderer/Ambient whatever
    else if ( GlobalPrefs.ScreenmodeType == 0 )
    {
        lunaPubScreen = LockPubScreen ( ( UBYTE *)"Workbench" );
    }
    
    // Close intuition for now
    CloseLibrary ( ( struct Library *)IntuitionBase );
    CloseLibrary ( ( struct Library *)DOSBase );
}

void HideOpenWindows ( )
{
    set ( toolbox, MUIA_Window_Open, FALSE );
    set ( WindowLayers, MUIA_Window_Open, FALSE );
    set ( WindowAnimation, MUIA_Window_Open, FALSE );
    set ( paletteWindow, MUIA_Window_Open, FALSE );
    set ( nwWindow, MUIA_Window_Open, FALSE );
    set ( aboutWindow, MUIA_Window_Open, FALSE );
    set ( offsetWindow, MUIA_Window_Open, FALSE );
    set ( exportWindow, MUIA_Window_Open, FALSE );
    set ( importWindow, MUIA_Window_Open, FALSE );
    set ( PrefsWindow, MUIA_Window_Open, FALSE );
    set ( AlertWindow, MUIA_Window_Open, FALSE );	
    set ( LunaBackdrop, MUIA_Window_Open, FALSE );
    set ( textToBrushWindow, MUIA_Window_Open, FALSE );   
    
    /*WindowList *lst = canvases;
    
    while ( lst != NULL )
    {
        set ( lst->win, MUIA_Window_Open, FALSE );
        lst = lst->nextwin;
    }*/

}

void ReopenWindows ( )
{
    set ( toolbox, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( WindowLayers, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( WindowAnimation, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( paletteWindow, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( nwWindow, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( aboutWindow, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( offsetWindow, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( exportWindow, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( importWindow, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( PrefsWindow, MUIA_Window_Screen, ( IPTR )lunaPubScreen );	
    set ( AlertWindow, MUIA_Window_Screen, ( IPTR )lunaPubScreen );	
    set ( LunaBackdrop, MUIA_Window_Screen, ( IPTR )lunaPubScreen );
    set ( textToBrushWindow, MUIA_Window_Screen, ( IPTR )lunaPubScreen );   
    
    if ( GlobalPrefs.ScreenmodeType > 0 )
        set ( LunaBackdrop, MUIA_Window_Open, TRUE );
    set ( toolbox, MUIA_Window_Open, TRUE );
}

void Init_Backdrop ( )
{
    ULONG window_Height = lunaPubScreen->Height;
    ULONG window_Delta = lunaPubScreen->BarHeight;
    
    LunaBackdrop = WindowObject,
        MUIA_Window_Title, ( IPTR )NULL,
        MUIA_Window_ScreenTitle, ( IPTR )LUNA_SCREEN_TITLE,
        MUIA_Window_Screen, ( IPTR )lunaPubScreen,
        MUIA_Window_Backdrop, TRUE,
        MUIA_Window_Borderless, TRUE,
        MUIA_Window_CloseGadget, FALSE,
        MUIA_Window_SizeGadget, FALSE,
        MUIA_Window_DepthGadget, FALSE,
        MUIA_Window_DragBar, FALSE,
        MUIA_Window_Width, MUIV_Window_Width_Screen ( 100 ),
        MUIA_Window_Height, window_Height - window_Delta,
        MUIA_Window_TopEdge, window_Delta,
        MUIA_Window_LeftEdge, 0,
        MUIA_Window_UseBottomBorderScroller,        FALSE,
        MUIA_Window_UseRightBorderScroller,         FALSE,
        WindowContents, ( IPTR )VGroup,
            MUIA_Background, ( IPTR )"5:Lunapaint:data/backdrop.png",
            Child, RectangleObject, End,
        End,
    End;
}

void Init_AlertWindow ( )
{
    AlertWindow = WindowObject,
        MUIA_Window_Title, ( IPTR )"Alert",
        MUIA_Window_ScreenTitle, ( IPTR )"Alert",
        MUIA_Window_Screen, ( IPTR )lunaPubScreen,
        WindowContents, ( IPTR )VGroup,
            Child, ( IPTR )( AlertText = TextObject,
                MUIA_Text_Contents, ( IPTR )"",
            End ),
            Child, ( IPTR )( AlertOk = SimpleButton ( "OK" ) ),
        End,
    End;
    DoMethod ( 
        AlertWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
        AlertWindow, 3, MUIM_Set, MUIA_Window_Open, FALSE 
    );
    DoMethod ( 
        AlertOk, MUIM_Notify, MUIA_Pressed, FALSE,
        AlertWindow, 3, MUIM_Set, MUIA_Window_Open, FALSE 
    );
}

void ShowAlert ( unsigned char *text )
{
    set ( AlertText, MUIA_Text_Contents, ( IPTR )text );
    set ( AlertWindow, MUIA_Window_Open, TRUE );
}
