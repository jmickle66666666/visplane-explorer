
Visplane Explorer 1.0
=====================

by Andrew Apted  |  September 2014


INTRODUCTION

Visplane Explorer is a stand-alone program to visualize possible
visplane overflow (VPO) errors in DOOM maps.  This is useful when
mapping for vanilla DOOM, because when an overflow occurs the game
simply quits with an error message (or crashes), which is pretty
harsh on the person playing.

Visplane Explorer can check not only "visplanes", but also "draw-segs"
and "solid-segs".  When draw-segs overflow, it is not too bad since it
just produces rendering glitches, but solid-seg overflow will corrupt
memory and probably crash.  Visplane Explorer works by using a stripped
down version of the DOOM rendering code, with increased limits and
extra checks to prevent overflowing the buffers and crashing.

CAVEAT : while this program is a useful tool, it cannot predict every
situation (for example, an Archvile making the player jump in the air,
which means more geometry can be seen and hence higher visplane counts).
You should always test your map in the original game.


COMPILING

See the INSTALL.txt document (in source code)


KEYBOARD and MOUSE SUMMARY

* scroll map with mouse button or cursor keys
* zoom with the mouse wheel or + / - keys

* switch modes quickly with V / D / S keys
* toggle opening/closing doors with O key
* toggle heat colors with H key


CREDITS

This GUI program was created by Andrew Apted (me), and I continue
to develop it.

The MacOS X package was made by Ioan Chera.

It includes a very stripped down version of the DOOM rendering code,
which I took from Chocolate Doom (by Simon Howard), which in turn is
based on the DOOM source code release by Id Software.  Hence these
dudes are included in the copyright notice.


COPYRIGHT and LICENSE

  Visplane Explorer

  Copyright (C) 2012-2014 Andrew Apted
  Copyright (C) 2005-2008 Simon Howard
  Copyright (C) 1993-1996 Id Software, Inc.

  This program is free software; you can redistribute it and/or modify  
  it under the terms of the GNU General Public License as published  
  by the Free Software Foundation; either version 2 of the License,
  or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.  

  You should have received a copy of the GNU General Public License
  along with this software.  If not, please visit the following
  web page: http://www.gnu.org/licenses/gpl-2.0.txt


CONTACT DETAILS

  Web: http://eureka-editor.sourceforge.net/?n=VisExp.Main

  Email: <ajapted@users.sf.net>

