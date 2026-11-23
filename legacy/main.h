//---------------------------------------------------------------------------

#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ScktComp.hpp>
#include <Sockets.hpp>
#include <ExtCtrls.hpp>
#include <TabNotBk.hpp>
#include "send_notif.h"
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TTabbedNotebook *TabbedNotebook1;
        TServerSocket *ServerSocket1;
        TServerSocket *ServerSocket2;
        TTimer *Timer1;
        TGroupBox *GroupBox1;
        TLabel *Label1;
        TLabel *Label2;
        TLabel *Label4;
        TLabel *Label10;
        TLabel *Label11;
        TLabel *Label12;
        TLabel *Label13;
        TComboBox *ComboBox1;
        TComboBox *ComboBox2;
        TEdit *Edit1;
        TEdit *Edit6;
        TComboBox *ComboBox3;
        TEdit *Edit7;
        TEdit *Edit8;
        TGroupBox *GroupBox2;
        TLabel *Label6;
        TLabel *Label7;
        TLabel *Label8;
        TLabel *Label9;
        TEdit *Edit2;
        TEdit *Edit3;
        TEdit *Edit5;
        TListBox *ListBox1;
        TLabel *Label3;
        TLabel *Label5;
        TCheckBox *CheckBox1;
        TButton *Button1;
        TLabel *Label14;
        TEdit *Edit9;
        TLabel *Label15;
        TEdit *Edit10;
        TEdit *Edit4;
        TListBox *ListBox2;
        TListBox *ListBox3;
        TLabel *Label16;
        TLabel *Label17;
        TButton *Button2;
        TButton *Button3;
        TListBox *ListBox4;
        TLabel *Label18;
        TListBox *ListBox5;
        TLabel *Label19;
        TGroupBox *GroupBox3;
        TLabel *Label20;
        TLabel *Label21;
        TLabel *Label22;
        TLabel *Label23;
        TLabel *Label24;
        TLabel *Label25;
        TLabel *Label26;
        TLabel *Label27;
        TLabel *Label28;
        TLabel *Label29;
        TLabel *Label30;
        TLabel *Label31;
        TLabel *Label32;
        TLabel *Label33;
        TEdit *Edit11;
        TEdit *Edit12;
        TComboBox *ComboBox4;
        TEdit *Edit13;
        TEdit *Edit14;
        TEdit *Edit15;
        TEdit *Edit16;
        TEdit *Edit17;
        TEdit *Edit18;
        TEdit *Edit19;
        TComboBox *ComboBox5;
        TComboBox *ComboBox6;
        TComboBox *ComboBox7;
        TComboBox *ComboBox8;
        TGroupBox *GroupBox4;
        TGroupBox *GroupBox5;
        TGroupBox *GroupBox6;
        TLabel *Label34;
        TEdit *Edit20;
        TLabel *Label35;
        TLabel *Label36;
        TEdit *Edit21;
        TComboBox *ComboBox9;
        TLabel *Label37;
        TComboBox *ComboBox10;
        TCheckBox *CheckBox2;
        TLabel *Label38;
        TCheckBox *CheckBox3;
        TLabel *Label39;
        TLabel *Label40;
        TEdit *Edit22;
        TLabel *Label41;
        TComboBox *ComboBox11;
        TLabel *Label42;
        TEdit *Edit23;
        TLabel *Label43;
        TEdit *Edit24;
        TLabel *Label44;
        TEdit *Edit25;
        TLabel *Label45;
        TComboBox *ComboBox12;
        TLabel *Label46;
        TEdit *Edit26;
        TLabel *Label47;
        TLabel *Label48;
        TEdit *Edit28;
        TComboBox *ComboBox13;
        TLabel *Label49;
        TEdit *Edit27;
        TLabel *Label50;
        TEdit *Edit29;
        TLabel *Label51;
        TEdit *Edit30;
        TLabel *Label52;
        TGroupBox *GroupBox7;
        TCheckBox *CheckBox4;
        TTimer *Timer2;
        TCheckBox *CheckBox5;
        TCheckBox *CheckBox6;
        TListBox *ListBox6;
        TCheckBox *CheckBox7;
        TListBox *ListBox7;
        TCheckBox *CheckBox8;
        TComboBox *ComboBox14;
        TUdpSocket *UdpSocket1;
        TGroupBox *GroupBox8;
        TRadioButton *RadioButton1;
        TRadioButton *RadioButton2;
        TCheckBox *CheckBox9;
        TEdit *Edit31;
        TClientSocket *ClientSocket1;
        TTimer *Timer3;
        TCheckBox *CheckBox10;
        TEdit *Edit32;
        void __fastcall FormCreate(TObject *Sender);
        void __fastcall CheckBox1Click(TObject *Sender);
        void __fastcall ServerSocket1ClientError(TObject *Sender,
          TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);
        void __fastcall ServerSocket1ClientConnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall ServerSocket1ClientDisconnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall ServerSocket1ClientRead(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall FormShow(TObject *Sender);
        void __fastcall ServerSocket2ClientRead(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall ServerSocket2ClientError(TObject *Sender,
          TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);
        void __fastcall ServerSocket2ClientConnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall ServerSocket2ClientDisconnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall CheckBox3Click(TObject *Sender);
        void __fastcall CheckBox2Click(TObject *Sender);
        void __fastcall ListBox2Click(TObject *Sender);
        void __fastcall ListBox3Click(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall Timer2Timer(TObject *Sender);
        void __fastcall CheckBox7Click(TObject *Sender);
        void __fastcall RadioButton1Click(TObject *Sender);
        void __fastcall RadioButton2Click(TObject *Sender);
        void __fastcall Timer3Timer(TObject *Sender);
        void __fastcall ClientSocket1Connect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall ClientSocket1Error(TObject *Sender,
          TCustomWinSocket *Socket, TErrorEvent ErrorEvent,
          int &ErrorCode);
        void __fastcall ClientSocket1Disconnect(TObject *Sender,
          TCustomWinSocket *Socket);
        void __fastcall CheckBox10Click(TObject *Sender);
        void __fastcall Edit32Change(TObject *Sender);
        
private:	// User declarations
public:		// User declarations

           bool thread_started;
           _notif_list_t *notification_list;
           int default_timing;
        __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------

typedef struct _port_t
{
        TCustomWinSocket *Socket;
        int connection_number;
        char file_name[20];
        int mms_size;
        char req_send; // 1 req ,0 send, 2 notification ,3 finish_send
        bool is_first;
        char action;   //0 send ok  1 send_finished  2 default
        bool message_worth_sending;
        struct _port_t *next;
}_socket_list;

typedef struct _process_queue_
{
      char type; // 0 -- delivery report 1-- read report
      char phone_number[40];
      char ip[16];
      char status;  // 0 - failed , 1 succes
      struct _process_queue_ *next;
} _process_queue;

void add_socket(TCustomWinSocket *Socket,int connection_number);

void delete_socket(TCustomWinSocket *Socket);

char *get_mms_message(unsigned char *buffer,int rec);

char *get_file_name(TCustomWinSocket *Socket);

int get_connection(TCustomWinSocket *Socket);

void set_mms_size(char *buffer,TCustomWinSocket *Socket);

int get_mms_size(TCustomWinSocket *Socket);

void set_filename(TCustomWinSocket *Socket);

int get_mms_buffer_size(char *buffer,int size,char *buffer2,TCustomWinSocket *Soket);

void send_ok(int con);

int decode_mms_destination(char *file);

void create_notification_header(int x);

bool send_notification(int x);

void get_from_field_mms(int x);

void get_mms_pdu_size(String file);

int get_dest_ip(int x);

void set_req_send(char *a,TCustomWinSocket *Socket);

char get_req_send(TCustomWinSocket *Socket);

void send_mms_buf(int nr_con,String ip,String ip_from,String file,TCustomWinSocket *Socket);

char *get_mms_buf(unsigned char *buf,int *length);

void update_dimension(char *a,int dimension);

int get_from_number_length(String number);

void send_finsih(int con);

bool is_first_receive(TCustomWinSocket *Socket);

void set_first_receive(TCustomWinSocket *Socket);

void set_action(unsigned char *buffer,int size,TCustomWinSocket *Socket);

char get_action(TCustomWinSocket *Socket);

void proces_queue();

void add_to_process_queue(char type,char *phonenumber,char *ip);

void delete_from_process_queue();

void decode_delivery(char *filename,char *ip);

void update_to_succes(int x);

void send_delivery_report(_process_queue *p);

void send_read_report(_process_queue *p);

int get_connection8002(TCustomWinSocket *Socket);

void delete_socket8002(TCustomWinSocket *Socket);

bool do_skip(unsigned char *buffer,int size);

String mms_get_message_to_send(char *buffer,String *tel_from,String *file,int size);

int get_timing(char *filename);

void update_timings_for_messages();

bool get_zero_time();

int get_index_for_zero_time();

bool set_message_worth_sending(bool value,TCustomWinSocket *Socket);

bool get_message_worth_sending(TCustomWinSocket *Socket);

bool send_sms_notifications(char *buffer,int length,String ip);

char *mms_get_subject_from_file(String file);

unsigned char mms_get_read_report(String file);

unsigned char mms_get_delivery_report(String file);

int mms_get_to_destination(unsigned char *buffer,int size,int offset,char *to);

int mms_get_cc_destination(unsigned char *buffer,int size,int offset,char *cc);

int mms_get_bcc_destination(unsigned char *buffer,int size,int offset,char *bcc);

void mms_add_cc_to_mms(String file,unsigned char *buf,int *length);

void mms_add_bcc_to_mms(String file,unsigned char *buf,int *length);

#endif
