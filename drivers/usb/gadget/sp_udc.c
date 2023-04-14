// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/dma-mapping.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/usb/gadget.h>
#include <linux/usb/composite.h>
#include <linux/usb/otg.h>
#include <dm.h>

#include <asm/cache.h>
#include <asm/unaligned.h>
#include <common.h>
#include <linux/dma-direction.h>
#include <linux/usb/ch9.h>
#include <stdlib.h>

#ifdef CONFIG_FIQ_GLUE
#include <asm/fiq.h>
#include <asm/fiq_glue.h>
#include <linux/pgtable.h>
#include <asm/hardware/gic.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/delay.h>
#endif

#include "sp_udc.h"
#include "sp_udc_regs.h"

#define ISO_DEBUG_INFO

//#define USE_DMA

#define IRQ_USB_DEV_PORT0			45
#define IRQ_USB_DEV_PORT1			48

#define TO_HOST					0
#define TO_DEVICE				1

/* ctrl rx singal keep the strongest time */
#define BUS_RESET_FOR_CHIRP_DELAY		2000
#define DMA_FLUSH_TIMEOUT_DELAY			300000

#define FULL_SPEED_DMA_SIZE			64
#define HIGH_SPEED_DMA_SIZE			512
#define UDC_FLASH_BUFFER_SIZE			(1024*64)
#define	DMA_ADDR_INVALID			(~(dma_addr_t)0)

static u32 bulkep_dma_block_size;

struct sp_ep_iso {
	struct list_head queue;
	int act;
};

static u32 dmsg = 1;
module_param(dmsg, uint, 0644);

u32 dma_len_ep1;
u32 dma_xferlen_ep11;
spinlock_t plock, qlock;

/* coverity[declared_but_not_referenced] */
static const char gadget_name[] = "sp_udc";

static void __iomem *base_addr;
static void __iomem *moon0_reg;
static void __iomem *moon1_reg;
static void __iomem *moon4_reg;
static void __iomem *uphy_reg;
static void __iomem *hb_gp_reg;

static struct sp_udc *the_controller;

static int dmesg_buf_size = 3000;
module_param(dmesg_buf_size, uint, 0644);
static int dqueue = 1;
module_param(dqueue, uint, 0644);
static char *sp_kbuf;

static void dprintf(const char *fmt, ...)
{
	char buf[200];
	int len;
	int x;
	char *pBuf;
	char *pos;
	char org;
	static int i;
	static int bfirst = 1;
	va_list args;

	spin_lock(&plock);

	sp_kbuf = kzalloc(dmesg_buf_size, GFP_KERNEL);

	va_start(args, fmt);
	vsnprintf(buf, 200, fmt, args);

	len = strlen(buf);
	if (len) {
		if (i+len <= dmesg_buf_size)
			memcpy(sp_kbuf+i, buf, len);
	}

	i += len;
	va_end(args);

	if (dqueue) {
		pr_info("%s\n", buf);
	} else if (i >= dmesg_buf_size && bfirst == 1) {
		bfirst = 0;
		i = 0;
		dmsg = 32;
		x = 0;

		pr_info(">>> start dump\n");
		pr_info("sp_kbuf: %d, i = %d\n", strlen(sp_kbuf), i);

		pBuf = sp_kbuf;
		while (1) {
			pos = strstr(pBuf, "\n");
			if (pos == NULL)
				break;

			org = *pos;
			*pos = 0;
			pr_info("%s", pBuf);
			pBuf = pos + 1;
		}

		pr_info(">>> end dump\n");
	}

	spin_unlock(&plock);
}

#define DEBUG_INFO(fmt, arg...)		do { if (dmsg&(1<<0)) dprintf(fmt, ##arg); } while (0)
#define DEBUG_NOTICE(fmt, arg...)	do { if (dmsg&(1<<1)) dprintf(fmt, ##arg); } while (0)
#define DEBUG_DBG(fmt, arg...)		do { if (dmsg&(1<<2)) dprintf(fmt, ##arg); } while (0)
#define DEBUG_DUMP(fmt, arg...)		do { if (dmsg&(1<<3)) dprintf(fmt, ##arg); } while (0)
#define DEBUG_PRT(fmt, arg...)		do { if (dmsg&(1<<4)) dprintf(fmt, ##arg); } while (0)

static struct sp_udc memory;

static inline u32 udc_read(u32 reg);
static inline void udc_write(u32 value, u32 reg);


static inline struct sp_ep *to_sp_ep(struct usb_ep *ep)
{
	return container_of(ep, struct sp_ep, ep);
}

static inline struct sp_udc *to_sp_udc(struct usb_gadget *gadget)
{
	return container_of(gadget, struct sp_udc, gadget);
}

static inline struct sp_request *to_sp_req(struct usb_request *req)
{
	return container_of(req, struct sp_request, req);
}

static inline u32 udc_read(u32 reg)
{
	return readl(base_addr + reg);
}

static inline void udc_write(u32 value, u32 reg)
{
	writel(value, base_addr + reg);
}

void init_ep_spin(void)
{
	int i;

	for (i = 0; i < SP_MAXENDPOINTS; i++)
		spin_lock_init(&memory.ep[i].lock);
}

static inline int sp_udc_get_ep_fifo_count(int is_pingbuf, u32 ping_c)
{
	int tmp = 0;

	if (is_pingbuf)
		tmp = udc_read(ping_c);
	else
		tmp = udc_read(ping_c + 0x4);

	return (tmp & 0x3ff);
}

static void sp_udc_done(struct sp_ep *ep, struct sp_request *req, int status)
{
	unsigned int halted = ep->halted;

	DEBUG_DBG(">>> %s...", __func__);

	list_del_init(&req->queue);
	if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;
	else
		status = req->req.status;

	ep->halted = 1;
	usb_gadget_giveback_request(&ep->ep, &req->req);
	ep->halted = halted;

	DEBUG_DBG("<<< %s...", __func__);
}

static void sp_udc_nuke(struct sp_udc *udc, struct sp_ep *ep, int status)
{
	/* Sanity check */
	DEBUG_DBG(">>> %s...", __func__);
	if (&ep->queue == NULL)
		return;

	while (!list_empty(&ep->queue)) {
		struct sp_request *req;

		req = list_entry(ep->queue.next, struct sp_request, queue);
		sp_udc_done(ep, req, status);
	}

	DEBUG_DBG("<<< %s...", __func__);
}

static int sp_udc_set_halt(struct usb_ep *_ep, int value)
{
	struct sp_ep *ep = to_sp_ep(_ep);
	u32 idx;
	u32 v = 0;
	unsigned long flags;

	if (unlikely(!_ep || (!ep->desc && ep->ep.name != ep0name))) {
		pr_err("%s: inval 2\n", __func__);
		return -EINVAL;
	}

	DEBUG_DBG(">>> %s...", __func__);

	local_irq_save(flags);

	idx = ep->bEndpointAddress & 0x7F;
	DEBUG_DBG("udc set halt ep=%x val=%x", idx, value);

	switch (idx) {
	case EP1:
		v = SETEP1STL;
		break;

	case EP3:
		v = SETEP3STL;
		break;

	case EP11:
		v = SETEPBSTL;
		break;

	default:
		return -EINVAL;
	}

	if ((!value) && v)
		v = v << 16;

	udc_write((udc_read(UDLCSTL) | v), UDLCSTL);
	DEBUG_DBG("udc set halt v=%xh, UDLCSTL=%x", v, udc_read(UDLCSTL));
	ep->halted = value ? 1 : 0;

	local_irq_restore(flags);
	DEBUG_DBG("<<< %s...", __func__);

	return 0;
}

static int sp_udc_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct sp_udc *dev;
	struct sp_ep *ep;
	unsigned long flags;
	u32 max;
	u32 linker_int_en;
	u32 bulk_int_en;

	DEBUG_DBG(">>> %s", __func__);

	ep = to_sp_ep(_ep);
	if (!_ep || !desc || _ep->name == ep0name || desc->bDescriptorType != USB_DT_ENDPOINT) {
		pr_err("%s.%d,,%p,%p,%p,%s,%d\n", __func__, __LINE__,
				_ep, desc, ep->desc, _ep->name, desc->bDescriptorType);

		return -EINVAL;
	}

	dev = ep->dev;
	DEBUG_DBG("dev->driver = %xh, dev->gadget.speed = %d", dev->driver, dev->gadget.speed);
	if (!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN)
		return -ESHUTDOWN;

	max = le16_to_cpu(desc->wMaxPacketSize) & 0x1fff;

	local_irq_save(flags);

	_ep->maxpacket = max & 0x7ff;
	ep->desc = desc;
	ep->ep.desc = desc;
	ep->halted = 0;
	ep->bEndpointAddress = desc->bEndpointAddress;

	linker_int_en   = udc_read(UDLIE);
	bulk_int_en     = udc_read(UDNBIE);

	DEBUG_DBG("ep->num = %d\n", ep->num);

	switch (ep->num) {
	case EP0:
		linker_int_en |= EP0S_IF;
		break;

	case EP1:
		linker_int_en |= EP1I_IF;
		udc_write(EP_DIR | EP_ENA | RESET_PIPO_FIFO, UDEP12C);
		break;

	case EP3:
		linker_int_en |= EP3I_IF;
		break;

	case EP11:
		bulk_int_en |= EP11O_IF;
		udc_write(EP_ENA | RESET_PIPO_FIFO, UDEPABC);
		break;

	default:
		return -EINVAL;
	}

	udc_write(linker_int_en, UDLIE);
	udc_write(bulk_int_en, UDNBIE);

	/* print some debug message */
	DEBUG_DBG("enable %s(%d) ep%x%s-blk max %02x", _ep->name, ep->num, desc->bEndpointAddress,
					desc->bEndpointAddress & USB_DIR_IN ? "in" : "out", max);

	local_irq_restore(flags);
	sp_udc_set_halt(_ep, 0);

	DEBUG_DBG("<<< %s", __func__);

	return 0;
}

