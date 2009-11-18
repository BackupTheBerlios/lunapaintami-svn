/****************************************************************************
*                                                                           *
* preferences.c -- Lunapaint, http://www.sub-ether.org/lunapaint            *
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

#include "preferences.h"


AROS_UFH3 ( void, PrefsHandler_func,
    AROS_UFHA ( struct Hook*, h, A0 ),
    AROS_UFHA ( APTR, obj, A2 ),
    AROS_UFHA ( APTR, param, A1 )
)
{
    AROS_USERFUNC_INIT

    set ( PrefsWindow, MUIA_Window_Open, FALSE );

    BOOL save = *( BOOL *)param;

    int screenchoice = XGET ( PrefsCycScrType, MUIA_Cycle_Active );
    if ( screenchoice != GlobalPrefs.ScreenmodeType )
    {
        GlobalPrefs.ScreenmodeType = screenchoice;
        set ( PrefsCycScrType, MUIA_Cycle_Active, ( IPTR )GlobalPrefs.ScreenmodeType );
    }
    // Open the screen again and move windows
    if ( canvases != NULL )
    {
        ShowAlert ( "The screenmode can't be changed if you\nhave open canvas windows. Please\nclose any windows and save/use again." );
    }
    else
    {
        HideOpenWindows ( );
        InitLunaScreen ( );
        ReopenWindows ( );
    }

    // View
    GlobalPrefs.LayerBackgroundMode = XGET ( PrefsLayBackMode, MUIA_Cycle_Active );

    if ( save == 1 )
        savePreferences ( );

    AROS_USERFUNC_EXIT
}



void Init_PrefsWindow ( )
{
    static const char * titles[] = { "General", "View", NULL };
    static const char * scrntypes[] = { "Use Wanderer", "Clone Wanderer", NULL };
    static const char * layerbacktypes[] = { "Checkers", "Black", "Gray", "White", NULL };
    // Init the window itself
    PrefsWindow = WindowObject,
        MUIA_Window_Title, ( IPTR )"Preferences",
        MUIA_Window_SizeGadget, TRUE,
        MUIA_Window_Screen, ( IPTR )lunaPubScreen,
        MUIA_Window_CloseGadget, TRUE,
        WindowContents, ( IPTR )VGroup,
            Child, ( IPTR )RegisterObject,
                MUIA_Register_Titles, ( IPTR )titles,
                Child, ( IPTR )GroupObject,
                    MUIA_FrameTitle, ( IPTR )"Screen preferences",
                    MUIA_Frame, MUIV_Frame_Group,
                    Child, ( IPTR )( PrefsCycScrType = CycleObject,
                        MUIA_Cycle_Entries, ( IPTR )scrntypes,
                        MUIA_Cycle_Active, 0,
                    End ),
                End,
                Child, ( IPTR )GroupObject,
                    MUIA_FrameTitle, ( IPTR )"Layers",
                    MUIA_Frame, MUIV_Frame_Group,
                    Child, ( IPTR )( PrefsLayBackMode = CycleObject,
                        MUIA_Cycle_Entries, ( IPTR )layerbacktypes,
                        MUIA_Cycle_Active, 0,
                    End ),
                End,
            End,
            Child, ( IPTR )HGroup,
                Child, ( IPTR )( PrefsBtnSave = SimpleButton ( "Save" ) ),
                Child, ( IPTR )( PrefsBtnUse = SimpleButton ( "Use" ) ),
                Child, ( IPTR )( PrefsBtnCancel = SimpleButton ( "Cancel" ) ),
            End,
        End,
    End;

    PrefsHandler_hook.h_Entry = ( HOOKFUNC )&PrefsHandler_func;

    // Setting the prefs visible in the gui
    set ( PrefsCycScrType, MUIA_Cycle_Active, ( IPTR )GlobalPrefs.ScreenmodeType );
    set ( PrefsLayBackMode, MUIA_Cycle_Active, ( IPTR )GlobalPrefs.LayerBackgroundMode );

    DoMethod (
        PrefsWindow, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
        PrefsWindow, 3, MUIM_Set, MUIA_Window_Open, FALSE
    );
    DoMethod (
        PrefsBtnCancel, MUIM_Notify, MUIA_Pressed, FALSE,
        PrefsWindow, 3, MUIM_Set, MUIA_Window_Open, FALSE
    );
    DoMethod (
        PrefsBtnUse, MUIM_Notify, MUIA_Pressed, FALSE,
        PrefsBtnUse, 3, MUIM_CallHook, &PrefsHandler_hook, FALSE
    );
    DoMethod (
        PrefsBtnSave, MUIM_Notify, MUIA_Pressed, FALSE,
        PrefsBtnSave, 3, MUIM_CallHook, &PrefsHandler_hook, TRUE
    );

}

BOOL savePreferences ( )
{
    struct PrefHeader head = { 0 };
    struct IFFHandle *handle;
    BOOL result = FALSE;
    BPTR filehandle = NULL;

    if ( ( filehandle = Open ( "Lunapaint:lunapaint.prefs", MODE_NEWFILE ) ) == NULL )
        return FALSE;

    if ( !( handle = AllocIFF() ) )
    {
        goto prefs_save_ending;
    }

    handle->iff_Stream = ( IPTR )filehandle;

    InitIFFasDOS( handle );

    if ( OpenIFF( handle, IFFF_WRITE ) )
    {
        goto prefs_save_ending;
    }

    if ( PushChunk( handle, ID_PREF, ID_FORM, IFFSIZE_UNKNOWN ) )
    {
        goto prefs_save_ending;
    }

    head.ph_Version = ( IPTR )"0.1";
    head.ph_Type = 0;

    if ( PushChunk( handle, ID_PREF, ID_PRHD, IFFSIZE_UNKNOWN ) )
    {
        goto prefs_save_ending;
    }

    if ( !WriteChunkBytes( handle, &head, sizeof( struct PrefHeader ) ) )
    {
        goto prefs_save_ending;
    }

    if ( PopChunk( handle ) )
    {
        goto prefs_save_ending;
    }

    if ( PushChunk( handle, ID_PREF, ID_LUNAPAINT, sizeof( LunapaintPrefs ) ) )
    {
        goto prefs_save_ending;
    }

    // Save prefs struct
    if ( !WriteChunkBytes( handle, &GlobalPrefs, sizeof( LunapaintPrefs ) ) )
    {
        goto prefs_save_ending;
    }
    if ( PopChunk( handle ) )
    {
        goto prefs_save_ending;
    }

    // End saving
    if ( PopChunk(handle) )
    {
        goto prefs_save_ending;
    }

    result = TRUE;

    prefs_save_ending:

    if ( handle != NULL ) CloseIFF( handle );
    if ( filehandle != NULL ) Close ( filehandle );
    if ( handle != NULL ) FreeIFF( handle );

    return result;
}

BOOL loadPreferences ( )
{
    struct ContextNode *context;
    struct IFFHandle *handle;
    BOOL result = FALSE;
    BPTR filehandle = NULL;


    if ( !( handle = AllocIFF( ) ) )
        return FALSE;

    if ( ( filehandle = Open ( "Lunapaint:lunapaint.prefs", MODE_OLDFILE ) ) == NULL )
        goto prefs_load_ending;

    handle->iff_Stream = ( IPTR )filehandle;
    InitIFFasDOS( handle );

    if ( OpenIFF( handle, IFFF_READ ) )
        goto prefs_load_ending;

    if ( StopChunk( handle, ID_PREF, ID_LUNAPAINT ) )
        goto prefs_load_ending;

    if ( ParseIFF( handle, IFFPARSE_SCAN ) )
        goto prefs_load_ending;

    context = CurrentChunk( handle );

    if ( ReadChunkBytes ( handle, &GlobalPrefs, sizeof( LunapaintPrefs ) ) )
        goto prefs_load_ending;

    result = TRUE;

    prefs_load_ending:

    if ( handle != NULL ) CloseIFF(handle);
    if ( filehandle != NULL ) Close (filehandle);
    if ( handle != NULL ) FreeIFF(handle);

    return result;
}
