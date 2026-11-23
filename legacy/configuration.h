//---------------------------------------------------------------------------

#ifndef configurationH
#define configurationH
#include <vcl.h>
void create_default_config();

void load_settings();

void load_phone_numbers();

void Enable(bool var);

void initialize_configuration();

char *find_string(char *buffer,String s);
#endif
