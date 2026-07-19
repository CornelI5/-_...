#include "FarlandRuntime.h"

void init_farland_runtime() {
    install_nopanic_handlers();
    
    activate_integer_desync_layer();
    activate_address_drift_engine();
    
    printf("[RUNTIME] Farland Kernel Initialized. Determinism: OFF.\n");
}
