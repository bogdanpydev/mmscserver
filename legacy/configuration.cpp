//---------------------------------------------------------------------------


#pragma hdrstop

#include "configuration.h"
#include <stdio.h>
#include "log.h"
#include "main.h"

void initialize_configuration()
{
   FILE *f;

   log("initialize configuration --- load the settings from config file");
   f=fopen("config.cfg","rb");
   if(f== NULL)
     create_default_config();
   else
     fclose(f);

   load_settings();
}


void Enable(bool var)
{
   log("Enable buttons , lists , edit boxes etc ");
   Form1->ComboBox1->Enabled=var;
   Form1->ComboBox2->Enabled=var;
   Form1->ComboBox3->Enabled=var;
   Form1->Edit1->Enabled=var;
   Form1->Edit6->Enabled=var;
   Form1->Edit7->Enabled=var;
   Form1->Edit8->Enabled=var;
   Form1->Label1->Enabled=var;
   Form1->Label10->Enabled=var;
   Form1->Label11->Enabled=var;
   Form1->Label12->Enabled=var;
   Form1->Label13->Enabled=var;
   Form1->Label2->Enabled=var;
   Form1->Label4->Enabled=var;
   if(Form1->thread_started==false)
   {
       Form1->Button1->Enabled=var;
   }
   Form1->Edit10->Enabled=var;
   Form1->Label15->Enabled=var;
   Form1->Label21->Enabled=var;
   Form1->Edit12->Enabled=var;
   Form1->Label22->Enabled=var;
   Form1->ComboBox4->Enabled=var;
   Form1->Edit13->Enabled=var;
   Form1->Label23->Enabled=var;
   Form1->Edit14->Enabled=var;
   Form1->Label24->Enabled=var;
   Form1->Edit15->Enabled=var;
   Form1->Label25->Enabled=var;
   Form1->Edit16->Enabled=var;
   Form1->Label26->Enabled=var;
   Form1->Edit17->Enabled=var;
   Form1->Label27->Enabled=var;
   Form1->Edit18->Enabled=var;
   Form1->Label28->Enabled=var;
   Form1->Edit19->Enabled=var;
   Form1->Label29->Enabled=var;
   Form1->Label30->Enabled=var;
   Form1->ComboBox5->Enabled=var;
   Form1->Label31->Enabled=var;
   Form1->ComboBox6->Enabled=var;
   Form1->Label32->Enabled=var;
   Form1->ComboBox7->Enabled=var;
   Form1->Label33->Enabled=var;
   Form1->ComboBox8->Enabled=var;
   Form1->Label34->Enabled=var;
   Form1->Edit20->Enabled=var;
   Form1->Label35->Enabled=var;
   Form1->Edit21->Enabled=var;
   Form1->Label36->Enabled=var;
   Form1->ComboBox9->Enabled=var;
   Form1->Label37->Enabled=var;
   Form1->ComboBox10->Enabled=var;

}



void create_default_config()
{
  FILE *f;

  f=fopen("config.cfg","w");

  fprintf(f,"X_MMS_TYPE=0\n");
  fprintf(f,"X_MMS_VERSION=1\n");
  fprintf(f,"X_MSG_TRANSACTION=Transaction-ID-123456789\n");
  fprintf(f,"X_MMS_MESSAGE_CLASS=0\n");
  fprintf(f,"X_MMS_EXPIRY=286331153\n");
  fprintf(f,"WSP_PUSH_ID=0x04\n");
  fprintf(f,"WSP_PUSH_PDU=0x06\n");
  fprintf(f,"WSP_HEADER_LENGTH=0x37\n");
  fprintf(f,"WSP_PDU_CONTENT=application/vnd.wap.mms-message\n");
  fprintf(f,"WSP_ENCODING_VERSION=1.1\n");
  fprintf(f,"DELIVERY_REPORT=0\n");
  fprintf(f,"READ_REPORT=0\n");
  fprintf(f,"PRIORITY=1\n");

  fclose(f);
}