static int sp_udc_ep_disable(struct usb_ep *_ep)
{
	struct sp_ep *ep = to_sp_ep(_ep);
	u32 int_udll_ie;
	u32 int_udn_ie;
	unsigned long flags;

	DEBUG_DBG(">>> %s...", __func__);

	if (!_ep || !ep->desc) {
		DEBUG_DBG("%s not enabled", _ep ? ep->ep.name : NULL);
		return -EINVAL;
	}

	DEBUG_DBG("ep_disable: %s", _ep->name);

	local_irq_save(flags);

	ep->desc = NULL;
	ep->ep.desc = NULL;
	ep->halted = 1;

	sp_udc_nuke(ep->dev, ep, -ESHUTDOWN);

	/* disable irqs */
	int_udll_ie = udc_read(UDLIE);
	int_udn_ie = udc_read(UDNBIE);
	switch (ep->num) {
	case EP0:
		int_udll_ie &= ~(EP0I_IF | EP0O_IF | EP0S_IF);
		break;

	case EP1:
		int_udll_ie &= ~(EP1I_IF);

		if (udc_read(UEP12DMACS) & DMA_EN) {
			dma_len_ep1 = 0;
			udc_write(udc_read(UEP12DMACS) | DMA_FLUSH, UEP12DMACS);

			while (!(udc_read(UEP12DMACS) & DMA_FLUSHEND))
				DEBUG_DBG("wait dma 1 flush");
		}

		dma_len_ep1 = 0;
		break;

	case EP3:
		int_udll_ie &= ~(EP3I_IF);
		break;

	case EP11:
		int_udn_ie &= ~(EP11O_IF);

		if (udc_read(UDEPBDMACS) & DMA_EN) {
			dma_xferlen_ep11 = 0;
			udc_write(udc_read(UDEPBDMACS) | DMA_FLUSH, UDEPBDMACS);

			while (!(udc_read(UDEPBDMACS) & DMA_FLUSHEND))
				DEBUG_DBG("wait dma 11 flush");
		}

		dma_xferlen_ep11 = 0;
		break;

	default:
		return -EINVAL;
	}

	udc_write(int_udll_ie, UDLIE);
	udc_write(int_udn_ie, UDNBIE);

	local_irq_restore(flags);

	DEBUG_DBG("%s disabled", _ep->name);
	DEBUG_DBG("<<< %s...", __func__);

	return 0;
}

static struct usb_request *sp_udc_alloc_request(struct usb_ep *_ep, gfp_t mem_flags)
{
	struct sp_request *req;

	DEBUG_DBG(">>> %s...", __func__);

	if (!_ep)
		return NULL;

	req = kzalloc(sizeof(struct sp_request), mem_flags);
	if (!req)
		return NULL;

	req->req.dma = DMA_ADDR_INVALID;
	INIT_LIST_HEAD(&req->queue);
	DEBUG_DBG("<<< %s...", __func__);

	return &req->req;
}

static void sp_udc_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct sp_ep *ep = to_sp_ep(_ep);
	struct sp_request *req = to_sp_req(_req);

	DEBUG_DBG("%s free rquest", _ep->name);

	if (!ep || !_req || (!ep->desc && _ep->name != ep0name))
		return;

	kfree(req);
}

static void clearHwState_UDC(void)
{
	/* INFO: we don't disable udc interrupt when we are clear udc hw state,			*/
	/* 1.since when we are clearing, we are in ISR , will not the same interrupt reentry	*/
	/*   problem.										*/
	/* 2.after we finish clearing , we will go into udc ISR again, if there are interrupts	*/
	/*   occur while we are clearing ,we want to catch them immediately			*/
	/*											*/

	/* ===== check udc DMA state, and flush it ===== */
	int tmp = 0;

	if (udc_read(UDEP2DMACS) & DMA_EN) {
		udc_write(udc_read(UDEP2DMACS) | DMA_FLUSH, UDEP2DMACS);
		while (!(udc_read(UDEP2DMACS) & DMA_FLUSHEND)) {
			tmp++;
			if (tmp > DMA_FLUSH_TIMEOUT_DELAY) {
				DEBUG_DBG("##");
				tmp = 0;
			}
		}
	}

	/* Disable Interrupt */
	/* Clear linker layer Interrupt source */
	udc_write(0xefffffff, UDLIF);
	/* EP0 control status */
	/* clear ep0 out vld = 1, clear set ep0 in vld = 0, set ctl dir to OUT direction = 0 */
	udc_write(CLR_EP0_OUT_VLD, UDEP0CS);
	udc_write(0x0, UDEP0CS);

	udc_write(CLR_EP_OVLD | RESET_PIPO_FIFO, UDEP12C);

	/* Clear Stall Status */
	udc_write((CLREPBSTL | CLREPASTL | CLREP9STL | CLREP8STL | CLREP3STL | CLREP2STL
							| CLREP1STL | CLREP0STL), UDLCSTL);
}

static int vbusInt_handle(void)
{
	DEBUG_DBG(">>> %s... UDCCS=%xh, UDLCSET=%xh",
				__func__, udc_read(UDCCS), udc_read(UDLCSET));

	/* host present */
	if (udc_read(UDCCS) & VBUS) {
		/* if soft discconect ->force connect */
		if (udc_read(UDLCSET) & SOFT_DISC)
			udc_write(udc_read(UDLCSET) & 0xFE, UDLCSET);
	} else {/* host absent */
		/* soft connect ->force disconnect */
		if (!(udc_read(UDLCSET) & SOFT_DISC))
			udc_write(udc_read(UDLCSET) | SOFT_DISC, UDLCSET);
	}

	DEBUG_DBG("<<< %s...", __func__);
	return 0;
}

static int sp_udc_readep0_fifo_crq(struct usb_ctrlrequest *crq)
{
	unsigned char *outbuf = (unsigned char *)crq;

	DEBUG_DBG("read ep0 fifi crq ,len= %d", udc_read(UDEP0DC));
	memcpy((unsigned char *)outbuf, (char *)(UDEP0SDP + base_addr), 4);
	mb();		/* make sure settings are effective */
	memcpy((unsigned char *)(outbuf + 4), (char *)(UDEP0SDP + base_addr), 4);

	return 8;
}

static int sp_udc_get_status(struct sp_udc *dev, struct usb_ctrlrequest *crq)
{
	u32 status = 0;
	u8 ep_num = crq->wIndex & 0x7F;
	struct sp_ep *ep = &memory.ep[ep_num];

	switch (crq->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_INTERFACE:
		break;

	case USB_RECIP_DEVICE:
		status = dev->devstatus;
		break;

	case USB_RECIP_ENDPOINT:
		if (ep_num > 14 || crq->wLength > 2)
			return 1;

		status = ep->halted;
		break;

	default:
		return 1;
	}

	udc_write(EP0_DIR | CLR_EP0_OUT_VLD, UDEP0CS);
	udc_write(((1 << 2) - 1), UDEP0VB);
	memcpy((char *)(base_addr + UDEP0DP), (char *)(&status), 4);
	udc_write(udc_read(UDLIE) | EP0I_IF, UDLIE);
	udc_write(EP0_DIR | SET_EP0_IN_VLD, UDEP0CS);

	return 0;
}

static void sp_udc_handle_ep0_idle(struct sp_udc *dev, struct sp_ep *ep,
					struct usb_ctrlrequest *crq, u32 ep0csr, int cf)
{
	int len;
	int ret, state;

	DEBUG_DBG(">>> %s...", __func__);

	/* start control request? */
	sp_udc_nuke(dev, ep, -EPROTO);

	len = sp_udc_readep0_fifo_crq(crq);
	DEBUG_DBG("len  = %d", len);

	if (len != sizeof(*crq)) {
		pr_err("setup begin: fifo READ ERROR wanted %d bytes got %d. Stalling out...",
										sizeof(*crq), len);

		/* error send stall */
		udc_write((udc_read(UDLCSTL) | SETEP0STL), UDLCSTL);

		return;
	}

	DEBUG_DBG("bRequestType = %02xh, bRequest = %02xh, wValue = %04xh, wIndex = %04xh,\t"
			"\b\b\b\b\bwLength = %04xh",
			crq->bRequestType, crq->bRequest, crq->wValue, crq->wIndex, crq->wLength);

	/****************************************/
	/* bRequestType				*/
	/* Bit 7 : data transfer direction	*/
	/* 0b = Host-to-device			*/
	/* 1b = Device-to-host			*/
	/* Bit 6 ...				*/
	/* Bit 5 : type				*/
	/* 00b = Standard			*/
	/* 01b = Class				*/
	/* 10b = Vendor				*/
	/* 11b = Reserved			*/
	/* Bit 4 ...				*/
	/* Bit 0 : recipient			*/
	/* 00000b = Device			*/
	/* 00001b = Interface			*/
	/* 00010b = Endpoint			*/
	/* 00011b = Other			*/
	/* 00100b to 11111b = Reserved		*/
	/****************************************/

	/****************************************/
	/* wValue, high-byte			*/
	/* 1 = Device				*/
	/* 2 = Configuration			*/
	/* 3 = String				*/
	/* 4 = Interface			*/
	/* 5 = Endpoint				*/
	/* 6 = Device Qualifier			*/
	/* 7 = Other Speed Configuration	*/
	/* 8 - Interface Power			*/
	/* 9 - On-The-Go (OTG)			*/
	/* 21 = HID (Human interface device)	*/
	/* 22 = Report				*/
	/* 23 = Physical			*/
	/* **************************************/

	/* cope with automagic for some standard requests. */
	dev->req_std = (crq->bRequestType & USB_TYPE_MASK) == USB_TYPE_STANDARD;
	dev->req_config = 0;
	dev->req_pending = 1;
	state = dev->gadget.state;

