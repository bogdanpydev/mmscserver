//---------------------------------------------------------------------------


#pragma hdrstop

#include "send_notif.h"
#include <alloc.h>
#include "main.h"


void add_to_notification_list(int size,String ip,String port,char *pdu)
{
   _notif_list_t *p,*aux;

   p=(_notif_list_t*)malloc(sizeof(_notif_list_t));

   p->size = size;
   p->ip =strdup(ip.c_str());
   p->port = port.ToInt();
   p->pdu = strdup(pdu);
   p->next = NULL;

   if(Form1->notification_list == NULL)
   {
     Form1->notification_list = p;
   }
   else
   {
     aux = Form1->notification_list;

     while(aux->next != NULL)
     {
       aux=aux->next;
     }

     aux->next = p;
   }
}

_notif_list_t *get_from_notification_list(String ip)
{
   _notif_list_t *ret = NULL;

   ret = Form1->notification_list;

   while (ret!=NULL)
   {
     if(strcmp(ret->ip,ip.c_str())!=0)
     {
        return ret;
     }
     else
     {
       ret = ret->next;
     }
   }

   return NULL;
}

void delete_from_notification_list(_notif_list_t *p)
{
   _notif_list_t *aux,*del,*prev=NULL;

   aux = Form1->notification_list;

   if(p==aux)
   {
     Form1->notification_list = aux->next;
     free(aux->ip);
     free(aux->pdu);
     free(aux);
   }
   else
   {
      while (aux!=NULL)
      {
         if(aux==p)
         {
            prev->next= aux->next;
            free(p->ip);
            free(p->pdu);
            free(p);
         }
      }
   }

}

#pragma package(smart_init)
