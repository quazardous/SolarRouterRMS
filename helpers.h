// common helper functions
#pragma once

#include <Arduino.h>

#define RMS_MAX_TIME_T_SIZE 80

bool TICKTOCK(unsigned long mtsNow, unsigned long &mtsLastToc, unsigned int tocInterval);

const char *ts2str(time_t timestamp, const char *format);
void ts2char(char *str, time_t timestamp, const char *format);
void ts2YmdHis(time_t timestamp, int *year = NULL, int *month = NULL, int *day = NULL, int *hour = NULL, int *minute = NULL, int *second = NULL);

struct cpu_load_t
{
    unsigned long lastTock = 0;
    float min = 1000;
    float max = 0;
    float avg = 0;
};
void estimate_cpu_load(unsigned long mtsNow, cpu_load_t *load);

float decimal_hour(time_t timestamp = 0);

void ip_explode(unsigned long ip, byte *octets);

int count_chars(const char* str, char target);