	switch (crq->bRequest) {
	case USB_REQ_GET_DESCRIPTOR:
		DEBUG_DBG(" ******* USB_REQ_GET_DESCRIPTOR ... ");
		DEBUG_DBG("start get descriptor after bus reset");
		{
			u32 DescType = ((crq->wValue) >> 8);

			if (DescType == 0x1) {
				if (udc_read(UDLCSET) & CURR_SPEED) {
					DEBUG_DBG("DESCRIPTOR SPeed = USB_SPEED_FULL");
					dev->gadget.speed = USB_SPEED_FULL;
					bulkep_dma_block_size = FULL_SPEED_DMA_SIZE;
				} else {
					DEBUG_DBG("DESCRIPTOR SPeed = USB_SPEED_HIGH");
					dev->gadget.speed = USB_SPEED_HIGH;
					bulkep_dma_block_size = HIGH_SPEED_DMA_SIZE;
				}
			}
		}
		break;

	case USB_REQ_SET_CONFIGURATION:
		DEBUG_DBG(" ******* USB_REQ_SET_CONFIGURATION ...");
		DEBUG_DBG("dev->gadget.state = %d", dev->gadget.state);
		dev->req_config = 1;
		udc_write(udc_read(UDLCADDR) | (crq->wValue) << 16, UDLCADDR);
		break;

	case USB_REQ_SET_INTERFACE:
		DEBUG_DBG("***** USB_REQ_SET_INTERFACE ****");
		dev->req_config = 1;
		break;

	case USB_REQ_SET_ADDRESS:
		DEBUG_DBG("USB_REQ_SET_ADDRESS ...");
		return;

	case USB_REQ_GET_STATUS:
		DEBUG_DBG("USB_REQ_GET_STATUS ...");

		if (dev->req_std) {
			if (!sp_udc_get_status(dev, crq))
				return;
		}
		break;

	case USB_REQ_CLEAR_FEATURE:
		DEBUG_DBG(">>> USB_REQ_CLEAR_FEATURE ...");
		break;

	case USB_REQ_SET_FEATURE:
		DEBUG_DBG("USB_REQ_SET_FEATURE ...");
		break;

	default:
		break;
	}

	if (crq->bRequestType & USB_DIR_IN) {
		dev->ep0state = EP0_IN_DATA_PHASE;
	} else {
		dev->ep0state = EP0_OUT_DATA_PHASE;
		DEBUG_NOTICE("ep0 fifo %x\n", udc_read(UDEP0CS));
		udc_write(0, UDEP0CS);
	}

	if (!dev->driver)
		return;

	/* deliver the request to the gadget driver */
	/* android_setup composite_setup */
	ret = dev->driver->setup(&dev->gadget, crq);
	DEBUG_DBG("dev->driver->setup = %x", ret);

	if (ret < 0) {
		if (dev->req_config) {
			pr_err("config change %02x fail %d?", crq->bRequest, ret);
			return;
		}

		if (ret == -EOPNOTSUPP)
			pr_err("Operation not supported");
		else
			pr_err("dev->driver->setup failed. (%d)", ret);

		udelay(5);

		/* set ep0 stall */
		udc_write(0x1, UDLCSTL);
		dev->ep0state = EP0_IDLE;
	} else if (dev->req_pending) {
		DEBUG_DBG("dev->req_pending... what now?");
		dev->req_pending = 0;

		/* MSC reset command */
		if (crq->bRequest == 0xff) {
			/* ep1SetHalt = 0; */
			/* ep2SetHalt = 0; */
		}
	}

	DEBUG_DBG("ep0state *** %s, Request=%d, RequestType=%d, from=%d",
				ep0states[dev->ep0state], crq->bRequest, crq->bRequestType, cf);
	DEBUG_DBG("<<< %s...", __func__);
}

static inline int sp_udc_write_packet_with_buf(int fifo, u8 *buf, unsigned int len, int reg)
{
	int n = 0;
	int m = 0;
	int i = 0;

	n = len / 4;
	m = len % 4;

	if (n > 0) {
		udc_write(0xf, reg);

		for (i = 0; i < n; i++)
			*(u32 *) (base_addr + fifo) = *((u32 *) (buf + (i * 4)));
	}

	if (m > 0) {
		udc_write(((1 << m) - 1), reg);
		*(u32 *) (base_addr + fifo) = *((u32 *) (buf + (i * 4)));
	}

	return len;
}

static inline int sp_udc_write_packet(int fifo, struct sp_request *req, unsigned int max,
										int offset)
{
	unsigned int len = min(req->req.length - req->req.actual, max);

	u8 *buf = req->req.buf + req->req.actual;

	return sp_udc_write_packet_with_buf(fifo, buf, len, offset);
}

static int sp_udc_write_ep0_fifo(struct sp_ep *ep, struct sp_request *req)
{
	unsigned int count;
	int is_last;

	udc_write(EP0_DIR | CLR_EP0_OUT_VLD, UDEP0CS);
	count = sp_udc_write_packet(UDEP0DP, req, ep->ep.maxpacket, UDEP0VB);
	udc_write(EP0_DIR | SET_EP0_IN_VLD, UDEP0CS);

	req->req.actual += count;
	if (count != ep->ep.maxpacket)
		is_last = 1;
	else if (req->req.length != req->req.actual || req->req.zero)
		is_last = 0;
	else
		is_last = 1;

	DEBUG_DBG("write ep0: count=%d, actual=%d, length=%d, last=%d, zero=%d",
			count, req->req.actual, req->req.length, is_last, req->req.zero);

	if (is_last) {
		int cc = 0;

		while (udc_read(UDEP0CS) & EP0_IVLD) {
			udelay(5);
			if (cc++ > 1000)
				break;
		}

		sp_udc_done(ep, req, 0);
		udc_write(udc_read(UDLIE) & (~EP0I_IF), UDLIE);
		udc_write(udc_read(UDEP0CS) & (~EP0_DIR), UDEP0CS);
	} else {
		udc_write(udc_read(UDLIE) | EP0I_IF, UDLIE);
	}

	return is_last;
}

static inline int sp_udc_read_fifo_packet(int fifo, u8 *buf, int length, int reg)
{
	int n = 0;
	int m = 0;
	int i = 0;

	n = length / 4;
	m = length % 4;

	udc_write(0xf, reg);

	for (i = 0; i < n; i++)
		*((u32 *) (buf + (i * 4))) = *(u32 *) (base_addr + fifo);

	if (m > 0) {
		udc_write(((1 << m) - 1), reg);
		*((u32 *) (buf + (i * 4))) = *(u32 *) (base_addr + fifo);
	}

	return length;
}

static int sp_udc_read_ep0_fifo(struct sp_ep *ep, struct sp_request *req)
{
	u8 *buf;
	unsigned int count;
	u8 ep0_len;
	int is_last;

	if (!req->req.length) {
		DEBUG_DBG("%s: length = 0", __func__);

#if 0
		udc_write(udc_read(UDLIE) | EP0I_IF, UDLIE);
		udc_write(SET_EP0_IN_VLD | EP0_DIR, UDEP0CS);
		udc_write(udc_read(UDLIE) & (~EP0O_IF), UDLIE);
		sp_udc_done(ep, req, 0);
#endif

		is_last = 1;
	} else {
		udc_write(udc_read(UDLIE) | EP0O_IF, UDLIE);

		buf = req->req.buf + req->req.actual;
		udc_write(udc_read(UDEP0CS) & (~EP0_DIR), UDEP0CS);	/* read direction */

		ep0_len = udc_read(UDEP0DC);

		if (ep0_len > ep->ep.maxpacket)
			ep0_len = ep->ep.maxpacket;

		count = sp_udc_read_fifo_packet(UDEP0DP, buf, ep0_len, UDEP0VB);
		udc_write(udc_read(UDEP0CS) | CLR_EP0_OUT_VLD, UDEP0CS);

		req->req.actual += count;

		if (count != ep->ep.maxpacket)
			is_last = 1;
		else if (req->req.length != req->req.actual || req->req.zero)
			is_last = 0;
		else
			is_last = 1;

		DEBUG_DBG("read ep0: count=%d, actual=%d, length=%d, maxpacket=%d, last=%d,\t"
				"\b\b\b\b\bzero=%d", count, req->req.actual, req->req.length,
				ep->ep.maxpacket, is_last, req->req.zero);
	}
	if (is_last) {
		DEBUG_DBG("%s: is_last = 1", __func__);
		udc_write(udc_read(UDLIE) | EP0I_IF, UDLIE);
		udc_write(SET_EP0_IN_VLD | EP0_DIR, UDEP0CS);
		udc_write(udc_read(UDLIE) & (~EP0O_IF), UDLIE);
		sp_udc_done(ep, req, 0);
	}

	return is_last;
}

static int sp_udc_handle_ep0_proc(struct sp_ep *ep, struct sp_request *req, int cf)
{
	struct usb_ctrlrequest crq;
	u32 ep0csr;
	struct sp_udc *dev = ep->dev;
	int bOk = 1;

	ep0csr = udc_read(UDEP0CS);

	switch (dev->ep0state) {
	case EP0_IDLE:
		DEBUG_DBG("EP0_IDLE_PHASE ... what now?");
		sp_udc_handle_ep0_idle(dev, ep, &crq, ep0csr, cf);
		break;

	case EP0_IN_DATA_PHASE:
		DEBUG_DBG("EP0_IN_DATA_PHASE ... what now?");
		if (sp_udc_write_ep0_fifo(ep, req)) {
			ep->dev->ep0state = EP0_IDLE;
			DEBUG_DBG("ep0 in0 done");
		} else {
			bOk = 0;
		}
		break;

	case EP0_OUT_DATA_PHASE:
		DEBUG_DBG("EP0_OUT_DATA_PHASE *** what now?");
		if (sp_udc_read_ep0_fifo(ep, req)) {
			ep->dev->ep0state = EP0_IDLE;
			DEBUG_DBG("ep0 out1 done");
		} else {
			bOk = 0;
		}
		break;
	}

	return bOk;
}

static void sp_udc_handle_ep0(struct sp_udc *dev)
{
	struct sp_ep *ep = &dev->ep[0];
	struct usb_composite_dev *cdev = get_gadget_data(&dev->gadget);
	struct usb_request *req_g = NULL;
	struct sp_request *req = NULL;

	DEBUG_DBG(">>> %s ...", __func__);

	if (!cdev) {
		pr_err("cdev invalid");
		return;
	}

	req_g = cdev->req;
	req = to_sp_req(req_g);

	if (!req) {
		pr_err("req invalid");
		return;
	}

	sp_udc_handle_ep0_proc(ep, req, 1);
	DEBUG_DBG("<<< %s ... ", __func__);
}

