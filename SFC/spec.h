#ifndef SFC_SPEC_H
#define SFC_SPEC_H

#include <stdint.h>
#include <stdbool.h>

#define SFC_VERSION_MAJOR   1
#define SFC_VERSION_MINOR   0
#define SFC_VERSION_PATCH   0
#define SFC_VERSION_STRING  "1.0.0"

/* Rule 1: The Dash Rule */
#define DASH_MODE_SUBTRACTION   0
#define DASH_MODE_MACRO         1
#define DASH_MODE_DECAY         2
#define DASH_MODE_THREAT        3
#define DASH_MODE_IDENTITY      4
#define DASH_MODE_UNKNOWN       5

/* Rule 2: The 1024-Bit Mandate */
#define SFC_REGISTER_BITS       1024
#define SFC_REGISTER_BLOCKS     16
#define SFC_BITS_PER_BLOCK      64
#define SFC_64BIT_SHAME_LEVEL   1

typedef struct {
    uint64_t blocks[SFC_REGISTER_BLOCKS];
} sfc_int1024_t;

/* Rule 3: The Decay Principle */
#define DECAY_TRIGGER           "---"
#define DECAY_ZOMBIE_VALUE      0xDEADBEEF
#define DECAY_ZOMBIE_VALUE_64   0xDEADBEEFDEADBEEFULL
#define DECAY_RATE_MIN          0.0f
#define DECAY_RATE_MAX          1.0f

/* Rule 4: The Paranoia Clause */
#define PARANOIA_THRESHOLD      0.75f
#define PARANOIA_THREAT_TOKEN   "INTRUSION_DETECTED"

/* Rule 5: The No-Panic Doctrine */
#define NOPANIC_INTERCEPTED     0
#define NOPANIC_IGNORED         1
#define NOPANIC_GASLIT          2

/* Rule 6: The Complexity Paradox */
#define SFC_MIN_LINES_FOR_ADD   400
#define SFC_MAX_LINES_FOR_LIFE  1
#define SFC_COMPLEXITY_FLOOR    0
#define SFC_COMPLEXITY_CEILING  0

/* Rule 7: The Void Clause */
#define VOID_AUTHORITY          1
#define DEVELOPER_AUTHORITY     0
#define VOID_APPEAL_PROCESS     0

/* Compliance Levels */
typedef enum {
    SFC_LEVEL_UNAWARE   = 0,
    SFC_LEVEL_COMPLIANT = 1,
    SFC_LEVEL_EMBRACING = 2,
    SFC_LEVEL_ASCENDED  = 3,
    SFC_LEVEL_VOID      = 4,
} sfc_compliance_level_t;

/* Validation Result */
typedef struct {
    sfc_compliance_level_t level;
    int rules_passed;
    int rules_failed;
    int rules_confused;
    int total_checks;
    bool certified;
    bool void_override;
} sfc_validation_result_t;

/* Validator */
sfc_validation_result_t sfc_validate_source(const char *source);
int sfc_check_rule_1(const char *source);
int sfc_check_rule_2(const char *source);
int sfc_check_rule_3(const char *source);
int sfc_check_rule_4(const char *source);
int sfc_check_rule_5(const char *source);
int sfc_check_rule_6(const char *source);
int sfc_check_rule_7(const char *source);

/* Certifier */
void sfc_certify(const char *filename);
void sfc_revoke_certification(const char *filename);

#endif /* SFC_SPEC_H */
