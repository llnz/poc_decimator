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

#include "jp2kcs.h"

struct jp2kcs_t* jp2kcs_create(){
    struct jp2kcs_t* cs = malloc(sizeof(struct jp2kcs_t));
    cs->numtiles = 0;
    cs->tiles = NULL;
    cs->header.numcomments = 0;
    cs->header.comments = NULL;
    cs->header.siz = malloc(sizeof(struct jp2kcs_siz_t));
    cs->header.cod = malloc(sizeof(struct jp2kcs_cod_t));
    cs->header.qcd = malloc(sizeof(struct jp2kcs_qcd_t));
    cs->header.siz->capability = 0;
    cs->header.siz->xref = 0;
    cs->header.siz->yref = 0;
    cs->header.siz->ximgoffset = 0;
    cs->header.siz->yimgoffset = 0;
    cs->header.siz->reftilewidth = 0;
    cs->header.siz->reftilehight = 0;
    cs->header.siz->xtileoffset = 0;
    cs->header.siz->ytileoffset = 0;
    cs->header.siz->numcomponents = 0;
    cs->header.siz->componentrefs = NULL;
    cs->header.cod->codingstyle = 0;
    cs->header.cod->progessorder = 0;
    cs->header.cod->numlayers = 0;
    cs->header.cod->mctransform = 0;
    cs->header.cod->compparams.decomplvls = 0;
    cs->header.cod->compparams.cbwidthexp = 0;
    cs->header.cod->compparams.cbhightexp = 0;
    cs->header.cod->compparams.cbstyle = 0;
    cs->header.cod->compparams.qmfbid = 0;
    cs->header.cod->compparams.sizepps = 0;
    cs->header.cod->compparams.ppslist = NULL;
    cs->header.qcd->quantstyle = 0;
    cs->header.qcd->stepsizeslen = 0;
    cs->header.qcd->quantstepsizes = NULL;
    cs->header.qcc = NULL;
    cs->header.numqcc = 0;
    
    return cs;
}

void jp2kcs_delete(struct jp2kcs_t* cs){
    int i;
    if(cs->header.qcd->quantstepsizes != NULL){
        free(cs->header.qcd->quantstepsizes);
    }
    if(cs->header.cod->compparams.ppslist != NULL){
        free(cs->header.cod->compparams.ppslist);
    }
    if(cs->header.siz->componentrefs != NULL){
        for(i = 0; i < cs->header.siz->numcomponents; i++){
            free(cs->header.siz->componentrefs[i]);
        }
        free(cs->header.siz->componentrefs);
    }
    if(cs->header.comments != NULL){
        for(i = 0; i < cs->header.numcomments; i++){
            free(cs->header.comments[i]->data);
            free(cs->header.comments[i]);
        }
        free(cs->header.comments);
    }
    if(cs->header.qcc != NULL){
        for(i = 0; i < cs->header.numqcc; i++){
            free(cs->header.qcc[i]->quantstepsizes);
            free(cs->header.qcc[i]);
        }
        free(cs->header.qcc);
    }
    free(cs->header.cod);
    free(cs->header.qcd);
    free(cs->header.siz);
    if(cs->tiles != NULL){
        for(i = 0; i < cs->numtiles; i++){
            
            if(cs->tiles[i]->codedata != NULL){
                
                free(cs->tiles[i]->codedata);
            }
            free(cs->tiles[i]);
        }
        free(cs->tiles);
    }
    free(cs);
}


