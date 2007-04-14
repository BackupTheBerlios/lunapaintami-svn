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

oCanvas* Init_Canvas ( 
    unsigned int w, unsigned int h, 
    unsigned int layers, unsigned int frames,
    BOOL generateBuffers
)
{
    int bufsize = w * h * 8;
    int scope = w * h;
    oCanvas *canv = AllocVec ( sizeof ( oCanvas ), MEMF_ANY );
    canv->screenbuffer = NULL;
    canv->buffer = NULL;
    
    // Reserve memory for all the images in the linked list:
    if ( generateBuffers )
    {
        canv->buffer = AllocVec ( sizeof ( gfxbuffer ), MEMF_ANY );
        gfxbuffer *tmp = canv->buffer;
        int span = layers * frames;
        int a = 0; for ( ; a < span; a++ )
        {	
            tmp->buf = AllocVec ( bufsize, MEMF_ANY );
            tmp->opacity = 100;
            tmp->visible = TRUE;
            tmp->name = AllocVec ( 12, MEMF_ANY|MEMF_CLEAR );
            strcpy ( tmp->name, "Empty layer" );
            
            // Clear the buffer
            int i = 0; for ( ; i < scope; i++ ) 
                tmp->buf[ i ] = TRANSCOLOR;
            
            if ( a < span - 1 )
                tmp->nextbuf = AllocVec ( sizeof ( gfxbuffer ), MEMF_ANY );
            else tmp->nextbuf = NULL; // <- marks the end
            tmp = tmp->nextbuf;
        }
    }
    
    // Set variables
    canv->width = w;
    canv->height = h;
    canv->zoom = 1;
    canv->offsetx = 0;
    canv->offsety = 0;
    canv->currentFrame = 0;
    canv->totalFrames = frames;
    canv->currentLayer = 0;
    canv->previousLayer = 0;
    canv->totalLayers = layers;
    canv->screenbuffer = NULL;
    canv->screenstorage = NULL;
    canv->tmpBuf = NULL;
    canv->swapbuffer = AllocVec ( bufsize, MEMF_ANY );
    int i = 0; for ( i = 0; i < scope; i++ ) canv->swapbuffer[ i ] = TRANSCOLOR;
    canv->winHasChanged = TRUE;
    return canv;
}

void NextFrame ( oCanvas *canvas )
{
    if ( canvas->totalFrames > canvas->currentFrame + 1 )
        canvas->currentFrame++;
    else canvas->currentFrame = 0;
    setActiveBuffer ( canvas );
}

void PrevFrame ( oCanvas *canvas )
{
    if ( canvas->currentFrame > 0 )
        canvas->currentFrame--;
    else canvas->currentFrame = canvas->totalFrames - 1;
    setActiveBuffer ( canvas );
}

void Destroy_Canvas ( oCanvas *canvas )
{	
    globalActiveCanvas = NULL;
    
    // Reset all pointers to data here
    if ( canvas->screenbuffer != NULL )
        FreeVec ( canvas->screenbuffer );
    if ( canvas->screenstorage != NULL )
        FreeVec ( canvas->screenstorage );
    if ( canvas->swapbuffer != NULL )
        FreeVec ( canvas->swapbuffer );
    if ( canvas->tmpBuf != NULL )
        FreeVec ( canvas->tmpBuf );
        
    canvas->activebuffer = NULL;
    globalActiveWindow->canvas = NULL;
    
    Destroy_Buffer ( canvas );
    
    if ( canvas != NULL )
        FreeVec ( canvas );
}

void Destroy_Buffer ( oCanvas *canv )
{
    gfxbuffer *buf = canv->buffer;
    canv->buffer = NULL;   
    while ( buf != NULL )
    {
        if ( buf->name != NULL ) FreeVec ( buf->name );
        if ( buf->buf != NULL ) FreeVec ( buf->buf );
        gfxbuffer *tmp = buf;
        buf = buf->nextbuf;
        FreeVec ( tmp );
    }
}

void setActiveBuffer ( oCanvas *canvas )
{
    if ( canvas == NULL ) return;
    gfxbuffer *buf = canvas->buffer;
    int i = 0; while ( buf != NULL )
    {
        int f = i / canvas->totalLayers;
        int l = i % canvas->totalLayers;
        
        if ( f == canvas->currentFrame && l == canvas->currentLayer ) 
        {
            canvas->activebuffer = buf->buf;
            return;
        }
        i++;
        buf = buf->nextbuf;
    }
    return;
}

unsigned long long int *getNextFrame ( oCanvas *canvas )
{
    gfxbuffer *buf = canvas->buffer;
    int target = ( canvas->currentFrame + 1 ) % canvas->totalFrames;
    int size = canvas->totalFrames * canvas->totalLayers;
    int i = 0; for ( ; i < size; i++ )
    {
        int l = i % canvas->totalLayers;
        if ( l == canvas->currentLayer )
            if ( i / canvas->totalLayers == target ) return buf->buf;
        buf = buf->nextbuf;
    }
    return NULL;
}

