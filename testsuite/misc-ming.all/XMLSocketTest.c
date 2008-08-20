/*
 *   Copyright (C) 2005, 2006 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */ 



/* The test sends all the 



*/

#include <stdlib.h>
#include <stdio.h>
#include <ming.h>

#include "ming_utils.h"

#define OUTPUT_VERSION 7
#define OUTPUT_FILENAME "XMLSocketTest.swf"

int
main(int argc, char** argv)
{

    /* Basic setup */

	SWFMovieClip dejagnuclip;
    SWFMovie mo;
	const char* srcdir = ".";

    char longString[15000];

	if ( argc>1 ) srcdir=argv[1];
	else
	{
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return 1;
	}

    Ming_init();

    mo = newSWFMovieWithVersion(OUTPUT_VERSION);
    SWFMovie_setDimension(mo, 800, 600);
    SWFMovie_setRate (mo, 10.0);

    dejagnuclip = get_dejagnu_clip((SWFBlock)get_default_font(srcdir), 10, 0, 0, 800, 600);
    SWFMovie_add(mo, (SWFBlock)dejagnuclip);

    /* Setup XMLsocket */

    /* Advance to next frame on connection */
    add_actions(mo,
        "function handleConnect(connectionStatus) { "
        "    if (connectionStatus)                  "
        "    {                                      "
        "        trace('Connected');                "
        "        connected = true;                  "
        "        play();                            "
        "    }                                      "
        "    else { trace('Connection failed.'); }  "
        "};                                         "
     );

    /* Add any data recieved to an array, advance to next
       frame */
    add_actions(mo, 
        "function handleData(data) {                "
        "    play();                                "
        "    trace('Data received: ' + data);       "
        "    receivedArray.push(data);              "
        "    trace ('Pushed data');"
        "};                                         "
    );

    /* Advance to next frame on losing connection */
    add_actions(mo,
        "function handleDisconnect() {              "
        "    trace('Connection lost.');             "
        "    connected = false;                     "
        "    play();                                "
        "};                                         "
    );

    /* Wait for connection */
    add_actions(mo, "stop();");

    add_actions(mo,
        "wait = 0;"
        "count = -1;"
        "connected = false;"
        "myXML = new XMLSocket;"
        "myXML.onConnect = handleConnect;"
        "myXML.onData = handleData;"
        "myXML.onClose = handleDisconnect;"
        "myXML.connect(\"localhost\", 2229);"
    );

/*function closeConnection(){*/
/*    trace("Closing connection to server.");*/
/*    myXML.close();*/
/*}*/


    /* The data we're going to send */

    add_actions(mo, 
        "xmlArray = new Array();"
        "xmlArray[0] = 'Plain text';"
        "xmlArray[1] = 'Plain *NEWLINE* text';"
        "xmlArray[2] = 'Plain *NULL* text';"
        "xmlArray[3] = 'Plain *NULL**NEWLINE* text';"
        "xmlArray[4] = '<xml>Some XML</xml>';"
        "xmlArray[5] = '<xml>Some XML*NEWLINE*</xml>';"
        "xmlArray[6] = '<xml>Some XML*NULL*</xml>';"
        "xmlArray[7] = '<xml>Some XML*NEWLINE**NULL*</xml>';"
        "xmlArray[8] = undefined;"
        "xmlArray[9] = 9;"
        "xmlArray[10] = '';"
    );

    memset(longString, 'a', 15000);
    strncpy(longString, "xmlArray[11] = '", 16);
    longString[14998] = '\'';
    longString[14999] = ';';

    add_actions(mo, longString);

    add_actions(mo, "xmlArray[12] = 'Last Item';");

    /* The data we should get back */
    add_actions(mo, 
        "expectedArray = new Array();"
        "expectedArray[0] = 'Plain text';"
        "expectedArray[1] = 'Plain \n text';"
        "expectedArray[2] = 'Plain ';"
        "expectedArray[3] = ' text';"
        "expectedArray[4] = 'Plain ';"
        "expectedArray[5] = '\n text';"
        "expectedArray[6] = '<xml>Some XML</xml>';"
        "expectedArray[7] = '<xml>Some XML\n</xml>';"
        "expectedArray[8] = '<xml>Some XML';"
        "expectedArray[9] = '</xml>';"
        "expectedArray[10] = '<xml>Some XML\n';"
        "expectedArray[11] = '</xml>';"
        "expectedArray[12] = 'undefined';"
        "expectedArray[13] = 9;"
        "expectedArray[14] = '';"
        "expectedArray[15] = 'aaa';"
        "expectedArray[16] = 'Last Item';"
    );


    /* Where we're going to put the data */
    add_actions(mo, 
        "receivedArray = new Array();"
    );



    /* Frame 2 */
    SWFMovie_nextFrame(mo); 
    /* We should be connected here */
	check_equals(mo, "connected", "true");




    /* Frame 3 */
    /* This is where we send the socket data */
    SWFMovie_nextFrame(mo); 

    add_actions(mo,
        "count++;"
        "if (count >= xmlArray.length) { _root.gotoAndStop(4); };"
        "trace('Frame 3 (iteration ' + count + '): sending ' + xmlArray[count]);"
        "myXML.send(xmlArray[count]);"
        "play();"
    );
    

    /* Frame 4 */
    /* This is here to make a small pause between each send 
       (a quarter of a second) */
    SWFMovie_nextFrame(mo);

    add_actions(mo,
        "trace ('Frame 4');"
        "play();"
    );

    /* Frame 5 */
    /* If we have sent all the data, continue. Otherwise send the next item */
    SWFMovie_nextFrame(mo);

    add_actions(mo,
        "trace ('Frame 5');"
        "if (count < xmlArray.length - 1 ) { _root.gotoAndStop(3); };"
        "play();"
    );

    /* Frame 6 */
    /* This is a bit of a hackish loop to wait for data */
    SWFMovie_nextFrame(mo);

    add_actions(mo,
        "play();"
    );

    /* Frame 7 */
    SWFMovie_nextFrame(mo);

    add_actions(mo,
        "if (receivedArray[receivedArray.length -1] != 'Last Item' && wait++ < 100)"
        "{ _root.gotoAndStop(6); };"
        "play();"
    );


    /* Last frame (8) */
    SWFMovie_nextFrame(mo);

    check_equals(mo, "receivedArray.length", "expectedArray.length");
    
    check_equals(mo, "receivedArray[0]", "expectedArray[0]");
    check_equals(mo, "receivedArray[1]", "expectedArray[1]");
    check_equals(mo, "receivedArray[2]", "expectedArray[2]");
    check_equals(mo, "receivedArray[3]", "expectedArray[3]");
    check_equals(mo, "receivedArray[4]", "expectedArray[4]");
    check_equals(mo, "receivedArray[5]", "expectedArray[5]");
    check_equals(mo, "receivedArray[6]", "expectedArray[6]");
    check_equals(mo, "receivedArray[7]", "expectedArray[7]");
    check_equals(mo, "receivedArray[8]", "expectedArray[8]");
    check_equals(mo, "receivedArray[9]", "expectedArray[9]");
    check_equals(mo, "receivedArray[10]", "expectedArray[10]");    
    check_equals(mo, "receivedArray[11]", "expectedArray[11]"); 
    check_equals(mo, "receivedArray[12]", "expectedArray[12]"); 
    check_equals(mo, "receivedArray[13]", "expectedArray[13]");         
    check_equals(mo, "receivedArray[14]", "expectedArray[14]");
    check_equals(mo, "receivedArray[15].length", "14982");         
    
             
    check_equals(mo, "receivedArray[16]", "expectedArray[16]");         

    

    add_actions(mo, "totals(); stop();");

    /* Output movie */
    puts("Saving " OUTPUT_FILENAME );
    SWFMovie_save(mo, OUTPUT_FILENAME);

  return 0;
}
