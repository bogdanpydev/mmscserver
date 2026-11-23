//---------------------------------------------------------------------------


#pragma hdrstop

#include "sms_pdu.h"
#include <string.h>
#include <alloc.h>
#include <stdio.h>
#include "main.h"
#include "log.h"

char add_sms_sca_address(char *buffer)
{
  log("add sca address  ----- 07910427024037F5");

  buffer[0] = 0x07;
  buffer[1] = 0x91;
  buffer[2] = 0x04;
  buffer[3] = 0x27;
  buffer[4] = 0x02;
  buffer[5] = 0x40;
  buffer[6] = 0x37;
  buffer[7] = 0xf5;
  return 8;

}

char add_sms_mti(char *buffer,unsigned char position,char buffer_length,bool segment,bool mti)
{
   if(mti==true)
   {
     if(segment == false)
     {
        log("ADD MTI -----44");
        buffer[position] = 0x44;
     }
     else
     {
        log("ADD MTI -----40");
        buffer[position] = 0x40;
     }
   }
   else
   {
     log("ADD MTI -----44");
     buffer[position] = 0x44;
   }
  return position+1;
}

char add_sms_time_stamp(char *buffer,unsigned char position)
{
    log("Add time stamp  -----90201230459000");

    buffer[position++]= 0x90;
    buffer[position++]= 0x20;
    buffer[position++]= 0x12;
    buffer[position++]= 0x30;
    buffer[position++]= 0x45;
    buffer[position++]= 0x90;
    buffer[position++]= 0x00;

    return position;
}

char add_sms_coding_scheme(char *buffer,unsigned char position)
{
   log("Add coding scheme ---- 04");

  buffer[position] = 0x04;

  return position+1;
}

char add_sms_add_buffer(char *buffer_dest,char *buffer_source,unsigned char position,unsigned char length)
{
   char *aux;
   aux = buffer_dest+position;
   for(int i=0;i<length;i++)
   {
     aux[i]=buffer_source[i];
   }

   log("sms add user data , length ");
   log(length);

   return position+length;
}

unsigned char add_sms_user_data_length(unsigned char *buffer,unsigned char position,unsigned char length)
{

     buffer[position++] = length;

     log("add user data length ");
     log(length);

     return position;
}

char add_sms_protocol_identifier(char *buffer,unsigned char position)
{
   log("add protocol identifier  ------ 3B");

   buffer[position] = 0x3b;
   return position+1;
}

char add_sms_orignator_address(char *buffer,unsigned char position)
{

  log("add sms_originator_address     0C95912303612113");

  buffer[position++] = 0x0c;
  buffer[position++] = 0x95;
  buffer[position++] = 0x91;
  buffer[position++] = 0x23;
  buffer[position++] = 0x03;
  buffer[position++] = 0x61;
  buffer[position++] = 0x21;
  buffer[position++] = 0x13;

  return position;
}

char add_sms_udhl(char *buffer,unsigned char position,bool concat)
{
    if(concat == true)
    {
       buffer[position++] = 12;
       log("add sms udhl   12");
    }
    else
    {
       buffer[position++] = 6;
       log("add sms udhl   6");
    }
    return position;
}

char add_sms_ie_concat(char *buffer,unsigned char position,char seg_number,char number_of_segments,unsigned char x1, unsigned char x2)
{
   buffer[position++] = 0x08;
   buffer[position++] = 0x04;
   buffer[position++] = x1;
   buffer[position++] = x2;
   buffer[position++] = number_of_segments;
   buffer[position++] = seg_number;

   log("add ie_concat ");
   log(" ID ");
   log(x1*x2);
   log("number_of_segments");
   log(number_of_segments);
   log("segment number");
   log(seg_number);


   return position;
}

char add_sms_ie_app_port(char *buffer,unsigned char position)
{

   log("add ie_app_port  0B8423F0");
   buffer[position++] = 0x05;
   buffer[position++] = 0x04;
   buffer[position++] = 0x0b;
   buffer[position++] = 0x84;
   buffer[position++] = 0x23;
   buffer[position++] = 0xf0;

   return position;
}

char create_pdu(char *buffer,char *buffer_source,char length,char nr_segment,char number_of_segments,bool segment,bool mti,unsigned char x1,unsigned char x2)
{
   unsigned char position=0;

   log("create notification pdu");

   position=add_sms_sca_address(buffer);
   position=add_sms_mti(buffer,position,length,segment,mti);
   position=add_sms_orignator_address(buffer,position);
   position=add_sms_protocol_identifier(buffer,position);
   position=add_sms_coding_scheme(buffer,position);
   position=add_sms_time_stamp(buffer,position);
   if(mti==false)
   {
      position=add_sms_user_data_length(buffer,position,(unsigned)length+7);
      position=add_sms_udhl(buffer,position,false);
      position=add_sms_ie_app_port(buffer,position);
   }
   else
   {
      position=add_sms_user_data_length(buffer,position,(unsigned)length+13);
      position=add_sms_udhl(buffer,position,true);
      position=add_sms_ie_app_port(buffer,position);
      position=add_sms_ie_concat(buffer,position,nr_segment,number_of_segments,x1,x2);

   }
   position = add_sms_add_buffer(buffer,buffer_source,position,length);

   return position;
}

unsigned char *transform_pdu(unsigned char *buffer,unsigned char length)
{
   unsigned char *pdu = (char*)malloc(2*length+1);

   log("transform pdu form char to wchar_t");

   for(int i=0;i<length;i++)
   {
       snprintf(pdu+2*i,2,"%.2X",buffer[i]);
   }

   pdu[2*length] = 0;

   log((char*)pdu);

   return pdu;
}

#pragma package(smart_init)
