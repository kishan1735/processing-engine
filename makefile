CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3 -march=native -flto -DNDEBUG -fprofile-use
DRIVER_SRC = driver.cpp
ENGINE_SRC = engine.cpp
BUILD_DIR = build
TARGET_DRIVER = $(BUILD_DIR)/driver
TARGET_ENGINE = $(BUILD_DIR)/engine
LIBS = -lzmq -lpthread

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
	@rm -f $(BUILD_DIR)/engine
	@rm -f $(BUILD_DIR)/driver
	@rm -f output.txt
	@echo "Cleanup complete."

