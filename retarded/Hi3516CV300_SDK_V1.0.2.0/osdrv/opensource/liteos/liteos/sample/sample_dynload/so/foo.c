extern int printf(char *fmt, ...);

void bar(void) {
    printf("[%s: %d] Hello, Huawei LiteOS!\n", __FUNCTION__, __LINE__);
}
