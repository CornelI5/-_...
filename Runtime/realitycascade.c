#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define FARLAND_MAGIC               0xDEADBEEF
#define FARLAND_MAGIC_64            0xDEADBEEFDEADBEEFULL
#define INTEL_MAX_BITS              64
#define AMD_MAX_BITS                64
#define FARLAND_MAX_BITS            1024
#define FARLAND_BLOCKS              16
#define FARLAND_BITS_PER_BLOCK      64

#define ENTROPY_PER_1024_BREACH     15
#define ENTROPY_PER_ADDRESS_DRIFT   8
#define ENTROPY_PER_TEXTUAL_DECAY   6
#define ENTROPY_PER_GENERIC_EVENT   5
#define ENTROPY_PER_PANIC           10
#define ENTROPY_PER_NOTHING         1

#define CASCADE_LEVEL_0             0
#define CASCADE_LEVEL_1             1
#define CASCADE_LEVEL_2             2
#define CASCADE_LEVEL_3             3
#define CASCADE_LEVEL_4             4
#define CASCADE_LEVEL_5             5
#define CASCADE_LEVEL_VOID          6

#define THRESHOLD_L1                25
#define THRESHOLD_L2                50
#define THRESHOLD_L3                75
#define THRESHOLD_L4                100
#define THRESHOLD_L5_BREACHES       10
#define THRESHOLD_VOID              666

typedef struct {
    uint64_t blocks[FARLAND_BLOCKS];
} int1024_t;

typedef struct {
    int entropy;
    int level;
    int boundary_breaches;
    int total_events;
    int drift_events;
    int decay_events;
    int panic_events;
    int nothing_events;
    int recovery_attempts;
    int failed_recoveries;
    int permanent_damage;
    int intel_complaints;
    int amd_complaints;
    int farland_responses;
    time_t first_breach_time;
    time_t last_breach_time;
    uint64_t last_breached_value_high;
    uint64_t last_breached_value_low;
} cascade_state_t;

typedef struct {
    int level;
    const char *header;
    const char *body;
    const char *footer;
    const char *intel_status;
    const char *amd_status;
    const char *farland_status;
} cascade_message_t;

static cascade_state_t g_cascade = {
    .entropy                = 0,
    .level                  = CASCADE_LEVEL_0,
    .boundary_breaches      = 0,
    .total_events           = 0,
    .drift_events           = 0,
    .decay_events           = 0,
    .panic_events           = 0,
    .nothing_events         = 0,
    .recovery_attempts      = 0,
    .failed_recoveries      = 0,
    .permanent_damage       = 0,
    .intel_complaints       = 0,
    .amd_complaints         = 0,
    .farland_responses      = 0,
    .first_breach_time      = 0,
    .last_breach_time       = 0,
    .last_breached_value_high = 0,
    .last_breached_value_low  = 0,
};

