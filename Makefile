###############################################################################
### Makefile for the Whistle Detector                                       ###
### Author: Thomas Hamboeck, Austrian Kangaroos 2014                        ###
###############################################################################

OUTPUT          = main
LIBRARIES       = fftw3f asound boost_system
SRC_FOLDER      = src

GXX_LIBRARIES   = $(patsubst %,-l%,$(LIBRARIES))

ifeq ($(NOCOLOR),1)
CYAN            = ""
NORMAL          = ""
else
CYAN            = `tput setaf 6`
NORMAL          = `tput sgr0`
endif

WARN_FLAGS      = -Wall -Wextra
WARN_FLAGS     += -Wno-unused-parameter -Wno-ignored-qualifiers
STD             = -std=c++11
OPT             = -O3

GXX             = g++
MKDIR           = mkdir -p
RM              = rm -fr

RECURSIVE_FIND  = $(shell find $(1) -name '$(2)')
SOURCES         = $(call RECURSIVE_FIND,$(SRC_FOLDER),*.cpp)
OBJECTS         = $(patsubst %.cpp,%.o,$(SOURCES))

COMPILE         = $(GXX) $(STD) $(OPT) $(CXXFLAGS) $(DEFINES) $(WARN_FLAGS)
LINK            = $(GXX) $(LDFLAGS)

.PHONY: all run clean

all: $(OUTPUT)

$(OUTPUT): $(OBJECTS)
	@echo "Linking    "$(CYAN)$@$(NORMAL)
	@$(LINK) -o $(OUTPUT) $(OBJECTS) $(GXX_LIBRARIES)

%.o: %.cpp
	@echo "Compiling  "$(CYAN)$<$(NORMAL)
	@$(COMPILE) -c -o $@ $<

run: $(OUTPUT)
	@./$(OUTPUT)

clean:
	@$(RM) $(OUTPUT) $(OBJECTS)
