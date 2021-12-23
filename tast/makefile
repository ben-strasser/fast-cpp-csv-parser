CXX = g++ -std=c++11

TARGET = tastme.exe
SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp,%.o,$(SRC))
INC += -I..

CXXFLAGS += -g
CXXFLAGS += $(INC)

$(TARGET) : $(OBJ)
	$(CXX) -o $@ $^ -lpthread