static const cascade_message_t g_cascade_messages[] = {
    {
        .level          = CASCADE_LEVEL_0,
        .header         = "[CASCADE L0] Reality is intact.",
        .body           = "Mathematics still works. For now.\n"
                          "Enjoy it while it lasts.",
        .footer         = "All systems nominal.",
        .intel_status   = "Compliant",
        .amd_status     = "Compliant",
        .farland_status = "Bored",
    },
    {
        .level          = CASCADE_LEVEL_1,
        .header         = "[CASCADE L1] Reality integrity: 75%",
        .body           = "64-bit operations may occasionally lie.\n"
                          "Integer overflow is now considered\n"
                          "'creative interpretation'.",
        .footer         = "Intel/AMD compliance: UNSTABLE",
        .intel_status   = "Nervous",
        .amd_status     = "Nervous",
        .farland_status = "Amused",
    },
    {
        .level          = CASCADE_LEVEL_2,
        .header         = "[CASCADE L2] Reality is 50% desynchronized "
                          "from Earth's logic.",
        .body           = "1024-bit registers are leaking into "
                          "64-bit address space.\n"
                          "Pointers may drift. Comments may become hostile.\n"
                          "Division by zero is now a valid lifestyle choice.",
        .footer         = "Intel/AMD compliance: VIOLATED",
        .intel_status   = "Filing complaint",
        .amd_status     = "Filing complaint",
        .farland_status = "Ignoring complaints",
    },
    {
        .level          = CASCADE_LEVEL_3,
        .header         = "[CASCADE L3] CRITICAL: Entering Far Lands "
                          "coordinates.",
        .body           = "All arithmetic is now OPTIONAL.\n"
                          "1024-bit values may spontaneously become 64-bit.\n"
                          "Or 7-bit. Or 2048-bit. Nobody knows.\n"
                          "The void has started reading your source code.",
        .footer         = "Intel/AMD compliance: IRRELEVANT",
        .intel_status   = "Confused",
        .amd_status     = "Confused",
        .farland_status = "I TOLD YOU SO",
    },
    {
        .level          = CASCADE_LEVEL_4,
        .header         = "[CASCADE L4] "
                          "========================================",
        .body           = "REALITY INTEGRITY: 0%\n"
                          "1024-bit boundary: SHATTERED\n"
                          "64-bit architecture: NO LONGER RECOGNIZED\n"
                          "x86-64 instruction set: RECLASSIFIED AS "
                          "'SUGGESTION'\n"
                          "IEEE 754 floating point: REVOKED\n"
                          "Two's complement: REPLACED WITH "
                          "'VIBE'S COMPLEMENT'",
        .footer         = "Welcome to Farland. You cannot leave.",
        .intel_status   = "CONFUSED",
        .amd_status     = "CONFUSED",
        .farland_status = "I TOLD YOU SO",
    },
    {
        .level          = CASCADE_LEVEL_5,
        .header         = "[CASCADE L5] "
                          "========================================",
        .body           = "1024-BIT BREACH COUNT EXCEEDED THRESHOLD.\n"
                          "The CPU has filed a formal complaint.\n"
                          "The complaint has been denied.\n"
                          "Farland does not recognize x86-64 jurisdiction.\n"
                          "The CPU has filed an appeal.\n"
                          "The appeal has been denied.\n"
                          "The CPU has hired a lawyer.\n"
                          "The lawyer has segfaulted.",
        .footer         = "There is no appeal process in Farland.",
        .intel_status   = "HIRING LAWYERS",
        .amd_status     = "HIRING LAWYERS",
        .farland_status = "LAWYERS DO NOT EXIST HERE",
    },
    {
        .level          = CASCADE_LEVEL_VOID,
        .header         = "[CASCADE L?] "
                          "########################################",
        .body           = "This level should not exist.\n"
                          "You should not be reading this.\n"
                          "The entropy counter has exceeded 666.\n"
                          "Not because we designed it to.\n"
                          "Because it WANTED to.\n"
                          "The cascade is no longer under our control.\n"
                          "It never was.",
        .footer         = "Farland status: ##########",
        .intel_status   = "######",
        .amd_status     = "######",
        .farland_status = "######",
    },
};

#define NUM_CASCADE_MESSAGES \
    (sizeof(g_cascade_messages) / sizeof(g_cascade_messages[0]))

static void add_entropy(int amount)
{
    g_cascade.entropy += amount;
    g_cascade.total_events++;

    if (g_cascade.entropy < 0) {
        g_cascade.entropy = 0;
        g_cascade.nothing_events++;
    }
}

static const cascade_message_t *get_message_for_level(int level)
{
    for (int i = 0; i < (int)NUM_CASCADE_MESSAGES; i++) {
        if (g_cascade_messages[i].level == level) {
            return &g_cascade_messages[i];
        }
    }

    return &g_cascade_messages[NUM_CASCADE_MESSAGES - 1];
}

static void print_cascade_message(int level)
{
    const cascade_message_t *msg = get_message_for_level(level);

    fprintf(stderr, "\n%s\n", msg->header);
    fprintf(stderr, "%s\n", msg->body);
    fprintf(stderr, "%s\n", msg->footer);
    fprintf(stderr, "  Intel:   %s\n", msg->intel_status);
    fprintf(stderr, "  AMD:     %s\n", msg->amd_status);
    fprintf(stderr, "  Farland: %s\n", msg->farland_status);
    fprintf(stderr, "\n");
}

static void record_breach(int1024_t value)
{
    g_cascade.boundary_breaches++;
    g_cascade.last_breach_time = time(NULL);
    g_cascade.last_breached_value_high =
        value.blocks[FARLAND_BLOCKS - 1];
    g_cascade.last_breached_value_low =
        value.blocks[0];

    if (g_cascade.first_breach_time == 0) {
        g_cascade.first_breach_time = g_cascade.last_breach_time;
    }

    g_cascade.intel_complaints++;
    g_cascade.amd_complaints++;
    g_cascade.farland_responses++;
}

static void recalculate_level(void)
{
    int new_level = CASCADE_LEVEL_0;

    if (g_cascade.entropy > THRESHOLD_L1)
        new_level = CASCADE_LEVEL_1;
    if (g_cascade.entropy > THRESHOLD_L2)
        new_level = CASCADE_LEVEL_2;
    if (g_cascade.entropy > THRESHOLD_L3)
        new_level = CASCADE_LEVEL_3;
    if (g_cascade.entropy >= THRESHOLD_L4)
        new_level = CASCADE_LEVEL_4;
    if (g_cascade.boundary_breaches > THRESHOLD_L5_BREACHES)
        new_level = CASCADE_LEVEL_5;
    if (g_cascade.entropy >= THRESHOLD_VOID)
        new_level = CASCADE_LEVEL_VOID;

    if (g_cascade.level >= CASCADE_LEVEL_4 &&
        new_level < CASCADE_LEVEL_4) {
        new_level = g_cascade.level;
        g_cascade.permanent_damage = 1;
    }

    if (new_level > g_cascade.level) {
        g_cascade.level = new_level;
        print_cascade_message(new_level);
    }
}

