/****************************************************************************
*                                                                           *
* main.c -- Lunapaint, http://www.sub-ether.org/lunapaint                   *
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

#include <stdio.h>

#include <exec/types.h>
#include <libraries/mui.h>

#define AROS 1

#include "math/math.h"

#include "common.h"
#include "core/canvas.h"
#include "core/drawtools.h"
#include "core/tools.h"

#ifdef AROS
#define MUIM_Think 9999
#include "app/gui.h"
#include "app/parts.h"
#include <aros/debug.h>
#include "config.h"
#define DEBUG 1
#endif

char *Version = VERSIONSTRING;

void doEvents ( )
{
	if ( globalActiveWindow == NULL ) return;
	if ( globalActiveCanvas == NULL ) return;
		
	// Go through events
	if ( globalActiveWindow->isActive )
	{
		if ( MouseHasMoved )
		{
			/* If we get an action response it means the tool */
			/* Did something. So we then reset MouseHasMoved  */
			BOOL response = FALSE;
			switch ( globalCurrentTool )
			{
				case LUNA_TOOL_BRUSH:		
					response = toolBrush ( );
					break;	
				case LUNA_TOOL_FILL:		
					response = toolFill ( );				
					break;
				case LUNA_TOOL_LINE:		
					response = toolLine ( );				
					break;
				case LUNA_TOOL_RECTANGLE:		
					response = toolRectangle ( );				
					break;
				case LUNA_TOOL_CIRCLE:		
					response = toolCircle ( );				
					break;
				case LUNA_TOOL_CLIPBRUSH:		
					response = toolClipBrush ( );				
					break;
				case LUNA_TOOL_COLORPICKER:		
					response = toolColorPicker ( );				
					break;
				default:
					break;
			}
			if ( response )
				MouseHasMoved = FALSE;
		}		
		// Show outline of tool
		callToolPreview ( );
	}
}

int main ( int argc, char *argv[] )
{
	ULONG sigs;
	globalEvents = -1;
	globalCurrentTool = LUNA_TOOL_BRUSH;
	
	// Init brush
	brushTool.antialias = TRUE; // TODO: load from config set toolbox cycle!
	brushTool.width = 1;
	brushTool.height = 1;
	brushTool.feather = TRUE; // TODO: load from config and set toolbox cycle!
	
	// Set the colors
	prevColor = 0;
	currColor = 0;
	globalColor = 0;
	
	Init_Application ( );

	while ( getSignals ( &sigs ) )
	{
		// Check for signals from GUI
		if ( checkSignalBreak ( &sigs ) )
			break;	
		
		// Execute pending events
		doEvents ( );
		
		// Reset events
		globalEvents = -1;
		
		// Mouse clicks
		if ( mouseClickCount > 0 ) mouseClickCount--;
            
        // Delayed canvas redraw
        if ( redrawTimes == 1 )
        {
            if ( globalActiveWindow != NULL )
                DoMethod ( globalActiveWindow->area, MUIM_Draw );
            redrawTimes--;       
        }
        if ( redrawTimes > 0 ) redrawTimes--;
	}

	Exit_Application ( );

	return 0;
}