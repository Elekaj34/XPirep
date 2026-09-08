BUILDDIR	:=	./build
SRC_BASE	:=	.
TARGET		:= XPirep

SOURCES = \
	XPirep.cpp

LIBS =

INCLUDES = \
	-I$(SRC_BASE)/SDK/CHeaders/XPLM \
	-I$(SRC_BASE)/SDK/CHeaders/Widgets \
	-I$(SRC_BASE)/lvgl

############################################################################
# Platform definitions

LIN_DEFINES = \
	-DXPLM200=1 \
	-DXPLM210=1 \
	-DXPLM300=1 \
	-DXPLM301=1 \
	-DAPL=0 \
	-DIBM=0 \
	-DLIN=1

WIN_DEFINES = \
	-DXPLM200=1 \
	-DXPLM210=1 \
	-DXPLM300=1 \
	-DXPLM301=1 \
	-DAPL=0 \
	-DIBM=1 \
	-DLIN=0

############################################################################

VPATH = $(SRC_BASE)

CSOURCES	:= $(filter %.c, $(SOURCES))
CXXSOURCES	:= $(filter %.cpp, $(SOURCES))

############################################################################
# Linux

LIN_OBJDIR	:= $(BUILDDIR)/obj64/linux
LIN_BINDIR	:= $(BUILDDIR)/$(TARGET)/64

LIN_COBJECTS	:= $(patsubst %.c,$(LIN_OBJDIR)/%.o,$(CSOURCES))
LIN_CXXOBJECTS	:= $(patsubst %.cpp,$(LIN_OBJDIR)/%.o,$(CXXSOURCES))
LIN_OBJECTS	:= $(sort $(LIN_COBJECTS) $(LIN_CXXOBJECTS))

LIN_CDEPS	:= $(patsubst %.c,$(LIN_OBJDIR)/%.cdep,$(CSOURCES))
LIN_CXXDEPS	:= $(patsubst %.cpp,$(LIN_OBJDIR)/%.cppdep,$(CXXSOURCES))
LIN_DEPS	:= $(sort $(LIN_CDEPS) $(LIN_CXXDEPS))

LIN_CFLAGS	:= $(LIN_DEFINES) $(INCLUDES) -fPIC -fvisibility=hidden

############################################################################
# Windows

WIN_OBJDIR	:= $(BUILDDIR)/obj64/windows
WIN_BINDIR	:= $(BUILDDIR)/$(TARGET)/64

WIN_COBJECTS	:= $(patsubst %.c,$(WIN_OBJDIR)/%.o,$(CSOURCES))
WIN_CXXOBJECTS	:= $(patsubst %.cpp,$(WIN_OBJDIR)/%.o,$(CXXSOURCES))
WIN_OBJECTS	:= $(sort $(WIN_COBJECTS) $(WIN_CXXOBJECTS))

WIN_CDEPS	:= $(patsubst %.c,$(WIN_OBJDIR)/%.cdep,$(CSOURCES))
WIN_CXXDEPS	:= $(patsubst %.cpp,$(WIN_OBJDIR)/%.cppdep,$(CXXSOURCES))
WIN_DEPS	:= $(sort $(WIN_CDEPS) $(WIN_CXXDEPS))

WIN_CFLAGS	:= $(WIN_DEFINES) $(INCLUDES) -fvisibility=hidden

WIN_LIBDIR	:= $(SRC_BASE)/SDK/Libraries/Win
WIN_LIBS	:= $(WIN_LIBDIR)/XPLM_64.lib \
		   $(WIN_LIBDIR)/XPWidgets_64.lib \
		   -lopengl32

############################################################################
# Phony targets

.PHONY: all clean linux windows $(TARGET)

.SECONDARY: $(LIN_OBJECTS) $(LIN_DEPS) $(WIN_OBJECTS) $(WIN_DEPS)

############################################################################
# Default target

all: linux windows

$(TARGET): all

############################################################################
# Linux target

linux: $(LIN_BINDIR)/lin.xpl

$(LIN_BINDIR)/lin.xpl: $(LIN_OBJECTS)
	@echo Linking $@
	mkdir -p $(dir $@)
	g++ -m64 -static-libgcc -shared \
		-Wl,--version-script=exports.txt \
		-o $@ $(LIN_OBJECTS) $(LIBS)

############################################################################
# Windows target

windows: $(WIN_BINDIR)/win.xpl

$(WIN_BINDIR)/win.xpl: $(WIN_OBJECTS)
	@echo Linking $@
	mkdir -p $(dir $@)
	x86_64-w64-mingw32-g++ -m64 -static-libgcc -static-libstdc++ \
		-shared \
		-o $@ $(WIN_OBJECTS) $(WIN_LIBS)

############################################################################
# Linux compiler rules

$(LIN_OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	gcc $(LIN_CFLAGS) -m64 -c $< -o $@
	gcc $(LIN_CFLAGS) -MM -MT $@ -o $(@:.o=.cdep) $<

$(LIN_OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	g++ $(LIN_CFLAGS) -m64 -c $< -o $@
	g++ $(LIN_CFLAGS) -MM -MT $@ -o $(@:.o=.cppdep) $<

############################################################################
# Windows compiler rules

$(WIN_OBJDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	x86_64-w64-mingw32-gcc $(WIN_CFLAGS) -m64 -c $< -o $@
	x86_64-w64-mingw32-gcc $(WIN_CFLAGS) -MM -MT $@ -o $(@:.o=.cdep) $<

$(WIN_OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	x86_64-w64-mingw32-g++ $(WIN_CFLAGS) -m64 -c $< -o $@
	x86_64-w64-mingw32-g++ $(WIN_CFLAGS) -MM -MT $@ -o $(@:.o=.cppdep) $<

############################################################################
# Clean

clean:
	@echo Cleaning out everything.
	rm -rf $(BUILDDIR)

############################################################################
# Include dependency files

-include $(LIN_DEPS)
-include $(WIN_DEPS)