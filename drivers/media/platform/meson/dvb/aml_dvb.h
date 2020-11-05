// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright (C) 2018-present Team CoreELEC (https://coreelec.org)

#ifndef _AML_DVB_H_
#define _AML_DVB_H_

#include <linux/interrupt.h>
#include <linux/socket.h>
#include <linux/netdevice.h>
#include <linux/i2c.h>

#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#include <linux/dvb/dmx.h>
#include <linux/dvb/ca.h>
#include <linux/dvb/osd.h>
#include <linux/dvb/net.h>
#include <linux/dvb/frontend.h>

#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/interrupt.h>

#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif


#include "media/dvbdev.h"
#include "media/demux.h"
#include "media/dvb_demux.h"
#include "media/dmxdev.h"
#include "media/dvb_net.h"
#include "media/dvb_ringbuffer.h"
#include "media/dvb_frontend.h"

#include <linux/of.h>
#include <linux/pinctrl/consumer.h>

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (a*1000+b*100+c)
#endif
#ifndef LINUX_VERSION_CODE
#define LINUX_VERSION_CODE 	KERNEL_VERSION(4, 20, 2)
#endif

#define TS_IN_COUNT       3
#define S2P_COUNT         2

#define DMX_DEV_COUNT     3
#define FE_DEV_COUNT      2
#define CHANNEL_COUNT     31
#define FILTER_COUNT      31
#define FILTER_LEN        15
#define SEC_BUF_GRP_COUNT 4
#define SEC_BUF_BUSY_SIZE 4
#define SEC_BUF_COUNT     (SEC_BUF_GRP_COUNT*8)
#define ASYNCFIFO_COUNT   2

#define TOTAL_AML_INPUTS 	2
#define BASE_IRQ 32
#define AM_IRQ(reg)             (reg + BASE_IRQ)
#define INT_DEMUX               AM_IRQ(23)
#define INT_DEMUX_1             AM_IRQ(5)
#define INT_DEMUX_2             AM_IRQ(53)
#define INT_ASYNC_FIFO_FILL     AM_IRQ(18)
#define INT_ASYNC_FIFO_FLUSH    AM_IRQ(19)
#define INT_ASYNC_FIFO2_FILL    AM_IRQ(24)
#define INT_ASYNC_FIFO2_FLUSH   AM_IRQ(25)

typedef enum dmx_source {
	DMX_SOURCE_FRONT0 = 0,
	DMX_SOURCE_FRONT1,
	DMX_SOURCE_FRONT2,
	DMX_SOURCE_FRONT3,
	DMX_SOURCE_DVR0   = 16,
	DMX_SOURCE_DVR1,
	DMX_SOURCE_DVR2,
	DMX_SOURCE_DVR3,
	DMX_SOURCE_FRONT0_OFFSET = 100,
	DMX_SOURCE_FRONT1_OFFSET,
	DMX_SOURCE_FRONT2_OFFSET
} dmx_source_t;

enum aml_dmx_id_t {
	AM_DMX_0 = 0,
	AM_DMX_1,
	AM_DMX_2,
	AM_DMX_MAX,
};

enum aml_ts_source_t {
	AM_TS_SRC_TS0,
	AM_TS_SRC_TS1,
	AM_TS_SRC_TS2,
	AM_TS_SRC_S_TS0,
	AM_TS_SRC_S_TS1,
	AM_TS_SRC_S_TS2,
	AM_TS_SRC_HIU,
	AM_TS_SRC_DMX0,
	AM_TS_SRC_DMX1,
	AM_TS_SRC_DMX2
};

struct aml_sec_buf {
	unsigned long        addr;
	int                  len;
};

struct aml_channel {
	int                  type;
	enum dmx_ts_pes	pes_type;
	int                  pid;
	int                  used;
	int                  filter_count;
	struct dvb_demux_feed     *feed;
	struct dvb_demux_feed     *dvr_feed;
};

struct aml_ch_ignore {
	int                  type;
	enum dmx_ts_pes	pes_type;
	int                  pid;
	int                  used;
};

struct aml_filter {
	int                  chan_id;
	int                  used;
	struct dmx_section_filter *filter;
	u8                   value[FILTER_LEN];
	u8                   maskandmode[FILTER_LEN];
	u8                   maskandnotmode[FILTER_LEN];
	u8                   neq;
};


struct aml_smallsec {
	struct aml_dmx *dmx;

	int	enable;
	int	bufsize;
#define SS_BUFSIZE_DEF (16*4*256) /*16KB*/
	long	buf;
	long	buf_map;
};