unsigned long long int *getPrevFrame ( oCanvas *canvas )
{
    gfxbuffer *buf = canvas->buffer;
    int target = ( canvas->currentFrame - 1 );
    if ( target < 0 ) target = canvas->totalFrames - 1;
    int size = canvas->totalFrames * canvas->totalLayers;
    int i = 0; for ( ; i < size; i++ )
    {
        int l = i % canvas->totalLayers;
        if ( l == canvas->currentLayer )
            if ( i / canvas->totalLayers == target ) return buf->buf;
        buf = buf->nextbuf;
    }
    return NULL;
}

gfxbuffer *getActiveGfxbuffer ( oCanvas *canvas )
{
    gfxbuffer *buf = canvas->buffer;
    int size = canvas->totalFrames * canvas->totalLayers;
    int i = 0; for ( ; i < size; i++ )
    {
        int l = i % canvas->totalLayers;
        if ( l == canvas->currentLayer )
            if ( i / canvas->totalLayers == canvas->currentFrame ) return buf;
        buf = buf->nextbuf;
    }
    return NULL;
}

gfxbuffer *getGfxbuffer ( oCanvas *canvas, int layer, int frame )
{
    gfxbuffer *buf = canvas->buffer;
    int size = canvas->totalFrames * canvas->totalLayers;
    int i = 0; for ( ; i < size; i++ )
    {
        int l = i % canvas->totalLayers;
        if ( l == layer )
            if ( i / canvas->totalLayers == frame ) return buf;
        buf = buf->nextbuf;
    }
    return NULL;
}

gfxbuffer *getGfxbufferFromList ( gfxbuffer *buf, int layer, int frame, int totallayers, int totalframes )
{
    if ( buf == NULL ) return ( gfxbuffer *)NULL;
    int size = totalframes * totallayers;
    int i = 0; for ( ; i < size; i++ )
    {
        int f = i / totallayers;
        int l = i % totallayers;
        if ( f == frame && l == layer ) return buf;
        buf = buf->nextbuf;
    }
    return NULL;
}

