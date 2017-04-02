#ifndef btlog_HEADER
#define btlog_HEADER

void btlog_continuous (const char * filename);

void btlog (const char * format, ...);

void btlog_error (const char * format, ...);

void write_btlog (const char * filename);

#endif