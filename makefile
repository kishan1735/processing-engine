CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3 -march=native -flto -DNDEBUG
DRIVER_SRC = driver.cpp
ENGINE_SRC = engine.cpp
BUILD_DIR = build
TARGET_DRIVER = $(BUILD_DIR)/driver
TARGET_ENGINE = $(BUILD_DIR)/engine
LIBS = -lzmq -lpthread

# --- STATIC LINKING (Optional) ---
# To use static linking:
# 1. Uncomment the lines below.
# 2. Comment out the LIBS variable in the "DYNAMIC LINKING" section above.
# 3. Ensure the paths to your compiled libzmq.a and headers are correct.
#
# CPPZMQ_INCLUDE_PATH = ./libs/cppzmq # Path to cppzmq header
# LIBZMQ_INCLUDE_PATH = ./libs/libzmq/include # Path to libzmq headers
# LIBZMQ_STATIC_LIB_PATH = ./libs/libzmq/build/lib/libzmq.a # Path to the static library
#
# CXXFLAGS += -I$(CPPZMQ_INCLUDE_PATH) -I$(LIBZMQ_INCLUDE_PATH)
# LIBS = $(LIBZMQ_STATIC_LIB_PATH) -lpthread -lstdc++

.PHONY: all clean
all: $(TARGET_DRIVER) $(TARGET_ENGINE)

$(TARGET_DRIVER): $(DRIVER_SRC)
	@echo "Compiling driver..."
	@mkdir -p $(BUILD_DIR) # Create the build directory if it doesn't exist
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)
	@echo "Successfully compiled: $@"

$(TARGET_ENGINE): $(ENGINE_SRC)
	@echo "Compiling engine..."
	@mkdir -p $(BUILD_DIR) # Create the build directory if it doesn't exist
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)
	@echo "Successfully compiled: $@"

clean:
	@echo "Cleaning up build artifacts..."
	@rm -rf $(BUILD_DUR)
	@rm -f output.txt
	@echo "Cleanup complete."

