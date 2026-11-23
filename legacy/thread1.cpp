//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "thread1.h"
#include "main.h"
#pragma package(smart_init)

__fastcall thread1::thread1(bool CreateSuspended)
        : TThread(CreateSuspended)
{
  FreeOnTerminate=true;
}
//---------------------------------------------------------------------------
void __fastcall thread1::Execute()
{
   Form1->thread_started=true;
   int i=0;

   if(Form1->ListBox1->Count>0)
       Form1->ListBox1->ItemIndex=0;

   while(i<Form1->ListBox1->Count)
   {

      i=get_index_for_zero_time();

      if(i== -1)
      {
        break;
      }

      Form1->ListBox1->ItemIndex=i;
      Form1->ListBox4->ItemIndex=i;
      Form1->ListBox5->ItemIndex=i;

      get_mms_pdu_size(Form1->ListBox1->Items->Strings[Form1->ListBox1->ItemIndex]);

      get_from_field_mms(Form1->ListBox1->ItemIndex);

      create_notification_header(Form1->ListBox1->ItemIndex);

      if(send_notification(Form1->ListBox1->ItemIndex)==true)
      {
        Form1->ListBox7->Items->Delete(i);
        Form1->ListBox7->Items->Insert(i,-1);
      }
      else
      {
       Form1->ListBox7->Items->Delete(i);
       Form1->ListBox7->Items->Insert(i,10);
      }

      i++;

   }

   Form1->thread_started=false;
   Form1->Timer1->Enabled=false;
   if ( Form1->CheckBox1->Checked==true)
   {
     Form1->Button1->Enabled=true;
   }
}
//---------------------------------------------------------------------------

