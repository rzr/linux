#ifndef _BUFFALO_UART_H_
#define _BUFFALO_UART_H_

void BuffaloInitUart(void);
void BuffaloConsoleOutput(const unsigned char *buff);
void BuffaloMiconOutput(const unsigned char *buff, int len);
int BuffaloMiconInput(unsigned char *ch, unsigned tmout);
void BuffaloConsoleOutput(const unsigned char *buff);
int BuffaloConsoleInput(unsigned char *ch, unsigned tmout);
void BuffaloMiconOutput(const unsigned char *buff, int len);
int BuffaloMiconInput(unsigned char *ch, unsigned tmout);

#endif
