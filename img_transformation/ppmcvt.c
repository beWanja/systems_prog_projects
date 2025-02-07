#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <string.h>
#include "pbm.h"

void print_usage(){
      fprintf(stderr, "Usage: ppmcvt [-bgirsmtno] [FILE]\n");
      exit(1);
}

//Decalring transfromation functions
PBMImage* bitmap(PPMImage *p);
PGMImage* grayscale(PPMImage * p, int value);
PPMImage* isolate(PPMImage * p, char* color);
PPMImage* remove_channel(PPMImage * p, char* color);
PPMImage* sepia(PPMImage * p);
PPMImage* mirror(PPMImage * p);
PPMImage* thumbnail(PPMImage * p, int scale);
PPMImage* tile(PPMImage * p, int scale);

int main( int argc, char *argv[] )
{
    int opt;
    int transformation = 0; //Keeps track of no. of transf. applied/specified - always [0-1]
    int grayscale_max = 0; 
    char *channel; //Store the color channel specified for isolate and remove
    int scale = 0; //scale factor for thumbnail and tile transformation
    char *output_file = NULL; 
    char *input_file = NULL;    
    
    int pbm_mode = 0; //Default mode 
    int pgm_mode = 0, iso_mode = 0, rem_mode = 0, sepia_mode = 0;
    int mirr_mode = 0, tnail_mode = 0, tile_mode = 0;
    
    //getopt parsing the command line arguments
    while((opt = getopt(argc, argv, "bg:i:r:smt:n:o:")) != -1){
      switch(opt){
        case 'b': //
          if(transformation){
            fprintf(stderr, "Error: Multiple transformations specified\n");
            exit(1); 
          }
          pbm_mode = 1;
          transformation = 1;
          break;
          
        case 'g': // convert to pgm format
          if(transformation){
            fprintf(stderr, "Error: Multiple transformations specified\n");
            exit(1); 
          }
          
          grayscale_max = atoi(optarg);
          if(grayscale_max <= 0 || grayscale_max > 65535){
            fprintf(stderr, "Error: Invalid max grayscale pixel value: %s; must be less than 65,536\n", optarg);
            exit(1);
          }
                    
          pgm_mode = 1;
          transformation = 1;
          break;
      
        case 'i': //Isolate a color channel
          if(transformation){
            fprintf(stderr, "Error: Multiple transformations specified\n");
            exit(1); 
          }
          channel = optarg;
          if (strcmp(channel, "red") != 0 && strcmp(channel, "green") != 0 && strcmp(channel, "blue") != 0) {
            fprintf(stderr, "Error: Invalid channel specification: (%s); should be 'red', 'green', or 'blue'\n", channel);
            exit(1);
          }
          iso_mode = 1;
          transformation = 1;
          break;
        
        case 'r': //remove a color channel
          if(transformation){
            fprintf(stderr, "Error: Multiple transformations specified\n");
            exit(1); 
          }
          channel = optarg;
          if (strcmp(channel, "red") != 0 && strcmp(channel, "green") != 0 && strcmp(channel, "blue") != 0) {
            fprintf(stderr, "Error: Invalid channel specification: (%s); should be 'red', 'green', or 'blue'\n", channel);
            exit(1);
          }
          rem_mode = 1;
          transformation = 1;
          break;
          
        case 's': //sepia transformation
          if(transformation){
            fprintf(stderr, "Error: Multiple transformations specified\n");
            exit(1); 
          }
          sepia_mode = 1;
          transformation = 1;
          break;
          
        case 'm': //mirror the image
          if(transformation){
            fprintf(stderr, "Error: Multiple transformations specified\n");
            exit(1); 
          }
          mirr_mode = 1;
          transformation = 1;
          break;
          
        case 't': //thumbnail transformation
          if(transformation){
            fprintf(stderr, "Error: Multiple transformations specified\n");
            exit(1); 
          }
          
          scale =  atoi(optarg);
          
          if (scale <= 0) {
            fprintf(stderr, "Error: Invalid scale factor: %d; must be greater than 0\n", scale);
            exit(1);
          }
          
          tnail_mode = 1;
          transformation = 1;
          break;
        
        case 'n': //tiling thumbnails
          if(transformation){
            fprintf(stderr, "Error: Multiple transformations specified\n");
            exit(1); 
          }
          scale =  atoi(optarg);
          
          if (scale < 1 || scale > 8 ) {
            fprintf(stderr, "Error: Invalid scale factor: %d; must be 1-8\n", scale);
            exit(1);
          }
          
          tile_mode = 1;
          transformation = 1;
          break;
          
        case 'o': //handling the output option
          output_file = optarg;
          break;
          
        default: 
          print_usage();
              
      }//end switch
    } //end while
    
    //Handling remaining arguments
    if (optind >= argc) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage();
    } else {
        input_file = argv[optind]; 
    }
    
    if (output_file == NULL) {
        fprintf(stderr, "Error: No output file specified\n");
        print_usage();
    }
    
    if (transformation == 0) { //set bitmap transformation as default
      pbm_mode = 1;
    }

    
    //Call the transformations
    if(pbm_mode){
        printf("Converting %s to PBM and saving to %s\n", input_file, output_file);
        
        PPMImage *img = read_ppmfile(input_file);
        PBMImage *pbm = bitmap(img);
        write_pbmfile(pbm, output_file);
        del_pbmimage(pbm);
        del_ppmimage(img);
        
    }else if(pgm_mode) {
        printf("Converting %s to PGM with max grayscale %d and saving to %s\n", input_file, grayscale_max, output_file);
        
        PPMImage *img = read_ppmfile(input_file);
        PGMImage *pgm = grayscale(img, grayscale_max);
        write_pgmfile(pgm, output_file);
        del_pgmimage(pgm);
        del_ppmimage(img);        
        
    }else if (iso_mode){
        printf("Isolating %s channel from %s and saving to %s\n", channel, input_file, output_file);
        
        PPMImage *img = read_ppmfile(input_file);
        PPMImage *new_img = isolate(img, channel);
        write_ppmfile(new_img, output_file);
        del_ppmimage(new_img);
        del_ppmimage(img);
           
    }else if(rem_mode){
        printf("Removing %s channel from %s and saving to %s\n", channel, input_file, output_file);
        
        PPMImage *img = read_ppmfile(input_file);
        PPMImage *new_img = remove_channel(img, channel);
        write_ppmfile(new_img, output_file);
        del_ppmimage(new_img);
        del_ppmimage(img);
        
    }else if(sepia_mode) {
        printf("Applying sepia transformation to %s and saving to %s\n", input_file, output_file);
        
        PPMImage *img = read_ppmfile(input_file);
        PPMImage *new_img = sepia(img);
        write_ppmfile(new_img, output_file);
        del_ppmimage(new_img);
        del_ppmimage(img);
        
    }else if(mirr_mode){
        printf("Applying vertical mirror to %s and saving to %s\n", input_file, output_file);
        
        PPMImage *img = read_ppmfile(input_file);
        PPMImage *new_img = mirror(img);
        write_ppmfile(new_img, output_file);
        del_ppmimage(new_img);
        del_ppmimage(img);
        
    }else if(tnail_mode) {
        printf("Reducing %s to a thumbnail with scale factor %d and saving to %s\n", input_file, scale, output_file);
        
        PPMImage *img = read_ppmfile(input_file);
        PPMImage *new_img = thumbnail(img, scale);
        write_ppmfile(new_img, output_file);
        del_ppmimage(new_img);
        del_ppmimage(img);        
        
    }else if(tile_mode) {
        printf("Tiling %s into %d thumbnails and saving to %s\n", input_file, scale, output_file);
        
        PPMImage *img = read_ppmfile(input_file);
        PPMImage *new_img = tile(img, scale);
        write_ppmfile(new_img, output_file);
        del_ppmimage(new_img);
        del_ppmimage(img);
    }    
    
    return 0;
}

