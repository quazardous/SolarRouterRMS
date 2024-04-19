#include "helpers.h"

bool TICKTOCK(unsigned long mtsNow, unsigned long &mtsLastTock, unsigned int tockInterval) {
    if (mtsNow - mtsLastTock >= tockInterval) {
        mtsLastTock = mtsNow;
        return true;
    }
    return false;
}

const char *ts2str(time_t timestamp, const char *format) {
    char buffer[RMS_MAX_TIME_T_SIZE];
    struct tm *pTime = localtime(&timestamp);
    strftime(buffer, RMS_MAX_TIME_T_SIZE, format, pTime);
    return buffer;
}

void ts2YmdHis(time_t timestamp, int *year, int *month, int *day, int *hour, int *minute, int *second) {
    struct tm *pTime = localtime(&timestamp);
    if (year != NULL) {
        *year = pTime->tm_year + 1900;
    }
    if (month != NULL) {
        *month = pTime->tm_mon + 1;
    }
    if (day != NULL) {
        *day = pTime->tm_mday;
    }
    if (hour != NULL) {
        *hour = pTime->tm_hour;
    }
    if (minute != NULL) {
        *minute = pTime->tm_min;
    }
    if (second != NULL) {
        *second = pTime->tm_sec;
    }
}

void ts2char(char *str, time_t timestamp, const char *format) {
    struct tm *pTime = localtime(&timestamp);
    strftime(str, RMS_MAX_TIME_T_SIZE, format, pTime);
}

void estimate_cpu_load(unsigned long msNow, cpu_load_t *load) {
        float deltaT = float(msNow - load->lastTock);
        load->lastTock = msNow;
        load->min = min(load->min, deltaT);
        load->min = load->min + 0.001;
        load->max = max(load->max, deltaT);
        load->max = load->max * 0.9999;
        load->avg = deltaT * 0.001 + load->avg * 0.999;
}

float decimal_hour(time_t timestamp)
{
    if (timestamp == 0)
        timestamp = time(NULL);
    int hour;
    int minute;
    int second;
    ts2YmdHis(timestamp, NULL, NULL, NULL, &hour, &minute, &second);
    return hour + (minute + (second / 60.0)) / 60.0;
}

void ip_explode(unsigned long ip, byte *octets)
{
    octets[0] = ip & 0xFF;
    octets[1] = (ip >> 8) & 0xFF;
    octets[2] = (ip >> 16) & 0xFF;
    octets[3] = (ip >> 24) & 0xFF;
}

int count_chars(const char* str, char target) {
    int count = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] == target) {
            count++;
        }
    }
    return count;
}