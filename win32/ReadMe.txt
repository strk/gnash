gnash.exe - it's a standalone executable file.
It is located in http://web.cvs.savannah.gnu.org/viewcvs/gnash/snapshots/?root=gnash

It depends on the next DLL files:
SDL.dll, libjpeg6b.dll, LIBCURL.dll, libMYSQL.dll, MSVCP80.dll, MSVCR80.dll.

To start gnash.exe, for example, you may copy these files to
C:\WINDOWS\System32 
directory. Then, you can type "gnash c:\mydir\myfile.swf".

npgnash.dll - it's a plugin for Mozilla Firefox browser.
It is located in http://web.cvs.savannah.gnu.org/viewcvs/gnash/snapshots/?root=gnash

To use it, for example, you may copy this file to 
C:\Program Files\Mozilla Firefox\plugin.
Then restart Mozilla Firefox.
Then, you can view SWF files via browser window.
npgnash.dll has the same dependencies as gnash.exe. 

WARNING:The Firefox Gnash Plugin has the next bugs:

 -  Key events are not handled
 -  Resizing browser window when plugin is running, will crash browser.
 
We'll correct this bugs as soon as possible.

NOTE: This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

These files are demo versions of our Flash player and "currently unsupported" & "under development".


		gnash development team