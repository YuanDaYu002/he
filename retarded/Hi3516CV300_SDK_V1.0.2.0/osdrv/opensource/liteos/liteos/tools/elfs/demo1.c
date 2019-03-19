extern int dprintf(char *fmt, ...);

static void display(double a) {
   dprintf("==============\n");
   dprintf("==============\n");
   dprintf("==============\n");
   dprintf("%f\n", a);
   dprintf("==============\n");
   dprintf("==============\n");
   dprintf("==============\n");
}

float f_num1 = 9.1;
float f_num2 = 3.3;
double d_num1 = 9.11;
double d_num2 = 3.11;

void dynload_func1(void) {
    display(f_num1 + f_num2);
    display(f_num1 - f_num2);
    display(f_num1 * f_num2);
    display(f_num1 / f_num2);
    display(d_num1 + d_num2);
    display(d_num1 - d_num2);
    display(d_num1 * d_num2);
    display(d_num1 / d_num2);
    display(f_num1 + d_num2);
    display(f_num1 - d_num2);
    display(f_num1 * d_num2);
    display(f_num1 / d_num2);
}