int jp2kcs_decode(struct jp2kcs_t* cs, void* data, int len){
    if(*((uint8_t*)(data)) != 0xff || *((uint8_t*)(data + 1)) != 0x4f){
        printf("Missing initial marker, was %x%x\n", *((uint8_t*)(data)), *((uint8_t*)(data + 1)));
        return 0;
    }
    if(*((uint8_t*)(data + 2)) != 0xff || *((uint8_t*)(data + 3)) != 0x51){
        printf("Missing siz marker, was %x%x\n", *((uint8_t*)(data+2)), *((uint8_t*)(data + 3)));
        return 0;
    }
    
    uint16_t temp16;
    memcpy(&temp16, data+4, 2);
    int mlen = ntohs(temp16);
    memcpy(&temp16, data + 6, 2);
    cs->header.siz->capability = ntohs(temp16);
    uint32_t temp32;
    memcpy(&temp32, data+ 8, 4);
    cs->header.siz->xref = ntohl(temp32);
    memcpy(&temp32, data + 12, 4);
    cs->header.siz->yref = ntohl(temp32);
    memcpy(&temp32, data + 16, 4);
    cs->header.siz->ximgoffset = ntohl(temp32);
    memcpy(&temp32, data + 20, 4);
    cs->header.siz->yimgoffset = ntohl(temp32);
    memcpy(&temp32, data + 24, 4);
    cs->header.siz->reftilewidth = ntohl(temp32);
    memcpy(&temp32, data + 28, 4);
    cs->header.siz->reftilehight = ntohl(temp32);
    memcpy(&temp32, data + 32, 4);
    cs->header.siz->xtileoffset = ntohl(temp32);
    memcpy(&temp32, data + 36, 4);
    cs->header.siz->ytileoffset = ntohl(temp32);
    memcpy(&temp16, data + 40, 2);
    cs->header.siz->numcomponents = ntohs(temp16);
    cs->header.siz->componentrefs = malloc(sizeof(struct jp2kcs_siz_compref_t*) * cs->header.siz->numcomponents);
    int offset = 42;
    int i;
    for(i = 0; i < cs->header.siz->numcomponents; i++){
        cs->header.siz->componentrefs[i] = malloc(sizeof(struct jp2kcs_siz_compref_t));
        cs->header.siz->componentrefs[i]->bitdepth = *((uint8_t*)(data + offset));
        cs->header.siz->componentrefs[i]->xsep = *((uint8_t*)(data + offset + 1));
        cs->header.siz->componentrefs[i]->ysep = *((uint8_t*)(data + offset + 2));
        offset += 3;
    }
    
    /* check that the mlen was right */
    if(offset != mlen + 4){
        printf("Didn't decode fixed headers correctly\n");
        return 0;
    }
    
    /* main header marker loop */
    while(1){
        if(*((uint8_t*)(data + offset)) != 0xff){
            printf("Lost synchronisation\n");
            return 0;
        }
        if(*((uint8_t*)(data + offset + 1)) == 0x90){
            break;
        }
        switch(*((uint8_t*)(data + offset + 1))){
            case 0x52:
                /* cod */
                memcpy(&temp16, data + offset + 2, 2);
                mlen = ntohs(temp16);
                cs->header.cod->codingstyle = *((uint8_t*)(data + offset + 4));
                cs->header.cod->progessorder = *((uint8_t*)(data + offset + 5));
                memcpy(&temp16, data + offset + 6, 2);
                cs->header.cod->numlayers = ntohs(temp16);
                cs->header.cod->mctransform = *((uint8_t*)(data + offset + 8));
                cs->header.cod->compparams.decomplvls = *((uint8_t*)(data + offset + 9));
                cs->header.cod->compparams.cbwidthexp = *((uint8_t*)(data + offset + 10));
                cs->header.cod->compparams.cbhightexp = *((uint8_t*)(data + offset + 11));
                cs->header.cod->compparams.cbstyle = *((uint8_t*)(data + offset + 12));
                cs->header.cod->compparams.qmfbid = *((uint8_t*)(data + offset + 13));
                
                cs->header.cod->compparams.sizepps = mlen - 12;
                cs->header.cod->compparams.ppslist = malloc(cs->header.cod->compparams.sizepps);
                memcpy(cs->header.cod->compparams.ppslist, data + offset + 14, cs->header.cod->compparams.sizepps);
                offset += mlen + 2;
                break;
            case 0x5c:
                /* qcd */
                memcpy(&temp16, data + offset + 2, 2);
                mlen = ntohs(temp16);
                cs->header.qcd->quantstyle = *((uint8_t*)(data + offset + 4));
                cs->header.qcd->stepsizeslen = mlen - 3;
                cs->header.qcd->quantstepsizes = malloc(cs->header.qcd->stepsizeslen);
                memcpy(cs->header.qcd->quantstepsizes, data + offset + 5, cs->header.qcd->stepsizeslen);
                offset += mlen + 2;
                break;
            case 0x5d:
                /* qcc */
                memcpy(&temp16, data + offset + 2, 2);
                mlen = ntohs(temp16);
                if(cs->header.qcc == NULL){
                    cs->header.qcc = malloc(sizeof(struct jp2kcs_qcc_t*));
                }else{
                    cs->header.qcc = realloc(cs->header.qcc, sizeof(struct jp2kcs_qcc_t*) * (cs->header.numqcc + 1));
                }
                cs->header.qcc[cs->header.numqcc] = malloc(sizeof(struct jp2kcs_qcc_t));
                offset += 4;
                mlen -= 2;
                if(cs->header.siz->numcomponents < 257){
                    cs->header.qcc[cs->header.numqcc]->compnum.compnum8 = *((uint8_t*)(data + offset));
                    offset += 1;
                    mlen -= 1;
                }else{
                    memcpy(&temp16, data + offset, 2);
                    cs->header.qcc[cs->header.numqcc]->compnum.compnum16 = ntohs(temp16);
                    offset += 2;
                    mlen -= 2;
                }
                cs->header.qcc[cs->header.numqcc]->quantstyle = *((uint8_t*)(data + offset));
                cs->header.qcc[cs->header.numqcc]->stepsizeslen = mlen - 1;
                cs->header.qcc[cs->header.numqcc]->quantstepsizes = malloc(mlen - 1);
                memcpy(cs->header.qcc[cs->header.numqcc]->quantstepsizes, data + offset + 1, mlen - 1);
                cs->header.numqcc++;
                offset += mlen;
                break;
            case 0x64:
                /* cme */
                memcpy(&temp16, data + offset + 2, 2);
                mlen = ntohs(temp16);
                if(cs->header.comments == NULL){
                    cs->header.comments = malloc(sizeof(struct jp2kcs_cme_t*));
                }else{
                    cs->header.comments = realloc(cs->header.comments, sizeof(struct jp2kcs_cme_t*) * (cs->header.numcomments + 1));
                }
                cs->header.comments[cs->header.numcomments] = malloc(sizeof(struct jp2kcs_cme_t));
                memcpy(&temp16, data + offset + 4, 2);
                cs->header.comments[cs->header.numcomments]->regvalue = ntohs(temp16);
                cs->header.comments[cs->header.numcomments]->length = mlen - 4;
                cs->header.comments[cs->header.numcomments]->data = malloc(mlen - 3); /*one extra char for nul */
                memcpy(cs->header.comments[cs->header.numcomments]->data, data + offset + 6, mlen - 4);
                memset(cs->header.comments[cs->header.numcomments]->data + mlen - 3, 0, 1);
                cs->header.numcomments++;
                offset += mlen + 2;
                break;
            
            default:
                printf("Unknown main header %d\n", *((uint8_t*)(data + offset + 1)));
                return 0;
        }
    }
    /* tile loop */
    while(1){
        if(*((uint8_t*)(data + offset)) != 0xff){
            printf("Lost synchronisation in tiles\n");
            return 0;
        }
        if(*((uint8_t*)(data + offset + 1)) == 0xd9){
            break;
        }
        if(*((uint8_t*)(data + offset + 1)) != 0x90){
            printf("Lost synchronisation at tile header\n");
            return 0;
        }
        if(cs->tiles == NULL){
            cs->tiles = malloc(sizeof(struct jp2kcs_tile_t*));
        }else{
            cs->tiles = realloc(cs->tiles, sizeof(struct jp2kcs_tile_t*) * (cs->numtiles + 1));
        }
        int t = cs->numtiles;
        cs->tiles[t] = malloc(sizeof(struct jp2kcs_tile_t));
        cs->tiles[t]->header.qcc = NULL;
        cs->tiles[t]->header.numqcc = 0;
        memcpy(&temp16, data + offset + 2, 2);
        mlen = ntohs(temp16);
        memcpy(&temp16, data + offset + 4, 2);
        cs->tiles[t]->header.tilenum = ntohs(temp16);
        memcpy(&temp32, data + offset + 6, 4);
        int dataendoffset = offset + ntohl(temp32);
        cs->tiles[t]->header.partnum = *((uint8_t*)(data + offset + 10));
        cs->tiles[t]->header.numparts = *((uint8_t*)(data + offset + 11));
        offset += mlen + 2;
        
        cs->tiles[t]->header.coc = NULL;
        cs->tiles[t]->header.qcc = NULL;
        
        /* tile headers loop */
        while(1){
            if(*((uint8_t*)(data + offset)) != 0xff){
                printf("Lost synchronisation in tile headers\n");
                return 0;
            }
            if(*((uint8_t*)(data + offset + 1)) == 0x93){
                break;
            }
            
            switch(*((uint8_t*)(data + offset + 1))){
                case 0x53:
                    /* coc */
                    memcpy(&temp16, data + offset + 2, 2);
                    mlen = ntohs(temp16);
                    cs->tiles[t]->header.coc = malloc(sizeof(struct jp2kcs_coc_t));
                    offset += 4;
                    mlen -= 2;
                    if(cs->header.siz->numcomponents < 257){
                        cs->tiles[t]->header.coc->numcomps.numcomps8 = *((uint8_t*)(data + offset));
                        offset += 1;
                        mlen -= 1;
                    }else{
                        memcpy(&temp16, data + offset, 2);
                        cs->tiles[t]->header.coc->numcomps.numcomps16 = ntohs(temp16);
                        offset += 2;
                        mlen -= 2;
                    }
                    cs->tiles[t]->header.coc->codingstyle = *((uint8_t*)(data + offset));
                    cs->tiles[t]->header.coc->compparams.decomplvls = *((uint8_t*)(data + offset + 1));
                    cs->tiles[t]->header.coc->compparams.cbwidthexp = *((uint8_t*)(data + offset + 2));
                    cs->tiles[t]->header.coc->compparams.cbhightexp = *((uint8_t*)(data + offset + 3));
                    cs->tiles[t]->header.coc->compparams.cbstyle = *((uint8_t*)(data + offset + 4));
                    cs->tiles[t]->header.coc->compparams.qmfbid = *((uint8_t*)(data + offset + 5));
                
                    cs->tiles[t]->header.coc->compparams.sizepps = mlen - 6;
                    cs->tiles[t]->header.coc->compparams.ppslist = malloc(cs->tiles[t]->header.coc->compparams.sizepps);
                    memcpy(cs->tiles[t]->header.coc->compparams.ppslist, data + offset + 6, cs->tiles[t]->header.coc->compparams.sizepps);
                    
                    offset += mlen;
                    break;
                    
                case 0x5d:
                    /* qcc */
                    memcpy(&temp16, data + offset + 2, 2);
                    mlen = ntohs(temp16);
                    if(cs->tiles[t]->header.qcc == NULL){
                        cs->tiles[t]->header.qcc = malloc(sizeof(struct jp2kcs_qcc_t*));
                    }else{
                        cs->tiles[t]->header.qcc = realloc(cs->tiles[t]->header.qcc, sizeof(struct jp2kcs_qcc_t*) * (cs->tiles[t]->header.numqcc + 1));
                    }
                    cs->tiles[t]->header.qcc[cs->tiles[t]->header.numqcc] = malloc(sizeof(struct jp2kcs_qcc_t));
                    offset += 4;
                    mlen -= 2;
                    if(cs->header.siz->numcomponents < 257){
                        cs->tiles[t]->header.qcc[cs->tiles[t]->header.numqcc]->compnum.compnum8 = *((uint8_t*)(data + offset));
                        offset += 1;
                        mlen -= 1;
                    }else{
                        memcpy(&temp16, data + offset, 2);
                        cs->tiles[t]->header.qcc[cs->tiles[t]->header.numqcc]->compnum.compnum16 = ntohs(temp16);
                        offset += 2;
                        mlen -= 2;
                    }
                    cs->tiles[t]->header.qcc[cs->tiles[t]->header.numqcc]->quantstyle = *((uint8_t*)(data + offset));
                    cs->tiles[t]->header.qcc[cs->tiles[t]->header.numqcc]->stepsizeslen = mlen - 1;
                    cs->tiles[t]->header.qcc[cs->tiles[t]->header.numqcc]->quantstepsizes = malloc(mlen - 1);
                    memcpy(cs->tiles[t]->header.qcc[cs->tiles[t]->header.numqcc]->quantstepsizes, data + offset + 1, mlen - 1);
                    cs->tiles[t]->header.numqcc++;
                    offset += mlen;
                    break;
                default:
                    printf("Broken, add tile header %x\n", *((uint8_t*)(data + offset + 1)));
                    return 0;
            }
            
            
            
        }
        
        offset += 2;
        
        cs->tiles[t]->header.tilelength = dataendoffset - offset;
        cs->tiles[t]->codelength = cs->tiles[t]->header.tilelength;
        cs->tiles[t]->codedata = malloc(cs->tiles[t]->codelength);
        memcpy(cs->tiles[t]->codedata, data + offset, cs->tiles[t]->codelength);
        offset += cs->tiles[t]->codelength;
        cs->numtiles += 1;
    }
    return 1;
}

