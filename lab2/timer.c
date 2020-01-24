#include <lcom/lcf.h>
#include <lcom/timer.h>

#include <stdint.h>

#include "i8254.h"

int timer_counter;
int hook_id = 0x00;

int (timer_get_conf)(uint8_t timer, uint8_t *st) {
  uint32_t read_back_command;
  read_back_command = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_SEL(timer);
  sys_outb(TIMER_CTRL,read_back_command);
  if     (timer == 0) util_sys_inb(TIMER_0, st);
  else if(timer == 1) util_sys_inb(TIMER_1, st);
  else               {util_sys_inb(TIMER_2, st);}  
  return 0;
}

int (timer_display_conf)(uint8_t timer, uint8_t st, enum timer_status_field field) {
  
  union timer_status_field_val control_word;

  uint8_t counting_mode = ((BIT(1) | BIT(2) | BIT(3)) & st);
  counting_mode >>= 1;

  uint8_t initialization_mode = ((BIT(4) | BIT(5)) & st);
  initialization_mode >>= 4;

  switch (field)
  {
    case tsf_all:  

      control_word.byte = st;          
      break;
  
    case tsf_initial:    
      switch (initialization_mode)
      {
        case 0:                        
          control_word.in_mode = INVAL_val;  
          break;
        case 1:
          control_word.in_mode = LSB_only;
          break;
        case 2:
          control_word.in_mode = MSB_only;
          break;
        case 3:
          control_word.in_mode = MSB_after_LSB;
          break;      
      }
      break;
    
    case tsf_mode:
      if(counting_mode == 6)
        control_word.count_mode = 2;
      else if(counting_mode == 7)
        control_word.count_mode = 3;
      else
        control_word.count_mode = counting_mode;    
      break;

    case tsf_base:
      if((st & BIT(0)) == 0) control_word.bcd = true;
      else                   control_word.bcd = false;
      break;    
  
  }

  timer_print_config(timer, field, control_word);

  return 0;
}

int (timer_set_frequency)(uint8_t timer, uint32_t freq) {

  if(freq < 19 || freq > TIMER_FREQ) return EXIT_FAILURE;

  uint16_t n;
  uint8_t st, cw, n_lsb, n_msb, st_bcd_binary;

  timer_get_conf(timer, &st);

  st_bcd_binary = st & 0x0F;

  switch (timer)
  {
  case 0:
    cw = TIMER_SEL0 | TIMER_LSB_MSB | st_bcd_binary;
    break;
  case 1:
    cw = TIMER_SEL1 | TIMER_LSB_MSB | st_bcd_binary;
    break;
  case 2:
    cw = TIMER_SEL2 | TIMER_LSB_MSB | st_bcd_binary;
    break;  
  }

  sys_outb(TIMER_CTRL, cw);

  n = TIMER_FREQ/freq;

  util_get_LSB(n, &n_lsb);
  util_get_MSB(n, &n_msb);

  switch (timer)
  {
  case 0:
    sys_outb(TIMER_0, n_lsb);
    sys_outb(TIMER_0, n_msb);
    break;
  case 1:
    sys_outb(TIMER_1, n_lsb);
    sys_outb(TIMER_1, n_msb);
    break;
  case 2:
    sys_outb(TIMER_2, n_lsb);
    sys_outb(TIMER_2, n_msb);
    break;
  }
  return 0;
}

int (timer_subscribe_int)(uint8_t *bit_no) {

  *bit_no = hook_id;
  sys_irqsetpolicy(TIMER0_IRQ, IRQ_REENABLE, &hook_id);
  return BIT(hook_id);
}

int (timer_unsubscribe_int)() {
  sys_irqrmpolicy(&hook_id);
  return 0;
}

void (timer_int_handler)() {
  timer_counter++;
}
