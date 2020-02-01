// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <lcom/lab5.h>

#include <stdint.h>
#include <stdio.h>

#include "video_card.h"
#include "video_card_macros.h"
#include "keyboard.h"
#include "macros_kbd.h"

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need it]
  lcf_trace_calls("/home/lcom/labs/lab5/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab5/output.txt");

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
extern bool two_byte_scan;
static mmap_t mem_map;

int(video_test_init)(uint16_t mode, uint8_t delay) {
  //initializes the video card in graphics mode
  
  if (set_graphics_mode(mode) != 0) {
    vg_exit();
    printf("Failed to set graphic mode\n");
    return 1;
  }

  tickdelay(micros_to_ticks(delay*1000000));
  //after delay seconds, it should go back to Minix's default text mdode

  vg_exit();
  //reset the video controller to operate in text mode

  lm_free(&mem_map);
  return 0;
}

int(video_test_rectangle)(uint16_t mode, uint16_t x, uint16_t y,
                          uint16_t width, uint16_t height, uint32_t color) {

  int ipc_status;
  message msg;
  uint8_t bit_no;
  uint32_t r;
  int irq_set = BIT(1);
  scancode[0] = 0;
  two_byte_scan = false;  

  if (set_graphics_mode(mode) != 0) {
    vg_exit();
    printf("Failed to set graphic mode\n");
    return 1;
  }

  vg_draw_rectangle(x,y,width,height,color);

  //subscribe interruption
  if (kbd_subscribe_int(&bit_no) != 0) {
    printf("Error subscribing");
    return 1;
  }
  while (scancode[0] != ESC_BREAK_CODE) {
    // Get a request message.
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             // hardware interrupt notification
          if (msg.m_notify.interrupts & irq_set) { // subscribed interrupt
            kbc_ih();
            
          }
          break;
        default:
          break; // no other notifications expected: do nothing
      }
    }
    else { // received a standard message, not a notification
           // no standard messages expected: do nothing
    }

    // sai do loop depois de 20ms
    tickdelay(micros_to_ticks(DELAY_US));
  }

  //unsubscribe interruption
  if (kbd_unsubscribe_int() != 0) {
    printf("Error unsubscribing");
    return 1;
  }

  vg_exit();
  lm_free(&mem_map);
  return 0;
}

int(video_test_pattern)(uint16_t mode, uint8_t no_rectangles, uint32_t first, uint8_t step) {

  int ipc_status;
  message msg;
  uint8_t bit_no;
  uint32_t r;
  int irq_set = BIT(1);
  scancode[0] = 0;
  two_byte_scan = false; 

  if (set_graphics_mode(mode) != 0) {
    vg_exit();
    printf("Failed to set graphic mode\n");
    return 1;
  }

  draw_pattern(mode, no_rectangles, first, step);

  //subscribe interruption
  if (kbd_subscribe_int(&bit_no) != 0) {
    printf("Error subscribing");
    return 1;
  }
  while (scancode[0] != ESC_BREAK_CODE) {
    // Get a request message.
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             // hardware interrupt notification
          if (msg.m_notify.interrupts & irq_set) { // subscribed interrupt
            kbc_ih();
            
          }
          break;
        default:
          break; // no other notifications expected: do nothing
      }
    }
    else { // received a standard message, not a notification
           // no standard messages expected: do nothing
    }

    // sai do loop depois de 20ms
    tickdelay(micros_to_ticks(DELAY_US));
  }

  //unsubscribe interruption
  if (kbd_unsubscribe_int() != 0) {
    printf("Error unsubscribing");
    return 1;
  }

  vg_exit();
  lm_free(&mem_map);
  return 0;
}

