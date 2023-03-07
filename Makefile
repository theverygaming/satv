CXXFLAGS += -O3 -Wall -Wextra 
LDFLAGS += 

CXXFILES = src/tx.cpp

OBJS = $(addsuffix .o, $(basename $(CXXFILES)))

.PHONY: all
all: vid_tx

vid_tx: $(OBJS)
	$(CXX) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) -o $@ -c $< $(CFLAGS)

clean:
	rm -rf vid_tx $(OBJS)