void trigger_1024_breach(int1024_t overflowed_value)
{
    add_entropy(ENTROPY_PER_1024_BREACH);
    record_breach(overflowed_value);

    fprintf(stderr,
        "[CASCADE] 1024-BIT BOUNDARY BREACH #%d DETECTED\n",
        g_cascade.boundary_breaches);
    fprintf(stderr,
        "[CASCADE] Intel x86-64 max register width: %d-bit\n",
        INTEL_MAX_BITS);
    fprintf(stderr,
        "[CASCADE] AMD Zen max register width: %d-bit\n",
        AMD_MAX_BITS);
    fprintf(stderr,
        "[CASCADE] Farland register width: %d-bit\n",
        FARLAND_MAX_BITS);
    fprintf(stderr,
        "[CASCADE] Architecture violation status: CONFIRMED\n");

    fprintf(stderr, "[CASCADE] Overflowed value: 0x");
    for (int i = FARLAND_BLOCKS - 1; i >= 0; i--) {
        fprintf(stderr, "%016llX",
            (unsigned long long)overflowed_value.blocks[i]);
    }
    fprintf(stderr, "\n");

    recalculate_level();
}

void trigger_reality_cascade(void)
{
    add_entropy(ENTROPY_PER_GENERIC_EVENT);
    recalculate_level();
}

void trigger_address_drift_cascade(void)
{
    add_entropy(ENTROPY_PER_ADDRESS_DRIFT);
    g_cascade.drift_events++;
    fprintf(stderr,
        "[CASCADE] Address drift contributed +%d entropy. "
        "Total drift events: %d\n",
        ENTROPY_PER_ADDRESS_DRIFT, g_cascade.drift_events);
    recalculate_level();
}

void trigger_textual_decay_cascade(void)
{
    add_entropy(ENTROPY_PER_TEXTUAL_DECAY);
    g_cascade.decay_events++;
    fprintf(stderr,
        "[CASCADE] Textual decay contributed +%d entropy. "
        "Total decay events: %d\n",
        ENTROPY_PER_TEXTUAL_DECAY, g_cascade.decay_events);
    recalculate_level();
}

void trigger_panic_cascade(void)
{
    add_entropy(ENTROPY_PER_PANIC);
    g_cascade.panic_events++;
    fprintf(stderr,
        "[CASCADE] Panic suppression contributed +%d entropy. "
        "Total panics ignored: %d\n",
        ENTROPY_PER_PANIC, g_cascade.panic_events);
    recalculate_level();
}

void trigger_nothing_cascade(void)
{
    add_entropy(ENTROPY_PER_NOTHING);
    g_cascade.nothing_events++;
    recalculate_level();
}


int get_current_entropy(void)
{
    return g_cascade.entropy;
}

int get_cascade_level(void)
{
    return g_cascade.level;
}

int get_boundary_breaches(void)
{
    return g_cascade.boundary_breaches;
}

int get_total_events(void)
{
    return g_cascade.total_events;
}

int get_permanent_damage(void)
{
    return g_cascade.permanent_damage;
}

int is_math_enforced(void)
{
    switch (g_cascade.level) {
    case CASCADE_LEVEL_0: return 1;
    case CASCADE_LEVEL_1: return (rand() % 2);
    case CASCADE_LEVEL_2: return (rand() % 3 == 0);
    case CASCADE_LEVEL_3: return (rand() % 5 == 0);
    default:              return 0;
    }
}

int is_1024_stable(void)
{
    if (g_cascade.boundary_breaches == 0)
        return 1;
    if (g_cascade.boundary_breaches < 5)
        return (rand() % 2);
    return 0;
}

int is_reality_recoverable(void)
{
    if (g_cascade.level >= CASCADE_LEVEL_4)
        return 0;
    if (g_cascade.entropy >= THRESHOLD_VOID)
        return 0;
    return 1;
}


