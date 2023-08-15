#ifndef _GB_DSK_H
 #define _GB_DSK_H

 #include <stddef.h>
 #include "dsk/dskpackgames00.h"
 #define max_list_dsk 1
 
 static const char * gb_list_dsk_title[max_list_dsk]={
  "Pack00",
  };
   
 //Datos
 static const unsigned char * gb_list_dsk_data[max_list_dsk]={
  gb_dsk_packgames00,
 };
  
#endif
