//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "main.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "thread1.h"
#include "sms_pdu.h"
#include "configuration.h"
#include "add_phones.h"
#include "log.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
thread1 *send_thread;


_socket_list *socket_list=NULL;
_socket_list *get_socket_list=NULL;
_process_queue *process_queue = NULL;

int number_of_connections80=0;
int number_of_connections8002=0;

bool sms_notification = true;



void add_socket(TCustomWinSocket *Socket,int connection_number)
{
  _socket_list *p;

  log("add client socket for port 80");

  p=(_socket_list*)malloc(sizeof(_socket_list));

  p->Socket = Socket;
  p->connection_number = connection_number;
  p->file_name[0]='\0';
  p->req_send=2;
  p->mms_size=-1;
  p->next   = NULL;
  p->is_first = TRUE;
  p->action = 2;
  p->message_worth_sending = TRUE;

  if (socket_list == NULL)
  {
     socket_list=p;
  }
  else
  {
     p->next = socket_list;
     socket_list =p;
  }
}

void delete_socket(TCustomWinSocket *Socket)
{
    _socket_list *s,*aux=NULL;
    int aux_con=-1;
    s=socket_list;

    log("delete client socket for port 80");

    while((s!=NULL)&&(s->Socket!=Socket))
     { aux=s;
       s=s->next;
     }

     if(aux==NULL)
     {
       socket_list=socket_list->next;
       free(s);
       aux_con=0;
     }
     else
     {
       aux->next=s->next;
       aux_con=s->connection_number;
       free(s);
     }

    if(aux_con != -1)
    {
     s=socket_list;
     while(s!=NULL)
     {
        if(s->connection_number>=aux_con)
           s->connection_number--;
        s=s->next;
     }
    }
}


void add_socket8002(TCustomWinSocket *Socket,int connection_number)
{
  _socket_list *p;

  p=(_socket_list*)malloc(sizeof(_socket_list));

  log("add client socket for port 8002");

  p->Socket = Socket;
  p->connection_number = connection_number;
  p->file_name[0]='\0';
  p->req_send=0;
  p->mms_size=-1;
  p->next   = NULL;

  if (get_socket_list == NULL)
  {
     get_socket_list=p;
  }
  else
  {
     p->next = get_socket_list;
     get_socket_list =p;
  }
}

void delete_socket8002(TCustomWinSocket *Socket)
{
    _socket_list *s,*aux=NULL;
    int aux_con=-1;
    s=get_socket_list;

    log("delete client socket for port 80");

    while((s!=NULL)&&(s->Socket!=Socket))
     { aux=s;
       s=s->next;
     }

     if(aux==NULL)
     {
       get_socket_list=get_socket_list->next;
       free(s);
       aux_con=0;
     }
     else
     {
       aux->next=s->next;
       aux_con=s->connection_number;
       free(s);
     }

    if(aux_con != -1)
    {
     s=get_socket_list;
     while(s!=NULL)
     {
        if(s->connection_number>=aux_con)
           s->connection_number--;
        s=s->next;
     }
    }
}

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//---------------------------------------------------------------------------



void __fastcall TForm1::FormCreate(TObject *Sender)
{

    log("Form create");

    thread_started=false;
    initialize_configuration();
    Enable(false);
    load_phone_numbers();

    Form1->notification_list = NULL;
    Form1->Edit32->Text = Form1->default_timing;

}
//---------------------------------------------------------------------------



void __fastcall TForm1::CheckBox1Click(TObject *Sender)
{
bool var;

  log("Manual check box clicked");

 if(CheckBox1->Checked==TRUE)
 {
    Enable(true);
 }
 else
 {
        Enable(false);
 }

var=CheckBox1->Checked;
CheckBox2->Checked = var;
CheckBox3->Checked = var;

}
//---------------------------------------------------------------------------
void __fastcall TForm1::ServerSocket1ClientError(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{

   ErrorCode=0;
   log("Server Error for port 80");

}
//---------------------------------------------------------------------------
void __fastcall TForm1::ServerSocket1ClientConnect(TObject *Sender,
      TCustomWinSocket *Socket)
{

        log("Client connect for port 80");
       add_socket(Socket,number_of_connections80);
       number_of_connections80++;
       
       if(Edit1->Text.Length()==0)
       {
         Edit1->Text="http://";
         Edit1->Text=Edit1->Text+Socket->LocalAddress;
         Edit1->Text=Edit1->Text+":8002";
       }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ServerSocket1ClientDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
      char *filename;
      int index;

      log("Client disconnect for port 80");
      if((get_message_worth_sending(Socket)==true)&&
        ((filename=get_file_name(Socket))!=NULL))
       {

           ListBox4->Items->Add(Socket->RemoteAddress);

           ListBox7->Items->Add(get_timing(filename));

           /*adds in the list all the destinations */
           index=decode_mms_destination(filename);

           if(get_dest_ip(index)==0)
           {
             update_to_succes(index);
           }

           if(Timer1->Enabled==false)
           {
                Timer1->Interval=1000;
                Timer1->Enabled=true;
           }
       }

           delete_socket(Socket);
           number_of_connections80--;

     

}
//---------------------------------------------------------------------------
void __fastcall TForm1::ServerSocket1ClientRead(TObject *Sender,
      TCustomWinSocket *Socket)
{
  int nr_con,rec,size=0;
  unsigned char *filename,*buf;
  unsigned char buffer[100000];
  FILE *f=NULL;
  bool is_first,val_bul;

  log("receive mms data");

  nr_con=get_connection(Socket);

  Sleep(500);
  rec=ServerSocket1->Socket->Connections[nr_con]->ReceiveBuf(buffer,100000);

  val_bul=do_skip(buffer,rec);

  if(val_bul==false)
  {

        filename=get_file_name(Socket);

        if( strlen(filename)<=8)
        {
               set_filename(Socket);
               filename=get_file_name(Socket);
        }

        f=fopen(filename,"a+b");
  }

        if( f== NULL)
        {
          log("Error at opening file ");
           set_message_worth_sending(false,Socket);
          send_finsih(nr_con);

        }
        else
        {
                if(get_mms_size(Socket)<0)
                {
                        set_mms_size(buffer,Socket);
                        set_req_send(buffer,Socket);
                }

             buf=get_mms_message(buffer,rec);


             is_first=is_first_receive(Socket);


             if((buf==NULL)&&(is_first!=FALSE))
                {
                }
             else
                {
                  if(is_first)
                  {
                   size=get_mms_buffer_size(buffer,rec,buf,Socket);
                   log(" size remaining ");
                   log(size);
                   set_action(buf,size,Socket);
                   fwrite(buf,1,size,f);

                  }
                  else
                  {
                        size=rec;
                        fwrite(buffer,1,size,f);
                        get_mms_buffer_size(buffer,rec,buf,Socket);
                  }


                int k=get_mms_size(Socket);

                if((k<5000)&&(k>=0))
                {
                   if(get_action(Socket)==1)
                   {
                      set_message_worth_sending(false,Socket);
                      send_finsih(nr_con);
                   }
                   else
                   {
                     send_ok(nr_con);

                     if((Form1->CheckBox5->Checked == true)||(Form1->CheckBox6->Checked == true))
                     {
                        decode_delivery(filename,Form1->ServerSocket1->Socket->Connections[nr_con]->RemoteAddress.c_str());

                        Form1->Timer2->Enabled=true;
                     }
                   }
                }
                set_first_receive(Socket);
              }
             fclose(f);
    }
}
//---------------------------------------------------------------------------

int get_mms_buffer_size(char *buffer,int size,char *buffer2,TCustomWinSocket *Socket)
{
   _socket_list *s;
   int i=0;
   char *c;

   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }
   if(s== NULL)
   {
     log("connection doesn't exist");
     return -1;
   }
   else
   {
     i=0;
     for(int j=0;j<size;j++)
     {
       if(buffer++==buffer2)
          {
            i=j;
          }
     }

     if(buffer2)
     {
       s->mms_size=s->mms_size-size+i;
       log("size ......");
       log(s->mms_size);
     }
     else
     {
       s->mms_size=s->mms_size-size;
       log("size ......");
       log(s->mms_size);
     }

     if(buffer2)
     {
       return size-i;
     }
     else
     {
       return size;
     }
   }
}

