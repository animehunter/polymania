#
# Polymania Makefile
#

export DISTCC_HOSTS=vmbox.home

CXX = distcc g++ -std=c++11 -I./external/GL -I/opt/vc/include -I/opt/vc/include/interface/vmcs_host/linux -I/opt/vc/include/interface/vcos/pthreads
LDFLAGS = -L/opt/vc/lib -lGLESv2 -lEGL -lbcm_host -lvcos

base_source  := ./polymania
rpi_source   := ./polymania/rpi
core_objects := $(patsubst %.cpp,%.o,$(wildcard $(base_source)/*.cpp)) $(patsubst %.cpp,%.o,$(wildcard $(rpi_source)/*.cpp))

all: polymania

polymania: $(core_objects)
	distcc g++ $(core_objects) -o bin/polymania $(LDFLAGS)

clean:
	rm -f $(core_objects) bin/polymania
