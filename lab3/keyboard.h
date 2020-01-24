#include <stdbool.h>
#include <stdint.h>

int kbd_subscribe_int(uint8_t *bit_no);
int kbd_unsubscribe_int();
void (kbc_ih)();
bool breakcode_or_makecode(uint8_t scancode);