#if 0
static void sp_print_hex_dump_bytes(const char *prefix_str, int prefix_type,
							const void *buf, size_t len)
{
	if (dmsg & (1 << 3))
		print_hex_dump(KERN_NOTICE, prefix_str, prefix_type, 16, 1,
				buf, len, true);
}
#endif

#ifdef USE_DMA
static int sp_udc_ep11_bulkout_pio(struct sp_ep *ep, struct sp_request *req);

static int sp_udc_ep11_bulkout_dma(struct sp_ep *ep, struct sp_request *req)
{
	u8 *buf;
	int actual_length = 0;
	int cur_length = req->req.length;
	int dma_xferlen;
	unsigned long t;
	int ret = 0;

	DEBUG_DBG(">>> %s...", __func__);

	if (dma_xferlen_ep11 == 0) {
		if (cur_length <= bulkep_dma_block_size) {
			ret = sp_udc_ep11_bulkout_pio(ep, req);
			return ret;
		}

		udc_write(udc_read(UDNBIE) & (~EP11O_IF), UDNBIE);

		DEBUG_DBG("req.length=%d req.actual=%d, req->req.dma=%xh UDCIF=%xh",
				req->req.length, req->req.actual, req->req.dma, udc_read(UDCIF));

		if (req->req.dma == DMA_ADDR_INVALID) {
			req->req.dma = dma_map_single(ep->dev->gadget.dev.parent,
								(u8 *)req->req.buf,
								cur_length, DMA_FROM_DEVICE);
			if (dma_mapping_error(ep->dev->gadget.dev.parent, req->req.dma)) {
				DEBUG_DBG("dma_mapping_error");
				return 1;
			}
		}

		while (actual_length < cur_length) {
			buf = (u8 *) (req->req.dma + actual_length);
			dma_xferlen = min(cur_length - actual_length, (int)UDC_FLASH_BUFFER_SIZE);
			dma_xferlen_ep11 = dma_xferlen;

			udc_write((u32) buf, UDEPBDMADA);
			udc_write((udc_read(UDEPBDMACS) & (~DMA_COUNT_MASK)) |
					DMA_COUNT_ALIGN | DMA_WRITE | dma_xferlen | DMA_EN,
					UDEPBDMACS);

			/* verB DMA bug */
			if ((udc_read(UDEPBFS) & 0x22) == 0x20)
				udc_write(udc_read(UDEPBPPC) | SWITCH_BUFF, UDEPBPPC);

			DEBUG_DBG("cur_len=%d actual_len=%d req.dma=%xh dma_len=%d\t"
					"\b\b\b\b\bUDEPBDMACS = %xh UDEPBFS = %xh UDCIF = %xh",
					cur_length, actual_length, req->req.dma, dma_xferlen,
					udc_read(UDEPBDMACS), udc_read(UDEPBFS), udc_read(UDCIF));

			t = jiffies;

			while ((udc_read(UDEPBDMACS) & DMA_EN) != 0) {
				udc_write(udc_read(UDCIE) | EPB_DMA_IF, UDCIE);
				DEBUG_DBG(">>cur_len=%d actual_len=%d req.dma=%xh dma_len=%d\t"
						"\b\b\b\b\bUDEPBDMACS = %xh UDCIE = %xh UDCIF = %xh",
						cur_length, actual_length, req->req.dma,
						dma_xferlen, udc_read(UDEPBDMACS), udc_read(UDCIE),
						udc_read(UDCIF));

				if (time_after(jiffies, t + 10 * HZ)) {
					DEBUG_DBG("dma error: UDEPBDMACS = %xh",
								udc_read(UDEPBDMACS));
					break;
				}
			}

			DEBUG_DBG("UDCIF = %xh", udc_read(UDCIF));

			actual_length += dma_xferlen;
		}

		cur_length -= actual_length;

		if (req->req.dma != DMA_ADDR_INVALID) {
			dma_unmap_single(ep->dev->gadget.dev.parent, req->req.dma,
						req->req.length, DMA_FROM_DEVICE);

			req->req.dma = DMA_ADDR_INVALID;
		}

		udc_write(udc_read(UDNBIE) | EP11O_IF, UDNBIE);

		DEBUG_DBG("UDEPBDMACS = %x", udc_read(UDEPBDMACS));
		DEBUG_DBG("<<< %s...", __func__);

		return 1;
	}

	DEBUG_DBG("<<< %s...", __func__);

	return ret;
}
#endif

static void sp_udc_bulkout_pio(u8 *buf, u32 avail)
{
	sp_udc_read_fifo_packet(UDEPBFDP, buf, avail, UDEPBVB);
	udc_write(udc_read(UDEPABC) | CLR_EP_OVLD, UDEPABC);
}

static int sp_udc_ep11_bulkout_pio(struct sp_ep *ep, struct sp_request *req)
{
	u8 *buf;
	u32 count;
	u32 avail;
	int is_last;
	int is_pingbuf;
	int pre_is_pingbuf;
	int delay_count;

	DEBUG_DBG(">>> %s UDEPBFS = %xh", __func__, udc_read(UDEPBFS));

#if 0
	if (down_trylock(&ep->wsem))
		return 0;
#endif

	DEBUG_DBG("1.req.length=%d req.actual=%d req->req.dma = %xh UDEPBFS = %xh",
			req->req.length, req->req.actual, req->req.dma, udc_read(UDEPBFS));

	is_last = 0;
	delay_count = 0;
	is_pingbuf = (udc_read(UDEPBPPC) & CURR_BUFF) ? 1 : 0;

	do {
		pre_is_pingbuf = is_pingbuf;
		count = sp_udc_get_ep_fifo_count(is_pingbuf, UDEPBPIC);
		if (!count) {
#if 0
			up(&ep->wsem);
#endif

			return 1;
		}

		buf = req->req.buf + req->req.actual;

		if (count > ep->ep.maxpacket)
			avail = ep->ep.maxpacket;
		else
			avail = count;

		sp_udc_bulkout_pio(buf, avail);
		req->req.actual += avail;

		if (count < ep->ep.maxpacket || req->req.length <= req->req.actual)
			is_last = 1;

		DEBUG_DBG("2.req.length = %d req.actual = %d UDEPBFS = %xh UDEPBPPC = %xh\t"
				"\b\b\b\b\bUDEPBPOC = %xh UDEPBPIC = %xh avail = %d is_last = %d",
				req->req.length, req->req.actual, udc_read(UDEPBFS),
				udc_read(UDEPBPPC), udc_read(UDEPBPOC), udc_read(UDEPBPIC),
				avail, is_last);

		if (is_last)
			break;

out_fifo_retry:
		if ((udc_read(UDEPBFS)&0x22) == 0) {
			udelay(1);
			if (delay_count++ < 20)
				goto out_fifo_retry;

			delay_count = 0;
		}

out_fifo_controllable:
		is_pingbuf = (udc_read(UDEPBPPC) & CURR_BUFF) ? 1 : 0;

		if (is_pingbuf == pre_is_pingbuf)
			goto out_fifo_controllable;
	} while (1);

	DEBUG_DBG("3.req.length=%d req.actual=%d UDEPBFS = %xh count=%d is_last=%d",
			req->req.length, req->req.actual, udc_read(UDEPBFS), count, is_last);

#if 0
	if (req->req.actual >= 320)
		sp_print_hex_dump_bytes("30->20-", DUMP_PREFIX_OFFSET, req->req.buf, 320);
	else
		sp_print_hex_dump_bytes("30->20-", DUMP_PREFIX_OFFSET, req->req.buf,
									req->req.actual);
#endif

	sp_udc_done(ep, req, 0);

#if 0
	up(&ep->wsem);
#endif

	DEBUG_DBG("<<< %s", __func__);

	return is_last;
}

static int sp_udc_ep1_bulkin_pio(struct sp_ep *ep, struct sp_request *req)
{
	u32 count, w_count;
	int is_pingbuf;
	int is_last;
	int delay_count;
	int pre_is_pingbuf;

	DEBUG_DBG(">>> %s", __func__);

#if 0
	if (down_trylock(&ep->wsem))
		return 0;
#endif

	is_last = 0;
	delay_count = 0;

	DEBUG_DBG("1.req.actual = %d req.length=%d req->req.dma=%xh UDEP12FS = %xh",
			req->req.actual, req->req.length, req->req.dma, udc_read(UDEP12FS));

	is_pingbuf = (udc_read(UDEP12PPC) & CURR_BUFF) ? 1 : 0;

	while (1) {
		pre_is_pingbuf = is_pingbuf;
		count = sp_udc_get_ep_fifo_count(is_pingbuf, UDEP12PIC);

		w_count = sp_udc_write_packet(UDEP12FDP, req, ep->ep.maxpacket, UDEP12VB);
		udc_write(udc_read(UDEP12C) | SET_EP_IVLD, UDEP12C);
		req->req.actual += w_count;

		if (w_count != ep->ep.maxpacket)
			is_last = 1;
		else if (req->req.length == req->req.actual && !req->req.zero)
			is_last = 1;
		else
			is_last = 0;

		DEBUG_DBG("2.req.length = %d req.actual = %d UDEP12FS = %xh UDEP12PPC = %xh\t"
				"\b\b\b\b\bUDEP12POC = %xh UDEP12PIC = %xh count = %d w_count = %d\t"
				"\b\b\b\b\bis_last = %d",
				req->req.length, req->req.actual, udc_read(UDEP12FS),
				udc_read(UDEP12PPC), udc_read(UDEP12POC), udc_read(UDEP12PIC),
				count, w_count, is_last);

		if (is_last)
			break;

in_fifo_controllable:
		is_pingbuf = (udc_read(UDEP12PPC) & CURR_BUFF) ? 1 : 0;

		if (is_pingbuf == pre_is_pingbuf)
			goto in_fifo_controllable;
	}

	sp_udc_done(ep, req, 0);

#if 0
	up(&ep->wsem);
#endif

	DEBUG_DBG("3.req.actual = %d, count = %d, req.length=%d, UDEP12FS = %xh, is_last = %d",
			req->req.actual, count, req->req.length, udc_read(UDEP12FS), is_last);

#if 0
	if (req->req.actual >= 142)
		sp_print_hex_dump_bytes("20->30-", DUMP_PREFIX_OFFSET, req->req.buf, 142);
	else
		sp_print_hex_dump_bytes("20->30-", DUMP_PREFIX_OFFSET, req->req.buf,
						req->req.actual);
#endif

	DEBUG_DBG("<<< %s...", __func__);

	return is_last;
}

