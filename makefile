CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -I./include
LDFLAGS  := -lraylib -lGL -lm -lpthread -ldl -lrt -lX11
TARGET   := linklang
SRCDIR   := src
OBJDIR   := obj

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

.PHONY: all build clean run debug release install

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	@echo "Linking $(TARGET)..."
	@$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@ 2>> error.txt && \
		echo "Build successful!" || \
		(echo "Build failed — see error.txt" && exit 1)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@echo "  Compiling $<..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@ 2>> error.txt

$(OBJDIR):
	@mkdir -p $(OBJDIR)

debug: CXXFLAGS += -g -O0 -DDEBUG
debug: build

release: CXXFLAGS += -O2 -DNDEBUG
release: build

run: build
	@./$(TARGET)

install: release
	@cp $(TARGET) /usr/local/bin/$(TARGET)
	@echo "Installed to /usr/local/bin/$(TARGET)"

clean:
	@rm -rf $(OBJDIR) $(TARGET) error.txt
	@echo "Cleaned."
