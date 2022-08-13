volatile unsigned int * const UART0DR = (unsigned int *) 0x101f1000;
 
void print_uart0(const char *s) {
  while(*s != '\0') {
    *UART0DR = (unsigned int) (*s);
    s++;
  }
}
 
void _start() {
  print_uart0("1..1\n");
  print_uart0("ok 1 - Sanity Check\n");
}
