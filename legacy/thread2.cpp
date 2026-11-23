//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "thread2.h"
#include "main.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------

//   Important: Methods and properties of objects in VCL can only be
//   used in a method called using Synchronize, for example:
//
//      Synchronize(UpdateCaption);
//
//   where UpdateCaption could look like:
//
//      void __fastcall Thread2::UpdateCaption()
//      {
//        Form1->Caption = "Updated in a thread";
//      }
//---------------------------------------------------------------------------

__fastcall Thread2::Thread2(bool CreateSuspended)
        : TThread(CreateSuspended)
{
        success = false;
}
//---------------------------------------------------------------------------
void __fastcall Thread2::Execute()
{
    char *x ;
   int i;

   //while(Form1->ClientSocket1->Socket->Connected == false)
    {
       // Form1->ClientSocket1->Active = false;
        Sleep(100);
        //Form1->ClientSocket1->Address ="86.106.12.76";
        Form1->ClientSocket1->Active = true;
    }


   x=strdup("068055555555550405814444f40000902091226060000933da8c36a3cd6833\0");

    i = Form1->ClientSocket1->Socket->SendBuf(x,strlen(x));

    Sleep(500);

   if(i== strlen(x))
   {
    success= true;
   }
   else
   {
        success=false;
   }

   Suspend();
}
//---------------------------------------------------------------------------
