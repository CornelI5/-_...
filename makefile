CC = gcc
CFLAGS = -O0 -w 
SRC = runtime/NoPanic.c \
      runtime/IntegerDesync.c \
      runtime/AddressDrift.c \
      runtime/TextualDecay.c \
      runtime/RealityCascade.c \
      runtime/VoidRecovery.c \
      runtime/HashValidator.c \
      runtime/FarlandRuntime.c

TARGET = farland_vm

all: $(TARGET)
	@echo "=================================================="
	@echo "[COMPILER] Binary 'from Farland' successfully generated."
	@echo "[WARNING] Determinism check has been bypassed."
	@echo "[WARNING] Any attempt to debug this binary will accelerate Reality Cascade."
	@echo "=================================================="

$(TARGET):
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)
	@echo "[VOID] Traces of Farland removed from Earth's storage."
