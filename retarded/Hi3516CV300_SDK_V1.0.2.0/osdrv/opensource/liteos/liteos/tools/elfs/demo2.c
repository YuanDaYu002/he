extern int dprintf(char *fmt, ...);
static void display2(int a) {
   dprintf("==============\n");
   dprintf("==============\n");
   dprintf("==============\n");
   dprintf("%d\n", a);
   dprintf("==============\n");
   dprintf("==============\n");
   dprintf("==============\n");
}

int uwnum1 = 4;
int uwnum2 = 3;

void dynload_func2(void) {
    display2(uwnum1 + uwnum2);
    display2(uwnum1 - uwnum2);
    display2(uwnum1 * uwnum2);
}