inline unsigned int *renderCanvas ( 
    oCanvas *canvas, unsigned int rx, unsigned int ry, 
    unsigned int rw, unsigned int rh, BOOL Transparent
)
{	
    // Some vars
    int layerNum = canvas->totalLayers;
    int frameNum = canvas->totalFrames;
    int zoomedWidth = rw * canvas->zoom;
    int zoom = canvas->zoom;
    int zoomZoomwidth = zoom * zoomedWidth;
    int layerNumAndTrans = layerNum + 1; // with transparent layer   
    int len = frameNum * layerNumAndTrans;
    int ryrh = ry + rh;
    int rxrw = rx + rw;

    // Allocate memory for temporary buffer
    unsigned int *tempBuf = AllocVec ( zoomedWidth * ( rh * zoom ) * 4, MEMF_ANY );
    if ( tempBuf == NULL ){ return NULL; };
    
    // Start!
    gfxbuffer *buf = canvas->buffer;
    int i = 0; for ( ; i < len; i++ )
    {
        int l = ( i % layerNumAndTrans ) - 1;
        int f = i / layerNumAndTrans;
        // Skip irrelevant layers
        if ( l >= 0 && ( buf->opacity == 0 || !buf->visible ) ) 
        {
            buf = buf->nextbuf;
            continue;
        }
        if ( f == canvas->currentFrame )
        {
            unsigned long long int *currentBuf = buf->buf;
            unsigned long long int *onionBuf = getPrevFrame ( canvas );      
                
            int y = ry, YminRY = 0; for ( ; y < ryrh; y ++ )
            {
                if ( y >= canvas->height ) break;
                
                // Some often used calcs :-)    
                int tboffsety = YminRY * zoomZoomwidth; // <- heavily optimized! 
                int canvymul = canvas->width * y;
                int YminRYrw = YminRY * rw;
                
                int x = rx, XminRX = 0; for ( ; x < rxrw; x++ )
                {
                    if ( x >= canvas->width ) break;
                    
                    // Color that it will result in
                    unsigned int color = 0;
                    
                    // Make "checks" in RGBA
                    if ( l == -1 )
                    {
                        if ( Transparent )
                            color = *( unsigned int *)&( ( rgba32 ){ 128, 128, 128, 0 } );
                        else
                        {               
                            switch ( GlobalPrefs.LayerBackgroundMode )
                            {
                                case 1:
                                    color = *( unsigned int *)&( ( rgba32 ){ 0, 0, 0, 255 } );
                                    break;
                                case 2:
                                    color = *( unsigned int *)&( ( rgba32 ){ 128, 128, 128, 255 } );
                                    break;
                                case 3:
                                    color = *( unsigned int *)&( ( rgba32 ){ 255, 255, 255, 255 } );
                                    break;
                                default:
                                    {
                                        color = *( unsigned int *)&( ( rgba32 ){ 128, 128, 128, 255 } );
                                        int checkCol = ( ( x % 32 ) + ( int )( y / 16 ) * 16 ) % 32;
                                        if ( checkCol >= 16 ) 
                                            color = *( unsigned int *)&( ( rgba32 ){ 90, 90, 90, 255 } );
                                    }
                                    break;
                            }
                        }
                    }
                    else
                    {
                        // Get tempbuffer color and current layer color
                        int boffset = XminRX * zoom + tboffsety;
                        rgba32 col1 = *( rgba32 *)&tempBuf[ boffset ];	
                        
                        // Get color from current canvas buffer
                        rgba64 col2t = *( rgba64 *)&currentBuf[ canvymul + x ];
                        
                        // Convert 16-bit pr pixel to 8
                        int     a = col2t.r / 256, 
                                r = col2t.g / 256, 
                                g = col2t.b / 256, 
                                b = col2t.a / 256;
                                
                        
                        // Apply layer opacity
                        if ( buf->opacity < 100 ) a = a / 100.0 * buf->opacity;
                                
                        // If current canvas is transparent, disregard it's colors
                        if ( a <= 0 ){ r = col1.r; g = col1.g; b = col1.b; a = 0; }
                        
                        // Mix colors
                        double alpha = a / 255.0;
                        col1.r -= ( col1.r - r ) * alpha;
                        col1.g -= ( col1.g - g ) * alpha;
                        col1.b -= ( col1.b - b ) * alpha;
                        if ( !Transparent ) 
                            col1.a = 255;
                        else
                        {
                            if ( ( col1.a + a ) > 255 ) col1.a = 255;
                            else col1.a += a; 
                        }
                        
                        // Add onion skin if needed
                        if ( canvas->onion == 1 )
                        {
                            rgba64 onioncol = *( rgba64 *)&onionBuf[ canvymul + x ];
                            r = onioncol.a / 256;
                            g = onioncol.b / 256;
                            b = onioncol.g / 256;
                            a = onioncol.r / 256;
                            
                            // Mix colors
                            alpha = a / 512.0;
                            col1.r -= ( col1.r - r ) * alpha;
                            col1.g -= ( col1.g - g ) * alpha;
                            col1.b -= ( col1.b - b ) * alpha;
                        }
                        
                        // Check if area needs tool preview
                        // (only when no buttons are down)
                        if ( globalCurrentTool != -1 && l == canvas->currentLayer )
                        {
                            unsigned int test = drawToolPreview ( x, y );
                            if ( test != 0 )
                            {
                                rgba32 blend = *( rgba32 *)&test;
                                double balph = blend.a / 255.0;
                                col1.r -= ( col1.r - blend.b ) * balph;
                                col1.g -= ( col1.g - blend.g ) * balph;
                                col1.b -= ( col1.b - blend.r ) * balph;
                                if ( col1.a + blend.a < 255 ) 
                                    col1.a += blend.a; 
                                else col1.a = 255;
                            }
                        }
                        
                        // ( we skip col1.a as we know it's 255 from the checkCol )
                        color = *( unsigned int *)&col1;
                    }
                    
                    if ( zoom == 1 )
                        tempBuf[ YminRYrw + XminRX ] = color;
                    else
                    {
                        // Zoom over on the tempBuf
                        int xzoom = XminRX * zoom;
                        int yzoom = YminRY * zoom;
                        int zy = 0; for ( ; zy < zoom; zy++ )
                        {
                            int zoompos = ( yzoom + zy ) * zoomedWidth + xzoom;
                            int zx = 0; for ( ; zx < zoom; zx++ )
                                tempBuf[ zoompos + zx ] = color;
                        }
                    }
                    XminRX++;
                }
                YminRY++;
            }
        }
        if ( l >= 0 ) buf = buf->nextbuf;
    }
    return tempBuf;
}

BOOL redrawScreenbufferRect ( 
    oCanvas *canvas, unsigned int rx, unsigned int ry, unsigned int rw, unsigned int rh, 
    BOOL updateStorage
)
{	
    // Get an unsigned int
    unsigned int *tempBuf = renderCanvas ( canvas, rx, ry, rw, rh, FALSE );
    if ( tempBuf == NULL ) return FALSE;
    
    // Some vars
    int zoom = canvas->zoom;
    int zoomedWidth = rw * zoom;
    int zoomedHeight = rh * zoom;
    
    // Update screenbuffer:
    
    int datalen = canvas->visibleWidth * canvas->visibleHeight;   
    int datasize = 4 * datalen;
    int limitx = zoomedWidth < canvas->visibleWidth ? zoomedWidth : canvas->visibleWidth;
    int limity = zoomedHeight < canvas->visibleHeight ? zoomedHeight : canvas->visibleHeight;   
    
    
    if ( canvas->screenbuffer != NULL )
        FreeVec ( canvas->screenbuffer );
    canvas->screenbuffer = AllocVec ( datasize, MEMF_ANY );	
    
    int y = 0, x = 0; rgba32 tmp = { 0, 0, 0, 0 };
    for ( ; y < limity; y++ )
    {	
        int tempzoomy 		= zoomedWidth * y;
        int screenzoomy 	= canvas->visibleWidth * y;
        
        for ( x = 0; x < limitx; x++ )
        {
            // Set resulting color in ABGR format
            tmp = *( rgba32 *)&tempBuf[ tempzoomy + x ];
            canvas->screenbuffer[ screenzoomy + x ] = *( unsigned int *)&tmp;
        }
    }
    
    // Update screen storage buffer (not while using tools)
    if ( canvas->winHasChanged && updateStorage && canvas->screenbuffer != NULL )
    {
        if ( canvas->screenstorage != NULL )
        {
            FreeVec ( canvas->screenstorage );
            canvas->screenstorage = NULL;
        }
        canvas->screenstorage = AllocVec ( datasize, MEMF_ANY );
        canvas->scrStorageWidth = canvas->visibleWidth;
        canvas->scrStorageHeight = canvas->visibleHeight;
        memcpy ( canvas->screenstorage, canvas->screenbuffer, datasize );
        canvas->winHasChanged = FALSE;
    }
    
    // Free temp buffer and exit function
    if ( tempBuf != NULL ) FreeVec ( tempBuf );
    return TRUE;
}

