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
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "box.h"

#ifdef htonll
# define htonq		htonll
# define ntohq		ntohll
#endif

#ifndef htonq
# define htonq(i)	( ((int64_t)(htonl((i) & 0xffffffff)) << 32) | htonl(((i) >> 32) & 0xffffffff ) )
# define ntohq		htonq
#endif

const int box_content_type_unknown = 0;
const int box_content_type_parent = 1;

struct box_t* box_create(){
    struct box_t* box = malloc(sizeof(struct box_t));
    box->lBox = 0;
    memcpy(box->tBox, "    ", 4);
    box->xlBox = 0;
    box->dBox = NULL;
    box->content_type = 0;
    box->content_free = NULL;
    box->contents = NULL;
    return box;
}

void box_delete(struct box_t* box){
    if(box != NULL){
        if(box_get_content_size(box) != 0){
            if(box->dBox != NULL){
                free(box->dBox);
            }
        }
        if(box->contents != NULL){
            if(box->content_free != NULL){
                (box->content_free)(box->contents);
            }else{
                free(box->contents);
            }
        }
        free(box);
    }
}

void box_print_impl(struct box_t* box, int level){
    int spaces = level * 2;
    printf("%*sBox: type %c%c%c%c, length %d, xlength %lld\n", 
           spaces, "",
           box->tBox[0], box->tBox[1], box->tBox[2], box->tBox[3],
           box->lBox, box->xlBox);
    if(box->content_type == box_content_type_parent){
        int i;
        struct parentbox_t* parent = box->contents;
        for(i = 0; i < parent->numboxes; i++){
            box_print_impl(parent->boxes[i], level + 1);
        }
    }
}

void box_print(struct box_t* box){
    box_print_impl(box, 0);
}

int box_parse_from_file(struct box_t* box, int file){
    int datalen = 0;
    int headersize = 8;
    char buff[8];
    int rtv;
    
    /* get length */
    rtv = read(file, buff, 4);
    if(rtv == 0){
        return 0;
    }
    box->lBox = ntohl(*((int*)buff));
    datalen = box->lBox;
    /* get type */
    rtv = read(file, box->tBox, 4);
    if(rtv == 0){
        return 0;
    }
    /* get xlength if length = 1 */
    if(box->lBox == 1){
        rtv = read(file, buff, 8);
        if(rtv == 0){
            return 0;
        }
        box->xlBox = ntohq(*((uint64_t*)buff));
        datalen = box->xlBox;
        headersize = 16;
    }
    
    /* get data */
    if(datalen != 0){
        /* size is bounded */
        int actualsize = datalen - headersize;
        if(actualsize > 0){
            box->dBox = malloc(datalen-headersize);
            if(box->dBox == NULL){
                return 0;
            }
            rtv = read(file, box->dBox, datalen-headersize);
            if(rtv == 0){
                return 0;
            }
        }else{
            box->dBox = NULL;
        }
    }else{
        /* size is to the end of file */
        uint64_t allocsize = 1024 * 1024; /* 1MB */
        uint64_t sizesofar = 0;
        int recvsize = 1;
        
        box->dBox = malloc(allocsize); /* 1MB */
        if(box->dBox == NULL){
            return 0;
        }
        while(recvsize != 0){
            recvsize = read(file, box->dBox + sizesofar, allocsize - sizesofar);
            sizesofar += recvsize;
            if(allocsize - sizesofar < 1024){
                void* tempptr = realloc(box->dBox, allocsize * 2);
                if(tempptr == NULL){
                    return 0;
                }
                allocsize += allocsize;
                box->dBox = tempptr;
            }
        }
        /* Set the size (this breaks rebuilding exactly the same file) */
        if(sizesofar < UINT32_MAX){
            box->lBox = sizesofar + 8;
        }else{
            box->lBox = 1;
            box->xlBox = sizesofar + 16;
        }
    }
    return 1;
}