char *get_mms_message(unsigned char *buffer,int rec)
{
  char *p=NULL;

  for(int i=0;i<rec-1;i++)
  {
   if(( buffer[i]==0x8c)&&((buffer[i+1]<0x94)&&(buffer[i+1]>=0x80)))
     {
       p=buffer+i;
       break;
     }
  }

  if(p==NULL)
  {
     return NULL;
  }

  return p;
}

char *get_file_name(TCustomWinSocket *Socket)
{
   _socket_list *s;

   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
       return NULL;
   }
   else
   {
       return s->file_name;
   }
}

int get_connection(TCustomWinSocket *Socket)
{
   _socket_list *s;

   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
       log("Connection doesn't exist anymore");
       return -1;
   }
   else
   {
       return s->connection_number; 
   }

}

int get_connection8002(TCustomWinSocket *Socket)
{
   _socket_list *s;

   s=get_socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
       log("Connection doesn't exist anymore");
       return -1;
   }
   else
   {
       return s->connection_number;
   }

}

void set_mms_size(char *buffer,TCustomWinSocket *Socket)
{
    _socket_list *s;
    char *aux;
    int sum=0;

    s=socket_list;

    while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s == NULL)
   {
      log("Error , connection doesn't exist in the list");
   }
   else
   {
     aux = strstr(buffer,"Content-Length: ");

     if(aux == NULL)
     {
         log(" Length not found in the string");
     }
     else
     {
       /* points to the first digit */
       aux=aux+16;

       for(int i=0;i<6;i++)
       {
          if(isdigit(aux[i]))
          {
            sum=sum*10+aux[i]-0x30;
          }
          else
          {
            break;
          }
       }

       s->mms_size=sum;
       log("mms set size ");
       log (sum);
     }
   }
}

int get_mms_size(TCustomWinSocket *Socket)
{
   _socket_list *s;

   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
       log("Connection doesn't exist anymore");
       return -1;
   }
   else
   {
       return s->mms_size;
   }

}

void set_filename(TCustomWinSocket *Socket)
{
   _socket_list *s;
   FILE *f;
   String s0="mmsmessage";
   String s1;
   int n=1;
   bool fexists=true;

   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
     log("connection doesn't exist");
   }
   else
   {
        while(fexists==true)
        {
             s1=s0+n;
             f=fopen(s1.c_str(),"r");
             if(f!=NULL)
             {
               fclose(f);
               fexists=true;
               n++;
             }
             else
             {
               fexists=false;
             }
         }

         strcpy(s->file_name,s1.c_str());
    }

}


void send_ok(int con)
{
 FILE *f;
 char buf[2048];
 int length;

 f=fopen("response","r");
 fseek(f,0,2);
 length=ftell(f);
 fseek(f,0,0);

 fread(buf,1,length,f);
 fclose(f);

 Form1->ServerSocket1->Socket->Connections[con]->SendBuf(buf,length);

}

void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
  update_timings_for_messages();

  if(get_zero_time()==TRUE)
  {
        Button1->Click();
  }
}
//---------------------------------------------------------------------------


int get_from_number_length(String number)
{
  /* lungimea /type plmn plus 2 bytes pentru codare */
  int i=12;
  int length;

  length=number.Length();

  i=i+length;

  return i;

}