BOOL redrawScreenbuffer ( oCanvas *canvas )
{
    int offsetx 	= canvas->offsetx / canvas->zoom;
    int offsety 	= canvas->offsety / canvas->zoom;
    int width 		= canvas->visibleWidth / canvas->zoom + 1;
    int height 		= canvas->visibleHeight / canvas->zoom + 1;
    
    if ( !redrawScreenbufferRect ( canvas, offsetx, offsety, width, height, TRUE ) )
        return FALSE;
    return TRUE;
}


unsigned int canvasToARGBPixel ( unsigned long long int source )
{
    // Fetch RGBA from 64-bit source;
    unsigned long long int R, G, B, A;
    R = source;         R = R >> 48;
    G = source << 16;   G = G >> 48;
    B = source << 32;   B = B >> 48;
    A = source << 48;   A = A >> 48;
    
    // Convert to 8-bit integer
    int r, g, b, a;
    r = ( int )( R / 256 );
    g = ( int )( G / 256 );
    b = ( int )( B / 256 );
    a = ( int )( A / 256 );
    
    unsigned int pixel = ( r << 8 ) | ( g << 16 ) | ( b << 24 ) | 0;
    return pixel;
}

unsigned int canvasToWindowPixel ( unsigned long long int source )
{
    // Fetch RGBA from 64-bit source;
    unsigned long long int R, G, B, A;
    R = source; 				R = R >> 48;
    G = source << 16;		G = G >> 48;
    B = source << 32; 	B = B >> 48;
    A = source << 48; 	A = A >> 48;
    
    // Convert to 8-bit integer
    int r, g, b, a;
    r = ( int )( R / 256 );
    g = ( int )( G / 256 );
    b = ( int )( B / 256 );
    a = ( int )( A / 256 );
    
    unsigned int pixel = ( r ) | ( g << 8 ) | ( b << 16 ) | 0;
    return pixel;
}

void initPalette ( )
{
    // Reserve memory
    globalPalette = AllocVec ( 256 * sizeof ( unsigned int ), MEMF_ANY );
    // Load in default palette
    loadDefaultPalette ( );
}

rgbData paletteColorToRGB ( unsigned int color )
{
    unsigned int r, g, b;
    r = color << 24; 	r = r >> 24;
    g = color << 16; 	g = g >> 24;
    b = color << 8; 	b = b >> 24;
    
    rgbData mydata = { r, g, b }; 
    return mydata;
}

rgbaDataL canvasColorToRGBA_ull ( unsigned long long int color )
{
    unsigned long long int r, g, b, a;
    r = color >> 48;
    g = color << 16; g = g >> 48;
    b = color << 32; b = b >> 48;
    a = color << 48; a = a >> 48;
    
    rgbaDataL mydata = { r, g, b, a };
    return mydata;
}

rgbaData canvasColorToRGBA ( unsigned long long int color )
{
    rgbaDataL rgbal = canvasColorToRGBA_ull ( color );
    unsigned int r, g, b, a;
    r = rgbal.r / 256;
    g = rgbal.g / 256;
    b = rgbal.b / 256;
    a = rgbal.a / 256;
    rgbaData rgba = { r, g, b, a };
    return rgba;
}

unsigned int RGBtoPaletteColor ( unsigned int r, unsigned int g, unsigned int b )
{
    unsigned int rgb = ( r ) | ( g << 8 ) | ( b << 16 );
    return rgb;
}

unsigned int bufferToScreenColor ( unsigned long long int col )
{
    rgbaDataL rgbal = canvasColorToRGBA_ull ( col );
    unsigned int r, g, b, a;
    r = rgbal.r / 256;
    g = rgbal.g / 256;
    b = rgbal.b / 256;
    a = rgbal.a / 256;
    return ( unsigned int )( r << 24 | g << 16 | b << 8 | a );
}