#ifdef USE_DMA
static int sp_ep1_bulkin_dma(struct sp_ep *ep, struct sp_request *req)
{
	u8 *buf;
	int dma_xferlen;
	int actual_length = 0;

	DEBUG_DBG(">>> %s", __func__);

	if (req->req.dma == DMA_ADDR_INVALID) {
		req->req.dma = dma_map_single(ep->dev->gadget.dev.parent, req->req.buf, dma_len_ep1,
			(ep->bEndpointAddress & USB_DIR_IN) ? DMA_TO_DEVICE : DMA_FROM_DEVICE);
	}

	buf = (u8 *) (req->req.dma + req->req.actual + actual_length);
	dma_xferlen = min(dma_len_ep1 - req->req.actual - actual_length,
						(unsigned int)UDC_FLASH_BUFFER_SIZE);
	DEBUG_DBG("%p dma_xferlen = %d %d", buf, dma_xferlen, dma_len_ep1);
	actual_length = dma_xferlen;

	if (dma_xferlen > 4096)
		DEBUG_NOTICE("dma in len err %d", dma_xferlen);

	udc_write(dma_xferlen | DMA_COUNT_ALIGN, UEP12DMACS);
	udc_write((u32) buf, UEP12DMADA);
	udc_write(udc_read(UEP12DMACS) | DMA_EN, UEP12DMACS);

	if (udc_read(UEP12DMACS) & DMA_EN) {
		udc_write(udc_read(UDLIE) | EP1_DMA_IF, UDLIE);
		return 0;
	}

	udc_write(EP1_DMA_IF, UDLIF);

	req->req.actual += actual_length;
	DEBUG_DBG("req->req.actual = %d UEP12DMACS = %xh", req->req.actual,
		  udc_read(UEP12DMACS));

	if (req->req.dma != DMA_ADDR_INVALID) {
		dma_unmap_single(ep->dev->gadget.dev.parent, req->req.dma, dma_len_ep1,
				(ep->bEndpointAddress & USB_DIR_IN)
					? DMA_TO_DEVICE : DMA_FROM_DEVICE);
		req->req.dma = DMA_ADDR_INVALID;
	}

	dma_len_ep1 = 0;
	DEBUG_DBG("<<< %s", __func__);

	return 1;
}

static int sp_udc_ep1_bulkin_dma(struct sp_ep *ep, struct sp_request *req)
{
	u32 count;
	int is_last = 0;

	DEBUG_DBG(">>> %s", __func__);

	if ((req->req.actual) || (req->req.length == 0))
		goto _TX_BULK_IN_DATA;

	/* DMA Mode */
	dma_len_ep1 = req->req.length - (req->req.length % bulkep_dma_block_size);

	if (dma_len_ep1 == bulkep_dma_block_size) {
		dma_len_ep1 = 0;
		goto _TX_BULK_IN_DATA;
	}

	if (dma_len_ep1) {
		DEBUG_DBG("ep1 bulk in dma mode,zero=%d", req->req.zero);

		udc_write(udc_read(UDLIE) & (~EP1I_IF), UDLIE);

		if (!sp_ep1_bulkin_dma(ep, req)) {
			return 0;
		} else if (req->req.length == req->req.actual && !req->req.zero) {
			is_last = 1;
			goto done_dma;
		} else if (udc_read(UDEP12FS) & 0x1) {
			DEBUG_DBG("ep1 dma->pio wait write!");
			goto done_dma;
		} else {
			count = sp_udc_write_packet(UDEP12FDP, req, ep->ep.maxpacket, UDEP12VB);
			udc_write(udc_read(UDEP12C) | SET_EP_IVLD, UDEP12C);
			req->req.actual += count;
			sp_udc_done(ep, req, 0);
			DEBUG_DBG("ep1 dma->pio write!");
			udc_write(udc_read(UDLIE) | EP1I_IF, UDLIE);

			return 1;
		}
	}

_TX_BULK_IN_DATA:
	udc_write(udc_read(UDLIE) | EP1I_IF, UDLIE);
	is_last = sp_udc_ep1_bulkin_pio(ep, req);

	return is_last;

done_dma:
	DEBUG_DBG("is_last = %d", is_last);
	if (is_last)
		sp_udc_done(ep, req, 0);

	udc_write(udc_read(UDLIE) | EP1I_IF, UDLIE);
	DEBUG_DBG("<<< %s...", __func__);

	return is_last;
}
#endif

static int sp_udc_int_in(struct sp_ep *ep, struct sp_request *req)
{
	int count;

	DEBUG_DBG(">>> %s...", __func__);

	count = sp_udc_write_packet(UDEP3DATA, req, ep->ep.maxpacket, UDEP3VB);
	udc_write((1 << 0) | (1 << 3), UDEP3CTRL);
	req->req.actual += count;

	DEBUG_DBG("write ep3, count = %d, actual = %d, length = %d, zero = %d",
				count, req->req.actual, req->req.length, req->req.zero);

	if (req->req.actual == req->req.length) {
		DEBUG_DBG("write ep3, sp_udc_done");
		sp_udc_done(ep, req, 0);
	}

	DEBUG_DBG("<<< %s...", __func__);

	return 1;
}

static int sp_udc_ep1_bulkin(struct sp_ep *ep, struct sp_request *req)
{
	int ret;

#ifdef USE_DMA
	if (dma_len_ep1 == 0)
		ret = sp_udc_ep1_bulkin_dma(ep, req);
#else
	ret = sp_udc_ep1_bulkin_pio(ep, req);
#endif

	return ret;
}

static int sp_udc_ep11_bulkout(struct sp_ep *ep, struct sp_request *req)
{
	int ret = 0;

#ifdef USE_DMA
	ret = sp_udc_ep11_bulkout_dma(ep, req);
#else
	ret = sp_udc_ep11_bulkout_pio(ep, req);
#endif

	return ret;
}

static int sp_udc_handle_ep(struct sp_ep *ep, struct sp_request *req)
{
	int ret = 0;
	int idx = ep->bEndpointAddress & 0x7F;

	DEBUG_DBG(">>> %s", __func__);

	if (!req) {
		int empty = list_empty(&ep->queue);

		if (empty) {
			req = NULL;
			ret = 1;
			DEBUG_DBG("idx = %d, req is NULL", idx);
		} else {
			req = list_entry(ep->queue.next, struct sp_request, queue);
		}
	}

	if (req) {
		switch (idx) {
		case EP1:
			if ((udc_read(UDEP12FS) & 0x1) == 0)
				ret = sp_udc_ep1_bulkin(ep, req);

			break;
		case EP3:
			ret = sp_udc_int_in(ep, req);

			break;
		case EP11:
			if (udc_read(UDEPBFS) & 0x22)
				ret = sp_udc_ep11_bulkout(ep, req);

			break;
		}
	}

	DEBUG_DBG("<<< %s ret = %d", __func__, ret);

	return ret;
}

#ifdef USE_DMA
static void ep1_dma_handle(struct sp_udc *dev)
{
	struct sp_ep *ep = &memory.ep[1];
	struct sp_request *req;
	int ret;

	if (list_empty(&ep->queue)) {
		req = NULL;
		pr_err("ep1_dma req is NULL\t!");

		return;
	}

	req = list_entry(ep->queue.next, struct sp_request, queue);
	if (req->req.actual != 0) {
		pr_err("WHY ep1");

		if (req->req.actual != req->req.length)
			return;
	}

	req->req.actual += dma_len_ep1;
	if (req->req.dma != DMA_ADDR_INVALID) {
		dma_unmap_single(ep->dev->gadget.dev.parent, req->req.dma, dma_len_ep1,
					(ep->bEndpointAddress & USB_DIR_IN)
						? DMA_TO_DEVICE : DMA_FROM_DEVICE);

		req->req.dma = DMA_ADDR_INVALID;
	}

	if (req->req.length == req->req.actual && !req->req.zero) {
		sp_udc_done(ep, req, 0);
		dma_len_ep1 = 0;

		if (!(udc_read(UDEP12FS) & 0x1))
			sp_udc_handle_ep(ep, NULL);

		DEBUG_DBG("ep1 dma: %d", udc_read(UDEP12FS));

		return;
	}

	if (!(udc_read(UDEP12FS) & 0x1)) {
		ret = sp_udc_write_packet(UDEP12FDP, req, ep->ep.maxpacket, UDEP12VB);
		udc_write(udc_read(UDEP12C) | SET_EP_IVLD, UDEP12C);
		req->req.actual += ret;
		sp_udc_done(ep, req, 0);

		DEBUG_DBG("DMA->write fifo by pio count=%d!", ret);
	} else {
		DEBUG_DBG("wait DMA->write fifo by pio!");
	}

	dma_len_ep1 = 0;
	udc_write(udc_read(UDLIE) | EP1I_IF, UDLIE);
}

static void ep11_dma_handle(struct sp_udc *dev)
{
	struct sp_ep *ep = &memory.ep[11];
	struct sp_request *req;

	DEBUG_DBG(">>> %s", __func__);

	if (list_empty(&ep->queue)) {
		req = NULL;
		DEBUG_DBG("ep11_dma req is NULL\t!");

		return;
	}

	req = list_entry(ep->queue.next, struct sp_request, queue);	// req->req.actual == 0 ?
	req->req.actual += dma_xferlen_ep11;

	if (req->req.length == req->req.actual) {
		dma_xferlen_ep11 = 0;

		if (req->req.dma != DMA_ADDR_INVALID) {
			dma_unmap_single(ep->dev->gadget.dev.parent, req->req.dma, req->req.length,
						(ep->bEndpointAddress & USB_DIR_IN)
							? DMA_TO_DEVICE : DMA_FROM_DEVICE);
			req->req.dma = DMA_ADDR_INVALID;
		}

		udc_write(udc_read(UDCIE) & (~EPB_DMA_IF), UDCIE);
		sp_udc_done(ep, req, 0);
	}

	DEBUG_DBG("<<< %s", __func__);
}
#endif