void create_notification_header(int x)
{
   char packet_data[65000];
   int k=0,l;
   int dimension;
   char aux[1024];
   packet_data[k++]=(char)Form1->Edit2->Text.ToInt();

   packet_data[k++]=(char)Form1->Edit3->Text.ToInt();
   k++;

   strcpy(aux,Form1->Edit5->Text.c_str());

   for(int i=0;aux[i]!=0;i++)
   {
     packet_data[k++]=aux[i];
   }

   packet_data[k++]='\0';

   strcpy(aux,"Encoding-version");

   for(int i=0;aux[i]!=0;i++)
   {
     packet_data[k++]=aux[i];
   }

   packet_data[k++]='\0';

   strcpy(aux,Form1->Edit9->Text.c_str());

   for(int i=0;aux[i]!=0;i++)
   {
     packet_data[k++]=aux[i];
   }

   packet_data[k++]='\0';
   packet_data[k++]=0xaf;
   packet_data[k++]=0x84;
   packet_data[2]=k-3;

   /* MMS pdu */

   /*message type */
   packet_data[k++]=0x8c;
   packet_data[k++]=0x82;


   /* transaction ID */
   packet_data[k++]=0x98;

   strcpy(aux,Form1->Edit6->Text.c_str());

   for(int i=0;aux[i]!=0;i++)
   {
     packet_data[k++]=aux[i];
   }

   packet_data[k++]=0x00;

   /* mms version */
   packet_data[k++]=0x8d;

   switch (Form1->ComboBox2->ItemIndex)
   {
       case 0: packet_data[k++]=0x90;break;
       case 1: packet_data[k++]=0x91;break;
       case 2: packet_data[k++]=0x92;break;
       case 3: packet_data[k++]=0x93;break;
   }

   /* from field */
   packet_data[k++]=0x89;
   packet_data[k++]=get_from_number_length(Form1->Edit10->Text);
   packet_data[k++]=0x80;

   /* from number */
   strcpy(aux,Form1->Edit10->Text.c_str());

   for(int i=0;aux[i]!=0;i++)
   {
     packet_data[k++]=aux[i];
   }

   strcpy(aux,"/TYPE=PLMN");

   for(int i=0;aux[i]!=0;i++)
   {
     packet_data[k++]=aux[i];
   }

   packet_data[k++]='\0';

   /* message class */

   packet_data[k++]=0x8a;

   switch (Form1->ComboBox3->ItemIndex)
   {
       case 0: packet_data[k++]=0x80;break;
       case 1: packet_data[k++]=0x81;break;
       case 2: packet_data[k++]=0x82;break;
       case 3: packet_data[k++]=0x83;break;
   }

   /* message size */

   packet_data[k++]=0x8e;
   dimension=k;
   k++;
   k++;

   /* expiry */
   /*l=Form1->Edit8->Text.ToInt();
   aux[2]=l%256;
   l=l/256;
   aux[1]=l%256;
   l=l/256;
   aux[0]=l%256;
     */
   packet_data[k++]=0x88;
   packet_data[k++]=0x05;
   packet_data[k++]=0x81;
   packet_data[k++]=0x03;
   packet_data[k++]=0x02;
   packet_data[k++]=0xa3;
   packet_data[k++]=0x00;

   /*for(int i=0;i<3;i++)
   {
     packet_data[k++]=aux[i];
   }
    */
   /* content location */
   packet_data[k++]=0x83;

   strcpy(aux,Form1->Edit1->Text.c_str());
   strcat(aux,"/\0");
   strcat(aux,Form1->ListBox1->Items->Strings[x].c_str());

   for(int i=0;aux[i]!=0;i++)
   {
     packet_data[k++]=aux[i];
   }

   packet_data[k++]=0x00;

   /*calculata lungimea  mms -56 -14*/
   packet_data[dimension]=0x01;
   packet_data[dimension+1]=k-70;
   //de facut o smecherie mai buna aici

   FILE *f;

   f=fopen("send","w");
   fwrite(packet_data,1,k,f);
   fclose(f);
}

 int decode_mms_destination(char *file)
{
   FILE *f;
   char *buffer[3000];
   int size;
   int offset=0;
   String sender;
   int j;
   char to[40],cc[40],bcc[40];
   int ret,last=0;
   bool first = true;

   Form1->ListBox1->Items->Add(file);
   ret = Form1->ListBox1->Count-1;

   j=Form1->ListBox1->Items->IndexOf(file);
   sender=Form1->ListBox4->Items->Strings[j];

   f=fopen(file,"rb");
   size= fread(buffer,1,3000,f);
   fclose(f);

   do
   {
    offset=mms_get_to_destination((unsigned char*)buffer,size,offset,to);
    if(offset>0)
    {
       if(Form1->ListBox3->Items->IndexOf(to)<0)
       {
         add_to_process_queue(0,to,sender.c_str());

       }
       else
       {
          if(first == false)
          {
            Form1->ListBox1->Items->Add(file);
            Form1->ListBox4->Items->Add(sender);
            Form1->ListBox5->Items->Add(Form1->ListBox2->Items->Strings[Form1->ListBox3->Items->IndexOf(to)]);
            add_to_process_queue(0,to,sender.c_str());
            update_to_succes(Form1->ListBox1->Count-1);
          }
          else
          {
            first = false;
          }
       }
       last = offset;
    }

   }while(offset>0);

   offset =last;

   do
   {
    offset=mms_get_cc_destination((unsigned char*)buffer,size,offset,cc);
    if(offset>0)
    {
       if(Form1->ListBox3->Items->IndexOf(cc)<0)
       {
         add_to_process_queue(0,cc,sender.c_str());
       }
       else
       {
          Form1->ListBox1->Items->Add(file);
          Form1->ListBox4->Items->Add(sender);
          Form1->ListBox5->Items->Add(Form1->ListBox2->Items->Strings[Form1->ListBox3->Items->IndexOf(cc)]);
          add_to_process_queue(0,cc,sender.c_str());
          update_to_succes(Form1->ListBox1->Count-1);
       }
       last = offset;
    }

   }while(offset>0);

   offset =last;

   do
   {
    offset=mms_get_bcc_destination((unsigned char*)buffer,size,offset,bcc);


    if(offset>0)
    {
       if(Form1->ListBox3->Items->IndexOf(bcc)<0)
       {
         add_to_process_queue(0,bcc,sender.c_str());
       }
       else
       {
          Form1->ListBox1->Items->Add(file);
          Form1->ListBox4->Items->Add(sender);
          Form1->ListBox5->Items->Add(Form1->ListBox2->Items->Strings[Form1->ListBox3->Items->IndexOf(bcc)]);
          add_to_process_queue(0,bcc,sender.c_str());
          update_to_succes(Form1->ListBox1->Count-1);
       }
    }

   }while(offset>0);

  return ret;
}

bool send_notification(int x)
{
   FILE *f;
   int length;
   char buffer[64000];
   int i;

   f=fopen("send","r");
   fseek(f,0,2);
   length=ftell(f);
   fseek(f,0,0);

   fread(buffer,1,length,f);
   fclose(f);
   //remove("send");


   log("Send notification");


    if(sms_notification == true)
    {
        if(x>=Form1->ListBox5->Count)
        {
          return false;
        }
        else
        {
          return send_sms_notifications(buffer,length,Form1->ListBox5->Items->Strings[x]);
        }
    }
    else
    {
     Form1->UdpSocket1->RemoteHost=Form1->ListBox5->Items->Strings[x];
     Form1->UdpSocket1->Active=true;
     Form1->UdpSocket1->Connect();

     i=Form1->UdpSocket1->SendBuf(buffer,length);
     log ("conect udp bytes sent");
     log(i);

     if(i==length)
     {
       Form1->UdpSocket1->Disconnect();
       Form1->UdpSocket1->Active=false;
       return true;
     }
     else
     {
       Form1->UdpSocket1->Active=false;
       return false;
     }
    }


}
void __fastcall TForm1::Button1Click(TObject *Sender)
{

  if((Timer1->Enabled==true)&&(Form1->thread_started==false))
  {
     Form1->thread_started=true;

     send_thread =new thread1(true);

     if( Button1->Enabled==true)
     {
        Button1->Enabled=false;
     }

     send_thread->Resume();
  }
}
//---------------------------------------------------------------------------

void get_from_field_mms(int x)
{
     String s,aux,phone;
     char s1[20],s2[20];
     int i=-1;
     s=Form1->ListBox4->Items->Strings[x];
     if( s.Length()>0)
     {
        i=Form1->ListBox2->Items->IndexOf(s);
        if(i>=0)
        {
          phone=Form1->ListBox3->Items->Strings[i];
        }
     }
   Form1->Edit10->Text=phone;
}