rgbaData bufferToRGBA ( unsigned int color )
{
    unsigned int r, g, b, a;
    r = color >> 24;
    g = color << 8; 	g = g >> 24;
    b = color << 16; 	b = b >> 24;
    a = color << 24;	a = a >> 24;
    rgbaData mydata = { r, g, b, a }; 
    return mydata;
}

unsigned long long int PaletteToBuffercolor ( unsigned int rgb )
{
    unsigned long long int r, g, b, rgba;
    r = ( rgb << 24 ) >> 24; r = ( int )( r / 255.0 * MAXCOLOR );
    g = ( rgb << 16 ) >> 24; g = ( int )( g / 255.0 * MAXCOLOR );
    b = ( rgb << 8  ) >> 24; b = ( int )( b / 255.0 * MAXCOLOR );
    rgba = r << 48 | g << 32 | b << 16 | MAXCOLOR;
    return rgba;
}

rgba64 PaletteToRgba64 ( unsigned int rgb )
{
    unsigned long long int r, g, b;
    r = ( rgb << 24 ) >> 24; r = ( int )( r / 255.0 * MAXCOLOR );
    g = ( rgb << 16 ) >> 24; g = ( int )( g / 255.0 * MAXCOLOR );
    b = ( rgb << 8  ) >> 24; b = ( int )( b / 255.0 * MAXCOLOR );
    return ( rgba64 ){ r, g, b, MAXCOLOR };
}

void loadDefaultPalette ( )
{	
    BPTR myfile;
    if ( ( myfile = Open ( "Lunapaint:default.palette", MODE_OLDFILE ) ) != NULL )
    {
        Read ( myfile, globalPalette, 1024 );
        Close ( myfile );
    }
    else printf ( "Failed to load default palette..\n" );
}

void addLayer ( oCanvas *canv )
{	
    gfxbuffer *pos = canv->buffer;
    
    // Go though each frame
    int size = canv->width * canv->height;
    int f = 0; for ( ; f < canv->totalFrames; f++ )
    {
        // Go to the last layer
        int l = 0; for ( ; l < canv->totalLayers - 1; l++ ) pos = pos->nextbuf;
        // Add new blank layer
        gfxbuffer *newlayer = AllocVec ( sizeof ( gfxbuffer ), MEMF_ANY );
        newlayer->buf = AllocVec ( size * 8, MEMF_ANY );
        int i = 0; for ( ; i < size; i++ ) newlayer->buf[ i ] = TRANSCOLOR;     
        newlayer->opacity = 100;
        newlayer->visible = TRUE;
        newlayer->nextbuf = pos->nextbuf;
        newlayer->name = AllocVec ( 10, MEMF_ANY|MEMF_CLEAR );
        strcpy ( newlayer->name, "New layer" );
        pos->nextbuf = newlayer;
        pos = newlayer->nextbuf;
    }
    
    // New layer num
    // Increment total layers
    canv->totalLayers++;
    canv->currentLayer = canv->totalLayers - 1;
    setActiveBuffer ( canv );
}

void swapLayers ( oCanvas *canv )
{
    gfxbuffer *curr = NULL, *prev = NULL, *pos = canv->buffer;
    
    // Go through frames and layers with pos and store addy to 
    // current gfxbuffer and previous selected gfxbuffer

    int size = canv->totalFrames * canv->totalLayers;
    int i = 0; for ( ; i < size; i++ )
    {
        int l = i % canv->totalLayers;
        int f = i / canv->totalLayers;
    if ( f == canv->currentFrame && l == canv->currentLayer )
            curr = pos;
        if ( f == canv->currentFrame && l == canv->previousLayer )
            prev = pos;
        pos = pos->nextbuf;
    }
    if ( prev == NULL || curr == NULL )
        return;
    
    // Temporarily store some values
    unsigned long long int *tmp = curr->buf;
    char opacity = curr->opacity;
    unsigned char *name = curr->name;
    BOOL visible = curr->visible;
    
    // New data
    curr->buf = prev->buf; 
    curr->opacity = prev->opacity;
    curr->visible = prev->visible;
    curr->name = prev->name;
    
    // Move old data
    prev->buf = tmp;
    prev->opacity = opacity;
    prev->visible = visible;
    prev->name = name;
    
    // Set active buffer
    setActiveBuffer ( canv );
}

void copyLayers ( oCanvas *canv )
{
    gfxbuffer *curr = NULL, *prev = NULL, *pos = canv->buffer;
    
    // Go through frames and layers with pos and store addy to 
    // current gfxbuffer and previous selected gfxbuffer
    int f; for ( f = 0; f < canv->totalFrames; f++ )
    {
        int l; for ( l = 0; l < canv->totalLayers; l++ )
        {
            if ( f == canv->currentFrame && l == canv->currentLayer )
                curr = pos;
            if ( f == canv->currentFrame && l == canv->previousLayer )
                prev = pos;
            pos = pos->nextbuf;
        }
    }
    // Copy prev buffer to current buffer
    memcpy ( curr->buf, prev->buf, canv->width * canv->height * 8 );
}

