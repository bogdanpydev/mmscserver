//---------------------------------------------------------------------------

#ifndef send_notifH
#define send_notifH

#include "log.h"


typedef struct _notif_list_t_
{
    int size;
    char *ip;
    int port;
    char *pdu;
    struct _notif_list_t_  *next;
}_notif_list_t;

void add_to_notification_list(int size,String ip,String port,char *pdu);

_notif_list_t *get_from_notification_list(String ip);

void delete_from_notification_list(_notif_list_t *p);

#endif