void get_mms_pdu_size(String file)
{
  FILE *f;
  int length;
  f=fopen(file.c_str(),"r");
  if( f == NULL)
  {
    log("Error opening file \n file may be missing");
  }
  else
  {
        fseek(f,0,2);
        length=ftell(f);
        fclose(f);

        Form1->Edit7->Text=length;
  }
}



void __fastcall TForm1::Button2Click(TObject *Sender)
{
OKRightDlg->Show();

}
//---------------------------------------------------------------------------

int get_dest_ip(int x)
{
  FILE *f;
  char buffer[400000];
  char phone[40],*aux;
  int length;
  String ip;

  if(x<0)
  {
    return -1;
  }

  f=fopen(Form1->ListBox1->Items->Strings[x].c_str(),"r");

  if(f== NULL)
  {
    return -1;
  }  

  fseek(f,0,2);
  length=ftell(f);
  fseek(f,0,0);

  fread(buffer,1,length,f);
  fclose(f);

  for(int j=0;j<length-4;j++)
  {
    if((buffer[j]=='P')&&(buffer[j+1]=='L')&&(buffer[j+2]=='M')&&(buffer[j+3]=='N'))
    {
      aux=buffer+j;
      break;
    }
  }

  aux=aux-7;

  for(int j=aux-buffer;j>0;j--)
  {
    if(!isdigit(buffer[j]))
    {
      aux=buffer+j;
      break;
    }
  }
  if(*aux!='+')
  aux++;

  int i=0;
  for(int j=0;aux[j]!='/';j++)
  {
    phone[i++]=aux[j];
  }
  phone[i]='\0';

  log(phone);

  i=-1;
  i=Form1->ListBox3->Items->IndexOf(phone);
  log("Index of phone");
  log(i);
  if(i>=0)
  {
    ip=Form1->ListBox2->Items->Strings[i];
    if(ip.Length()>0)
     {
       Form1->ListBox5->Items->Add(ip);
       log(ip);
      }
    return 0;
  }
  else
  {
    remove(Form1->ListBox1->Items->Strings[Form1->ListBox1->Count-1].c_str());
    Form1->ListBox5->Items->Delete(Form1->ListBox1->Count-1);
    Form1->ListBox7->Items->Delete(Form1->ListBox1->Count-1);
    Form1->ListBox4->Items->Delete(Form1->ListBox1->Count-1);
    Form1->ListBox1->Items->Delete(Form1->ListBox1->Count-1);
    return -1;
  }
}


void __fastcall TForm1::FormShow(TObject *Sender)
{
   TabbedNotebook1->Left=0 ;
   TabbedNotebook1->Top=0;
   TabbedNotebook1->Height=Form1->Height -28;
   TabbedNotebook1->Width=Form1->Width-10 ;

// pun indexul 1 la listele cu numere de telefon

  if(ListBox2->Count>0)
  {
     ListBox2->ItemIndex = 0;
  }
  else
  {
     Button3->Enabled = false;
  }

  if(ListBox3->Count>0)
  {
     ListBox3->ItemIndex = 0;
  }
  
}
//---------------------------------------------------------------------------

void set_req_send(char *a,TCustomWinSocket *Socket)
{
  _socket_list *s;
   char *aux;
   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
     log("error , connexion not available");
   }
   else
   {
     aux=strstr(a,"POST");
     if(aux!=NULL)
     {
        s->req_send=1;
     }
   }

}

char get_req_send(TCustomWinSocket *Socket)
{
    _socket_list *s;
   char aux;
   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
     log("error ,connection not available");
     return -1;
   }
   else
   {
     aux=s->req_send;
     return aux;
   }
}

void send_mms_buf(int nr_con,String ip,String ip_from,String file,TCustomWinSocket *Socket)
{
   FILE *f,*g;
   int dimension,length;
   int i;
   int k=0;
   int len;
   char mms_pdu[150000];
   char aux[100];
   char *aux1=NULL;
   char buf[150000];


   log(" destionatio ip ");
   log(ip);

   log("from field");
   log(ip_from);
   
   f=fopen("response","rb");

   //f=fopen("myfile.txt","rb");

   fseek(f,0,2);
   len=ftell(f);
   fseek(f,0,0);

   fread(buf,1,len-21,f);
//fread(buf,1,len,f);
   fclose(f);

   for(i=0;i<len;i++)
   {
      mms_pdu[k++]=buf[i];
   }

   //strcpy(mms_pdu,buf);
   k=len-21;
   //k++;
   dimension=k;
   //dimension = len;


   // to modify

   // message type
    mms_pdu[k++]=0x8c;
    mms_pdu[k++]=0x84;

    mms_pdu[k++]=0x98;

    // transaction ID
    strcpy(aux,Form1->Edit12->Text.c_str());

    for(unsigned i=0;i<strlen(aux);i++)
      mms_pdu[k++] = aux[i];

    mms_pdu[k++] = '\0';

    // mms version
    mms_pdu[k++]=0x8d;

    switch(Form1->ComboBox4->ItemIndex)
    {
        case 0: mms_pdu[k++]=0x90;break;
        case 1: mms_pdu[k++]=0x91;break;
        case 2: mms_pdu[k++]=0x92;break;
        case 3: mms_pdu[k++]=0x93;break;
    }

     //read report
   mms_pdu[k++]=0x90;
   mms_pdu[k++]=mms_get_read_report(file);


   //from
   mms_pdu[k++] = 0x89;
   mms_pdu[k++]=get_from_number_length(ip_from);
   mms_pdu[k++]=0x80;

   // from number
   strcpy(aux,ip_from.c_str());

   for(int i=0;aux[i]!=0;i++)
   {
     mms_pdu[k++]=aux[i];
   }

   strcpy(aux,"/TYPE=PLMN");

   for(int i=0;aux[i]!=0;i++)
   {
     mms_pdu[k++]=aux[i];
   }

   mms_pdu[k++]='\0';

   //x-mms-previously sent by

   //x-mms previously sent date

   //x-mms reply charging

   //x-mms reply charging deadline

   //x-mms reply charging size

   //x-mms reply charging - ID

   //x-mms retrieve status

   //x-mms retrieve text

   //x-mms mm state

   //x-mms mm flags

   //x mms distribution indicator





   //Priority

   mms_pdu[k++]=0x8f;
   mms_pdu[k++]=0x81;


   //TO
   mms_pdu[k++]=0x97;

   i=Form1->ListBox2->Items->IndexOf(ip);
   strcpy(aux,Form1->ListBox3->Items->Strings[i].c_str());
   for(int i=0;aux[i]!=0;i++)
   {
     mms_pdu[k++]=aux[i];
   }

   strcpy(aux,"/TYPE=PLMN");

   for(int i=0;aux[i]!=0;i++)
   {
     mms_pdu[k++]=aux[i];
   }

   mms_pdu[k++]='\0';

   //CC
   mms_add_cc_to_mms(file,buf,&length);
   if(length >0)
   {
     for (i=0;i<length;i++)
     {
       mms_pdu[k++]=buf[i];
     }
   }

   //BCC
    mms_add_bcc_to_mms(file,buf,&length);
   if(length >0)
   {
     for (i=0;i<length;i++)
     {
       mms_pdu[k++]=buf[i];
     }
   }


   //Subject
   aux1=mms_get_subject_from_file(file);
   if(aux1!=NULL)
   {
        mms_pdu[k++]= 0x96;

        for(int i=0;aux1[i]!=0;i++)
        {
             mms_pdu[k++]=aux1[i];
        }
        free(aux1);

        mms_pdu[k++]='\0';
   }

   // message class;

   mms_pdu[k++]=0x8A;
   mms_pdu[k++]=0x80;


    //message ID
    mms_pdu[k++] = 0x8B;
    strcpy(aux,file.c_str());
    for(i=0;i<file.Length();i++)
    {
      mms_pdu[k++] = aux[i];
    }

    mms_pdu[k++]=0x00;

   // to implement it when I know how
   // date
   mms_pdu[k++]=0x85;
   mms_pdu[k++]=0x04;
   mms_pdu[k++]=0x48;
   mms_pdu[k++]=0xc5;
   mms_pdu[k++]=0x23;
   mms_pdu[k++]=0x5b;




   // delivery report
   mms_pdu[k++]=0x86;
   mms_pdu[k++]=mms_get_delivery_report(file);

   //content type
   f=fopen(file.c_str(),"rb");
   fseek(f,0,2);
   len=ftell(f);
   fseek(f,0,0);
   fread(buf,1,len,f);
   fclose(f);

   aux1=get_mms_buf(buf,&len);

    if(aux1!=NULL)
    {
      for(i=0;i<len;i++)
       {
         mms_pdu[k++]=aux1[i];
       }
    }
    //k--;
   dimension=k-dimension;



   update_dimension(mms_pdu,dimension);



   g=fopen("send_mms","wb");

   fwrite(mms_pdu,k,1,g);

   fclose(g);

   log("numar conexiune");
   log(nr_con);
   log("numar conexiuni");
   log(Form1->ServerSocket2->Socket->ActiveConnections);
   i=Form1->ServerSocket2->Socket->Connections[nr_con]->SendBuf(mms_pdu,k);

   log("send bytes on 8002");
   log(i);


}