static int sp_udc_irq(struct sp_udc *dev)
{
	u32 irq_en1_flags;
	u32 irq_en2_flags;
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);

	irq_en1_flags = udc_read(UDCIF) & udc_read(UDCIE);
	irq_en2_flags = udc_read(UDLIE) & udc_read(UDLIF);

	if (irq_en2_flags & RESET_RELEASE_IF) {
		udc_write(RESET_RELEASE_IF, UDLIF);
		DEBUG_DBG("reset end irq");
	}

	if (irq_en2_flags & RESET_IF) {
		DEBUG_DBG("reset irq");
		/* two kind of reset :		*/
		/* - reset start -> pwr reg = 8	*/
		/* - reset end   -> pwr reg = 0	*/
		dev->gadget.speed = USB_SPEED_UNKNOWN;
		dev->address = 0;

		udc_write((udc_read(UDLCSET) | 8) & 0xFE, UDLCSET);
		udc_write(RESET_IF, UDLIF);

		/* Allow LNK to suspend PHY */
		udc_write(udc_read(UDCCS) & (~UPHY_CLK_CSS), UDCCS);
		clearHwState_UDC();

		dev->ep0state = EP0_IDLE;
		dev->gadget.speed = USB_SPEED_FULL;

		spin_unlock_irqrestore(&dev->lock, flags);

		return IRQ_HANDLED;
	}

	/* force disconnect interrupt */
	if (irq_en1_flags & VBUS_IF) {
		DEBUG_DBG("vbus_irq[%xh]", udc_read(UDLCSET));
		udc_write(VBUS_IF, UDCIF);
		vbusInt_handle();
	}

	if (irq_en2_flags & SUS_IF) {
		DEBUG_DBG(">>> IRQ: Suspend");
		DEBUG_DBG("clear  Suspend Event");
		udc_write(SUS_IF, UDLIF);

		if (dev->driver) {
			if (dev->gadget.speed != USB_SPEED_UNKNOWN
					&& dev->driver->suspend)
				dev->driver->suspend(&dev->gadget);

			if (dev->driver->disconnect)
				dev->driver->disconnect(&dev->gadget);
		}

		dev->gadget.speed = USB_SPEED_UNKNOWN;
		dev->address = 0;

		DEBUG_DBG("<<< IRQ: Suspend");
	}

	if (irq_en2_flags & EP0S_IF) {
		udc_write(EP0S_IF, UDLIF);
		DEBUG_DBG("IRQ:EP0S_IF %d, dev->ep0state = %d", udc_read(UDEP0CS), dev->ep0state);
		if ((udc_read(UDEP0CS) & (EP0_OVLD | EP0_OUT_EMPTY)) ==
						(EP0_OVLD | EP0_OUT_EMPTY))
			udc_write(udc_read(UDEP0CS) | CLR_EP0_OUT_VLD, UDEP0CS);

		if (dev->ep0state == EP0_IDLE)
			sp_udc_handle_ep0(dev);
	}

	if ((irq_en2_flags & EP0I_IF)) {
		DEBUG_DBG("IRQ:EP0I_IF %d, dev->ep0state = %d", udc_read(UDEP0CS), dev->ep0state);
		udc_write(EP0I_IF, UDLIF);

		if (dev->ep0state != EP0_IDLE)
			sp_udc_handle_ep0(dev);
		else
			udc_write(udc_read(UDEP0CS) & (~EP_DIR), UDEP0CS);
	}

	if ((irq_en2_flags & EP0O_IF)) {
		DEBUG_DBG("IRQ:EP0O_IF %d, dev->ep0state = %d", udc_read(UDEP0CS), dev->ep0state);
		udc_write(EP0O_IF, UDLIF);

		if (dev->ep0state != EP0_IDLE)
			sp_udc_handle_ep0(dev);
	}

#ifdef USE_DMA
	/* dma finish */
	if (irq_en2_flags & EP1_DMA_IF) {
		DEBUG_DBG("IRQ:UDC ep1 DMA");
		udc_write(EP1_DMA_IF, UDLIF);

		if (dma_len_ep1)
			ep1_dma_handle(dev);
	}
#endif

	if (irq_en2_flags & EP1I_IF) {
		DEBUG_DBG("IRQ:ep1 in %xh", udc_read(UDCIE));
		udc_write(EP1I_IF, UDLIF);
		sp_udc_handle_ep(&dev->ep[1], NULL);
	}

	if (irq_en2_flags & EP3I_IF) {
		DEBUG_DBG("IRQ:ep3 in int");
		udc_write(EP3I_IF, UDLIF);
		sp_udc_handle_ep(&dev->ep[3], NULL);
	}

#ifdef USE_DMA
	if (irq_en1_flags & EPB_DMA_IF) {
		DEBUG_DBG("IRQ:UDC ep11 DMA");
		mdelay(1);
		udc_write(EPB_DMA_IF, UDCIF);
		ep11_dma_handle(dev);
		sp_udc_handle_ep(&dev->ep[11], NULL);
	}
#endif

	if (udc_read(UDNBIF) & EP11O_IF) {
		udc_write(EP11O_IF, UDNBIF);
		udc_write(udc_read(UDNBIE) & (~EP11O_IF), UDNBIE);
		DEBUG_DBG("IRQ:ep11 out %xh %xh state=%d", udc_read(UDNBIE), udc_read(UDEPBFS),
									dev->gadget.state);
		sp_udc_handle_ep(&dev->ep[11], NULL);
	}

	if (udc_read(UDNBIF) & SOF_IF) {
		udc_write(SOF_IF, UDNBIF);
		udc_write(udc_read(UDNBIE) | (SOF_IF), UDNBIE);
	}

	spin_unlock_irqrestore(&dev->lock, flags);

	return IRQ_HANDLED;
}

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct sp_udc *udc = &memory;

	return sp_udc_irq(udc);
}

static int sp_udc_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct sp_udc *dev;
	unsigned long flags;
	int idx;
	struct sp_request *req = to_sp_req(_req);
	struct sp_ep *ep = to_sp_ep(_ep);

	DEBUG_DBG(">>> %s...", __func__);

	if (unlikely(!_ep || (!ep->ep.desc && ep->ep.name != ep0name))) {
		pr_notice("%s: invalid args", __func__);
		return -EINVAL;
	}

	dev = ep->dev;
	if (unlikely(!dev->driver || dev->gadget.speed == USB_SPEED_UNKNOWN))
		return -ESHUTDOWN;

	local_irq_save(flags);
	_req->status = -EINPROGRESS;
	_req->actual = 0;

	idx = ep->bEndpointAddress & 0x7F;
	if (list_empty(&ep->queue))
		if (idx == EP0 && sp_udc_handle_ep0_proc(ep, req, 0))
			req = NULL;

	DEBUG_DBG("req = %x, ep=%d, req_config=%d", req, idx, dev->req_config);
	if (likely(req)) {
		list_add_tail(&req->queue, &ep->queue);
		if (idx && idx != EP3)
			sp_udc_handle_ep(ep, NULL);
	}

	local_irq_restore(flags);
	DEBUG_DBG("<<< %s...", __func__);

	return 0;
}

static int sp_udc_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct sp_ep *ep;
	struct sp_udc *udc;
	struct sp_request *req = NULL;
	int retval = -EINVAL;

	DEBUG_DBG(">>> %s...", __func__);
	DEBUG_DBG("%s dequeue", _ep->name);

	if (!the_controller->driver)
		return -ESHUTDOWN;

	if (!_ep || !_req)
		return retval;

	ep = to_sp_ep(_ep);
	udc = to_sp_udc(ep->gadget);
	list_for_each_entry(req, &ep->queue, queue) {
		if (&req->req == _req) {
			list_del_init(&req->queue);
			_req->status = -ECONNRESET;
			retval = 0;
			break;
		}
	}

	if (retval == 0) {
		DEBUG_DBG("dequeued req %p from %s, len %d buf %p",
					req, _ep->name, _req->length, _req->buf);
		sp_udc_done(ep, req, -ECONNRESET);
	}

	DEBUG_DBG("<<< %s...retval = %d", __func__, retval);

	return retval;
}

/* return 0 : disconnect	*/
/* return 1 : connect		*/
static int sp_vbus_detect(void)
{
	return 1;
}

static void sp_udc_enable(struct sp_udc *udc)
{
	/*						*/
	/* usb device interrupt enable			*/
	/* ---force usb bus disconnect enable		*/
	/* ---force usb bus connect interrupt enable	*/
	/* ---vbus interrupt enable			*/
	/*						*/
	DEBUG_DBG(">>> %s", __func__);

	/* usb device controller interrupt flag */
	udc_write(udc_read(UDCIF) & 0xFFFF, UDCIF);
	/* usb device link layer interrupt flag */
	udc_write(0xefffffff, UDLIF);

	udc_write(VBUS_IF, UDCIE);
	udc_write(EP0S_IF | RESET_IF | RESET_RELEASE_IF, UDLIE);

	if (sp_vbus_detect()) {
		udc_write(udc_read(UDLIE) | SUS_IF, UDLIE);
		udelay(200);
		udc_write(udc_read(UDCCS) | UPHY_CLK_CSS, UDCCS);	/* PREVENT SUSP */
		udelay(200);
		/* Force to Connect */
	}

	DEBUG_DBG("func:%s line:%d", __func__, __LINE__);
	DEBUG_DBG("<<< %s", __func__);
}

static int sp_udc_get_frame(struct usb_gadget *_gadget)
{
	u32 sof_value;

	DEBUG_NOTICE(">>> %s...", __func__);
	sof_value = udc_read(UDFRNUM);
	DEBUG_NOTICE("<<< %s...", __func__);

	return sof_value;
}

static int sp_udc_wakeup(struct usb_gadget *_gadget)
{
	DEBUG_DBG(">>> %s...", __func__);
	DEBUG_DBG("<<< %s...", __func__);

	return 0;
}