//Implementations for Transformations 

//Convert ppm image to pbm
PBMImage* bitmap(PPMImage * p){
  PBMImage *pbm = new_pbmimage(p->width, p->height);
    
  for(unsigned int h = 0; h < p->height; h++) {
     for(unsigned int w = 0; w < p->width; w++) {
         unsigned int avg = (int)(p->pixmap[0][h][w] + p->pixmap[1][h][w] + p->pixmap[2][h][w]) / 3;
         if(avg < (int)(p->max/2)){
            pbm->pixmap[h][w] = 0;
         }else{
            pbm->pixmap[h][w] = 1;
         }
     }
   }
   return pbm;
}

//Convert the ppm image to a pgm 
PGMImage* grayscale(PPMImage * p, int value){
  PGMImage *pgm = new_pgmimage(p->width, p->height, value);

  for(unsigned int h = 0; h < p->height; h++) {
    for(unsigned int w = 0; w < p->width; w++) {
        unsigned int avg = (unsigned int)(p->pixmap[0][h][w] + p->pixmap[1][h][w] 
                            + p->pixmap[2][h][w])/3;
        pgm->pixmap[h][w] = (unsigned int)((avg*value)/(p->max));  
    }
  }
  return pgm;

}

//Isolate the specified RGB channel
PPMImage* isolate(PPMImage * p, char* color){
  PPMImage *new_p = new_ppmimage(p->width, p->height, p->max);
  int channel = (strcmp(color, "red") == 0) ? 0 : (strcmp(color, "green") == 0) ? 1 : 2; //WILL REVIEW THIS LATER

  for (unsigned int h = 0; h < p->height; h++) {
      for (unsigned int w = 0; w < p->width; w++) {
          for (int i = 0; i < 3; i++) {
              new_p->pixmap[i][h][w] = (i == channel) ? p->pixmap[i][h][w] : 0;
          }
      }
  }
  return new_p; 
  
}


