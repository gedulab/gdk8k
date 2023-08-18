WFLAGS := -Wstrict-prototypes -Wno-trigraphs -Wunused-result

LDFLAGS = -Map /var/tmp/lllmap.txt

EXTRA_CFLAGS := $(WFLAGS)

# EXTRA_CFLAGS += -g -Wa,-adhln=$(<:.c=.lst)

EXTRA_CFLAGS += -D_DEBUG -g3  -fno-stack-protector

MODULE = llaolao

obj-$(CONFIG_LLAOLAOLLL) += $(MODULE).o 

all:
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(shell pwd) modules

$(MODULE)-objs := main.o gearm.o

clean:
	rm -rf *.o *~ .*.cmd *.ko *.mod.c *.order *.symvers .tmp_versions built-in.o