void __fastcall TForm1::ServerSocket2ClientRead(TObject *Sender,TCustomWinSocket *Socket)
{
log("client read on 8002");
  int i;
  char buffer[110000];
  int nr_con;
  String ip,tel_from,file;

  log("numar conexiune");
  nr_con=get_connection8002(Socket);
  log(nr_con);

  i=Form1->ServerSocket2->Socket->Connections[nr_con]->ReceiveBuf(buffer,sizeof(buffer));
  log("Client read bytes");
  log(i);
  ip=mms_get_message_to_send(buffer,&tel_from,&file,i);
  if(ip!="")
  {
     send_mms_buf(nr_con,ip,tel_from,file,Socket);
  }
  else
  {
    Socket->Close();
  }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ServerSocket2ClientError(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
ErrorCode=0;
   log("Server Error");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ServerSocket2ClientConnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
   add_socket8002(Socket,number_of_connections8002);
       number_of_connections8002++;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ServerSocket2ClientDisconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
   int i;

   String ip;

   delete_socket8002(Socket);
   number_of_connections8002--;

   i=Form1->ListBox5->Items->IndexOf(Socket->RemoteAddress);

   if(Form1->ListBox1->Count<=i)
   {
     Socket->Close();
     return;
   }

   if (Form1->CheckBox8->Checked==true)
   {
     remove(Form1->ListBox1->Items->Strings[i].c_str());
     Form1->ListBox1->Items->Delete(i);
     Form1->ListBox4->Items->Delete(i);
     Form1->ListBox5->Items->Delete(i);
     Form1->ListBox7->Items->Delete(i);
   }

}
//---------------------------------------------------------------------------

char *get_mms_buf(unsigned char *buf,int *length)
{
     int j=-1,i;

     for(i=0;i<*length;i++)
     {
       if(buf[i]==0x84)
       {
         j=i;
         break;
       }
     }
     if(j== -1)
     {
        log(" Error , mms buffer corrupt");
        return NULL;
     }
     else
     {
       *length-=j;
       for(i=0;i<*length;i++)
       {
         buf[i]=buf[i+j];
       }
       return buf;
     }


}

void update_dimension(char *a,int dimension)
{
   char aux[10];
   char *con_len;
   String s;


   con_len=strstr(a,"Content-Length: ");

   con_len=con_len+16;

   s=dimension;

   strcpy(aux,s.c_str());

   for(int i=0;aux[i]!=0;i++)
   {
      con_len[i]=aux[i];
   }
}
void __fastcall TForm1::CheckBox3Click(TObject *Sender)
{
bool var;

     var = CheckBox3->Checked;
     CheckBox2->Checked = var;
     CheckBox1->Checked = var;

CheckBox1->OnClick(this);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox2Click(TObject *Sender)
{
bool var;

     var = CheckBox2->Checked;
     CheckBox3->Checked = var;
     CheckBox1->Checked = var;

CheckBox1->OnClick(this);}
//---------------------------------------------------------------------------


void __fastcall TForm1::ListBox2Click(TObject *Sender)
{
ListBox3->ItemIndex = ListBox2->ItemIndex;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ListBox3Click(TObject *Sender)
{
     ListBox2->ItemIndex = ListBox3->ItemIndex;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{

   FILE *f;
   int i;

  ListBox2->Items->Delete(ListBox2->ItemIndex);
  ListBox3->Items->Delete(ListBox3->ItemIndex);

  if(ListBox2->Count>0)
  {
     ListBox2->ItemIndex = 0;
     ListBox3->ItemIndex = 0;

     f=fopen("ip_phone.cfg","wb");

     if( f == NULL )
     {
       log(" ip_config can't be created");
     }
     else
     {
       for(i=0;i<Form1->ListBox2->Count;i++)
       {
         fprintf(f,"%s\n",ListBox2->Items->Strings[i].c_str());
         fprintf(f,"%s\n",ListBox3->Items->Strings[i].c_str());
       }
       log("delete ip-phone ");
       fclose(f);
     }
  }

  if (ListBox2->Count == 0)
  {
     Button3->Enabled=false;
  }

}
//---------------------------------------------------------------------------

void send_finsih(int con)
{
 FILE *f;
 char buf[2048];
 int length;

 log(" finish sending");

 f=fopen("finish_sending.txt","r");
 fseek(f,0,2);
 length=ftell(f);
 fseek(f,0,0);

 fread(buf,1,length,f);
 fclose(f);

 Form1->ServerSocket1->Socket->Connections[con]->SendBuf(buf,length);

 Sleep(500);
 Form1->ServerSocket1->Socket->Connections[con]->Disconnect(Form1->ServerSocket1->Socket->Connections[con]->SocketHandle);

}

bool is_first_receive(TCustomWinSocket *Socket)
{
    _socket_list *s;
   char aux;
   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
     log("error ,connection not available");
     return FALSE;
   }
   else
   {
     return s->is_first;
   }
}

void set_first_receive(TCustomWinSocket *Socket)
{
          _socket_list *s;
   char aux;
   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
     log("error ,connection not available");
     return ;
   }
   else
   {
     s->is_first=FALSE;
   }
}

void set_action(unsigned char *buffer,int size,TCustomWinSocket *Socket)
{
   _socket_list *s;
   int i;

   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
     log("error ,connection not available");
     return ;
   }
   else
   {
     for (i=0;i<size-1;i++)
     {
       if((buffer[i]==0x8c)&&(buffer[i+1]==0x80))
       {
         s->action= 0;
         log (" action set 0 received finished");
         break;
       }
       else if(( buffer[i]==0x8c) &&(buffer[i+1]==0x83))
       {
         s->action= 1;
         log (" action set 1 ... send finish ");
         break;
       }
     }
   }

}

char get_action(TCustomWinSocket *Socket)
{

   _socket_list *s;

   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
     log("error ,connection not available");
     return 2;
   }
   else
   {
      log ("action");
      log(s->action);
      return s->action;

   }
}

