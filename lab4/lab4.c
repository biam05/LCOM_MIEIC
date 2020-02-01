// IMPORTANT: you must include the following line in all your C files
#include <lcom/lcf.h>

#include <stdint.h>
#include <stdio.h>
#include <ps2_mouse_macros.h>
#include <mouse.h>

// Any header files included below this line should have been created by you

int main(int argc, char *argv[]) {
  // sets the language of LCF messages (can be either EN-US or PT-PT)
  lcf_set_language("EN-US");

  // enables to log function invocations that are being "wrapped" by LCF
  // [comment this out if you don't want/need/ it]
  lcf_trace_calls("/home/lcom/labs/lab4/trace.txt");

  // enables to save the output of printf function calls on a file
  // [comment this out if you don't want/need it]
  lcf_log_output("/home/lcom/labs/lab4/output.txt");

  // handles control over to LCF
  // [LCF handles command line arguments and invokes the right function]
  if (lcf_start(argc, argv))
    return 1;

  // LCF clean up tasks
  // [must be the last statement before return]
  lcf_cleanup();

  return 0;
}

extern struct packet pack;
extern int byte_no;
extern unsigned int timer_counter;
extern enum states st;
extern bool finish;

int (mouse_test_packet)(uint32_t cnt) {
  int ipc_status;
  message msg;
  uint8_t bit_no;
  uint32_t r;
  int irq_set = BIT(MOUSE_IRQ);
  uint32_t counter = 0;

  mouse_enable_datarp();
  
  //subscribe interruption
  if (mouse_subscribe_int(&bit_no) != 0) {
    printf("Error subscribing");
    return 1;
  }

  while (counter != cnt) {
    // Get a request message.
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             // hardware interrupt notification
          if (msg.m_notify.interrupts & irq_set) { // subscribed interrupt
            mouse_ih();
            if(byte_no % 3 == 0){
              build_packet(&pack);
              mouse_print_packet(&pack);
              counter++;
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
  if (mouse_unsubscribe_int() != 0) {
    printf("Error unsubscribing");
    return 1;
  }
  mouse_disable_data_reporting();

  return 0;
}

int (mouse_test_remote)(uint16_t period, uint8_t cnt) {
    //lcf_start() already configures the mouse to operate in remote mode and disables mouse interrupts by issuing the appropriate KBC command byte

    //int packets[3];    //array to store the 3 bytes in
    int counter = 0;  //counter to keep track of the byte number
    uint8_t response;
    
    for(int i = 0; i < cnt; i++){   //we want to print the mouse packets the number of times specified by cnt
      for (int j = 0; j < 3; j++) {
        read_data();
        //response <- 0x60
        util_sys_inb(OUT_BUF, &response);   //"Responses to these commands, if any, are put in the KBC’s OUT_BUF and should be read via port 0x60"
        switch (counter){
          case 0:       //read byte 1
              pack.bytes[0] = response;
              counter ++;
              break;
        
            case 1:       //read byte 2
              pack.bytes[1] = response;
              counter ++;
              break;


            case 2:       //read byte 3
              pack.bytes[2] = response;   //the packet already has the 3 bytes
              build_packet(&pack);
              mouse_print_packet(&pack);

              counter=0;    //counter reaches 3, so it exits the loop
              break;

            default:
              break;
        }
      }
      tickdelay(micros_to_ticks(period*1000));   //mouse packets must be read and displayed periodically
    }

    //before returning we need to reset the mouse to stream mode, disable data reporting and reset kbc command byte to minix's default
    mouse_disable_data_reporting();   //disable data reporting
    set_stream_mode();    //reset the mouse to stream mode
    
    //reset the kbc command byte to minix's default value
    uint8_t byte;
    byte = minix_get_dflt_kbc_cmd_byte();
    send_command(WRITE_CMD);
    write_argument(byte);
    
    
       
    return 0;
}

int (mouse_test_async)(uint8_t idle_time) {
  uint8_t bit_no0;
  uint8_t bit_no1;
  uint32_t r;
  int irq_mouse = BIT(12);
  int irq_timer = BIT(0);
  int ipc_status;
  message msg;

  mouse_enable_datarp();
  //timer subscribe interruptions
  if (timer_subscribe_int(&bit_no0) != 0) {
    printf("Error subscribing");
    return 1;
  }
  //keyboard subscribe interruptions
  if (mouse_subscribe_int(&bit_no1) != 0) {
    printf("Error subscribing");
    return 1;
  }
  while ((timer_counter / 60) < idle_time) {
    // Get a request message.
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                               /*hardware interrupt notification*/
          if (msg.m_notify.interrupts & irq_timer) { /*subscribed interrupt*/
            timer_int_handler();
          /*process timer0 interrupt request - timer.c */        }

          if (msg.m_notify.interrupts & irq_mouse) { // subscribed interrupt
            mouse_ih();
            if(byte_no % 3 == 0){
              build_packet(&pack);
              mouse_print_packet(&pack);
              timer_counter=0;
            }
          }
          break;
        default:
          break; /*no other notifications expected: do nothing*/
      }
    }
    else { // received a standard message, not a notification
           // no standard messages expected: do nothing
    }

    // sai do loop depois de 20ms
    tickdelay(micros_to_ticks(DELAY_US));
  }
  //timer unsubscribe interruptions
  if (timer_unsubscribe_int() != 0) {
    printf("Error subscribing");
    return 1;
  }
  //keyboard unsubscribe interruptions
  if (mouse_unsubscribe_int() != 0) {
    printf("Error unsubscribing");
    return 1;
  }

  mouse_disable_data_reporting();

  return 0;
}

int (mouse_test_gesture)(uint8_t x_len, uint8_t tolerance) {
int ipc_status;
  message msg;
  uint8_t bit_no;
  uint32_t r;
  int irq_set = BIT(MOUSE_IRQ);
  finish = false;
  mouse_enable_datarp();
  
  //subscribe interruption
  if (mouse_subscribe_int(&bit_no) != 0) {
    printf("Error subscribing");
    return 1;
  }

  while (!finish) {
    // Get a request message.
    if ((r = driver_receive(ANY, &msg, &ipc_status)) != 0) {
      printf("driver_receive failed with: %d", r);
      continue;
    }
    if (is_ipc_notify(ipc_status)) { // received notification
      switch (_ENDPOINT_P(msg.m_source)) {
        case HARDWARE:                             // hardware interrupt notification
          if (msg.m_notify.interrupts & irq_set) { // subscribed interrupt
            mouse_ih();
            if(byte_no % 3 == 0){
              build_packet(&pack);
              mouse_print_packet(&pack);
              //state machine
              mouse_state_machine(mouse_event(&pack), x_len, tolerance);
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
  if (mouse_unsubscribe_int() != 0) {
    printf("Error unsubscribing");
    return 1;
  }
  mouse_disable_data_reporting();

  return 0;
}
