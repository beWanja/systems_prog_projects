#include "pbm.h"
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

/*
Mallocing space(new) and freeing it up (del)
*/

PPMImage * new_ppmimage( unsigned int w, unsigned int h, unsigned int m )
{
  PPMImage *ppm = (PPMImage *)malloc(sizeof(PPMImage)); //Allocate mem for PPMImage struct
  if (ppm->pixmap == NULL) {
     perror("Failed to allocate memory for pixmap");
     free(ppm);
     exit(EXIT_FAILURE);
   }
  
  for (int i = 0; i < 3; i++) {
        // Allocate memory for the array of row pointers
        ppm->pixmap[i] = (unsigned int **)malloc(h * sizeof(unsigned int *));
        if (ppm->pixmap[i] == NULL) {
            perror("Failed to allocate memory for color channel rows");
            // Free already allocated memory for other channels
            for (int j = 0; j < i; j++) {
                for (unsigned int k = 0; k < h; k++) {
                    free(ppm->pixmap[j][k]);
                }
                free(ppm->pixmap[j]);
            }
            free(ppm);
            exit(EXIT_FAILURE);
        }
        
        for (unsigned int row = 0; row < h; row++) {
            ppm->pixmap[i][row] = (unsigned int *)malloc(w * sizeof(unsigned int));
            if (ppm->pixmap[i][row] == NULL) {
                perror("Failed to allocate memory for color channel pixels");
                // Free already allocated memory for this channel
                for (unsigned int k = 0; k < row; k++) {
                    free(ppm->pixmap[i][k]);
                }
                free(ppm->pixmap[i]);

                // Free already allocated channels
                for (int j = 0; j < i; j++) {
                    for (unsigned int k = 0; k < h; k++) {
                        free(ppm->pixmap[j][k]);
                    }
                    free(ppm->pixmap[j]);
                }
                free(ppm);
                exit(EXIT_FAILURE);
            }
        }
    }
    ppm->width = w;
    ppm->height = h;
    ppm->max = m;

    return ppm;
}

PBMImage * new_pbmimage( unsigned int w, unsigned int h )
{
  PBMImage *pbm = (PBMImage *)malloc(sizeof(PBMImage)); //Allocate mem for PBMImage struct
  if (pbm == NULL) {
    perror("Failed to allocate memory for PBMImage struct");
    exit(EXIT_FAILURE);
  }
  
  pbm->width = w;
  pbm->height = h;
  
  //Allocating pixmap memory
  pbm->pixmap = (unsigned int **)malloc(h * sizeof(unsigned int *));
  if (pbm->pixmap == NULL) {
    perror("Failed to allocate memory for pixmap");
    free(pbm);
    exit(EXIT_FAILURE);
  }
  
  //Allocate rows and col mem
  for (unsigned int i = 0; i < h; i++) {
    pbm->pixmap[i] = (unsigned int *)malloc(w * sizeof(unsigned int));
    if (pbm->pixmap[i] == NULL) {
        perror("Failed to allocate memory for pixmap row");
        for (unsigned int j = 0; j < i; j++) {
            free(pbm->pixmap[j]); // Free already allocated rows
        }
        free(pbm->pixmap);
        free(pbm);
        exit(EXIT_FAILURE);
      }
   }  
  return pbm;
}

PGMImage * new_pgmimage( unsigned int w, unsigned int h, unsigned int m )
{
  PGMImage *pgm = (PGMImage *)malloc(sizeof(PGMImage)); //Allocate mem for PGMImage struct
  if (pgm == NULL) {
    perror("Failed to allocate memory for PGMImage struct");
    exit(EXIT_FAILURE);
  }
  
  pgm->width = w;
  pgm->height = h;
  pgm->max = m;
  
  //Allocating pixmap memory
  pgm->pixmap = (unsigned int **)malloc(h * sizeof(unsigned int *));
  if (pgm->pixmap == NULL) {
     perror("Failed to allocate memory for pixmap");
     free(pgm);
     exit(EXIT_FAILURE);
   }
  
  
  //Allocate row and col mem
  for(unsigned int i = 0; i < h; i++){
    pgm->pixmap[i] = (unsigned int *)malloc(w * sizeof(unsigned int));
    if (pgm->pixmap[i] == NULL) {
     perror("Failed to allocate memory for pixmap row");
     for (unsigned int j = 0; j < i; j++) {
         free(pgm->pixmap[j]); // Free already allocated rows
     }
     free(pgm->pixmap);
     free(pgm);
     exit(EXIT_FAILURE);
    }
  }
  
  return pgm;
  
}

void del_ppmimage( PPMImage * p )
{
  if(p != NULL){
    unsigned int h = p->height;
    
    for(int i=0; i<3; i++) {
     if (p->pixmap[i] != NULL) {
      for(unsigned int j = 0; j < h; j++){
        free(p->pixmap[i][j]);
      }
      free(p->pixmap[i]);
      }
    }
    free(p);
  }
}  


void del_pbmimage( PBMImage * p )
{
  if(p != NULL){
    unsigned int h = p->height;
    for(unsigned int j = 0; j < h; j++){
      free(p->pixmap[j]);
    }
  free(p->pixmap);
  free(p);
  }
}


void del_pgmimage( PGMImage * p )
{
  if(p != NULL){
    unsigned int h = p->height;
    for(unsigned int j = 0; j < h; j++){
       free(p->pixmap[j]);
     }
    free(p->pixmap);
    free(p);
  }
}