void add_to_process_queue(char type,char *phonenumber,char *ip)
{
 _process_queue * p;

 p=(_process_queue*)malloc(sizeof(_process_queue));

 p->type=type;
 strcpy(p->phone_number,phonenumber);
 strcpy(p->ip,ip);
 p->status=0;
 p->next= NULL;

 if(process_queue == NULL)
 {
   process_queue = p;
 }
 else
 {
   _process_queue *q;
   q=process_queue;

   while(q->next!=NULL)
   {
     q=q->next;
   }
   q->next=p;


 }

}

void delete_from_process_queue()
{
  _process_queue *p;

  p=process_queue;
  process_queue=process_queue->next;

  free(p);

}

void decode_delivery(char *filename,char *ip)
{
  FILE *f;
  unsigned char buffer[100000];
  char phonenumber[40];
  long size;
  bool delivery = false;
  int i;

  f=fopen(filename,"rb");

  if(f==NULL)
  {
     log("Error at opening file ");
     log(filename);
     return ;
  }
  else
  {
    char *tmp=NULL;

    fseek(f,0,2);
    size=ftell(f);
    fseek(f,0,0);

    fread(buffer,size,1,f);
    fclose(f);

    for( i=0;i<size-1;i++)
    {
      if ((buffer[i]==0x86)&&((buffer[i+1]==0x80)||(buffer[i+1]==0x81)))
      {
        if( buffer[i+1] == 0x80 )
        {
          delivery = TRUE;
        }
        break;
      }
    }

    for(i=0;i<(size-4);i++)
    {
      if((buffer[i]=='/')&&
         (buffer[i+1]=='T')&&
         (buffer[i+2]=='Y')&&
         (buffer[i+3]=='P')&&
         (buffer[i+4]=='E')&&
         (buffer[i+5]=='='))
      {
        tmp = buffer+i;
        break;
      }
    }

    if(tmp == NULL)
    {
      phonenumber[0]='\0';

    }
    else
    {
      *tmp='\0';

       bool found = false;

       for(unsigned char *i=tmp;(tmp-i)<20;i--)
       {
        if(*i==0x97)
        {
                strcpy(phonenumber,i+1);
                found = true;
                break;
        }

       }
       if(found != true)
       {
         phonenumber[0]='\0';
       }
    }

    if( delivery == TRUE)
    {
      add_to_process_queue(0,phonenumber,ip);
    }
  }
}

void proces_queue()
{

  if(Form1->RadioButton2->Checked == true)
  {
        Sleep(2000);
  }
  else
  {
     Sleep(1000);
  }
 
  if (process_queue)
  {
    if(process_queue->type==0)
    {
      send_delivery_report(process_queue);
    }
    else if(process_queue->type ==1)
    {
      send_read_report(process_queue);
    }
  }

}

void update_to_succes(int x)
{
   _process_queue *q;

   q=process_queue;

   char ip[16];
   String phone_number;
   int j;

   strcpy(ip,Form1->ListBox4->Items->Strings[x].c_str());
   j=Form1->ListBox2->Items->IndexOf(ip);
   if(j>0)
   {
        phone_number=Form1->ListBox3->Items->Strings[j];
   }
   else
   {
     log("Error ... ip not found in the list ");
     return;
   }
   while(q!=NULL)
   {
    if(strcmp(q->phone_number,phone_number.c_str())==0)
    {
      q->status=1;
      return;
    }
    q=q->next;
   }
}

void send_delivery_report(_process_queue *p)
{
 unsigned char buffer[1000];

    int i;
    int k=0;

    char aux[100];


   buffer[k++]=(char)Form1->Edit2->Text.ToInt();

   buffer[k++]=(char)Form1->Edit3->Text.ToInt();
   k++;

   strcpy(aux,Form1->Edit5->Text.c_str());

   for(int i=0;aux[i]!=0;i++)
   {
     buffer[k++]=aux[i];
   }

   buffer[k++]='\0';

   strcpy(aux,"Encoding-version");

   for(int i=0;aux[i]!=0;i++)
   {
     buffer[k++]=aux[i];
   }

   buffer[k++]='\0';

   strcpy(aux,Form1->Edit9->Text.c_str());

   for(int i=0;aux[i]!=0;i++)
   {
     buffer[k++]=aux[i];
   }

   buffer[k++]='\0';
   buffer[k++]=0xaf;
   buffer[k++]=0x84;
   buffer[2]=k-3;

   // type

   buffer[k++]=0x8C;
   buffer[k++]=0x86;

   // mms version

   buffer[k++]=0x8D;
   buffer[k++]=0x90;

   //Mesage ID
   buffer[k++]=0x8B;

   // default value message;
   strcpy(aux,"message");

   for (unsigned i=0 ;i<strlen(aux);i++)
   {
      buffer[k++]=aux[i];
   }

   buffer[k++]=0x00;

   //to
   buffer[k++]=0x97;
   strcpy(aux,p->phone_number);
   strcat(aux,"/TYPE+PLMN");

   for(unsigned i=0;i<strlen(aux);i++)
   buffer[k++]=aux[i];

   buffer[k++]=0x00;

   //Date
   buffer[k++]=0x85;
   buffer[k++]=0x04;
   buffer[k++]=0x48;
   buffer[k++]=0xc5;
   buffer[k++]=0x23;
   buffer[k++]=0x5b;

   //message status

   buffer[k++]=0x95;

   if(p->status==0)
   {
     buffer[k++]=0x82;

   }
   else if (p->status==1)
   {
     buffer[k++]=0x81;
   }


  if(sms_notification == true)
  {
    send_sms_notifications(buffer,k,p->ip);
  }
  else
  {
        Form1->UdpSocket1->RemoteHost=p->ip;

        if(Form1->UdpSocket1->Active)
          {
             log("active");
          }

          Form1->UdpSocket1->Active=true;

          Form1->UdpSocket1->Connect();

        i=Form1->UdpSocket1->SendBuf(buffer,k);
          log("delivery report size");
          log(i);

          Form1->UdpSocket1->Disconnect();

          Form1->UdpSocket1->Active=false;
  }

  delete_from_process_queue();
  
  if( process_queue == NULL)
  {
   Form1->Timer2->Enabled=false;
  }

}

