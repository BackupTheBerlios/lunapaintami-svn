/****************************************************************************
*                                                                           *
* parts.c -- Lunapaint, http://www.sub-ether.org/lunapaint                  *
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
#include "parts.h"

char *getFilename ( )
{		
    keyboardEnabled = FALSE;
    
    // Get filename
    struct Library *IntuitionBase, *AslBase;
    BOOL result;
    
    // Create the asl file requestor and get a filename
    IntuitionBase = OpenLibrary ( "intuition.library", 0 );
    AslBase = OpenLibrary ( "asl.library", 0 );
    aslfileReq = AllocAslRequestTags ( ASL_FileRequest,
        ASLFR_Screen, lunaPubScreen,
        TAG_DONE 
    );
    result = RequestFile ( aslfileReq );
    CloseLibrary ( IntuitionBase ); CloseLibrary ( AslBase );
    
    char *file = NULL; 
    if ( result != FALSE )
    {
        // See if we have both file and drawer
        if ( aslfileReq->fr_Drawer == NULL || aslfileReq->fr_File == NULL )
            goto end;
        
        file = AllocVec ( 255, MEMF_ANY|MEMF_CLEAR );
        
        // See if we have a trailing slash or a colon
        // This control is important to get right
        char lastChar = 0;
        BOOL hasColon = FALSE;
        int a = 0; for ( ; aslfileReq->fr_Drawer[ a ] != 0; a++ )
        {
            lastChar = aslfileReq->fr_Drawer[ a ]; 
            if ( lastChar == ':' ) hasColon = TRUE;
        }
        if ( !hasColon && lastChar != '/' ) 
        {
            sprintf ( file, "Lunapaint:%s/%s", aslfileReq->fr_Drawer, aslfileReq->fr_File );
        }
        else if ( hasColon && ( lastChar == '/' || lastChar == ':' ) )
        {
            sprintf ( file, "%s%s", aslfileReq->fr_Drawer, aslfileReq->fr_File );
        }
        else if ( hasColon && lastChar != '/' && lastChar != ':' )
        {
            sprintf ( file, "%s/%s", aslfileReq->fr_Drawer, aslfileReq->fr_File );
        }
        else 
        {
            FreeVec ( file );
            file = NULL;
        }
    }
    end:
    FreeFileRequest ( aslfileReq );
    
    keyboardEnabled = TRUE;
    
    return file;
}

unsigned int getFilesize ( char *filename )
{
    struct Library *DosBase;
    DosBase = OpenLibrary ( "dos.library", 0 );

    if ( filename != NULL )
    {
        FILE *fp = NULL;
        
        if ( ( fp = Open ( filename, MODE_OLDFILE ) ) != NULL )
        {
            unsigned int size = 0;
            Seek ( fp, 0, OFFSET_END );
            size = Seek ( fp, 0, OFFSET_BEGINNING ) + 2;
            Close ( fp );
            CloseLibrary ( DosBase );
            return size;
        }
    }
    
    CloseLibrary ( DosBase );
    
    return 0;
}

