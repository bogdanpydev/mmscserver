//---------------------------------------------------------------------------

#ifndef sms_pduH
#define sms_pduH

char add_sms_sca_address(char *buffer);

char add_sms_time_stamp(char *buffer,unsigned char position);

char add_sms_coding_scheme(char *buffer,unsigned char position);

char add_sms_add_buffer(char *buffer_dest,char *buffer_source,unsigned char position,unsigned char length);

unsigned char add_sms_user_data_length(unsigned char *buffer,unsigned char position,unsigned char length);

char add_sms_protocol_identifier(char *buffer,unsigned char position);

char add_sms_orignator_address(char *buffer,unsigned char position);

char add_sms_udhl(char *buffer,unsigned char position,bool concat);

char add_sms_ie_concat(char *buffer,unsigned char position,char seg_number,char number_of_segments,unsigned char x1,unsigned char x2);

char add_sms_ie_app_port(char *buffer,unsigned char position);

char add_sms_mti(char *buffer,unsigned char position,char buffer_length,bool segment);

char create_pdu(char *buffer,char *buffer_source,char length,char nr_segment,char number_of_segments,bool segment,bool mti,unsigned char x1,unsigned char x2);

unsigned char *transform_pdu(unsigned char *buffer,unsigned char length);





//---------------------------------------------------------------------------
#endif
 