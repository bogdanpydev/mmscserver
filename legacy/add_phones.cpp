//---------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "add_phones.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------------------
#pragma resource "*.dfm"
TOKRightDlg *OKRightDlg;
//--------------------------------------------------------------------- 
__fastcall TOKRightDlg::TOKRightDlg(TComponent* AOwner)
	: TForm(AOwner)
{
}
//---------------------------------------------------------------------
void __fastcall TOKRightDlg::CancelBtnClick(TObject *Sender)
{
OKRightDlg->Close();
}
//---------------------------------------------------------------------------



void __fastcall TOKRightDlg::OKBtnClick(TObject *Sender)
{
  FILE *f;
  bool var;

  var = verificare(Edit1->Text,Edit2->Text);

  if((var == true)&&(Edit1->Text.Length()>6)&&(Edit2->Text.Length()>2))
     {
         Form1->ListBox2->Items->Add(Edit1->Text);
         Form1->ListBox3->Items->Add(Edit2->Text);
         f=fopen("ip_phone.cfg","a");
         fprintf(f,"%s\n",Edit1->Text.c_str());
        fprintf(f,"%s\n",Edit2->Text.c_str());
        fclose(f);
        Edit1->Clear();
        Edit2->Clear();
        OKRightDlg->Close();
     }
     else
     {
        if(Edit1->Text.Length()<7)
           Label1->Font->Color=clRed;
        if(Edit2->Text.Length()<3)
            Label2->Font->Color=clRed;
        if (var == true)
           ShowMessage(" Verify the fields in red please");
        else
           ShowMessage(" IP or phonenumber already exists in the list \n Please introduce another IP or phone number ");
     }
 if (Form1->ListBox2->Count>0)
{
  Form1->Button3->Enabled = true;
}
}
//---------------------------------------------------------------------------



void __fastcall TOKRightDlg::Edit2KeyPress(TObject *Sender, char &Key)
{
    if(Label2->Font->Color==clRed)
      Label2->Font->Color=clBlack;
}
//---------------------------------------------------------------------------

void __fastcall TOKRightDlg::Edit1KeyPress(TObject *Sender, char &Key)
{
    if(Label1->Font->Color==clRed)
      Label1->Font->Color=clBlack;
}
//---------------------------------------------------------------------------

bool verificare(String s1,String s2)
{
   for(int i=0;i<Form1->ListBox2->Count;i++)
   {
     if(strcmp(s1.c_str(),Form1->ListBox2->Items->Strings[i].c_str()) == 0)
     {
        return false;
     }
   }

   for( int j=0;j<Form1->ListBox3->Count;j++)
   {
      if(strcmp(s2.c_str(),Form1->ListBox3->Items->Strings[j].c_str()) == 0)
     {
        return false;
     }
   }

   return true;
}
void __fastcall TOKRightDlg::FormShow(TObject *Sender)
{
 Edit1->SetFocus();        
}
//---------------------------------------------------------------------------

