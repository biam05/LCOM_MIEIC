#include <lcom/lab3.h>
#include <lcom/lcf.h>

#include "keyboard.h"
#include "macros_kbd.h"
#include <stdbool.h>
#include <stdint.h>

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab3/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab3/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

extern uint8_t scancode[2];
extern bool two_byte;
extern int hook_id_kbd;
extern int hook_id;
extern int counter;
extern int timer_counter;

int(kbd_test_scan)() {
  int ipc_status, r;
  message msg;
  uint8_t kbd_id = 1;
  int irq_set = BIT(kbd_id);
  counter = 0;

  kbd_subscribe_int(&kbd_id);

  while(scancode[0] != ESC_BREAK_CODE) {
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
       continue;
      }
    if (is_ipc_notify(ipc_status)) { 
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set) { 
            kbc_ih();
            if(!two_byte && scancode[0] == TWO_BYTE_CODE)
              kbd_print_scancode(breakcode_or_makecode(scancode[1]), 2, scancode);
            else if(!two_byte)
            {
              kbd_print_scancode(breakcode_or_makecode(scancode[0]), 1, scancode);
            }
          }
          break;
        default:
          break;
        }
    } else { 
    }
  }

  kbd_unsubscribe_int();

  kbd_print_no_sysinb(counter);

  return 0;
}

int(kbd_test_poll)() {
  counter = 0;
  uint8_t status, scancode_byte;
  while(scancode[0] != ESC_BREAK_CODE){

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
      if ((!two_byte) && (scancode[0] == TWO_BYTE_CODE)) {
        kbd_print_scancode(breakcode_or_makecode(scancode[1]), 2, scancode);
      }
      else if (!two_byte) {
        kbd_print_scancode(breakcode_or_makecode(scancode[0]), 1, scancode);
      }
      tickdelay(micros_to_ticks(DELAY_US));
    }
  }

  uint8_t var1;
  sys_outb(STAT_REG, READ_CMD);
  util_sys_inb(OUT_BUF, &var1);
  counter++;
  var1 = (var1 | BIT(0));
  sys_outb(STAT_REG, WRITE_CMD);
  sys_outb(IN_BUF, var1);

  kbd_print_no_sysinb(counter);

  return 0;
}

int(kbd_test_timed_scan)(uint8_t n) {
  int ipc_status, r;
  message msg;
  uint8_t kbd_id = 1;
  uint8_t timer_id = 0;
  int irq_set_kbd = BIT(kbd_id);
  int irq_set_timer = BIT(timer_id);
  counter = 0;

  kbd_subscribe_int(&kbd_id);
  timer_subscribe_int(&timer_id);

  while(scancode[0] != ESC_BREAK_CODE && n) {
    if ( (r = driver_receive(ANY, &msg, &ipc_status)) != 0 ) { 
      printf("driver_receive failed with: %d", r);
       continue;
      }
    if (is_ipc_notify(ipc_status)) { 
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:
          if (msg.m_notify.interrupts & irq_set_kbd) { 
            kbc_ih();
            if(!two_byte && scancode[0] == TWO_BYTE_CODE)
              kbd_print_scancode(breakcode_or_makecode(scancode[1]), 2, scancode);
            else if(!two_byte)
            {
              kbd_print_scancode(breakcode_or_makecode(scancode[0]), 1, scancode);
            }
            timer_counter = 0;
            
          }
          if (msg.m_notify.interrupts & irq_set_timer) { 
            timer_int_handler();
            if(timer_counter % 60 == 0)
              n--;         
          }
          break;
        default:
          break;
        }
    } else { 
    }
  }

  kbd_unsubscribe_int();
  timer_unsubscribe_int();

  return 0;
}
