/***************************************************************************
 *   Copyright (C) 2009 by Lee Begg   *
 *   llnz@paradise.net.nz   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef DECIMATOR_H
#define DECIMATOR_H

struct box_t;
struct jp2kcs_t;
struct packet_codec_t;

void boxes_decimator(struct box_t** boxes, int numboxes, int outlow, int outrest, int maxrlevel, int maxlayer);
void jp2cbox_decimator(struct box_t* box, int outrest, int maxrlevel, int maxlayer);
void cs_decimator(struct jp2kcs_t* codestream, int outrest, int maxrlevel, int maxlayer);
void packet_decimator(struct packet_codec_t* codec, int outrest, int maxrlevel, int maxlayer);

void decimator(int infile, int outlow, int outrest, int maxrlevel, int maxlayer);

#endif
