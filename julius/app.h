#include <julius/juliuslib.h>

#include "config.h"
#ifdef CHARACTER_CONVERSION
#include "charconv.h"
#endif

/* recogloop.c */
void main_recognition_stream_loop(Recog **recog, int num);

/* module.c */
int module_send(int sd, char *fmt, ...);
void module_add_option();
boolean is_module_mode();
void module_setup(Recog *recog, void *data);
void module_recognition_stream_loop(Recog **recog, int num);

/* output_module.c */
void decode_output_selection(char *str);
void send_gram_info(Recog *recog);
void setup_output_msock(Recog *recog, void *data);

/* output_stdout.c */
void print_all_gram(Recog *recog);
void setup_output_tty(Recog *recog, void *data);

/* record.c */
void record_add_option();
void record_setup(Recog *recog, void *data);
