
_SOUND=0
_PAL=1
_NTSC=0
_VESA=0
_MOUSE=0
_IOPRESET=1
EE_OBJS =\
	chase.o \
	cl_demo.o \
	cl_input.o \
	cl_main.o \
	cl_parse.o \
	cl_tent.o \
	cmd.o \
	common.o \
	console.o \
	crc.o \
	cvar.o \
	draw.o \
	d_edge.o \
	d_fill.o \
	d_init.o \
	d_modech.o \
	d_part.o \
	d_polyse.o \
	d_scan.o \
	d_sky.o \
	d_sprite.o \
	d_surf.o \
	d_vars.o \
	d_zpoint.o \
	host.o \
	host_cmd.o \
	keys.o \
	menu.o \
	mathlib.o \
	model.o \
	nonintel.o \
	pr_cmds.o \
	pr_edict.o \
	pr_exec.o \
	r_aclip.o \
	r_alias.o \
	r_bsp.o \
	r_light.o \
	r_draw.o \
	r_efrag.o \
	r_edge.o \
	r_misc.o \
	r_main.o \
	r_sky.o \
	r_sprite.o \
	r_surf.o \
	r_part.o \
	r_vars.o \
	screen.o \
	sbar.o \
	sv_main.o \
	sv_phys.o \
	sv_move.o \
	sv_user.o \
	zone.o	\
	view.o	\
	wad.o \
	world.o \
	cd_null.o \
	net_vcr.o \
	net_main.o \
	net_loop.o \
	net_none.o \
	sys_ps2.o \
	in_ps2.o \
	ps2_gs.o \
	vid_ps2.o \
    usbd_irx.o \
	usbhdfsd_irx.o \
	iomanx_irx.o \
	pad.o 
	
EE_BIN = bin/quake.elf

EE_LIBS =   -lmouse -lkbd -lm -lc -lpad -lkernel
EE_INCS :=  -I$(PS2SDK)/sbv/include -I$(PS2SDK)/ports/include/SDL -I$(PS2SDK)/ee/include -I$(PS2DEV)/ee/ee/include
EE_CFLAGS = -g -Dstricmp=strcasecmp -funroll-loops -fomit-frame-pointer -fexpensive-optimizations
EE_LDFLAGS := -L$(PS2SDK)/sbv/lib -L$(PS2SDK)/ports/lib

ifeq ($(_SOUND), 1)
	 EE_CFLAGS += -D_SOUND -DSDL
	 EE_LIBS += -lsdl -lsdlmain
	 EE_OBJS += snd_sdl.o \
                snd_mix.o \
	            snd_dma.o \
	            snd_mem.o 
endif
ifeq ($(_SOUND), 0)
	 EE_OBJS += snd_null.o
endif
ifeq ($(_PAL), 1)
	 EE_CFLAGS += -D_PAL
endif
ifeq ($(_NTSC), 1)
	 EE_CFLAGS += -D_NTSC
endif
ifeq ($(_VESA), 1)
	 EE_CFLAGS += -D_VESA
endif
ifeq ($(_MOUSE), 1)
	 EE_CFLAGS += -D_MOUSE
endif
ifeq ($(_IOPRESET), 1)
	 EE_CFLAGS += -D_IOPRESET
endif

BIN2S = $(PS2SDK)/bin/bin2s

all: $(EE_BIN)
#	 ps2_packer/ps2_packer bin/quake.elf bin/packed_quake.elf
clean:
	rm -f $(EE_BIN) $(EE_OBJS) $(EE_OBJS +)

run:
	ps2client -h 10.0.0.10 execee host:$(EE_BIN)

iomanx_irx.s: $(PS2SDK)/iop/irx/iomanX.irx
	$(BIN2S) $< $@ iomanx_irx

usbd_irx.s: $(PS2SDK)/iop/irx/usbd.irx
	$(BIN2S) $< $@ usbd_irx

usbhdfsd_irx.s: $(PS2SDK)/iop/irx/usbhdfsd.irx
	$(BIN2S) $< $@ usbhdfsd_irx	
	
include $(PS2SDK)/samples/Makefile.pref
include $(PS2SDK)/samples/Makefile.eeglobal