int jp2kcs_encode(struct jp2kcs_t* cs, void* data){
    int offset = 4;
    *((uint8_t*)(data)) = 0xff;
    *((uint8_t*)(data + 1)) = 0x4f;
    *((uint8_t*)(data + 2)) = 0xff;
    *((uint8_t*)(data + 3)) = 0x51;
    uint16_t temp16;
    uint32_t temp32;
    temp16 = htons(38 + 3 * cs->header.siz->numcomponents);
    memcpy(data + 4, &temp16, 2);
    temp16 = htons(cs->header.siz->capability);
    memcpy(data + 6, &temp16, 2);
    temp32 = htonl(cs->header.siz->xref);
    memcpy(data + 8, &temp32, 4);
    temp32 = htonl(cs->header.siz->yref);
    memcpy(data + 12, &temp32, 4);
    temp32 = htonl(cs->header.siz->ximgoffset);
    memcpy(data + 16, &temp32, 4);
    temp32 = htonl(cs->header.siz->yimgoffset);
    memcpy(data + 20, &temp32, 4);
    temp32 = htonl(cs->header.siz->reftilewidth);
    memcpy(data + 24, &temp32, 4);
    temp32 = htonl(cs->header.siz->reftilehight);
    memcpy(data + 28, &temp32, 4);
    temp32 = htonl(cs->header.siz->xtileoffset);
    memcpy(data + 32, &temp32, 4);
    temp32 = htonl(cs->header.siz->ytileoffset);
    memcpy(data + 36, &temp32, 4);
    temp16 = htons(cs->header.siz->numcomponents);
    memcpy(data + 40, &temp16, 2);
    offset += 38;
    int i;
    for(i = 0; i < cs->header.siz->numcomponents; i++){
        *((uint8_t*)(data + offset)) = cs->header.siz->componentrefs[i]->bitdepth;
        *((uint8_t*)(data + offset + 1)) = cs->header.siz->componentrefs[i]->xsep;
        *((uint8_t*)(data + offset + 2)) = cs->header.siz->componentrefs[i]->ysep;
        
        offset += 3;
    }
    
    /* cod */
    *((uint8_t*)(data + offset)) = 0xff;
    *((uint8_t*)(data + offset + 1)) = 0x52;
    temp16 = htons(12 + cs->header.cod->compparams.sizepps);
    offset += 2;
    memcpy(data + offset, &temp16, 2);
    *((uint8_t*)(data + offset + 2)) = cs->header.cod->codingstyle;
    *((uint8_t*)(data + offset + 3)) = cs->header.cod->progessorder;
    temp16 = htons(cs->header.cod->numlayers);
    memcpy(data + offset + 4, &temp16, 2);
    *((uint8_t*)(data + offset + 6)) = cs->header.cod->mctransform;
    *((uint8_t*)(data + offset + 7)) = cs->header.cod->compparams.decomplvls;
    *((uint8_t*)(data + offset + 8)) = cs->header.cod->compparams.cbwidthexp;
    *((uint8_t*)(data + offset + 9)) = cs->header.cod->compparams.cbhightexp;
    *((uint8_t*)(data + offset + 10)) = cs->header.cod->compparams.cbstyle;
    *((uint8_t*)(data + offset + 11)) = cs->header.cod->compparams.qmfbid;
    offset += 12;
    memcpy(data + offset, cs->header.cod->compparams.ppslist, cs->header.cod->compparams.sizepps);
    offset += cs->header.cod->compparams.sizepps;
    
    /* qcd */
    *((uint8_t*)(data + offset)) = 0xff;
    *((uint8_t*)(data + offset + 1)) = 0x5c;
    offset += 2;
    temp16 = htons(3 + cs->header.qcd->stepsizeslen);
    memcpy(data + offset, &temp16, 2);
    *((uint8_t*)(data + offset + 2)) = cs->header.qcd->quantstyle;
    offset += 3;
    memcpy(data + offset, cs->header.qcd->quantstepsizes, cs->header.qcd->stepsizeslen);
    offset += cs->header.qcd->stepsizeslen;
    
    /* qcc */
    for(i = 0; i < cs->header.numqcc; i++){
        *((uint8_t*)(data + offset)) = 0xff;
        *((uint8_t*)(data + offset + 1)) = 0x5d;
        offset += 2;
        if(cs->header.siz->numcomponents < 257){
            temp16 = htons(4 + cs->header.qcc[i]->stepsizeslen);
            memcpy(data + offset, &temp16, 2);
            offset += 2;
            *((uint8_t*)(data + offset)) = cs->header.qcc[i]->compnum.compnum8;
            offset += 1;
        }else{
            temp16 = htons(5 + cs->header.qcc[i]->stepsizeslen);
            memcpy(data + offset, &temp16, 2);
            offset += 2;
            temp16 = htons(cs->header.qcc[i]->compnum.compnum16);
            memcpy(data + offset, &temp16, 2);
            offset += 2;
        }
        *((uint8_t*)(data + offset)) = cs->header.qcc[i]->quantstyle;
        offset++;
        memcpy(data + offset, cs->header.qcc[i]->quantstepsizes, cs->header.qcc[i]->stepsizeslen);
        offset += cs->header.qcc[i]->stepsizeslen;
    }
    
    /* cme */
    for(i = 0; i < cs->header.numcomments; i++){
        *((uint8_t*)(data + offset)) = 0xff;
        *((uint8_t*)(data + offset + 1)) = 0x64;
        offset += 2;
        temp16 = htons(4 + cs->header.comments[i]->length);
        memcpy(data + offset, &temp16, 2);
        temp16 = htons(cs->header.comments[i]->regvalue);
        memcpy(data + offset + 2, &temp16, 2);
        offset += 4;
        memcpy(data + offset, cs->header.comments[i]->data, cs->header.comments[i]->length);
        offset += cs->header.comments[i]->length;
    }
    
    /* tiles */
    int t;
    for(t = 0; t < cs->numtiles; t++){
        *((uint8_t*)(data + offset)) = 0xff;
        *((uint8_t*)(data + offset + 1)) = 0x90;
        offset += 2;
        temp16 = htons(10);
        memcpy(data + offset, &temp16, 2);
        temp16 = htons(cs->tiles[t]->header.tilenum);
        memcpy(data + offset + 2, &temp16, 2);
        int datalenoffset = offset - 2;
        *((uint8_t*)(data + offset + 8)) = cs->tiles[t]->header.partnum;
        *((uint8_t*)(data + offset + 9)) = cs->tiles[t]->header.numparts;
        offset += 10;
        
        /* coc */
        if(cs->tiles[t]->header.coc != NULL){
            *((uint8_t*)(data + offset)) = 0xff;
            *((uint8_t*)(data + offset + 1)) = 0x53;
            offset += 2;
            temp16 = htons((cs->header.siz->numcomponents < 257 ? 9 : 10) + cs->tiles[t]->header.coc->compparams.sizepps);
            memcpy(data + offset, &temp16, 2);
            if(cs->header.siz->numcomponents < 257){
                *((uint8_t*)(data + offset + 2)) = cs->tiles[t]->header.coc->numcomps.numcomps8;
                offset += 3;
            }else{
                temp16 = htons(cs->tiles[t]->header.coc->numcomps.numcomps16);
                memcpy(data + offset + 2, &temp16, 2);
                offset += 4;
            }
            *((uint8_t*)(data + offset)) = cs->tiles[t]->header.coc->codingstyle;
            *((uint8_t*)(data + offset + 1)) = cs->tiles[t]->header.coc->compparams.decomplvls;
            *((uint8_t*)(data + offset + 2)) = cs->tiles[t]->header.coc->compparams.cbwidthexp;
            *((uint8_t*)(data + offset + 3)) = cs->tiles[t]->header.coc->compparams.cbhightexp;
            *((uint8_t*)(data + offset + 4)) = cs->tiles[t]->header.coc->compparams.cbstyle;
            *((uint8_t*)(data + offset + 5)) = cs->tiles[t]->header.coc->compparams.qmfbid;
            offset += 6;
            memcpy(data + offset, cs->tiles[t]->header.coc->compparams.ppslist, cs->tiles[t]->header.coc->compparams.sizepps);
            offset += cs->tiles[t]->header.coc->compparams.sizepps;
        }
        
        /* qcc */
        for(i = 0; i < cs->tiles[t]->header.numqcc; i++){
            *((uint8_t*)(data + offset)) = 0xff;
            *((uint8_t*)(data + offset + 1)) = 0x5d;
            offset += 2;
            temp16 = htons((cs->header.siz->numcomponents < 257 ? 4 : 5) + cs->tiles[t]->header.qcc[i]->stepsizeslen);
            memcpy(data + offset, &temp16, 2);
            if(cs->header.siz->numcomponents < 257){
                *((uint8_t*)(data + offset + 2)) = cs->tiles[t]->header.qcc[i]->compnum.compnum8;
                offset += 3;
            }else{
                temp16 = htons(cs->tiles[t]->header.qcc[i]->compnum.compnum16);
                memcpy(data + offset + 2, &temp16, 2);
                offset += 4;
            }
            *((uint8_t*)(data + offset)) = cs->tiles[t]->header.qcc[i]->quantstyle;
            offset++;
            memcpy(data + offset, cs->tiles[t]->header.qcc[i]->quantstepsizes, cs->tiles[t]->header.qcc[i]->stepsizeslen);
            offset += cs->tiles[t]->header.qcc[i]->stepsizeslen;
            
        }
        
        /* sod */
        *((uint8_t*)(data + offset)) = 0xff;
        *((uint8_t*)(data + offset + 1)) = 0x93;
        offset += 2;
        
        temp32 = htonl(cs->tiles[t]->header.tilelength + (offset - datalenoffset));
        memcpy(data + datalenoffset + 6, &temp32, 4);
        
        memcpy(data + offset, cs->tiles[t]->codedata, cs->tiles[t]->codelength);
        offset += cs->tiles[t]->codelength;
    }
    
    /* eod */
    *((uint8_t*)(data + offset)) = 0xff;
    *((uint8_t*)(data + offset + 1)) = 0xd9;
    offset += 2;
    
    return offset;
}

