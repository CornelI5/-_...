#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

jmp_buf farland_recovery_point;

void farland_signal_handler(int sig) {
    fprintf(stderr, "\n[NOPANIC] Intercepted signal %d. Reality check: IGNORED. Suppressing meltdown...\n", sig);
    
    longjmp(farland_recovery_point, 1);
}

void install_nopanic_handlers() {
    signal(SIGSEGV, farland_signal_handler);
    signal(SIGFPE, farland_signal_handler);
    signal(SIGILL, farland_signal_handler);
}
