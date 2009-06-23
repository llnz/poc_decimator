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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "box.h"
#include "packet.h"
#include "jp2kcs.h"

#include "decimator.h"

void extract_parent_box(struct box_t* box){
    struct parentbox_t* parentbox = parentbox_create();
    if(parsentbox_parse_box(parentbox, box)){
        box->contents = parentbox;
        box->content_type = box_content_type_parent;
        box->content_free = (void(*)(void*))parentbox_delete;
    }
    int i;
    for(i = 0; i < parentbox->numboxes; i++){
        if(memcmp(parentbox->boxes[i]->tBox, "trak", 4) == 0 || memcmp(parentbox->boxes[i]->tBox, "mdia", 4) == 0 || memcmp(parentbox->boxes[i]->tBox, "minf", 4) == 0 || memcmp(parentbox->boxes[i]->tBox, "dinf", 4) == 0 || memcmp(parentbox->boxes[i]->tBox, "stbl", 4) == 0){
            extract_parent_box(parentbox->boxes[i]);
        }
    }
}

/* The word Decimate comes from Roman times. When a legon was found wanting,
 * the leader would kill every 10th man in the unit. Deci- means 1/10.
 * Modern usage is to reduce something greatly, this is what we are doing 
 * here with jpeg2000 images.
 */


void boxes_decimator(struct box_t** boxes, int numboxes, int outlow, int outrest, int maxrlevel, int maxlayer){
    int i;
    for(i = 0; i < numboxes; i++){
        box_print(boxes[i]);
        
        if(memcmp(boxes[i]->tBox, "mdat", 5) == 0){
            int f;
            struct parentbox_t* parentbox = boxes[i]->contents;
            for(f = 0; f < parentbox->numboxes; f++){
                jp2cbox_decimator(parentbox->boxes[f], outrest, maxrlevel, maxlayer);
            }
        }else if(memcmp(boxes[i]->tBox, "jp2c", 4) == 0){
            jp2cbox_decimator(boxes[i], outrest, maxrlevel, maxlayer);
        
        }
    }
    
    for(i = 0; i < numboxes; i++){
        box_format_to_file(boxes[i], outlow);
    }
}

void jp2cbox_decimator(struct box_t* box, int outrest, int maxrlevel, int maxlayer){
    if(memcmp(box->tBox, "jp2c", 4) != 0){
        printf("Box is not a jp2c box\n");
        return;
    }
    
    void *codestreamdata = box->dBox;
    int codestreamlength = box_get_content_size(box);
    
    struct jp2kcs_t* codestream = jp2kcs_create();
    if(jp2kcs_decode(codestream, codestreamdata, codestreamlength)){
        jp2kcs_print(codestream);
    }else{
        printf("Failed to decode codestream\n");
        jp2kcs_print(codestream);
        return;
    }
    
    cs_decimator(codestream, outrest, maxrlevel, maxlayer);
    
    void* newcodestreamdata = malloc(codestreamlength);
    int newcodestreamlength = jp2kcs_encode(codestream, newcodestreamdata);
    
    //create new boxes for keep, output to outlow
    
    printf("Rebox codestream\n");
    
    box->dBox = newcodestreamdata;
    box->lBox = 8 + newcodestreamlength;
    
}

void cs_decimator(struct jp2kcs_t* codestream, int outrest, int maxrlevel, int maxlayer){
    if(maxrlevel <= 0){
        maxrlevel = codestream->header.cod->compparams.decomplvls + 1;
    }
    if(maxlayer <= 0){
        maxlayer = codestream->header.cod->numlayers;
    }
    
    int t;
    for(t = 0; t < codestream->numtiles; t++){
        struct packet_codec_t* codec = packet_codec_create();
        packet_codec_parse(codec, codestream, codestream->tiles[t]->codedata, codestream->tiles[t]->codelength);
        packet_codec_print(codec);
        printf("\n");
        
        packet_decimator(codec, outrest, maxrlevel, maxlayer);
        
        printf("Creating new bitstream\n");
        int reqsize = 0;
        int i;
        for(i = 0; i < codec->numpackets; i++){
            reqsize += codec->packets[i]->hlen + codec->packets[i]->dlen;
            if(codec->packets[i]->hlen + codec->packets[i]->dlen == 0){
                reqsize += 7;
            }
        }
        void* keepbs = malloc(reqsize);
        packet_codec_write(codec, keepbs);
    
        printf("Updating codestream\n");
        codestream->tiles[t]->codedata = keepbs;
        codestream->tiles[t]->codelength = reqsize;
        codestream->tiles[t]->header.tilelength = reqsize;
        
    }
    
    codestream->header.comments = realloc(codestream->header.comments, sizeof(struct jp2kcs_cme_t*) * (codestream->header.numcomments + 1));
    codestream->header.comments[codestream->header.numcomments] = malloc(sizeof(struct jp2kcs_cme_t));
    codestream->header.comments[codestream->header.numcomments]->regvalue = 1;
    codestream->header.comments[codestream->header.numcomments]->data = malloc(27);
    codestream->header.comments[codestream->header.numcomments]->length = 26;
    memcpy(codestream->header.comments[codestream->header.numcomments]->data, "Decimated by poc_decimator", 27);
    codestream->header.numcomments++;
    
}

void packet_decimator(struct packet_codec_t* codec, int outrest, int maxrlevel, int maxlayer){
    
    printf("Removing unwanted packets\n");
    int i;
    for(i = 0; i < codec->numpackets; i++){
        if(codec->packets[i]->rlvl >= maxrlevel || codec->packets[i]->layer >= maxlayer){
            codec->packets[i]->hlen = 0;
            codec->packets[i]->dlen = 0;
        }
    }
    
}

void decimator(int infile, int outlow, int outrest, int maxrlevel, int maxlayer){
    struct box_t* boxes[100];
    int numboxes = 0;
    //read infile
    //parse into boxes
    
    while(1){
        struct box_t* box = box_create();
        if(box_parse_from_file(box, infile)){
            boxes[numboxes] = box;
            numboxes++;
            if(memcmp(box->tBox, "jp2h", 4) == 0){
                extract_parent_box(box);
            }
            if(memcmp(box->tBox, "moov", 4) == 0 || memcmp(box->tBox, "mdat", 4) == 0){
                extract_parent_box(box);
            }
        }else{
            box_delete(box);
            break;
        }
    }
    
    printf("\nNum boxes: %d\n", numboxes);
    
    boxes_decimator(boxes, numboxes, outlow, outrest, maxrlevel, maxlayer);
    
}