static int sp_udc_set_selfpowered(struct usb_gadget *_gadget, int is_selfpowered)
{
	DEBUG_DBG(">>> %s...", __func__);
	DEBUG_DBG("<<< %s...", __func__);

	return 0;
}

static int sp_udc_pullup(struct usb_gadget *gadget, int is_on)
{
	DEBUG_NOTICE(">>> %s...", __func__);

	if (is_on) {
		DEBUG_NOTICE("Force to Connect");
		/* Force to Connect */
		udc_write((udc_read(UDLCSET) | 8) & 0xFE, UDLCSET);
	} else {
		DEBUG_NOTICE("Force to Disconnect");
		/* Force to Disconnect */
		udc_write(udc_read(UDLCSET) | SOFT_DISC, UDLCSET);
	}

	DEBUG_NOTICE("<<< %s...", __func__);

	return 0;
}

static int sp_udc_vbus_session(struct usb_gadget *gadget, int is_active)
{
	DEBUG_DBG(">>> %s...", __func__);
	DEBUG_DBG("<<< %s...", __func__);

	return 0;
}

static int sp_vbus_draw(struct usb_gadget *_gadget, unsigned int ma)
{
	DEBUG_DBG(">>> %s...", __func__);
	DEBUG_DBG("<<< %s...", __func__);

	return 0;
}

static int sp_udc_start(struct usb_gadget *gadget, struct usb_gadget_driver *driver)
{
	struct sp_udc *udc = the_controller;

	DEBUG_NOTICE(">>> %s...", __func__);

	/* Sanity checks */
	if (!udc)
		return -ENODEV;
	if (udc->driver)
		return -EBUSY;

	if (!driver->bind || !driver->setup || driver->speed < USB_SPEED_FULL) {
		pr_err("Invalid driver: bind %p setup %p speed %d",
				driver->bind, driver->setup, driver->speed);

		return -EINVAL;
	}

	/* Hook the driver */
	udc->driver = driver;

	sp_udc_enable(udc);

	DEBUG_NOTICE("<<< %s...", __func__);

	return 0;
}

static int sp_udc_stop(struct usb_gadget *gadget)
{
	struct sp_udc *udc = the_controller;

	if (!udc)
		return -ENODEV;

	DEBUG_NOTICE(">>> %s...", __func__);

	/* report disconnect */
	if (udc->driver->disconnect)
		udc->driver->disconnect(&udc->gadget);

	udc->driver->unbind(&udc->gadget);
	udc->driver = NULL;

	DEBUG_NOTICE("<<< %s...", __func__);

	return 0;
}

static struct usb_ep *find_ep(struct usb_gadget *gadget, const char *name)
{
	struct usb_ep *ep;

	list_for_each_entry (ep, &gadget->ep_list, ep_list) {
		if (strcmp(ep->name, name) == 0)
			return ep;
	}

	return NULL;
}

static struct usb_ep *sp_match_ep(struct usb_gadget *_gadget,
					struct usb_endpoint_descriptor *desc,
						struct usb_ss_ep_comp_descriptor *ep_comp)
{
	struct usb_ep *ep;
	u8 type;

	type = desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK;
	desc->bInterval = 5;
	ep = NULL;

	if (type == USB_ENDPOINT_XFER_BULK)
		ep = find_ep(_gadget, (USB_DIR_IN & desc->bEndpointAddress) ?
							"ep1in-bulk" : "ep11out-bulk");
	else if (type == USB_ENDPOINT_XFER_INT)
		ep = find_ep(_gadget, "ep3in-int");
	else if (type == USB_ENDPOINT_XFER_ISOC)
		ep = find_ep(_gadget, (USB_DIR_IN & desc->bEndpointAddress) ?
			"ep5-iso" : "ep12-iso");

	return ep;
}

static void uphy_init(int port_num)
{
	unsigned int val, set;

	/* 1. Default value modification */
	if(0 == port_num) {
	    writel(RF_MASK_V(0xffff, 0x4002), moon4_reg + MO4_UPHY0_CTRL0);
	    writel(RF_MASK_V(0xffff, 0x8747), moon4_reg + MO4_UPHY0_CTRL1);
	} else if (1 == port_num) {
	    writel(RF_MASK_V(0xffff, 0x4004), moon4_reg + MO4_UPHY1_CTRL0);
	    writel(RF_MASK_V(0xffff, 0x8747), moon4_reg + MO4_UPHY1_CTRL1);
	}

	/* 2. PLL power off/on twice */
	if(0 == port_num){
	    writel(RF_MASK_V(0xffff, 0x88), moon4_reg + MO4_UPHY0_CTRL3);
	    mdelay(1);
	    writel(RF_MASK_V(0xffff, 0x80), moon4_reg + MO4_UPHY0_CTRL3);
	    mdelay(1);
	    writel(RF_MASK_V(0xffff, 0x88), moon4_reg + MO4_UPHY0_CTRL3);
	    mdelay(1);
	    writel(RF_MASK_V(0xffff, 0x80), moon4_reg + MO4_UPHY0_CTRL3);
	    mdelay(1);
	    writel(RF_MASK_V(0xffff, 0x0), moon4_reg + MO4_UPHY0_CTRL3);
	} else if (1 == port_num){
	    writel(RF_MASK_V(0xffff, 0x88), moon4_reg + MO4_UPHY1_CTRL3);
	    mdelay(1);
	    writel(RF_MASK_V(0xffff, 0x80), moon4_reg + MO4_UPHY1_CTRL3);
	    mdelay(1);
	    writel(RF_MASK_V(0xffff, 0x88), moon4_reg + MO4_UPHY1_CTRL3);
	    mdelay(1);
	    writel(RF_MASK_V(0xffff, 0x80), moon4_reg + MO4_UPHY1_CTRL3);
	    mdelay(1);
	    writel(RF_MASK_V(0xffff, 0x0), moon4_reg + MO4_UPHY1_CTRL3);
	}
	mdelay(1);

	/* 3. reset UPHY0/1 */
	if (0 == port_num) {
	    writel(RF_MASK_V_SET(1 << 13), moon0_reg + MO0_RESET2);
	    writel(RF_MASK_V_CLR(1 << 13), moon0_reg + MO0_RESET2);
	} else if (1 == port_num) {
	    writel(RF_MASK_V_SET(1 << 14), moon0_reg + MO0_RESET2);
	    writel(RF_MASK_V_CLR(1 << 14), moon0_reg + MO0_RESET2);
	}
	mdelay(1);

	/* 4. UPHY 0 internal register modification */
	writel(0x8b, uphy_reg + UPHY_CFG7);

	/* 5. USBC 0 reset */
	if (0 == port_num) {
	    writel(RF_MASK_V_SET(1 << 10), moon0_reg + MO0_RESET2);
	    writel(RF_MASK_V_CLR(1 << 10), moon0_reg + MO0_RESET2);
	} else if (1 == port_num) {
	    writel(RF_MASK_V_SET(1 << 11), moon0_reg + MO0_RESET2);
	    writel(RF_MASK_V_CLR(1 << 11), moon0_reg + MO0_RESET2);
	}

	/* 6. Backup solution to workaround real IC USB clock issue	  */
	/*    (issue: hang on reading EHCI_USBSTS after EN_ASYNC_SCHEDULE) */
	if(0 == port_num) {
	    if (readl(hb_gp_reg + ADDRESS_2_DATA) & 0x1) {	/* G350.2 bit[0] */
		    writel(RF_MASK_V_SET(1 << 6), moon4_reg + MO4_UPHY0_CTRL2);
	    }
	} else if (1 == port_num) {
	    if (readl(hb_gp_reg + ADDRESS_2_DATA) & 0x2) {	/* G350.2 bit[1] */
		    writel(RF_MASK_V_SET(1 << 6), moon4_reg + MO4_UPHY1_CTRL2);
	    }
	}

	/* 7. OTP for USB DISC (disconnect voltage) */
	val = readl(hb_gp_reg + ADDRESS_6_DATA);

	if (0 == port_num)
		set = val & 0x1f; 				/* UPHY0 DISC */
	else if (1 == port_num)
		set = (val >> 5) & 0x1f;			/* UPHY1 DISC */

	if (!set)
		set = DEFAULT_UPHY_DISC;
	else if (set <= ORIG_UPHY_DISC)
		set += 2;

	val = (readl(uphy_reg + UPHY_CFG7) & ~0x1f) | set;
	writel(val, uphy_reg + UPHY_CFG7);
}

static void usb_power_init(int is_host, int port_num)
{
   	/* a. enable pin mux control (sft_cfg_8, bit2/bit3)	*/
	/*    Host: enable					*/
    	/*    Device: disable					*/
	if (is_host)
		writel(RF_MASK_V_SET(1 << (2 + port_num)), moon1_reg + MO1_SFT_CFG_3);
	else
		writel(RF_MASK_V_CLR(1 << (2 + port_num)), moon1_reg + MO1_SFT_CFG_3);

    	/* b. USB control register:			*/
    	/*    Host:   ctrl=1, host sel=1, type=1	*/
    	/*    Device  ctrl=1, host sel=0, type=0	*/
	if (is_host) {
		if (0 == port_num)
			writel(RF_MASK_V_SET(7 << 4), moon4_reg + MO4_USBC_CTL);
		else if (1 == port_num)
			writel(RF_MASK_V_SET(7 << 12), moon4_reg + MO4_USBC_CTL);
	} else {
		if (0 == port_num) {
			writel(RF_MASK_V_SET(1 << 4), moon4_reg + MO4_USBC_CTL);
			writel(RF_MASK_V_CLR(3 << 5), moon4_reg + MO4_USBC_CTL);
		} else if (1 == port_num) {
			writel(RF_MASK_V_SET(1 << 12), moon4_reg + MO4_USBC_CTL);
			writel(RF_MASK_V_CLR(3 << 13), moon4_reg + MO4_USBC_CTL);
		}
	}
}



static const struct usb_gadget_ops sp_ops = {
	.get_frame		= sp_udc_get_frame,
	.wakeup			= sp_udc_wakeup,
	.set_selfpowered	= sp_udc_set_selfpowered,
	.pullup			= sp_udc_pullup,
	.vbus_session		= sp_udc_vbus_session,
	.vbus_draw		= sp_vbus_draw,
	.udc_start		= sp_udc_start,
	.udc_stop		= sp_udc_stop,
	.match_ep		= sp_match_ep,
};

