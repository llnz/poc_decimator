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
#ifndef BOX_H
#define BOX_H

#include <stdint.h>

struct box_t{
    uint32_t lBox;
    char tBox[4];
    uint64_t xlBox;
    void* dBox;
    void* contents;
    int content_type;
    void (*content_free)(void*);
};

extern const int box_content_type_unknown;
extern const int box_content_type_parent;


struct parentbox_t{
    uint32_t numboxes;
    struct box_t** boxes;
};

struct box_t* box_create();
void box_delete(struct box_t* box);

void box_print(struct box_t* box);


int box_parse_from_file(struct box_t* box, int file);
int box_parse_from_mem(struct box_t* box, void* data, int length);

int box_format_to_file(struct box_t* box, int file);
int box_format_to_mem(struct box_t* box, void* data);

uint64_t box_get_size(struct box_t* box);
uint64_t box_get_content_size(struct box_t* box);
void box_set_size(struct box_t* box, uint64_t size);


struct parentbox_t* parentbox_create();
void parentbox_delete(struct parentbox_t* pbox);

int parsentbox_parse_box(struct parentbox_t* pbox, struct box_t* box);

#endif
