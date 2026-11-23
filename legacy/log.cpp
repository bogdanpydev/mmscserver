//---------------------------------------------------------------------------


#pragma hdrstop

#include "log.h"
#include "main.h"
#include <stdio.h>

void log(String x)
{
 if(Form1->CheckBox4->Checked == true)
 {
  FILE *f;

  f=fopen("log.txt","a");
  fprintf(f,"%s \n",x.c_str());
  fclose(f);
 }
}

#pragma package(smart_init)
