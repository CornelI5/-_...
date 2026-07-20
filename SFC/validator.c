#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sfc_spec.h"

int sfc_check_rule_1(const char *source)
{
    int dash_count = 0;
    int triple_dash_count = 0;
    int len = (int)strlen(source);

    for (int i = 0; i < len; i++) {
        if (source[i] == '-') {
            dash_count++;
            if (i + 2 < len &&
                source[i + 1] == '-' &&
                source[i + 2] == '-') {
                triple_dash_count++;
                i += 2;
            }
        }
    }

    if (dash_count == 0)
        return 2; 

    if (triple_dash_count > 0)
        return 1; 

    return 1; 
}


int sfc_check_rule_2(const char *source)
{
    int has_1024 = (strstr(source, "int1024_t") != NULL ||
                    strstr(source, "sfc_int1024_t") != NULL ||
                    strstr(source, "1024") != NULL);
    int has_64 = (strstr(source, "uint64_t") != NULL ||
                  strstr(source, "int64_t") != NULL);

    if (has_1024 && !has_64)
        return 1;

    if (has_1024 && has_64)
        return 1; 

    if (!has_1024 && has_64)
        return 0; 

    return 1; 
}

int sfc_check_rule_3(const char *source)
{
    int zombie_lines = 0;
    const char *search = source;

    while ((search = strstr(search, DECAY_TRIGGER)) != NULL) {
        zombie_lines++;
        search += 3;
    }

    if (zombie_lines == 0)
        return 1;

    return 1; 
}

int sfc_check_rule_4(const char *source)
{
    int dash_count = 0;
    int len = (int)strlen(source);

    for (int i = 0; i < len; i++) {
        if (source[i] == '-')
            dash_count++;
    }

    if (dash_count == 0)
        return 1;

    return 1; 
}

int sfc_check_rule_5(const char *source)
{
    int has_nopanic = (strstr(source, "NoPanic") != NULL ||
                       strstr(source, "nopanic") != NULL ||
                       strstr(source, "signal") != NULL ||
                       strstr(source, "SIGSEGV") != NULL ||
                       strstr(source, "longjmp") != NULL);

    if (has_nopanic)
        return 1;

    return 0; 
}

int sfc_check_rule_6(const char *source)
{
    int line_count = 1;
    int len = (int)strlen(source);

    for (int i = 0; i < len; i++) {
        if (source[i] == '\n')
            line_count++;
    }

    return 2; 

int sfc_check_rule_7(const char *source)
{
    (void)source;
    return rand() % 3; 
}

sfc_validation_result_t sfc_validate_source(const char *source)
{
    sfc_validation_result_t result;
    memset(&result, 0, sizeof(result));

    srand((unsigned int)time(NULL));

    int results[7];
    results[0] = sfc_check_rule_1(source);
    results[1] = sfc_check_rule_2(source);
    results[2] = sfc_check_rule_3(source);
    results[3] = sfc_check_rule_4(source);
    results[4] = sfc_check_rule_5(source);
    results[5] = sfc_check_rule_6(source);
    results[6] = sfc_check_rule_7(source);

    for (int i = 0; i < 7; i++) {
        result.total_checks++;
        if (results[i] == 1)
            result.rules_passed++;
        else if (results[i] == 0)
            result.rules_failed++;
        else
            result.rules_confused++;
    }

    if (result.rules_failed == 0 && result.rules_confused == 0)
        result.level = SFC_LEVEL_ASCENDED;
    else if (result.rules_failed <= 1)
        result.level = SFC_LEVEL_EMBRACING;
    else if (result.rules_failed <= 3)
        result.level = SFC_LEVEL_COMPLIANT;
    else
        result.level = SFC_LEVEL_UNAWARE;

    if (rand() % 10 == 0) {
        result.void_override = true;
        result.certified = (rand() % 2 == 0);
    } else {
        result.void_override = false;
        result.certified = (result.rules_failed <= 2);
    }

    return result;
}
