# Isoviewer for Helios Automazioni srl CNC Machines

> This version is a C++ porting of the [Python version](https://github.com/LuigiCaradonna/isoviewer-for-helios-automazioni-cnc).

This software reads proprietary GCode files with PGR extension and shows the path followed by the tools during the job.

> This is **NOT** an official software developed by [Helios Automazioni srl](https://www.heliosautomazioni.com/it/home-it.html).
>
> I am computer engineer employed as a technical designer and a CNC programmer at [Centro Marmi D'Arcangeli srl](https://www.cmdarcangeli.com), sometimes I found useful to see what an ISO file contains, for this reason I decided to develop this tool.

## What's new in the latest version
Introduced multilanguage support (English and Italian)

## Features
- Import multiple files at once
- Possibility to specify the working area size
- Option to decide whether or not the drawing has to fit the visible area
- Option to automatically regenarate the drawing, when the main window is resized
- Option to show the drawing in grayscale or color
- Gradient effect applied to the segments to show the difference in depth of the milling
- Calculation of the distance traveled both for millings and repositionings
- Estimate of the working time given the speed of the tool (not reliable)

About the working time, the reason why it is not currently reliable, is that the machine doesn't really keep the speed constant while working, it could vary due to the hardness of the material, the complexity of the job and other reasons.
Where the job is at a constant depth the time results to be almost correct. 

## Tech
The whole software is written in `C++`, using `Qt6` to build the user interface.

## How to use
![Graphic User Interface](/ui.png)
The basic usage only requires to click on the `Load files` button (22) and select one or more PGR files, then to click on the `Process` button (23).  
However you have the possibility to set several options to have different behaviours.  
You can set the actual width and height (2 and 3) of the material to work and the drawing will respect its real position inside the material bounds, which will be displayed.  
The provided speed of the tool (4) will have an impact only on the estimated time, which as you can read above is not accurate due to several reasons not depending on the software but on the machine's behaviour.  
By selecting the `Fit` checkbox (17) the drawing will be scaled to fit the drawing area, this will have effect only for drawings which are smaller than the drawing area, those bigger, will be scaled down in any case, or part of them would not be visible.  
`Regenerate` (18), when checked, will cause the drawing to be regenerated if the main window is resized to adapt to the new drawing area size.  
If the file selected contains the instructions to engrave a sculpture, the `Sculpture` checkbox (19) **must** be checked because that kind of file is slightly different from the regular ones and the software needs to be informed to correctly read the file.  
Operating on the `Colors` checkbox (20) it is possible to decide to have the drawing displayed using a grayscale (unchecked) or colors (checked). Depending on the `Gradient` checkbox (21) status, the segments will be drawn using a solid line (unchecked) or a gradient effect (checked).  
The software only shows segments with positive coordinates, for this reason, when a drawing has parts with negative coordinates, it will be translated to have all of its parts in a positive area (first quadrant of the Cartesian plane). When such a condition occurs, inside the `Offset` field (10) will be stated along which axis the drawing has been translated (X, Y, or X and Y).

**1 -** Drawing area, here is where the drawing will be displayed  
**2 and 3 -** Width and height of the material to work on in millimeters  
**4 -** Movement speed of the tool in millimeters per minute  
**5 to 9 -** Minimum and maximum positions where the machine will engrave  
**10 -** Here will be stated whether the drawing was moved due to negative coordinates contained into the PGR file  
**11 -** Distance traveled while engraving in millimeters  
**12 -** Distance traveled while repositioning the tool in millimeters  
**13 -** Area occupied by the engraving  
**14 -** Estimated value of the required time to complete the job hh:mm:ss  
**15 and 16 -** x and y position of the mouse pointer on the drawing area in millimeters  
**17 -** When checked, the drawing will be scaled to fit the drawing area  
**18 -** When checked, the drawing will be automatically regenerated upon main window resizing  
**19 -** This must be checked when a file containing a sculpture is selected  
**20 -** When checked, the segments of the drawing are colored, otherwise they will be grayscaled  
**21 -** When checked, the segments will have a gradient effect to show the difference in depth  
**22 -** Button to select the file(s) to display  
**23 -** Starts the drawing process  
**24 -** Resets everything: input values, drawing area and eventual error messages  
**25 -** On this line will appear the name(s) of the selected file(s)  
**26 -** Status bar, this will contain eventual error messages  
**27 -** Language menu to change the interface language  

## How to compile
The project has been developed using Visual Studio 2022 CE.
To compile the software, `msvc2019+`, `Qt 6.0+` are required.

## Installation
Inside the folder `dist` the latest compiled version of the software for Windows is available in a zip format.
There is no need to install the software.
Unpack the zip package, inside the folder `Isoviewer`, run the file `Isoviewer.exe`.
> Windows 10 is required since previous versions are no more supported by `Qt`.