int box_parse_from_mem(struct box_t* box, void* data, int length){
    uint64_t offset = 8;
    uint64_t size;
    uint32_t temp32;
    if(length < 8){
        return 0;
    }
    
    memcpy(&temp32, data, 4);
    box->lBox = ntohl(temp32);
    memcpy(box->tBox, data + 4, 4);
    if(box->lBox == 1){
        box->xlBox = ntohq(*((uint64_t*)(data + 8)));
        offset = 16;
        size = box->xlBox;
    }else{
        size = box->lBox;
    }
    
    /* check length and size are the same */
    if(size != 0 && size > length){
        return 0;
    }
    
    box->dBox = malloc(size);
    if(box->dBox == NULL){
        return 0;
    }
    memcpy(box->dBox, data + offset, size);
    
    return 1;
}

int box_format_to_file(struct box_t* box, int file){
    uint32_t temp32;
    uint64_t realsize;
    int headerlength = 8;
    
    /* length */
    temp32 = htonl(box->lBox);
    write(file, (char*)(&temp32), 4);
    realsize = box->lBox;
    
    /* type */
    write(file, box->tBox, 4);
    
    /* extended length */
    if(realsize == 1){
        uint64_t temp64 = htonq(box->xlBox);
        write(file, (char*)(&temp64), 8);
        realsize = box->xlBox;
        headerlength = 16;
    }
    
    /* data */
    if(realsize - headerlength > 0){
        if(box->dBox == NULL){
            return 0;
        }
        write(file, box->dBox, realsize - headerlength);
    }
    
    return 1;
}

int box_format_to_mem(struct box_t* box, void* data){
    uint32_t temp32;
    uint64_t realsize;
    int offset = 8;
    int headersize = 8;
    
    /* length */
    temp32 = htonl(box->lBox);
    memcpy(data, &temp32, 4);
    realsize = box->lBox;
    
    /* type */
    memcpy(data + 4, box->tBox, 4);
    
    /* extended size if needed */
    if(realsize == 1){
        uint64_t temp64 = htonq(box->xlBox);
        memcpy(data + 8, &temp64, 8);
        realsize = box->xlBox;
        offset += 8;
        headersize = 16;
    }
    
    /* data */
    memcpy(data + offset, box->dBox, realsize - headersize);
    
    return 1;
}

uint64_t box_get_size(struct box_t* box){
    if(box->lBox == 1){
        return box->xlBox;
    }else{
        return box->lBox;
    }
}

uint64_t box_get_content_size(struct box_t* box){
    if(box->lBox == 1){
        return box->xlBox - 16;
    }else{
        return box->lBox - 8;
    }
}

void box_set_size(struct box_t* box, uint64_t size){
    if(size < UINT32_MAX){
        box->lBox = size;
    }else{
        box->lBox = 1;
        box->xlBox = size;
    }
}

struct parentbox_t* parentbox_create(){
    struct parentbox_t* parentbox = malloc(sizeof(struct parentbox_t));
    parentbox->numboxes = 0;
    parentbox->boxes = malloc(sizeof(struct parentbox_t*) * 500);
    return parentbox;
}

void parentbox_delete(struct parentbox_t* pbox){
    int i;
    for(i = 0; i < pbox->numboxes; i++){
        box_delete(pbox->boxes[i]);
    }
    free(pbox->boxes);
    free(pbox);
}

int parsentbox_parse_box(struct parentbox_t* pbox, struct box_t* box){
    int offset = 0;
    int boxlength = box_get_content_size(box);
    while(offset < boxlength){
        struct box_t* nbox = box_create();
        if(box_parse_from_mem(nbox, box->dBox + offset, boxlength - offset)){
            pbox->boxes[pbox->numboxes] = nbox;
            pbox->numboxes++;
            offset += box_get_size(nbox);
        }else{
            break;
        }
    }
    return 1;
}

