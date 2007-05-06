// 
//   Copyright (C) 2005, 2006, 2007 Free Software Foundation, Inc.
// 
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

// 
//

/* $Id: aqua.cpp,v 1.3 2007/05/06 22:43:17 nihilus Exp $ */

#include <CoreServices/CoreServices.h>

void MyInit( void );
void MyTimerProc( TMTaskPtr tmTaskPtr );

Boolean gQuitFlag = false;
int gCount = 0;

TimerUPP gMyTimerProc = NULL;

int main( int argc, char *argv[])
{
    MyInit();

    while ( false == gQuitFlag ) {
        ;
    }

    DisposeTimerUPP( gMyTimerProc );

    return 0;
}
 
void MyTimerProc( TMTaskPtr tmTaskPtr )
{
    DateTimeRec localDateTime;
    
    GetTime( &localDateTime );

    printf( "MyTimerProc at %d:%d:%d\n", localDateTime.hour, localDateTime.minute, localDateTime.second );

    gCount++;

    if ( gCount > 4 )
    {
        gQuitFlag = true;
    }
    else
    {
        PrimeTimeTask( ( QElemPtr )tmTaskPtr, 1000 );
    }
}

void MyInit( void )
{
    struct TMTask myTask;
    OSErr err = 0;

    gMyTimerProc = NewTimerUPP( MyTimerProc );

    if ( gMyTimerProc != NULL )
    {
        myTask.qLink = NULL;
        myTask.qType = 0;
        myTask.tmAddr = gMyTimerProc;
        myTask.tmCount = 0;
        myTask.tmWakeUp = 0;
        myTask.tmReserved = 0;
        
        err = InstallTimeTask( ( QElemPtr )&myTask );
        
        if ( err == noErr )
            PrimeTimeTask( ( QElemPtr )&myTask, 1000 );
        else {
            DisposeTimerUPP( gMyTimerProc );
            gMyTimerProc = NULL;
            gQuitFlag = true;
        }
    }
}
