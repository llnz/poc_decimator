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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int rlevel = 0;
    int layer = 0;
    int c;
    
    
    opterr = 0;
    
    while ((c = getopt (argc, argv, "r:l:")) != -1)
        switch (c)
    {
        case 'r':
            rlevel = atoi(optarg);
            break;
        case 'l':
            layer = atoi(optarg);
            break;
        case '?':
            if (optopt == 'r')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (optopt == 'l')
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr,
                         "Unknown option character `\\x%x'.\n",
                         optopt);
            return 1;
        default:
            abort ();
    }
    
    
    
  if(argc - optind != 3){
      printf("Usage:\n");
      printf("  %s [options] infile outlow outrest\n", argv[0]);
      printf("Options:\n\t-r val\tKeep resolutions less than val\n");
      printf("\t-l val\tKeep quality layers less than val\n"); 
      return 1;
  }

  int infile = open(argv[optind], O_RDONLY);
  if(infile < 1){
      fprintf(stderr, "Could not open infile \"%s\"\n", argv[optind]);
      return 2;
  }
  
  optind++;
  
  int outlow = open(argv[optind], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if(outlow < 1){
      fprintf(stderr, "Could not open outlow file \"%s\"\n", argv[optind]);
      close(infile);
      return 2;
  }
  
  optind++;
  
  int rest = open(argv[optind], O_WRONLY|O_CREAT|O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  if(rest < 1){
      fprintf(stderr, "Could not open outrest file \"%s\"\n", argv[optind]);
      close(infile);
      close(outlow);
      return 2;
  }
  
  decimator(infile, outlow, rest, rlevel, layer);
  
  close(rest);
  close(outlow);
  close(infile);
  
  return EXIT_SUCCESS;
}
