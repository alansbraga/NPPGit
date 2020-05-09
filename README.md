# NPPGit
Git with Notepad++

Plugin to use Git with Notepad++


The original plugin was posted here:
https://forum.lowyat.net/topic/1358320/all

I just needed to add Blame command, so I put the source code here.

## Installation

First you need to install TortoiseGit, because this plugin simply uses the command line features of TortoiseGit.
<https://tortoisegit.org/>

Then install [Visual C++ Redistributable for Visual Studio 2015](https://www.microsoft.com/en-us/download/details.aspx?id=48145).

**Important:** New versions of Notepad asks you to create a folder for each plugin

After that, take the DLL NPPGit.dll on the Release folder of this repository and copy to the Notepad\+\+ plugins folder.
By default located at C:\Program Files (x86)\Notepad++\plugins\NPPGit

If you are using Notepad\+\+ 64bits version use the dll on folder x64\Release.
The default folder for 64bits version is C:\Program Files\Notepad++\plugins\NPPGit

Next time you open Notepad\+\+ you can access through menu Plugins->Git

![Plugin](Plugin.png?raw=true "Plugin")