static const struct usb_ep_ops sp_ep_ops = {
	.enable			= sp_udc_ep_enable,
	.disable		= sp_udc_ep_disable,

	.alloc_request		= sp_udc_alloc_request,
	.free_request		= sp_udc_free_request,

	.queue			= sp_udc_queue,
	.dequeue		= sp_udc_dequeue,

	.set_halt		= sp_udc_set_halt,
};

static struct sp_udc memory = {
	.gadget = {
			.max_speed = USB_SPEED_HIGH,
			.ops = &sp_ops,
			.ep0 = &memory.ep[0].ep,
			.name = gadget_name,
		   },

	/* control endpoint */
	.ep[0] = {
		  .num = 0,
		  .ep = {
			 .name = ep0name,
			 .ops = &sp_ep_ops,
			 .maxpacket = EP0_FIFO_SIZE,
			 },
		  .dev = &memory,
		  },

	/* first group of endpoints */
	.ep[1] = {
		  .num = 1,
		  .ep = {
			 .name = "ep1in-bulk",
			 .ops = &sp_ep_ops,
			 .maxpacket = 512,
			 },
		  .dev = &memory,
		  .fifo_size = 64,
		  .bEndpointAddress = USB_DIR_IN | EP1,
		  .bmAttributes = USB_ENDPOINT_XFER_BULK,
		  },

#if 0
	.ep[2] = {
		  .num = 2,
		  .ep = {
			 .name = "ep2out-bulk",
			 .ops = &sp_ep_ops,
			 .maxpacket = FIFO_SIZE64,
			 },
		  .dev = &memory,
		  .fifo_size = 64,
		  .bEndpointAddress = USB_DIR_OUT | EP2,
		  .bmAttributes = USB_ENDPOINT_XFER_BULK,
		  },
#endif

	.ep[3] = {
		  .num = 3,
		  .ep = {
			 .name = "ep3in-int",
			 .ops = &sp_ep_ops,
			 .maxpacket = 64,
			 },
		  .dev = &memory,
		  .fifo_size = 64,
		  .bEndpointAddress = USB_DIR_IN | EP3,
		  .bmAttributes = USB_ENDPOINT_XFER_INT,
		  },

#if 0
	.ep[4] = {
		  .num = 4,
		  .ep = {
			 .name = "ep4in-int",
			 .ops = &sp_ep_ops,
			 .maxpacket = 64,
			 },
		  .dev = &memory,
		  .fifo_size = 64,
		  .bEndpointAddress = USB_DIR_IN | EP4,
		  .bmAttributes = USB_ENDPOINT_XFER_INT,
		  },

	.ep[5] = {
		  .num = 5,
		  .ep = {
			 .name = "ep5-iso",
			 .ops = &sp_ep_ops,
			 .maxpacket = 1024 * 3,
			 },
		  .dev = &memory,
		  .fifo_size = 64,
		  .bEndpointAddress = USB_DIR_OUT | EP5,
		  .bmAttributes = USB_ENDPOINT_XFER_ISOC,
		  },

	.ep[6] = {
		  .num = 6,
		  .ep = {
			 .name = "ep6in-int",
			 .ops = &sp_ep_ops,
			 .maxpacket = 64,
			 },
		  .dev = &memory,
		  .fifo_size = 64,
		  .bEndpointAddress = USB_DIR_IN | EP6,
		  .bmAttributes = USB_ENDPOINT_XFER_INT,
		  },

	.ep[7] = {
		  .num = 7,
		  .ep = {
			 .name = "ep7-iso",
			 .ops = &sp_ep_ops,
			 .maxpacket = FIFO_SIZE64,
			 },
		  .dev = &memory,
		  .fifo_size = 64,
		  .bEndpointAddress = USB_DIR_OUT | EP7,
		  .bmAttributes = USB_ENDPOINT_XFER_ISOC,
		  },

	.ep[8] = {
		  .num = 8,
		  .ep = {
			 .name = "ep8in-bulk",
			 .ops = &sp_ep_ops,
			 .maxpacket = FIFO_SIZE64,
			 },
		  .dev = &memory,
		  .fifo_size = 64,
		  .bEndpointAddress = USB_DIR_IN | EP8,
		  .bmAttributes = USB_ENDPOINT_XFER_BULK,
		  },

	.ep[9] = {
		  .num = 9,
		  .ep = {
			 .name = "ep9out-bulk",
			 .ops = &sp_ep_ops,
			 .maxpacket = FIFO_SIZE64,
			 },
		  .dev = &memory,
		  .fifo_size = 64,
		  .bEndpointAddress = USB_DIR_OUT | EP9,
		  .bmAttributes = USB_ENDPOINT_XFER_BULK,
		  },

	.ep[10] = {
		   .num = 10,
		   .ep = {
			  .name = "ep10in-bulk",
			  .ops = &sp_ep_ops,
			  .maxpacket = FIFO_SIZE64,
			  },
		   .dev = &memory,
		   .fifo_size = 64,
		   .bEndpointAddress = USB_DIR_IN | EP10,
		   .bmAttributes = USB_ENDPOINT_XFER_BULK,
		   },
#endif

	.ep[11] = {
		   .num = 11,
		   .ep = {
			  .name = "ep11out-bulk",
			  .ops = &sp_ep_ops,
			  .maxpacket = 512,
			  },
		   .dev = &memory,
		   .fifo_size = 512,
		   .bEndpointAddress = USB_DIR_OUT | EP11,
		   .bmAttributes = USB_ENDPOINT_XFER_BULK,
		   },

#if 0
	.ep[12] = {
		   .num = 12,
		   .ep = {
			  .name = "ep12-iso",
			  .ops = &sp_ep_ops,
			  .maxpacket = 1024,
			  },
		   .dev = &memory,
		   .fifo_size = 64,
		   .bEndpointAddress = USB_DIR_OUT | EP12,
		   .bmAttributes = USB_ENDPOINT_XFER_ISOC,
		   },
#endif
};

static void sp_udc_reinit(struct sp_udc *udc)
{
	u32 i = 0;

	DEBUG_DBG(">>> %s", __func__);

	/* device/ep0 records init */
	INIT_LIST_HEAD(&udc->gadget.ep_list);
	INIT_LIST_HEAD(&udc->gadget.ep0->ep_list);
	udc->ep0state = EP0_IDLE;

	for (i = 0; i < SP_MAXENDPOINTS; i++) {
		struct sp_ep *ep = &udc->ep[i];

		if (i != 0)
			list_add_tail(&ep->ep.ep_list, &udc->gadget.ep_list);

		ep->dev = udc;
		ep->desc = NULL;
		ep->halted = 0;
		INIT_LIST_HEAD(&ep->queue);
		usb_ep_set_maxpacket_limit(&ep->ep, ep->ep.maxpacket);
	}

	DEBUG_DBG("<<< %s", __func__);
}

#ifdef CONFIG_FIQ_GLUE
static int fiq_isr(int fiq, void *data)
{
	sp_udc_irq(irq_num, &memory);

	return IRQ_HANDLED;
}

static void fiq_handler(struct fiq_glue_handler *h, void *regs, void *svc_sp)
{
	void __iomem *cpu_base = gic_base(GIC_CPU_BASE);
	u32 irqstat, irqnr;

	irqstat = readl_relaxed(cpu_base + GIC_CPU_HIGHPRI);
	irqnr = irqstat & ~0x1c00;

	if (irqnr == irq_num) {
		readl_relaxed(cpu_base + GIC_CPU_INTACK);
		fiq_isr(irqnr, h);
		writel_relaxed(irqstat, cpu_base + GIC_CPU_EOI);
	}
}
static irqreturn_t udcThreadHandler(int irq, void *dev_id)
{
	DEBUG_DBG("<DSR>");

	return IRQ_HANDLED;
}
#endif

/************************************ PLATFORM DRIVER & DEVICE ************************************/
static int sp_udc_probe(struct udevice *udev)
{
	struct sp_udc *udc = &memory;
	fdt_addr_t base;
	int ret;

	base = dev_read_addr_index(udev, 0);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	base_addr = ioremap(base, 128);
	if (!base_addr)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 1);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	moon0_reg = ioremap(base, 128);
	if (!moon0_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 2);
		if (base == FDT_ADDR_T_NONE)
			return -EINVAL;

	moon1_reg = ioremap(base, 128);
	if (!moon1_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 3);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	moon4_reg = ioremap(base, 128);
	if (!moon4_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 4);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	uphy_reg = ioremap(base, 128);
	if (!uphy_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 5);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	hb_gp_reg = ioremap(base, 128);
	if (!hb_gp_reg)
		return -ENOMEM;

	udc->dev = udev;
	the_controller = udc;
	sp_udc_reinit(udc);

	udc->vbus = 0;

	spin_lock_init(&udc->lock);
	spin_lock_init(&plock);
	spin_lock_init(&qlock);

	init_ep_spin();

	ret = usb_add_gadget_udc((struct device *)udev, &udc->gadget);
	if (ret)
		return ret;

	/* phy configurations */
	uphy_init(udev->seq_);

	/* set USB device mode */
	usb_power_init(0, udev->seq_);

	return 0;
}

static int sp_udc_remove(struct udevice *udev)
{
	struct sp_udc *udc = &memory;

	usb_del_gadget_udc(&udc->gadget);

	/* set USB host mode */
	usb_power_init(1, udev->seq_);

	return 0;
}

static const struct udevice_id sp_udc_ids[] = {
	{ .compatible = "sunplus,sp7021-usb-udc0" },
	{ .compatible = "sunplus,sp7021-usb-udc1" },
	{ },
};

U_BOOT_DRIVER(sunplus_udc) = {
	.name = "sp-udc",
	.id = UCLASS_USB_GADGET_GENERIC,
	.of_match = sp_udc_ids,
	.probe = sp_udc_probe,
	.remove = sp_udc_remove,
	.priv_auto = sizeof(struct sp_udc),
};