void mergeLayers ( oCanvas *canv )
{
    if ( canv->currentLayer == canv->previousLayer )
        return;
        
    gfxbuffer *curr = NULL, *prev = NULL, *pos = canv->buffer;
    
    // Go through frames and layers with pos and store addy to 
    // current gfxbuffer and previous selected gfxbuffer
    int f = 0; for ( ; f < canv->totalFrames; f++ )
    {
        int l = 0; for ( ; l < canv->totalLayers; l++ )
        {
            if ( f == canv->currentFrame && l == canv->currentLayer )
                curr = pos;
            if ( f == canv->currentFrame && l == canv->previousLayer )
                prev = pos;
            pos = pos->nextbuf;
        }
    }

    // Mix colors
    
    double popacity = prev->opacity * 1.0;
    
    int i = 0; for ( ; i < canv->width * canv->height; i++ )
    {	
        rgba64 col1 = *( rgba64 *)&curr->buf[ i ];
        col1 = ( rgba64 ){ col1.a, col1.b, col1.g, col1.r };
        if ( curr->opacity < 100 ) col1.a = col1.a / 100.0 * curr->opacity;
        
        rgba64 col2 = *( rgba64 *)&prev->buf[ i ];	
        col2 = ( rgba64 ){ col2.a, col2.b, col2.g, col2.r };
        if ( prev->opacity < 100 ) col2.a = col2.a / 100.0 * prev->opacity;      
        double alphx = col2.a / 100.0 * popacity;      	
        
        // Disregard fully transparent source colors
        if ( col1.a <= 0 )
        { 
            col1.r = col2.r; col1.g = col2.g; 
            col1.b = col2.b; col1.a = col2.a;
        }
        else if ( alphx > 0 )
        {
            double alpha = alphx / MAXCOLOR_DBL;  
            col1.r -= ( ( int )col1.r - ( int )col2.r ) * alpha;
            col1.g -= ( ( int )col1.g - ( int )col2.g ) * alpha;
            col1.b -= ( ( int )col1.b - ( int )col2.b ) * alpha;
            if ( ( col1.a + alphx ) > MAXCOLOR ) col1.a = MAXCOLOR;
            else col1.a += alphx;
        }
        
        // Set new color
        curr->buf[ i ] = *( unsigned long long int *)&( ( rgba64 ){ col1.a, col1.b, col1.g, col1.r } );
    }
    canv->currentLayer = canv->previousLayer;
    deleteLayer ( canv );
}

void deleteLayer ( oCanvas *canv )
{
    // Just clear the last remaining layer..
    // we must have at least 1 layer!
    
    if ( canv->totalLayers <= 1 )
    {
        int size = canv->width * canv->height;
        int i = 0; for ( ; i < size; i++ ) canv->buffer->buf[ i ] = TRANSCOLOR;
        return;
    }
    
    // Delete the active layer on all frames
    
    gfxbuffer **pos = &canv->buffer;
    
    int f = 0; for ( ; f < canv->totalFrames; f++ )
    {
        int l = 0; for ( ; l < canv->totalLayers && pos != NULL; l++ )
        {
            if ( *pos == NULL ) goto deletelayerend;
                
            // Strip this
            if ( l == canv->currentLayer )
            {
                gfxbuffer *tmp = *pos;
                *pos = tmp->nextbuf;
                // Free layer from mem
                if ( tmp->buf != NULL )
                    FreeVec ( tmp->buf );
                if ( tmp->name != NULL )
                    FreeVec ( tmp->name );
                tmp->nextbuf = NULL;
                FreeVec ( tmp );
            }
            else
            {
                gfxbuffer *tmp = *pos;
                pos = &tmp->nextbuf;
            }
        }
    }
    deletelayerend:
    
    // Store the new layer num
    canv->totalLayers--;
    if ( canv->currentLayer >= canv->totalLayers )
        canv->currentLayer--;
    canv->previousLayer = canv->currentLayer;
    setActiveBuffer ( canv );
}

void copyToSwap ( oCanvas *canv )
{
    memcpy ( canv->swapbuffer, canv->activebuffer, canv->width * canv->height * 8 );
}

void swapCanvasBuffers ( oCanvas *canv )
{
    if ( globalActiveCanvas != NULL )
    {
        gfxbuffer *pos = canv->buffer;
        if ( !canv->activebuffer ) return;
        int f = 0; for ( ; f < canv->totalFrames; f++ )
        {
            int l = 0; for ( ; l < canv->totalLayers; l++ )
            {
                if ( l == canv->currentLayer && f == canv->currentFrame )
                {
                    unsigned long long int *tmp = canv->swapbuffer;
                    canv->swapbuffer = pos->buf;
                    canv->activebuffer = tmp;
                    pos->buf = tmp;
                    return;
                }
                pos = pos->nextbuf;
            }
        }
    }
}