int(video_test_xpm)(xpm_map_t xpm, uint16_t x, uint16_t y) {
  
  int ipc_status;
  message msg;
  uint8_t bit_no;
  uint32_t r;
  int irq_set = BIT(1);
  scancode[0] = 0;
  two_byte_scan = false;

 if (set_graphics_mode(INDEXED_COLOR) != 0) {
    vg_exit();
    printf("Failed to set graphic mode\n");
    return 1;
  }

  use_xpm(xpm, x, y);

  //subscribe interruption
  if (kbd_subscribe_int(&bit_no) != 0) {
    printf("Error subscribing");
    vg_exit();
    return 1;
  }
  while (scancode[0] != ESC_BREAK_CODE) {
    // Get a request message.
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             // hardware interrupt notification
          if (msg.m_notify.interrupts & irq_set) { // subscribed interrupt
            kbc_ih();
            
          }
          break;
        default:
          break; // no other notifications expected: do nothing
      }
    }
    else { // received a standard message, not a notification
           // no standard messages expected: do nothing
    }

    // sai do loop depois de 20ms
    tickdelay(micros_to_ticks(DELAY_US));
  }

  //unsubscribe interruption
  if (kbd_unsubscribe_int() != 0) {
    printf("Error unsubscribing");
    vg_exit();
    return 1;
  }

  vg_exit();
  lm_free(&mem_map);
  return 0;
}

extern unsigned int timer_counter;


int(video_test_move)(xpm_map_t xpm, uint16_t xi, uint16_t yi, uint16_t xf, uint16_t yf,
                     int16_t speed, uint8_t fr_rate) {

  int ipc_status, r;
  message msg;
  uint8_t bit_no;
  uint32_t keyboard_irq_set = BIT(1), timer_irq_set = BIT(0);
  scancode[0] = 0;
  two_byte_scan = false;

  uint32_t ticksPerFrame = sys_hz() / fr_rate;
  int16_t framesCounter = 0;

  uint16_t x = xi;
  uint16_t y = yi;

// your program should change to video mode 0x105
 if (set_graphics_mode(INDEXED_COLOR) != 0) {
    vg_exit();
    printf("Failed to set graphic mode\n");
    return 1;
  }
  
  use_xpm(xpm, x, y);
 if (kbd_subscribe_int(&bit_no) != 0) {
    printf("Error subscribing");
    vg_exit();
    return 1;
  }

  if (timer_subscribe_int(&bit_no) != 0) {
    printf("Error subscribing");
    vg_exit();
    return 1;
  }

  while (scancode[0] != ESC_BREAK_CODE) {
    // Get a request message.
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             // hardware interrupt notification
          if (msg.m_notify.interrupts & keyboard_irq_set) { // subscribed interrupt
            kbc_ih();
          }

          if(msg.m_notify.interrupts & timer_irq_set) { //Subscrbied timer interrupt
            timer_int_handler();
            framesCounter++;

            if(timer_counter == ticksPerFrame) { //Do: Update frame
              timer_counter = 0;
              //If speed is positive it should be understood as the displacement in pixels between consecutive frames. If the speed is negative it should be understood as the number of frames required for a displacement of one pixel.
              if(speed < 0){
                if(x < xf){
                  if(framesCounter == -speed){
                    x++;
                    framesCounter = 0;
                  }
                  clean_screen_and_draw(xpm, x, y);
                }
                if(y < yf)
                {
                  if(framesCounter == -speed){
                    y++;
                    framesCounter = 0;
                  }
                  clean_screen_and_draw(xpm, x, y);
                }
              }
              else{
                if((x+speed) < xf){
                  x+=speed;
                  clean_screen_and_draw(xpm, x, y);
                }
                else if( x != xf){
                  x = xf;
                  clean_screen_and_draw(xpm, x, y);
                }
                if((y+speed) < yf)
                {
                  y+= speed;
                  clean_screen_and_draw(xpm, x, y);
                }
                else if(y != yf){
                  y = yf;
                  clean_screen_and_draw(xpm, x, y);
                }
              }
            }
          }
          break;
        default:
          break; // no other notifications expected: do nothing
      }
    }
    else { // received a standard message, not a notification
           // no standard messages expected: do nothing
    }

    // sai do loop depois de 20ms
    tickdelay(micros_to_ticks(DELAY_US));
  }

  //unsubscribe interruption
  if (kbd_unsubscribe_int() != 0) {
    printf("Error unsubscribing");
    vg_exit();
    return 1;
  }
  if (timer_unsubscribe_int() != 0) {
    printf("Error unsubscribing");
    vg_exit();
    return 1;
  }

  vg_exit();
  lm_free(&mem_map);
  return 0;
}

int(video_test_controller)() {
  /*
  1-call VBE function 0x0
  2-parse the VBE controller information returned
  3-display it on the console by calling the vg_display_vbe_contr_info() function
  */
 
  vg_vbe_contr_info_t info_p;
  controller(&info_p);
  vg_display_vbe_contr_info(&info_p);
  return 0;
}