void send_read_report(_process_queue *p)
{
  unsigned char buffer[1000];
  int k;

  if(sms_notification == true)
  {
        send_sms_notifications(buffer,k,p->ip);
  }
  else
  {
  }
}
void __fastcall TForm1::Timer2Timer(TObject *Sender)
{
 if((process_queue!=NULL)&&(Form1->UdpSocket1->Connected==false))
 {
    proces_queue();
 }
}
//---------------------------------------------------------------------------


void __fastcall TForm1::CheckBox7Click(TObject *Sender)
{
 if (CheckBox7->Checked==true)
 {
  ListBox6->Enabled=true;
  ListBox6->Color=clWindow;
 }
 else
 {
  ListBox6->Enabled=false;
  ListBox6->Color=clCream;
 }
}
//---------------------------------------------------------------------------

bool do_skip(unsigned char *buffer,int size)
{
   if(((buffer[size-34]==0x8c) && (buffer[size-33]==0x83))||((buffer[size-30]==0x8c)&&(buffer[size-29]==0x85)))
    return true;
   else
    return false;
}

String mms_get_message_to_send(char *buffer,String *tel_from,String *file,int size)
{
  char *aux=NULL;
  int k=0;
  String s;
  String ip="";
  int y;

  for(int i=0;i<size;i++)
  {
    if(buffer[i]==' ')
    {
      if (k==1)
      {
        buffer[i]='\0';
        break;
      }
      else
      {
        aux=buffer+i+2;
        k++;
      }
    }
  }

  s= aux;
  if(s == "")
  {
    return ip;
  }
  *file=*file+s;
  y=Form1->ListBox1->Items->IndexOf(s);
  if(y==-1)
  {
    return ip;
  }
  ip=Form1->ListBox5->Items->Strings[y];
  *tel_from=Form1->ListBox3->Items->Strings[Form1->ListBox2->Items->IndexOf(Form1->ListBox4->Items->Strings[y]) ];

  return ip;
}

int get_timing(char *filename)
{
   return Form1->default_timing;
}

void update_timings_for_messages()
{
 bool exists_for_sending = false;

 for(int i=0;i<Form1->ListBox7->Count;i++)
 {
   int x=Form1->ListBox7->Items->Strings[i].ToInt();

   if(x >0)
   {
     x--;
     Form1->ListBox7->Items->Delete(i);
     Form1->ListBox7->Items->Insert(i,x);
     exists_for_sending = true;
   }
 }
}

bool get_zero_time()
{
  bool rezult;

  for(int i=0;i<Form1->ListBox7->Count;i++)
  {
    int x;

     x= Form1->ListBox7->Items->Strings[i].ToInt();
     if(x==0)
     {
       return true;
     }
  }

  return false;
}

int get_index_for_zero_time()
{
  for(int j=0;j<Form1->ListBox7->Count;j++)
   {
     int x= Form1->ListBox7->Items->Strings[j].ToInt();
     if(x==0)
     {
       return j;
     }
   }
  return -1;
}

bool set_message_worth_sending(bool value,TCustomWinSocket *Socket)
{
   _socket_list *s;

   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
     log("error ,connection not available");
     return false;
   }
   else
   {
      s->message_worth_sending = value;
      return true;
   }

}

bool get_message_worth_sending(TCustomWinSocket *Socket)
{
    _socket_list *s;

   s=socket_list;

   while ((s!=NULL)&&(s->Socket!=Socket))
   {
     s=s->next;
   }

   if(s==NULL)
   {
     log("error ,connection not available");
     return false;
   }
   else
   {
      log ("message worth sending ");
      log((char)s->message_worth_sending);
      return s->message_worth_sending;
   }

}