unsigned int drawToolPreview ( int x, int y )
{	
    switch ( globalCurrentTool )
    {
        
        case LUNA_TOOL_BRUSH:
            
            if ( !MouseButtonL && !MouseButtonR )
            {
                
                double xoff = 0, yoff = 0;
                if ( brushTool.width > 1 ) xoff = brushTool.width / 2.0;
                if ( brushTool.height > 1 ) yoff = brushTool.height / 2.0;
                
                int cbx = ( int )( cMouseX - xoff );
                int cby = ( int )( cMouseY - yoff );
                
                int cbxX = x - cbx;
                int cbyY = y - cby;
                
                if ( 
                    cbxX >= 0 && cbxX < brushTool.width &&
                    cbyY >= 0 && cbyY < brushTool.height
                )
                {
                    int bufpos = ( cbyY * brushTool.width ) + cbxX;
                    rgba64 *color = ( rgba64 *)&brushTool.buffer[ bufpos ];
                    
                    if ( color->r > 0 )
                    {
                        return 
                            ( color->r / 256 ) << 24 | 
                            ( color->g / 256 ) << 16 | 
                            ( color->b / 256 ) << 8 | 
                            ( color->a / 256 );
                    }
                }
            }	
            break;
        
        case LUNA_TOOL_COLORPICKER:
        case LUNA_TOOL_FILL:
        
            if ( !MouseButtonL && !MouseButtonR )
            {
                if ( x == ( int )cMouseX && y == ( int )cMouseY )
                {
                    rgbData data = paletteColorToRGB ( globalPalette[ currColor ] );
                    return ( 255 << 24 ) | ( data.b << 16 ) | ( data.g << 8 ) | data.r;
                }
            }
            break;
        
        case LUNA_TOOL_LINE:
        
            if ( lineTool.mode == 0 )
            {
                double xoff = 0, yoff = 0;
                if ( brushTool.width > 1 ) xoff = brushTool.width / 2.0;
                if ( brushTool.height > 1 ) yoff = brushTool.height / 2.0;
                
                int cbx = ( int )( ( int )cMouseX - xoff );
                int cby = ( int )( ( int )cMouseY - yoff );
                
                int cbxX = x - cbx;
                int cbyY = y - cby;
                
                if ( 
                    cbxX >= 0 && cbxX < brushTool.width &&
                    cbyY >= 0 && cbyY < brushTool.height
                )
                {
                    int bufpos = ( cbyY * brushTool.width ) + cbxX;
                    rgba64 *color = ( rgba64 *)&brushTool.buffer[ bufpos ];
                    if ( color->r > 0 )
                    {
                        return 
                            ( color->r / 256 ) << 24 | 
                            ( color->g / 256 ) << 16 | 
                            ( color->b / 256 ) << 8 | 
                            ( color->a / 256 );
                    }
                }
            }
            if ( lineTool.mode == 1 )
            {	
                // inside the lineTool.buffer the line is drawn with the offset of
                // the brushsize / 2 to center it
                if ( 
                    x >= lineTool.ox && y >= lineTool.oy && 
                    x < lineTool.ox + lineTool.w && y < lineTool.oy + lineTool.h
                )
                {
                    int dataoff = ( ( int )( y - lineTool.oy ) * ( int )lineTool.w ) + ( int )( x - lineTool.ox );
                    rgba64 *color = ( rgba64 *)&lineTool.buffer[ dataoff ];
                    
                    if ( color->r > 0 )
                    {
                        return 
                            ( color->r / 256 ) << 24 | 
                            ( color->g / 256 ) << 16 | 
                            ( color->b / 256 ) << 8 | 
                            ( color->a / 256 );
                    }
                }
            }
            break;	
            
        case LUNA_TOOL_RECTANGLE:
        
            if ( rectangleTool.mode == 0 )
            {
                double xoff = 0, yoff = 0;
                if ( brushTool.width > 1 ) xoff = brushTool.width / 2.0;
                if ( brushTool.height > 1 ) yoff = brushTool.height / 2.0;
                
                int cbx = ( int )( ( int )cMouseX - xoff );
                int cby = ( int )( ( int )cMouseY - yoff );
                
                int cbxX = x - cbx;
                int cbyY = y - cby;
                
                if ( 
                    cbxX >= 0 && cbxX < brushTool.width &&
                    cbyY >= 0 && cbyY < brushTool.height
                )
                {
                    int bufpos = ( cbyY * brushTool.width ) + cbxX;
                    rgba64 *color = ( rgba64 *)&brushTool.buffer[ bufpos ];
                    if ( color->r > 0 )
                    {
                        return 
                            ( color->r / 256 ) << 24 | 
                            ( color->g / 256 ) << 16 | 
                            ( color->b / 256 ) << 8 | 
                            ( color->a / 256 );
                    }
                }
            }
            if ( rectangleTool.mode == 1 )
            {	
                // inside the rectangleTool.buffer the line is drawn with the offset of
                // the brushsize / 2 to center it
                if ( 
                    x >= rectangleTool.ox && y >= rectangleTool.oy && 
                    x < rectangleTool.ox + rectangleTool.w && y < rectangleTool.oy + rectangleTool.h
                )
                {
                    int dataoff = ( ( int )( y - rectangleTool.oy ) * ( int )rectangleTool.w ) + ( int )( x - rectangleTool.ox );
                    rgba64 *color = ( rgba64 *)&rectangleTool.buffer[ dataoff ];
                    
                    if ( color->r > 0 )
                    {
                        return 
                            ( color->r / 256 ) << 24 | 
                            ( color->g / 256 ) << 16 | 
                            ( color->b / 256 ) << 8 | 
                            ( color->a / 256 );
                    }
                }
            }
            break;	
        
        case LUNA_TOOL_CIRCLE:
        
            // Brush preview part
            if ( circleTool.mode == 0 )
            {	
                double xoff = 0, yoff = 0;
                if ( brushTool.width > 1 ) xoff = brushTool.width / 2.0;
                if ( brushTool.height > 1 ) yoff = brushTool.height / 2.0;
                
                int cbx = ( int )( ( int )cMouseX - xoff );
                int cby = ( int )( ( int )cMouseY - yoff );
                
                int cbxX = x - cbx;
                int cbyY = y - cby;
                
                if ( 
                    cbxX >= 0 && cbxX < brushTool.width &&
                    cbyY >= 0 && cbyY < brushTool.height
                )
                {
                    int bufpos = ( cbyY * brushTool.width ) + cbxX;
                    rgba64 *color = ( rgba64 *)&brushTool.buffer[ bufpos ];
                    // Notice the reverse alignment here
                    if ( color->r > 0 )
                    {
                        return 
                            ( color->r / 256 ) << 24 | 
                            ( color->g / 256 ) << 16 | 
                            ( color->b / 256 ) << 8 | 
                            ( color->a / 256 );
                    }
                }
            }
            // Circle preview part
            if ( circleTool.mode == 1 )
            {	
                // inside the circleTool.buffer the line is drawn with the offset of
                // the brushsize / 2 to center it
                
                if ( 
                    x >= circleTool.ox && 
                    y >= circleTool.oy && 
                    x < circleTool.ox + circleTool.bufwidth && 
                    y < circleTool.oy + circleTool.bufheight
                )
                {
                    int dataoff = ( ( y - circleTool.oy ) * circleTool.bufwidth ) + ( x - circleTool.ox );
                
                    rgba64 *color = ( rgba64 *)&circleTool.buffer[ dataoff ];
                    
                    if ( color->r > 0 )
                    {
                        return 
                            ( color->r / 256 ) << 24 | 
                            ( color->g / 256 ) << 16 | 
                            ( color->b / 256 ) << 8 | 
                            ( color->a / 256 );
                    }
                }
            }
            break;	
            
        case LUNA_TOOL_CLIPBRUSH:
        
            if ( clipbrushTool.mode == 0 )
            {
                if ( x == cMouseX && y == cMouseY )
                {
                    rgbData d = paletteColorToRGB ( globalPalette[ currColor ] );
                    return ( 255 << 24 ) | ( d.b << 16 ) | ( d.g << 8 ) | d.r;
                }
            }
            if ( clipbrushTool.mode == 1 )
            {	
                // inside the rectangleTool.buffer the line is drawn with the offset of
                // the brushsize / 2 to center it
                if ( 
                    x >= clipbrushTool.ox && y >= clipbrushTool.oy && 
                    x < clipbrushTool.ox + clipbrushTool.w && y < clipbrushTool.oy + clipbrushTool.h
                )
                {
                    int dataoff = ( ( int )( y - clipbrushTool.oy ) * ( int )clipbrushTool.w ) + ( int )( x - clipbrushTool.ox );
                    rgba64 *color = ( rgba64 *)&clipbrushTool.buffer[ dataoff ];
                    
                    // Notice the reverse alignment here
                    if ( color->r > 0 )
                    {
                        return 
                            ( color->r / 256 ) << 24 | 
                            ( color->g / 256 ) << 16 | 
                            ( color->b / 256 ) << 8 | 
                            ( color->a / 256 );
                    }
                }
            }
            break;	
        
        default: 
            break;
    }
    return 0;
}

// Valuelist used to record if there is a pixel somewhere or not
void addListValue ( double x, double y, ValueList **lst )
{		
    ValueList *new = AllocVec ( sizeof ( ValueList ), MEMF_ANY|MEMF_CLEAR );
    if ( *lst != NULL )
    {
        new->x = ( *lst )->x;
        new->y = ( *lst )->y;
        ( *lst )->x = x;
        ( *lst )->y = y;
    }
    else
    {
        new->x = x;
        new->y = y;
    }
    new->Next = *lst;
    *lst = new;
}
void freeValueList ( ValueList **lst )
{
    ValueList *pos = *lst;
    while ( pos != NULL )
    {
        ValueList *tmp = pos;
        pos = pos->Next;
        FreeVec ( tmp );
    }
    *lst = NULL;
}