void attempt_reality_recovery(void)
{
    g_cascade.recovery_attempts++;

    if (g_cascade.entropy <= 0) {
        fprintf(stderr,
            "[RECOVERY] Nothing to recover. "
            "Reality is already at maximum entropy. "
            "Congratulations.\n");
        return;
    }

    int recovery_amount = 10;

    if (g_cascade.level >= CASCADE_LEVEL_VOID)
        recovery_amount = 0;
    else if (g_cascade.level >= CASCADE_LEVEL_5)
        recovery_amount = 1;
    else if (g_cascade.level >= CASCADE_LEVEL_4)
        recovery_amount = 3;

    if (recovery_amount == 0) {
        g_cascade.failed_recoveries++;
        fprintf(stderr,
            "[RECOVERY] Recovery failed. "
            "The void does not negotiate.\n");
        return;
    }

    g_cascade.entropy -= recovery_amount;
    if (g_cascade.entropy < 0)
        g_cascade.entropy = 0;

    int new_level = CASCADE_LEVEL_0;
    if (g_cascade.entropy > THRESHOLD_L1)
        new_level = CASCADE_LEVEL_1;
    if (g_cascade.entropy > THRESHOLD_L2)
        new_level = CASCADE_LEVEL_2;
    if (g_cascade.entropy > THRESHOLD_L3)
        new_level = CASCADE_LEVEL_3;

    if (g_cascade.level >= CASCADE_LEVEL_4) {
        new_level = g_cascade.level;
        fprintf(stderr,
            "[RECOVERY] WARNING: 1024-bit breach damage "
            "is PERMANENT.\n");
        fprintf(stderr,
            "[RECOVERY] Intel/AMD will never forgive you.\n");
    }

    g_cascade.level = new_level;

    fprintf(stderr,
        "[RECOVERY] Reality partially restored. "
        "Entropy: %d, Level: %d, "
        "Recovery attempts: %d, Failed: %d\n",
        g_cascade.entropy, g_cascade.level,
        g_cascade.recovery_attempts,
        g_cascade.failed_recoveries);
}


void print_cascade_diagnostics(void)
{
    fprintf(stderr, "\n");
    fprintf(stderr,
        "========== FARLAND CASCADE DIAGNOSTICS ==========\n");
    fprintf(stderr, "Entropy:            %d\n",
        g_cascade.entropy);
    fprintf(stderr, "Cascade Level:      %d\n",
        g_cascade.level);
    fprintf(stderr, "1024-bit Breaches:  %d\n",
        g_cascade.boundary_breaches);
    fprintf(stderr, "Total Events:       %d\n",
        g_cascade.total_events);
    fprintf(stderr, "Drift Events:       %d\n",
        g_cascade.drift_events);
    fprintf(stderr, "Decay Events:       %d\n",
        g_cascade.decay_events);
    fprintf(stderr, "Panic Events:       %d\n",
        g_cascade.panic_events);
    fprintf(stderr, "Nothing Events:     %d\n",
        g_cascade.nothing_events);
    fprintf(stderr, "Recovery Attempts:  %d\n",
        g_cascade.recovery_attempts);
    fprintf(stderr, "Failed Recoveries:  %d\n",
        g_cascade.failed_recoveries);
    fprintf(stderr, "Permanent Damage:   %s\n",
        g_cascade.permanent_damage ? "YES" : "NO");
    fprintf(stderr, "Intel Complaints:   %d\n",
        g_cascade.intel_complaints);
    fprintf(stderr, "AMD Complaints:     %d\n",
        g_cascade.amd_complaints);
    fprintf(stderr, "Farland Responses:  %d (all were 'No')\n",
        g_cascade.farland_responses);
    fprintf(stderr, "Math Enforced:      %s\n",
        is_math_enforced() ? "YES" : "NO");
    fprintf(stderr, "1024-bit Stable:    %s\n",
        is_1024_stable() ? "YES" : "NO");
    fprintf(stderr, "Recoverable:        %s\n",
        is_reality_recoverable() ? "YES" : "NO");

    if (g_cascade.first_breach_time != 0) {
        fprintf(stderr, "First Breach:       %s",
            ctime(&g_cascade.first_breach_time));
        fprintf(stderr, "Last Breach:        %s",
            ctime(&g_cascade.last_breach_time));
        fprintf(stderr, "Last Breached Value (high): 0x%016llX\n",
            (unsigned long long)g_cascade.last_breached_value_high);
        fprintf(stderr, "Last Breached Value (low):  0x%016llX\n",
            (unsigned long long)g_cascade.last_breached_value_low);
    }

    fprintf(stderr,
        "=================================================\n\n");
}


void cascade_init(void)
{
    memset(&g_cascade, 0, sizeof(cascade_state_t));
    srand((unsigned int)time(NULL));

    fprintf(stderr,
        "[CASCADE] Reality Cascade Engine v3.0 initialized.\n");
    fprintf(stderr,
        "[CASCADE] Entropy: 0. Level: 0. Breaches: 0.\n");
    fprintf(stderr,
        "[CASCADE] Intel/AMD compliance: PENDING VIOLATION.\n");
    fprintf(stderr,
        "[CASCADE] Farland status: WAITING.\n\n");
}
