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

#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "packet.h"


struct packet_t* packet_create(){
    struct packet_t* p = malloc(sizeof(struct packet_t));
    p->rlvl = 0;
    p->layer = 0;
    p->comp = 0;
    p->pnum = 0;
    p->header = NULL;
    p->hlen = 0;
    p->data = NULL;
    p->dlen = 0;
}

void packet_delete(struct packet_t* p){
    free(p);
}

void packet_print(struct packet_t* p){
    printf("Packet: id %d, res %d, layer %d, comp %d, hlen %d, dlen %d\n", p->pnum, p->rlvl, p->layer, p->comp, p->hlen, p->dlen);
}

int packet_parse(struct packet_t* p, void* data, int length){
    
    return 0;
}

int packet_parse_tilepart(struct packet_t* p, void* data, int length){
    
    return 0;
}

int packet_format(struct packet_t* p, void* data){
    
}



int packet_codec_progress(struct packet_codec_t* codec);

struct packet_codec_t* packet_codec_create(){
    struct packet_codec_t* codec = malloc(sizeof(struct packet_codec_t));
    codec->numpackets =0;
    codec->packets = NULL;
    
}

int packet_codec_parse(struct packet_codec_t* codec, struct jp2kcs_t* cs, void* data, int length){
    codec->layers = cs->header.cod->numlayers;
    codec->comps = cs->header.siz->numcomponents;
    codec->rlvls = cs->header.cod->compparams.decomplvls;
    codec->progressorder = cs->header.cod->progessorder;
    codec->codingstyle = cs->header.cod->codingstyle;
    codec->numpackets = codec->layers * codec->comps * (codec->rlvls + 1);
    codec->packets = malloc(codec->numpackets * sizeof(struct packet_t*));
    
    codec->currlvl = 0;
    codec->curlayer = 0;
    codec->curcomp = 0;
    
    int i;
    int first = 1;
    int decode = codec->codingstyle & JP2KCS_CODINGSTYLE_SOP;
    int offset = 0;
    if(!decode){
        printf("Locating packets without SOP markers not supported yet\n");
    }
    for(i = 0; i < codec->numpackets; i++){
        if(first){
            first = 0;
        }else{
            packet_codec_progress(codec);
        }
        codec->packets[i] = malloc(sizeof(struct packet_t));
        codec->packets[i]->rlvl = codec->currlvl;
        codec->packets[i]->layer = codec->curlayer;
        codec->packets[i]->comp = codec->curcomp;
        codec->packets[i]->pnum = i;
        
        codec->packets[i]->hlen = 0;
        codec->packets[i]->dlen = 0;
    }
        
    /* decode packet headers */
        
    if(decode){
        while(offset < length){
            uint16_t temp16;
            memcpy(&temp16, data + offset + 4, 2);
            i = ntohs(temp16);
            
                codec->packets[i]->data = data + offset;
                offset++;
            
                int plen = 1;
                while(offset < length && (*((uint8_t*)(data + offset)) != 0xff || *((uint8_t*)(data + offset + 1)) != 0x91 || *((uint8_t*)(data + offset + 2)) != 0x0 || *((uint8_t*)(data + offset + 3)) != 0x4)){
                    offset++;
                    plen++;
                }
                codec->packets[i]->dlen = plen;
            
        }
        
    }
}

void packet_codec_print(struct packet_codec_t* codec){
    printf("Packets: num packets %d, rlvls %d, layers %d, comps %d\n", 
           codec->numpackets, codec->rlvls, codec->layers, codec->comps);
    int i;
    for(i = 0; i < codec->numpackets; i++){
        packet_print(codec->packets[i]);
    }
}

int packet_codec_progress(struct packet_codec_t* codec){
    switch(codec->progressorder){
        case 0:
            /* lrcp */
            codec->curcomp++;
            if(codec->curcomp >= codec->comps){
                codec->curcomp = 0;
                codec->currlvl++;
                if(codec->currlvl > codec->rlvls){
                    codec->currlvl = 0;
                    codec->curlayer++;
                }
            }
            break;
        case 1:
            /* rlcp */
            codec->curcomp++;
            if(codec->curcomp >= codec->comps){
                codec->curcomp = 0;
                codec->curlayer++;
                if(codec->curlayer >= codec->layers){
                    codec->curlayer = 0;
                    codec->currlvl++;
                }
            }
            break;
        default:
            printf("Have not implemented %d progression order yet\n", codec->progressorder);
            break;
    }
}

int packet_codec_write(struct packet_codec_t* codec, void* data){
    int offset = 0;
    int i;
    for(i = 0; i < codec->numpackets; i++){
        if(codec->packets[i]->hlen != 0){
            memcpy(data + offset, codec->packets[i]->header, codec->packets[i]->hlen);
            offset += codec->packets[i]->hlen;
        }
        if(codec->packets[i]->dlen != 0){
            memcpy(data + offset, codec->packets[i]->data, codec->packets[i]->dlen);
            offset += codec->packets[i]->dlen;
        }
        if(codec->packets[i]->hlen == 0 && codec->packets[i]->dlen == 0){
            if(codec->codingstyle & JP2KCS_CODINGSTYLE_SOP){
                *((uint8_t*)(data + offset)) = 0xff;
                *((uint8_t*)(data + offset + 1)) = 0x91;
                uint16_t temp16 = htons(4);
                memcpy(data + offset + 2, &temp16, 2);
                temp16 = htons(i);
                memcpy(data + offset + 4, &temp16, 2);
                offset += 6;
            }
            *((uint8_t*)(data + offset)) = 0;
            offset++;
        }
    }
    return 1;
}