struct aml_dmxtimeout {
	struct aml_dmx *dmx;

	int	enable;

	int	timeout;
#define DTO_TIMEOUT_DEF (9000)       /*0.5s*/
	u32	ch_disable;
#define DTO_CHDIS_VAS   (0xfffffff8) /*v/a/s only*/
	int	match;

	int     trigger;
};

struct aml_dmx {
	struct dvb_demux     demux;
	struct dmxdev        dmxdev;
	int                  id;
	int                  feed_count;
	int                  chan_count;
	enum aml_ts_source_t      source;
	int                  init;
	int                  record;
	struct dmx_frontend  hw_fe[DMX_DEV_COUNT];
	struct dmx_frontend  mem_fe;
	struct dvb_net       dvb_net;
	int                  dmx_irq;
	int                  dvr_irq;
	struct tasklet_struct     dmx_tasklet;
	struct tasklet_struct     dvr_tasklet;
	unsigned long        sec_pages;
	unsigned long        sec_pages_map;
	int                  sec_total_len;
	struct aml_sec_buf   sec_buf[SEC_BUF_COUNT];
	unsigned long        pes_pages;
	unsigned long        pes_pages_map;
	int                  pes_buf_len;
	unsigned long        sub_pages;
	unsigned long        sub_pages_map;
	int                  sub_buf_len;
	struct aml_channel   channel[CHANNEL_COUNT];
	struct aml_filter    filter[FILTER_COUNT];
	irq_handler_t        irq_handler;
	void                *irq_data;
	int                  aud_chan;
	int                  vid_chan;
	int                  sub_chan;
	int                  pcr_chan;
	u32                  section_busy[SEC_BUF_BUSY_SIZE];
	struct dvb_frontend *fe;
	int                  int_check_count;
	u32                  int_check_time;
	int                  in_tune;
	int                  error_check;
	int                  dump_ts_select;
	int                  sec_buf_watchdog_count[SEC_BUF_COUNT];

	struct aml_smallsec  smallsec;
	struct aml_dmxtimeout timeout;

	int                  demux_filter_user;

	unsigned long sec_cnt[3];
	unsigned long sec_cnt_match[3];
	unsigned long sec_cnt_crc_fail[3];
	#define SEC_CNT_HW (0)
	#define SEC_CNT_SW (1)
	#define SEC_CNT_SS (2)
	#define SEC_CNT_MAX (3)

	int                   crc_check_count;
	u32                 crc_check_time;
	struct aml_ch_ignore ch_ignore[CHANNEL_COUNT];
};

struct aml_asyncfifo {
	int	id;
	int	init;
	enum aml_dmx_id_t	source;
	unsigned long	pages;
	unsigned long   pages_map;
	int	buf_len;
	int	buf_toggle;
	int buf_read;
	int flush_size;
	struct tasklet_struct     asyncfifo_tasklet;
	struct aml_dvb *dvb;
};

enum{
	AM_TS_DISABLE,
	AM_TS_PARALLEL,
	AM_TS_SERIAL
};

struct aml_ts_input {
	int                  mode;
	struct pinctrl      *pinctrl;
	int                  control;
	int                  s2p_id;
};

struct aml_s2p {
	int    invert;
};

struct aml_swfilter {
	int    user;
	struct aml_dmx *dmx;
	struct aml_asyncfifo *afifo;

	struct dvb_ringbuffer rbuf;
#define SF_BUFFER_SIZE (10*188*1024)

	u8     wrapbuf[188];
	int    track_dmx;
};

struct aml_dvb {
	struct dvb_device    	dvb_dev;
	struct aml_ts_input  	ts[TS_IN_COUNT];
	struct aml_s2p       	s2p[S2P_COUNT];
	struct aml_dmx       	dmx[DMX_DEV_COUNT];
	struct aml_asyncfifo 	asyncfifo[ASYNCFIFO_COUNT];
	struct dvb_adapter   	dvb_adapter;
	struct device       	*dev;
	struct platform_device 	*pdev;
	enum aml_ts_source_t 	stb_source;
	enum aml_ts_source_t 	tso_source;
	int                  	dmx_init;
	int                  	reset_flag;
	spinlock_t           	slock;
	struct timer_list    	watchdog_timer;
	int                  	dmx_watchdog_disable[DMX_DEV_COUNT];
	struct aml_swfilter  	swfilter;
	int		     	ts_out_invert;
	void __iomem 	     	*demux_base;
	void __iomem 	     	*afifo_base;
	int			demux_irq[DMX_DEV_COUNT];
	int			afifo_irq[ASYNCFIFO_COUNT];
	u32 			total_nims;
	struct dvb_frontend 	*fe[TS_IN_COUNT];
	struct i2c_adapter 	*i2c[TS_IN_COUNT];
	int 			fec_reset[TS_IN_COUNT];
	int 			power_ctrl[TS_IN_COUNT];
	int 			lock_led[TS_IN_COUNT];
	int			xtal[TS_IN_COUNT];
};