void load_settings()
{
  FILE *f;
  char *aux,*a;
  long length;

  f=fopen("config.cfg","r");

  fseek(f,0,2);
  length = ftell(f);
  fseek(f,0,0);

  aux=(char*)malloc(length+1);

  fread(aux,1,length,f);
  aux[length]='\0';

  a=find_string(aux,"X_MMS_TYPE=");
  if(a==NULL)
  {
    Form1->ComboBox1->ItemIndex=0;
  }
  else
  {
    Form1->ComboBox1->ItemIndex=*a-48;
    free(a);
  }

  a=find_string(aux,"X_MMS_VERSION=");
  if(a==NULL)
  {
    Form1->ComboBox2->ItemIndex=0;
  }
  else
  {
    Form1->ComboBox2->ItemIndex=*a-48;
    free(a);
  }

  a=find_string(aux,"X_MSG_TRANSACTION=");
  if(a==NULL)
  {
    Form1->Edit6->Text="Transaction-ID-123456789";
  }
  else
  {
    Form1->Edit6->Text=a;
    free(a);
  }

  a=find_string(aux,"X_MMS_MESSAGE_CLASS=");
  if(a==NULL)
  {
    Form1->ComboBox3->ItemIndex=0;
  }
  else
  {
    Form1->ComboBox3->ItemIndex=*a-48;
    free(a);
  }

  a=find_string(aux,"X_MMS_EXPIRY=");
  if(a==NULL)
  {
    Form1->Edit8->Text="286331153";
  }
  else
  {
    Form1->Edit8->Text=a;
    free(a);
  }

  a=find_string(aux,"WSP_PUSH_ID=");
  if(a==NULL)
  {
    Form1->Edit2->Text="0x04";
  }
  else
  {
    Form1->Edit2->Text=a;
    free(a);
  }


  a=find_string(aux,"WSP_PUSH_PDU=");
  if(a==NULL)
  {
    Form1->Edit3->Text="0x06";
  }
  else
  {
    Form1->Edit3->Text=a;
    free(a);
  }

  a=find_string(aux,"WSP_HEADER_LENGTH=");
  if(a==NULL)
  {
    Form1->Edit4->Text="0x37";
  }
  else
  {
    Form1->Edit4->Text=a;
    free(a);
  }

  a=find_string(aux,"WSP_PDU_CONTENT=");
  if(a==NULL)
  {
    Form1->Edit5->Text="application/vnd.wap.mms-message";
  }
  else
  {
    Form1->Edit5->Text=a;
    free(a);
  }


  a=find_string(aux,"WSP_ENCODING_VERSION=");
  if(a==NULL)
  {
    Form1->Edit9->Text="1.1";
  }
  else
  {
    Form1->Edit9->Text=a;
    free(a);
  }

  a=find_string(aux,"DELIVERY_REPORT=");
  if((a==NULL)||(a[0]=='0'))
  {
    Form1->CheckBox5->Checked = false;
  }
  else if (a[0]=='1')
  {
    Form1->CheckBox5->Checked = true;
  }
  free(a);

  a=find_string(aux,"READ_REPORT=");
  if((a==NULL)||(a[0]=='0'))
  {
    Form1->CheckBox6->Checked = false;
  }
  else if (a[0]=='1')
  {
    Form1->CheckBox6->Checked = true;
  }

  free(a);

  a=find_string(aux,"TIMING=");
  if(a==NULL)
  {
     Form1->default_timing = 10;
  }
  else
  {
     Form1->default_timing = atoi(a);
     free(a);
  }

  free(aux);
  fclose(f);
}

void load_phone_numbers()
{
   FILE *f;
   char x[50];

   f=fopen("ip_phone.cfg","r");

   if(f!=NULL)
   {
      while(fgets(x,50,f))
      {
        x[strlen(x)-1]='\0';
        Form1->ListBox2->Items->Add(x);

        fgets(x,50,f);
        x[strlen(x)-1]='\0';
        if(strlen(x)!=0)
          Form1->ListBox3->Items->Add(x);
        else
        {
          Form1->ListBox3->Items->Add("9999999999");
          break;
        }
       }

   }
}

char *find_string(char *buffer,String s)
{
   char *aux,*val=NULL;
   int i;

   aux = strstr(buffer,s.c_str());
   if(aux == NULL)
   {
     return NULL;
   }
   else
   {
     aux=aux+s.Length();
     for(i=0;aux[i]!=0;i++)
     {
       if((aux[i]==' ')||(aux[i]==0x0A))
       {
         aux[i]=0;
         val=strdup(aux);
         aux[i]=0x0d;
         break;
       }
     }

     return val;
   }
}





#pragma package(smart_init)
