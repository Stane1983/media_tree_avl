
#	Meson DVB drivers

obj-$(CONFIG_MESON_DVB) += aml_fe.o
obj-$(CONFIG_AVL6261) += aml_fe_avl62x1_av201x.o aml_fe_avl68x2_r848.o r848a.o

aml_fe-objs    = aml_dvb.o aml_dmx.o 
 

EXTRA_CFLAGS += -I. -DDUAL_TUNER
EXTRA_CFLAGS += -Idrivers/media/dvb-core -Idrivers/media/usb/dvb-usb -Idrivers/media/dvb-frontends -Idrivers/media/tuners
EXTRA_CFLAGS += -Idrivers/media/platform/meson/dvb/dvb-frontends-availink/availink/avl62x1/
EXTRA_CFLAGS += -Idrivers/media/platform/meson/dvb/dvb-frontends-availink/availink/avl68x2/