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
#ifndef JP2KCS_H
#define JP2KCS_H

#include <stdint.h>

#define JP2KCS_CODINGSTYLE_SOP 0x2

struct jp2kcs_siz_compref_t{
    uint8_t bitdepth;
    uint8_t xsep;
    uint8_t ysep;
};

struct jp2kcs_siz_t{
    uint16_t capability;
    uint32_t xref;
    uint32_t yref;
    uint32_t ximgoffset;
    uint32_t yimgoffset;
    uint32_t reftilewidth;
    uint32_t reftilehight;
    uint32_t xtileoffset;
    uint32_t ytileoffset;
    uint16_t numcomponents;
    struct jp2kcs_siz_compref_t** componentrefs;
};

struct jp2kcs_cme_t{
    uint16_t regvalue;
    uint16_t length;
    void* data;
};

struct jp2kcs_cox_comp_t{
    uint8_t decomplvls;
    uint8_t cbwidthexp;
    uint8_t cbhightexp;
    uint8_t cbstyle;
    uint8_t qmfbid;
    uint16_t sizepps;
    uint8_t* ppslist;
};

struct jp2kcs_cod_t{
    uint8_t codingstyle;
    uint8_t progessorder;
    uint16_t numlayers;
    uint8_t mctransform;
    struct jp2kcs_cox_comp_t compparams;
};

struct jp2kcs_qcd_t{
    uint8_t quantstyle;
    uint16_t stepsizeslen;
    void* quantstepsizes;
};



struct jp2kcs_coc_t{
    union {
        uint8_t numcomps8;
        uint16_t numcomps16;
    } numcomps;
    uint8_t codingstyle;
    struct jp2kcs_cox_comp_t compparams;
};

struct jp2kcs_qcc_t{
    union {
        uint8_t compnum8;
        uint16_t compnum16;
    } compnum;
    uint8_t quantstyle;
    uint16_t stepsizeslen;
    void* quantstepsizes;
};

struct jp2kcs_mainheader_t{
    struct jp2kcs_siz_t* siz;
    struct jp2kcs_cod_t* cod;
    struct jp2kcs_qcd_t* qcd;
    
    uint16_t numqcc;
    struct jp2kcs_qcc_t** qcc;
    
    uint8_t numcomments;
    struct jp2kcs_cme_t** comments;
    
};

struct jp2kcs_tileheader_t{
    uint16_t tilenum;
    uint32_t tilelength;
    uint8_t partnum;
    uint8_t numparts;
    struct jp2kcs_coc_t* coc;
    uint16_t numqcc;
    struct jp2kcs_qcc_t** qcc;
};

struct jp2kcs_tile_t{
    struct jp2kcs_tileheader_t header;
    uint32_t codelength;
    void* codedata;
};

struct jp2kcs_t{
    struct jp2kcs_mainheader_t header;
    uint16_t numtiles;
    struct jp2kcs_tile_t** tiles;
};


struct jp2kcs_t* jp2kcs_create();
void jp2kcs_delete(struct jp2kcs_t* cs);

int jp2kcs_decode(struct jp2kcs_t* cs, void* data, int len);
int jp2kcs_encode(struct jp2kcs_t* cs, void* data);

void jp2kcs_print(struct jp2kcs_t* cs);

#endif
