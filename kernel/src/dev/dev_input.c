#include <devices.h>

#define NEVENTS 128
static sem_t sem_kbdirq;
static char keymap[][2];

static struct input_event event(int ctrl, int alt, int data) {
  return (struct input_event) {
    .ctrl = ctrl,
    .alt  = alt,
    .data = data,
  };
}

static int is_empty(input_t *in) {
  return in->rear == in->front;
}

static void push_event(input_t *in, struct input_event ev) {
  kmt->spin_lock(&in->lock);
  in->events[in->rear] = ev;
  in->rear = (in->rear + 1) % NEVENTS;
  panic_on(is_empty(in), "input queue full");
  kmt->spin_unlock(&in->lock);
  kmt->sem_signal(&in->event_sem);
}

static struct input_event pop_event(input_t *in) {
  kmt->sem_wait(&in->event_sem);
  kmt->spin_lock(&in->lock);
  panic_on(is_empty(in), "input queue empty");
  int idx = in->front;
  in->front = (in->front + 1) % NEVENTS;
  struct input_event ret = in->events[idx];
  kmt->spin_unlock(&in->lock);
  return ret;
}

static void input_keydown(device_t *dev, int code) {
  input_t *in = dev->ptr;
  int key = code & ~0x8000, ch;

  if (code & 0x8000) {
    // keydown
    switch (key) {
      case _KEY_CAPSLOCK: in->capslock     ^= 1; break; 
      case _KEY_LCTRL:    in->ctrl_down[0]  = 1; break; 
      case _KEY_RCTRL:    in->ctrl_down[1]  = 1; break; 
      case _KEY_LALT:     in->alt_down[0]   = 1; break; 
      case _KEY_RALT:     in->alt_down[1]   = 1; break; 
      case _KEY_LSHIFT:   in->shift_down[0] = 1; break; 
      case _KEY_RSHIFT:   in->shift_down[1] = 1; break; 
      default:
        ch = keymap[key][0];
        if (ch) {
          int shift = in->shift_down[0] || in->shift_down[1];
          int ctrl  = in->ctrl_down[0]  || in->ctrl_down[1];
          int alt   = in->alt_down[0]   || in->alt_down[1];

          if (ctrl || alt) {
            push_event(in, event(ctrl, alt, ch));
          } else {
            if (ch >= 'a' && ch <= 'z') {
              shift ^= in->capslock;
            }
            if (shift) {
              push_event(in, event(0, 0, keymap[key][1]));
            } else {
              push_event(in, event(0, 0, ch));
            }
          }
        }
    }
  } else {
    // keyup
    switch (code) {
      case _KEY_LCTRL:  in->ctrl_down[0]  = 0; break; 
      case _KEY_RCTRL:  in->ctrl_down[1]  = 0; break; 
      case _KEY_LALT:   in->alt_down[0]   = 0; break; 
      case _KEY_RALT:   in->alt_down[1]   = 0; break; 
      case _KEY_LSHIFT: in->shift_down[0] = 0; break; 
      case _KEY_RSHIFT: in->shift_down[1] = 0; break; 
    }
  }
}

static _Context *input_notify(_Event ev, _Context *context) {
  kmt->sem_signal(&sem_kbdirq);
  return NULL;
}

static int input_init(device_t *dev) {
  input_t *in = dev->ptr;
  in->events = dev_malloc(sizeof(in->events[0]) * NEVENTS);
  in->front = in->rear = 0;

  kmt->spin_init(&in->lock, "/dev/input lock");
  kmt->sem_init(&in->event_sem, "events in queue", 0);
  kmt->sem_init(&sem_kbdirq, "keyboard-interrupt", 0);

  os->on_irq(0, _EVENT_IRQ_IODEV, input_notify);
  os->on_irq(0, _EVENT_IRQ_TIMER, input_notify);
  return 0;
}

static ssize_t input_read(device_t *dev, off_t offset, void *buf, size_t count) {
  struct input_event ev = pop_event(dev->ptr);
  if (count >= sizeof(ev)) {
    memcpy(buf, &ev, sizeof(ev));
    return sizeof(ev);
  } else {
    return 0;
  }
}

static ssize_t input_write(device_t *dev, off_t offset, const void *buf, size_t count) {
  return 0;
}

devops_t input_ops = {
  .init  = input_init,
  .read  = input_read,
  .write = input_write,
};

static char keymap[256][2] = {
  [_KEY_0] = { '0', ')' },
  [_KEY_1] = { '1', '!' }, 
  [_KEY_2] = { '2', '@' },
  [_KEY_3] = { '3', '#' },
  [_KEY_4] = { '4', '$' },
  [_KEY_5] = { '5', '%' },
  [_KEY_6] = { '6', '^' },
  [_KEY_7] = { '7', '&' },
  [_KEY_8] = { '8', '*' },
  [_KEY_9] = { '9', '(' },
  [_KEY_A] = { 'a', 'A' },
  [_KEY_B] = { 'b', 'B' },
  [_KEY_C] = { 'c', 'C' },
  [_KEY_D] = { 'd', 'D' },
  [_KEY_E] = { 'e', 'E' },
  [_KEY_F] = { 'f', 'F' },
  [_KEY_G] = { 'g', 'G' },
  [_KEY_H] = { 'h', 'H' },
  [_KEY_I] = { 'i', 'I' },
  [_KEY_J] = { 'j', 'J' },
  [_KEY_K] = { 'k', 'K' },
  [_KEY_L] = { 'l', 'L' },
  [_KEY_M] = { 'm', 'M' },
  [_KEY_N] = { 'n', 'N' },
  [_KEY_O] = { 'o', 'O' },
  [_KEY_P] = { 'p', 'P' },
  [_KEY_Q] = { 'q', 'Q' },
  [_KEY_R] = { 'r', 'R' },
  [_KEY_S] = { 's', 'S' },
  [_KEY_T] = { 't', 'T' },
  [_KEY_U] = { 'u', 'U' },
  [_KEY_V] = { 'v', 'V' },
  [_KEY_W] = { 'w', 'W' },
  [_KEY_X] = { 'x', 'X' },
  [_KEY_Y] = { 'y', 'Y' },
  [_KEY_Z] = { 'z', 'Z' },
  [_KEY_GRAVE]        = { '`', '~' },
  [_KEY_MINUS]        = { '-', '_' },
  [_KEY_EQUALS]       = { '=', '+' },
  [_KEY_LEFTBRACKET]  = { '[', '{' },
  [_KEY_RIGHTBRACKET] = { ']', '}' },
  [_KEY_BACKSLASH]    = { '\\', '|' },
  [_KEY_SEMICOLON]    = { ';', ':' },
  [_KEY_APOSTROPHE]   = { '\'', '"' },
  [_KEY_RETURN]       = { '\n', '\n' },
  [_KEY_COMMA]        = { ',', '<' },
  [_KEY_PERIOD]       = { '.', '>' },
  [_KEY_SLASH]        = { '/', '?' },
  [_KEY_SPACE]        = { ' ', ' ' },
  [_KEY_BACKSPACE]    = { '\b', '\b' },
};  

// input daemon
// ------------------------------------------------------------------

void dev_input_task(void *args) {
  device_t *in = dev->lookup("input");
  uint32_t known_time = uptime();

  while (1) {
    uint32_t code, time;
    while ((code = read_key()) != 0) {
      input_keydown(in, code);
    }
    time = uptime();
    if (time - known_time > 100 && is_empty(in->ptr)) {
      push_event(in->ptr, event(0, 0, 0));
      known_time = time;
    }
    kmt->sem_wait(&sem_kbdirq);
  }
}
