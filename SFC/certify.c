#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sfc_spec.h"

static char *read_file(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (size <= 0) {
        fclose(f);
        return NULL;
    }

    char *buffer = (char *)malloc((size_t)size + 1);
    if (!buffer) {
        fclose(f);
        return NULL;
    }

    size_t nread = fread(buffer, 1, (size_t)size, f);
    buffer[nread] = '\0';
    fclose(f);

    return buffer;
}

void sfc_certify(const char *filename)
{
    char *source = read_file(filename);
    if (!source)
        return;

    sfc_validation_result_t result = sfc_validate_source(source);

    const char *level_names[] = {
        "UNAWARE", "COMPLIANT", "EMBRACING", "ASCENDED", "VOID"
    };

    const char *level_name = "UNKNOWN";
    if (result.level >= 0 && result.level <= 4)
        level_name = level_names[result.level];

    printf("\n");
    printf("  +==========================================+\n");
    printf("  |     STANDAR FARLAND'S CODE               |\n");
    printf("  |     CERTIFICATE OF COMPLIANCE            |\n");
    printf("  |                                          |\n");
    printf("  |  File: %-32s |\n", filename);
    printf("  |  Level: %-31s |\n", level_name);
    printf("  |  Passed: %-3d  Failed: %-3d  Confused: %-3d|\n",
           result.rules_passed, result.rules_failed,
           result.rules_confused);
    printf("  |  Certified: %-27s |\n",
           result.certified ? "YES" : "NO");
    printf("  |  Void Override: %-23s |\n",
           result.void_override ? "YES" : "NO");
    printf("  |                                          |\n");
    printf("  |  Issued by: Farland Council of One       |\n");
    printf("  |  Valid until: The void decides otherwise |\n");
    printf("  |  Appeal process: None                    |\n");
    printf("  |                                          |\n");
    printf("  +==========================================+\n");
    printf("\n");

    free(source);
}

void sfc_revoke_certification(const char *filename)
{
    printf("\n");
    printf("  +==========================================+\n");
    printf("  |     STANDAR FARLAND'S CODE               |\n");
    printf("  |     CERTIFICATE REVOCATION               |\n");
    printf("  |                                          |\n");
    printf("  |  File: %-32s |\n", filename);
    printf("  |  Status: REVOKED                         |\n");
    printf("  |  Reason: The void decided.               |\n");
    printf("  |  Appeal: None.                           |\n");
    printf("  |                                          |\n");
    printf("  +==========================================+\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    srand((unsigned int)time(NULL));

    if (argc < 2) {
        printf("SFC Certifier v%s\n", SFC_VERSION_STRING);
        printf("Usage: %s <source_file>\n", argv[0]);
        printf("       %s --revoke <source_file>\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "--revoke") == 0) {
        if (argc < 3)
            return 1;
        sfc_revoke_certification(argv[2]);
    } else {
        sfc_certify(argv[1]);
    }

    return 0;
}
