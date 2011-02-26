# NESRomTool

The NESRomTool is a utility for the extraction and injection of sprite and program data for NES (regular Nintendo) ROM files (.nes).

In its current form, it is console only, but since the project contains some utility functions in a header, one could easily build a GUI wrapper for it. Anyone interested in creating a Windows, X, Classic MacOS, BeOS, or other GUI is welcome to fork the project.

Run the program with no arguments to print the usage. Man files are coming soon. Additional usage documentation is coming soon, as well.

## About
__Me__:               spike grobstein (<spike@sadistech.com>)    
__Homepage__:         http://sadistech.com/nesromtool    
__Project Homepage__: https://github.com/spikegrobstein/nesromtool    
__My Homepage__:      http://sadistech.com    

## Usage

nesromtool [ options ] &lt;action&gt; [ action-options ... ] &lt;file&gt; [ &lt;file&gt; ... ]

*example usage coming soon*

---

You should have a basic understanding of how data is stored in an NES ROM before you start editing (so you understand what you are doing with this tool). I've got a brief quick rundown which is viewable here:

http://sadistech.com/nesromtool/romdoc.html

There is an out of date tutorial on using the program (with screenshots) can be found:

http://sadistech.com/nesromtool/tutorial/

## Using Photoshop

The format that NESRomTool extracts as (.raw) can be opened in Photoshop. When you select the file, photoshop will ask for dimensions and stuff, use these settings:

width: 8
height: 8
channels: 1
header length: 0

When the file is opened, it will appear as a black box, this is normal. Set the mode of the image to indexed color (Image>Mode>Indexed Color), and change the first 4 colors in the color table (Image>Mode>Color Table...) to 4 contrasting colors to see the graphic. I'll post a tutorial on all of this stuff with screenshots on http://sadistech.com/nesromtool just as soon as I get everything together.

Any questions about the project should be directed to my email address above.

Please do not contact me regarding NES ROM files. I do not have any for distribution. A simple search on Google may yield acceptable results. ;)