void __fastcall TForm1::RadioButton1Click(TObject *Sender)
{
   sms_notification = true;

   RadioButton2->Checked = false;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::RadioButton2Click(TObject *Sender)
{
        sms_notification = false;

        RadioButton1->Checked = false;

}
//---------------------------------------------------------------------------

bool send_sms_notifications(char *buffer,int length,String ip)
{
   char buf[180];
   int left;
   char *pdu=NULL;
   unsigned char size;
   unsigned char x1,x2;
   int i;
   time_t t;

   t=clock();
   x1=t%256;
   x2=(t/256/256/256);

   if(length<=140)
   {
        size=create_pdu(buf,buffer,length,0,0,false,false,x1,x2);

        pdu=transform_pdu(buf,size);

        add_to_notification_list(strlen(pdu),ip,Form1->Edit31->Text,pdu);

        free(pdu);

        if((Form1->Timer3->Enabled == false)&&(Form1->ClientSocket1->Active ==false))
        {
          Form1->Timer3->Enabled = true;
        }

        return true;
   }
   else
   {
      left = length;
      int k=1;
      int nr_segmente;
      int adaos;

      int j=0;


      adaos = length%127;
      if(adaos>0)
      {
        adaos = 1;
      }

      nr_segmente = length/127 +adaos;

      while (left>0)
      {
         if(left<=140)
         {
            size=create_pdu(buf,buffer+(length-left),left,k,nr_segmente,false,true,x1,x2);
            left = 0;
         }
         else
         {
           size=create_pdu(buf,buffer+(length-left),127,k,nr_segmente,true,true,x1,x2);
           left = left-127;
         }
         k++;

         pdu=transform_pdu(buf,size);

         add_to_notification_list(strlen(pdu),ip,Form1->Edit31->Text,pdu);
         free(pdu);
         pdu=NULL;

         if(left == 0)
         {
           if((Form1->Timer3->Enabled == false)&&(Form1->ClientSocket1->Active ==false))
           {
               Form1->Timer3->Enabled = true;
           }

           return true;
         }
      }
   }
}

char *mms_get_subject_from_file(String file)
{
  FILE *f;
  unsigned char buffer[2000];
  int i;
  char *aux,*ret;
  long size;

  f=fopen(file.c_str(),"rb");
  size=fread(buffer,1,2000,f);
  fclose(f);

  for(i=0;i<size;i++)
  {
    if(buffer[i] == 0x96)
    {
      aux=buffer+i+1;
      break;
    }
    else if(buffer[i]==0x84)
    {
      return NULL;
    }
  }

  ret = strdup(aux);

  return ret;
}

unsigned char mms_get_read_report(String file)
{
   FILE *f;
   unsigned char buffer[2000];
   int size,i;
   unsigned char ret;

   f=fopen(file.c_str(),"rb");
   size = fread(buffer,1,2000,f);
   fclose(f);

   for(i=0;i<size;i++)
   {
      if(buffer[i]==0x90)
      {
        ret=buffer[i+1];
        break;
      }
   }

   if(Form1->CheckBox6->Checked != true)
   {
     ret = 0x81;
   }

   return ret;
}


unsigned char mms_get_delivery_report(String file)
{
   FILE *f;
   unsigned char buffer[2000];
   int size,i;
   unsigned char ret;

   f=fopen(file.c_str(),"rb");
   size = fread(buffer,1,2000,f);
   fclose(f);

   for(i=0;i<size;i++)
   {
      if(buffer[i]==0x86)
      {
        ret=buffer[i+1];
        break;
      }
   }

   if(Form1->CheckBox5->Checked != true)
   {
     ret = 0x81;
   }

   return ret;
}
void __fastcall TForm1::Timer3Timer(TObject *Sender)
{
     if(Form1->notification_list != NULL)
     {
       if(ClientSocket1->Active == false)
       {
         Form1->ClientSocket1->Address = Form1->notification_list->ip;
         Form1->ClientSocket1->Port = Form1->notification_list->port;
         Form1->ClientSocket1->Active = true;
         Timer3->Enabled = false;

       }
     }
     else
     {
       Timer3->Enabled = false;
     }
}
//---------------------------------------------------------------------------
static int max =0;

void __fastcall TForm1::ClientSocket1Connect(TObject *Sender,
      TCustomWinSocket *Socket)
{
  log ("Client connected on port 2950");
  int i=-1;

  if(Form1->notification_list != NULL)
  {
    i=Socket->SendBuf(Form1->notification_list->pdu,Form1->notification_list->size);

    if((i!= Form1->notification_list->size)&&(max <10))
    {
       Socket->Close();
       max++;
    }
    else
    {
      max =0;
      delete_from_notification_list(Form1->notification_list);
      Socket->Close();
    }
  }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ClientSocket1Error(TObject *Sender,
      TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode)
{
 ErrorCode = 0;
 log("Client Socket error on port 2950");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ClientSocket1Disconnect(TObject *Sender,
      TCustomWinSocket *Socket)
{
       ClientSocket1->Active = false;
       Form1->Timer3->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox10Click(TObject *Sender)
{
 if (CheckBox10->Checked == true)
 {
    Edit32->Enabled = true;
 }
 else
 {
    Edit32->Enabled = false;
 }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Edit32Change(TObject *Sender)
{
  Form1->default_timing = Edit32->Text.ToInt();
}
//---------------------------------------------------------------------------


int mms_get_to_destination(unsigned char *buffer,int size,int offset,char *to)
{
   int i,ret=-1;
   char *aux=NULL;
   bool found= false;

   for(i=offset;i<size;i++)
   {
      if(buffer[i]==0x97)
      {
         aux=buffer+i+1;
         found = true;
      }
      if((buffer[i]=='/')&&(found == true))
      {
        buffer[i]='\0';
        ret =i;
        strcpy(to,aux);
        break;
      }
      else if ((buffer[i]=='/')&&(found==false))
      {
        ret =-1;
      }
   }
   return ret;
}


int mms_get_bcc_destination(unsigned char *buffer,int size,int offset,char *bcc)
{
   int i,ret=-1;
   char *aux=NULL;
   bool found= false;

   for(i=offset;i<size;i++)
   {
      if((buffer[i]==0x81)&&(i>0)&&(buffer[i-1]==0))
      {
         aux=buffer+i+1;
         found = true;
      }
      if((buffer[i]=='/')&&(found == true))
      {
        buffer[i]='\0';
        ret =i;
        strcpy(bcc,aux);
        break;
      }
      else if ((buffer[i]=='/')&&(found==false))
      {
        ret =-1;
      }
   }
   return ret;
}

int mms_get_cc_destination(unsigned char *buffer,int size,int offset,char *cc)
{
   int i,ret=-1;
   char *aux=NULL;
   bool found= false;

   for(i=offset;i<size;i++)
   {
      if((buffer[i]==0x82)&&(i>0)&&(buffer[i-1]==0))
      {
         aux=buffer+i+1;
         found = true;
      }
      if((buffer[i]=='/')&&(found == true))
      {
        buffer[i]='\0';
        ret =i;
        strcpy(cc,aux);
        break;
      }
      else if ((buffer[i]=='/')&&(found==false))
      {
        ret =-1;
      }
   }
   return ret;
}

void mms_add_cc_to_mms(String file,unsigned char *buf,int *length)
{
  FILE *f;
  int size;
  int offset=0;
  int i,ret;
  char *aux;
  char buffer[2000];
  int k =0;


  f=fopen(file.c_str(),"rb");
  size=fread(buf,1,2000,f);
  fclose(f);


  do
  {
     ret =-1;
     for(i=offset;i<size;i++)
     {
       if((buf[i]==0x82)&&(i>0)&&(buf[i-1]==0))
       {
          aux=buf+i;
          for(int j=0;aux[j]!=0;j++)
          {
              buffer[k++]=aux[j];
              i++;
          }
          buffer[k++]=0x00;
          ret=0;
          offset=i;
          break;
       }
     }

  }while(ret!=-1);

  *length = k;

  for(i=0;i<k;i++)
  {
     buf[i]=buffer[i];
  }
}


void mms_add_bcc_to_mms(String file,unsigned char *buf,int *length)
{
  FILE *f;
  int size;
  int offset=0;
  int i,ret;
  char *aux;
  char buffer[2000];
  int k =0;


  f=fopen(file.c_str(),"rb");
  size=fread(buf,1,2000,f);
  fclose(f);


  do
  {
     ret =-1;
     for(i=offset;i<size;i++)
     {
       if((buf[i]==0x81)&&(i>0)&&(buf[i-1]==0))
       {
          aux=buf+i;
          for(int j=0;aux[j]!=0;j++)
          {
              buffer[k++]=aux[j];
              i++;
          }
          buffer[k++]=0x00;
          ret=0;
          offset=i;
          break;
       }
     }

  }while(ret!=-1);

  *length = k;

  for(i=0;i<k;i++)
  {
     buf[i]=buffer[i];
  }
}
