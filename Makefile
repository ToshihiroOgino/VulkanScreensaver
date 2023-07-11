CPPCOMPILER = g++
# CFLAGS    = -g -MMD -MP -Wall -Wextra -Winit-self -Wno-missing-field-initializers
CFLAGS		= -g -rdynamic -std=c++20 -Wall -Wno-unknown-pragmas
LDFLAGS		= -lglfw -lvulkan -ldl -lpthread -lX11 -lXxf86vm -lXrandr -lXi

SRCDIR		= ./src
OBJDIR    	= ./obj
BINDIR		= ./bin
SUBMODULE	= ./external
INCLUDE   	= -I$(SUBMODULE)/glm -I$(SUBMODULE)/stb -I$(SUBMODULE)/glfw/include -I$(SUBMODULE)/tinyobjloader -I$(SUBMODULE)/Vulkan-Headers/include

$(shell mkdir -p $(OBJDIR))
$(shell mkdir -p $(BINDIR))

TARGET    	= ./bin/$(shell basename `readlink -f .`)

SOURCES   	= $(wildcard $(SRCDIR)/*.cpp)
OBJECTS		= $(addprefix $(OBJDIR)/, $(notdir $(SOURCES:.cpp=.o)))
DEPENDS   	= $(OBJECTS:.o=.d)

$(TARGET): $(OBJECTS) $(LIBS)
	$(CPPCOMPILER) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	-mkdir -p $(OBJDIR)
	$(CPPCOMPILER) $(CFLAGS) $(INCLUDE) -o $@ -c $<

all: clean $(TARGET)
	mkdir -p $(BINDIR)

clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)

ARG = ./scripts/sample

run:
	$(TARGET) $(ARG)

-include $(DEPENDS)
