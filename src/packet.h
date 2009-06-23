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
#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#include "jp2kcs.h"

struct packet_t{
    uint32_t pnum;
    uint16_t layer;
    uint8_t rlvl;
    uint8_t comp;
    
    void* header;
    uint16_t hlen;
    
    void* data;
    uint32_t dlen;
};

struct packet_codec_t{
    uint8_t progressorder;
    uint8_t codingstyle;
    
    uint16_t layers;
    uint8_t rlvls;
    uint8_t comps;
    
    uint16_t curlayer;
    uint8_t currlvl;
    uint8_t curcomp;
    
    uint32_t numpackets;
    struct packet_t** packets;
};

struct packet_t* packet_create();
void packet_delete(struct packet_t* packet);

void packet_print(struct packet_t* p);

int packet_parse(struct packet_t* p, void* data, int length);
//int packet_format(struct packet_t* p, void* data);

//int packet_parse_tilepart(struct packet_t* p, void* data, int length);


struct packet_codec_t* packet_codec_create();
int packet_codec_parse(struct packet_codec_t* codec, struct jp2kcs_t* cs, void* data, int length);
void packet_codec_print(struct packet_codec_t* codec);

int packet_codec_write(struct packet_codec_t* codec, void* data);

#endif