void jp2kcs_print(struct jp2kcs_t* cs){
    printf("JP2K-CS: refsize %dx%d, imgoffset %dx%d, tileoffset %dx%d, tiles %d,",
           cs->header.siz->xref, cs->header.siz->yref, cs->header.siz->ximgoffset, cs->header.siz->yimgoffset,
           cs->header.siz->xtileoffset, cs->header.siz->ytileoffset, cs->numtiles);
    printf(" tilesize %dx%d, numcomponents %d\n", 
           cs->header.siz->reftilewidth, cs->header.siz->reftilehight, cs->header.siz->numcomponents);
    int i;
    for(i = 0; i < cs->header.siz->numcomponents; i++){
        printf("        component %d, bitdepth %d, xsep %d, ysep %d\n", 
               i, cs->header.siz->componentrefs[i]->bitdepth, 
               cs->header.siz->componentrefs[i]->xsep, cs->header.siz->componentrefs[i]->ysep);
    }
    printf("    COD: coding style %x, progessorder %d, numlayers %d, mctransform %d,", 
           cs->header.cod->codingstyle, cs->header.cod->progessorder,
           cs->header.cod->numlayers, cs->header.cod->mctransform);
    printf(" decomplvls %d, cbwidthexp %d, cbhightexp %d ,cbstyle %d, qmfbid %d, numpps %d\n",
           cs->header.cod->compparams.decomplvls, cs->header.cod->compparams.cbwidthexp,
           cs->header.cod->compparams.cbhightexp, cs->header.cod->compparams.cbstyle,
           cs->header.cod->compparams.qmfbid, cs->header.cod->compparams.sizepps);
    printf("    QCD: quantization style %x, paramlen %d\n",
           cs->header.qcd->quantstyle, cs->header.qcd->stepsizeslen);
    for(i = 0; i < cs->header.numqcc; i++){
        printf("    QCC: component %d, quantstyle %x, paramlen %d\n", 
               (cs->header.siz->numcomponents < 257) ? cs->header.qcc[i]->compnum.compnum8 : cs->header.qcc[i]->compnum.compnum16,
               cs->header.qcc[i]->quantstyle, cs->header.qcc[i]->stepsizeslen);
    }
    for(i = 0; i < cs->header.numcomments; i++){
        printf("    Comment: regval %x, length %d", 
               cs->header.comments[i]->regvalue, cs->header.comments[i]->length);
        if(cs->header.comments[i]->regvalue == 1){
            printf(" string \"%s\"\n", ((char*)(cs->header.comments[i]->data)));
        }else{
            printf("\n");
        }
    }
    
    /* tile */
    int t;
    for(t = 0; t < cs->numtiles; t++){
        printf("  Tile: tilenum %d, length %d, partnum %d, numparts %d\n",
               cs->tiles[t]->header.tilenum, cs->tiles[t]->header.tilelength, 
               cs->tiles[t]->header.partnum, cs->tiles[t]->header.numparts);
         if(cs->tiles[t]->header.coc != NULL){
             printf("      COC: numcomps %d, codingstyle %x, decomplvls %d, cbwdithexp %d, cbhightexp %d, cbstyle %d, qmfbid %d, numpps %d\n",
                    (cs->header.siz->numcomponents < 257)?(cs->tiles[t]->header.coc->numcomps.numcomps8):(cs->tiles[t]->header.coc->numcomps.numcomps16),
                    cs->tiles[t]->header.coc->codingstyle, cs->tiles[t]->header.coc->compparams.decomplvls,
                    cs->tiles[t]->header.coc->compparams.cbwidthexp, cs->tiles[t]->header.coc->compparams.cbhightexp, cs->tiles[t]->header.coc->compparams.cbstyle, cs->tiles[t]->header.coc->compparams.qmfbid, cs->tiles[t]->header.coc->compparams.sizepps);
         }
         for(i = 0; i < cs->tiles[t]->header.numqcc; i++){
             printf("      QCC: compnum %d, quantstyle %x, paramlen %d\n",
                    (cs->header.siz->numcomponents < 257)?(cs->tiles[t]->header.qcc[i]->compnum.compnum8):(cs->tiles[t]->header.qcc[i]->compnum.compnum16),
                   cs->tiles[t]->header.qcc[i]->quantstyle, cs->tiles[t]->header.qcc[i]->stepsizeslen);
         }
    }
}
