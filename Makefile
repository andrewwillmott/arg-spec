TARGET = ArgSpecExample
COMPILE_FLAGS = -std=c++11

all: $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CXX) $(COMPILE_FLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)