//Remove the specified RGB channel.
PPMImage* remove_channel(PPMImage * p, char* color){
  PPMImage *new_p = new_ppmimage(p->width, p->height, p->max);
  int channel = (strcmp(color, "red") == 0) ? 0 : (strcmp(color, "green") == 0) ? 1 : 2;
 
  for (unsigned int h = 0; h < p->height; h++) {
      for (unsigned int w = 0; w < p->width; w++) {
          for (int i = 0; i < 3; i++) {
              new_p->pixmap[i][h][w] = (i == channel) ? 0 : p->pixmap[i][h][w];
          }
      }
  }
  return new_p;
}


//Apply a sepia transformation
PPMImage* sepia(PPMImage * p){
  PPMImage *new_p = new_ppmimage(p->width, p->height, p->max);
  
  for (unsigned int h = 0; h < p->height; h++) {
      for (unsigned int w = 0; w < p->width; w++) {
          unsigned int red = p->pixmap[0][h][w];
          unsigned int green = p->pixmap[1][h][w];
          unsigned int blue = p->pixmap[2][h][w];
          
          unsigned int tr = 0.393 * red + 0.769 * green + 0.189 * blue;
          unsigned int tg = 0.349 * red + 0.686 * green + 0.168 * blue;
          unsigned int tb = 0.272 * red + 0.534 * green + 0.131 * blue;
          
          new_p->pixmap[0][h][w] = (tr > p->max) ? p->max : (unsigned int)tr;
          new_p->pixmap[1][h][w] = (tg > p->max) ? p->max : (unsigned int)tg;
          new_p->pixmap[2][h][w] = (tb > p->max) ? p->max : (unsigned int)tb;
      }
  }
  return new_p;

}

//Vertically mirror the first half of the image to the second half
PPMImage* mirror(PPMImage * p){ 
  PPMImage *new_p = new_ppmimage(p->width, p->height, p->max);
  
  // Copy first half of the image
  for (unsigned int h = 0; h < p->height; h++) {
      for (unsigned int w = 0; w < p->width / 2; w++) {
          for (int i = 0; i < 3; i++) {
              new_p->pixmap[i][h][w] = p->pixmap[i][h][w];
              new_p->pixmap[i][h][p->width - w- 1] = p->pixmap[i][h][w];  // Mirror vertically
          }
      }
  }
  return new_p;

}

//Reduce image to a thumbnail based on scale given
PPMImage* thumbnail(PPMImage * p, int scale){
  unsigned int new_width = p->width / scale;
  unsigned int new_height = p->height / scale;
  PPMImage *new_p = new_ppmimage(new_width, new_height, p->max);
  
  for (unsigned int h = 0; h < new_height; h++) {
      for (unsigned int w = 0; w < new_width; w++) {
          for (int i = 0; i < 3; i++) {
              // Average over scale x scale block
              unsigned int sum = 0;
              for (unsigned int y = 0; y < scale; y++) {
                  for (unsigned int x = 0; x < scale; x++) {
                      sum += p->pixmap[i][h*scale + y][w*scale + x];
                  }
              }
              new_p->pixmap[i][h][w] = (unsigned int)sum / (scale * scale);  // Average pixel
          }
      }
  }
  return new_p;
}

//Tile thumbnails based on scale given
PPMImage* tile(PPMImage * p, int scale){
  unsigned int thumb_width = p->width / scale;
  unsigned int thumb_height = p->height / scale;
  unsigned int new_width = p->width * scale;
  unsigned int new_height = p->height * scale;
  
  PPMImage *new_p = new_ppmimage(new_width, new_height, p->max);
  
  // Create thumbnails
  PPMImage *thumb = thumbnail(p, scale);
  
  // Tile the thumbnails
  for (unsigned int y = 0; y < scale; y++) {
      for (unsigned int x = 0; x < scale; x++) {
          for (unsigned int h = 0; h < thumb_height; h++) {
              for (unsigned int w = 0; w < thumb_width; w++) {
                  for (int i = 0; i < 3; i++) {
                      new_p->pixmap[i][y*thumb_height + h][x*thumb_width + w] = thumb->pixmap[i][h][w];
                  }
              }
          }
      }
  }
  
  del_ppmimage(thumb);
  return new_p;

}
