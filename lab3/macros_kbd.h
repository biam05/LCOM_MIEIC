
#ifndef _KDB_MACROS_H_
#define _KDB_MACROS_H_

#include <lcom/lcf.h>

#define KBD_IRQ                 1

#define STAT_REG                0x64
#define OUT_BUF                 0x60
#define IN_BUF                  0x60
#define READ_CMD                0x20
#define WRITE_CMD               0x60

#define PARITY                  BIT(7)
#define TIMEOUT                 BIT(6)
#define AUX                     BIT(5)
#define INH                     BIT(4)
#define A2                      BIT(3)
#define SYS                     BIT(2)
#define IBF                     BIT(1)
#define OBF                     BIT(0)

#define ESC_BREAK_CODE          0x81    //Break code of the Esc key
#define TWO_BYTE_CODE           0xe0    //To test when a code is two bytes long
#define DELAY_US                20000   //delay used in the delay function


#endif
