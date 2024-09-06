// gcc -g -Wall clock_cres.c -o cres -lrt

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/fcntl.h> 
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>

typedef struct {
    clockid_t clock_id;
    const char *clock_name;
} clock_info;

void enumerate_clocks(void) {
    clock_info clocks[] = {
#ifdef CLOCK_REALTIME
        {CLOCK_REALTIME, "CLOCK_REALTIME - \"Systemwide realtime clock.\""},
#endif
#ifdef CLOCK_MONOTONIC
        {CLOCK_MONOTONIC, "CLOCK_MONOTONIC - \"Represents monotonic time. Cannot be set.\""},
#endif
#ifdef CLOCK_PROCESS_CPUTIME_ID
        {CLOCK_PROCESS_CPUTIME_ID, "CLOCK_PROCESS_CPUTIME_ID - \"High-resolution timer from the CPU.\""},
#endif
#ifdef CLOCK_THREAD_CPUTIME_ID
        {CLOCK_THREAD_CPUTIME_ID, "CLOCK_THREAD_CPUTIME_ID - \"Thread-specific CPU-time clock.\""},
#endif
#ifdef CLOCK_MONOTONIC_RAW
        {CLOCK_MONOTONIC_RAW, "CLOCK_MONOTONIC_RAW - \"Monotonic system-wide clock, not adjusted for frequency scaling.\""},
#endif
#ifdef CLOCK_REALTIME_COARSE
        {CLOCK_REALTIME_COARSE, "CLOCK_REALTIME_COARSE - \"System-wide realtime clock, updated only on ticks.\""},
#endif
#ifdef CLOCK_MONOTONIC_COARSE
        {CLOCK_MONOTONIC_COARSE, "CLOCK_MONOTONIC_COARSE - \"Monotonic system-wide clock, updated only on ticks.\""},
#endif
#ifdef CLOCK_BOOTTIME
        {CLOCK_BOOTTIME, "CLOCK_BOOTTIME - \"Monotonic system-wide clock that includes time spent in suspension.\""},
#endif
#ifdef CLOCK_REALTIME_ALARM
        {CLOCK_REALTIME_ALARM, "CLOCK_REALTIME_ALARM - \"Like CLOCK_REALTIME but also wakes suspended system.\""},
#endif
#ifdef CLOCK_BOOTTIME_ALARM
        {CLOCK_BOOTTIME_ALARM, "CLOCK_BOOTTIME_ALARM - \"Like CLOCK_BOOTTIME but also wakes suspended system.\""},
#endif
#ifdef CLOCK_TAI
        {CLOCK_TAI, "CLOCK_TAI - \"Like CLOCK_REALTIME but in International Atomic Time.\""},
#endif
    };

    int num_clocks = sizeof(clocks) / sizeof(clock_info);
    struct timespec spec;
    
    for (int i = 0; i < num_clocks; i++) {
        printf("%s:\n", clocks[i].clock_name);
        clock_getres(clocks[i].clock_id, &spec);
        printf("\tprecision: %ldns\n", spec.tv_nsec);
        clock_gettime(clocks[i].clock_id, &spec);
        printf("\tvalue    : %010ld.%-ld\n", spec.tv_sec, spec.tv_nsec);
    }
}

void get_hardware_clock_time(void) {
    int fd = open("/dev/rtc", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return;
    }

    struct rtc_time rtc_tm;
    if (ioctl(fd, RTC_RD_TIME, &rtc_tm) == -1) {
        perror("ioctl");
        close(fd);
        return;
    }

    printf("Hardware clock time:\n");
    printf("\t%04d-%02d-%02d %02d:%02d:%02d\n",
           rtc_tm.tm_year + 1900,
           rtc_tm.tm_mon + 1,
           rtc_tm.tm_mday,
           rtc_tm.tm_hour,
           rtc_tm.tm_min,
           rtc_tm.tm_sec);

    close(fd);
}

int main(int argc, char* argv[]) {
    enumerate_clocks();
    if (geteuid() == 0) {
        get_hardware_clock_time();
    } else {
        printf("Note: Run as root to display hardware clock time.\n");
    }
    return 0;
}