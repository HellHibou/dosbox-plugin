/*
 *  Copyright (C) 2002-2005  The DOSBox Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DOSBOX_HARDWARE_H
#define DOSBOX_HARDWARE_H

#include <stdio.h>

class Section;
enum OPL_Mode {
	OPL_none,OPL_cms,OPL_opl2,OPL_dualopl2,OPL_opl3
};

void OPL_Init(Section* sec,OPL_Mode mode);
void CMS_Init(Section* sec);
void OPL_ShutDown(Section* sec);
void CMS_ShutDown(Section* sec);
extern Bit8u adlib_commandreg;
FILE * OpenCaptureFile(const char * type,const char * ext);


#endif