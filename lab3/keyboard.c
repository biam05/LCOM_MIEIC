#include <lcom/lcf.h>
#include <stdint.h>

#include "keyboard.h"
#include "macros_kbd.h"

int hook_id_kbd = 0x01;
bool two_byte = false;
uint8_t scancode[2];
int counter;

int kbd_subscribe_int(uint8_t *bit_no)
{
    *bit_no = hook_id_kbd;
    sys_irqsetpolicy(KBD_IRQ, IRQ_REENABLE | IRQ_EXCLUSIVE, &hook_id_kbd);
    return BIT(hook_id_kbd);
}

int kbd_unsubscribe_int()
{
    sys_irqrmpolicy(&hook_id_kbd);
    return 0;
}

void (kbc_ih)() {

    uint8_t status, scancode_byte;

    util_sys_inb(STAT_REG, &status);
    counter++;

    if(status & OBF)
    {
        if((status & (PARITY | TIMEOUT | AUX)) == 0)
        {
            util_sys_inb(OUT_BUF, &scancode_byte);
            counter++;

            if(two_byte)
            {
                scancode[1] = scancode_byte;
                two_byte = false;
            }
            else
            {
                scancode[0] = scancode_byte;
                if(scancode_byte == TWO_BYTE_CODE)
                    two_byte = true;
            }          
        }        
    }
}

bool breakcode_or_makecode(uint8_t scancode)
{
    uint8_t sc = (uint8_t) scancode;
    if(sc & BIT(7)) return false;
    else            return true;    
}

