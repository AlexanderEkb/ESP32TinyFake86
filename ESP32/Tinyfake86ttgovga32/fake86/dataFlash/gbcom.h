#ifndef _GB_COM_H
 #define _GB_COM_H

 #include <stddef.h>
 #include "com/combuck.h"
 #include "com/comgoody.h"
 #include "com/comarkanoid.h"
 
 #define max_list_com 3

 static const char * gb_list_com_title[max_list_com]={
  "buck rogers",
  "goody",
  "arkanoid",
 };
 
 //Tamanio en bytes
 static const unsigned short int gb_list_com_size[max_list_com]={
  59904,
  61440,
  36304,
 };
 
 static const unsigned char gb_list_seg_load[max_list_com]={
  0,
  1,
  1,
 };
 
 //Datos
 static const unsigned char * gb_list_com_data[max_list_com]={
  gb_com_buck,
  gb_com_goody,
  gb_com_arkanoid,
 };


#endif