/*AMLogic demux interface*/
extern int aml_dmx_hw_init(struct aml_dmx *dmx);
extern int aml_dmx_hw_deinit(struct aml_dmx *dmx);
extern int aml_dmx_hw_start_feed(struct dvb_demux_feed *dvbdmxfeed);
extern int aml_dmx_hw_stop_feed(struct dvb_demux_feed *dvbdmxfeed);
extern int aml_dmx_hw_set_source(struct dmx_demux *demux, dmx_source_t src);
extern int aml_stb_hw_set_source(struct aml_dvb *dvb, dmx_source_t src);
extern int aml_tso_hw_set_source(struct aml_dvb *dvb, dmx_source_t src);

extern int aml_dmx_set_skipbyte(struct aml_dvb *dvb, int skipbyte);
extern int aml_dmx_hw_set_dump_ts_select
		(struct dmx_demux *demux, int dump_ts_select);

extern int  dmx_alloc_chan(struct aml_dmx *dmx, int type,
				int pes_type, int pid);
extern void dmx_free_chan(struct aml_dmx *dmx, int cid);

extern int dmx_get_ts_serial(enum aml_ts_source_t src);


/*AMLogic ASYNC FIFO interface*/
extern int aml_asyncfifo_hw_init(struct aml_asyncfifo *afifo, int id);
extern int aml_asyncfifo_hw_deinit(struct aml_asyncfifo *afifo);
extern int aml_asyncfifo_hw_set_source(struct aml_asyncfifo *afifo,
					enum aml_dmx_id_t src);
extern int aml_asyncfifo_hw_reset(struct aml_asyncfifo *afifo, int id);

/*Get the Audio & Video PTS*/
extern u32 aml_dmx_get_video_pts(struct aml_dvb *dvb);
extern u32 aml_dmx_get_audio_pts(struct aml_dvb *dvb);
extern u32 aml_dmx_get_first_video_pts(struct aml_dvb *dvb);
extern u32 aml_dmx_get_first_audio_pts(struct aml_dvb *dvb);

/*Get the DVB device*/
extern struct aml_dvb *aml_get_dvb_device(void);

/*Demod interface*/
extern void aml_dmx_register_frontend(enum aml_ts_source_t src,
					struct dvb_frontend *fe);
extern void aml_dmx_before_retune(enum aml_ts_source_t src,
					struct dvb_frontend *fe);
extern void aml_dmx_after_retune(enum aml_ts_source_t src,
					struct dvb_frontend *fe);
extern void aml_dmx_start_error_check(enum aml_ts_source_t src,
					struct dvb_frontend *fe);
extern int  aml_dmx_stop_error_check(enum aml_ts_source_t src,
					struct dvb_frontend *fe);
//extern int aml_regist_dmx_class(void);
//extern int aml_unregist_dmx_class(void);
extern void dvb_frontend_retune(struct dvb_frontend *fe);

struct devio_aml_platform_data {
	int (*io_setup)(void *);
	int (*io_cleanup)(void *);
	int (*io_power)(void *, int enable);
	int (*io_reset)(void *, int enable);
};

/*Reset the demux device*/
void dmx_reset_hw(struct aml_dvb *dvb);
void dmx_reset_hw_ex(struct aml_dvb *dvb, int reset_irq);

/*Reset the individual demux*/
void dmx_reset_dmx_hw(struct aml_dvb *dvb, int id);
void dmx_reset_dmx_id_hw_ex(struct aml_dvb *dvb, int id, int reset_irq);
void dmx_reset_dmx_id_hw_ex_unlock(struct aml_dvb *dvb, int id, int reset_irq);
void dmx_reset_dmx_hw_ex(struct aml_dvb *dvb,
				struct aml_dmx *dmx,
				int reset_irq);
void dmx_reset_dmx_hw_ex_unlock(struct aml_dvb *dvb,
				struct aml_dmx *dmx,
				int reset_irq);

#endif
