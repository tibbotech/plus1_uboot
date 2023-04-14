#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/usb/composite.h>
#include <asm/unaligned.h>
#include <linux/usb/otg.h>
#include <linux/usb/gadget.h>
#include <linux/usb/ch9.h>
#include <malloc.h>
#include <common.h>
#include <linux/io.h>
#include <dm/devres.h>
#include <dm.h>

#include "sp_udc2.h"

#define DRIVER_NAME "sp-udc"

static const char* const ep_names[] = {
	"ep0",
	"ep1in",
	"ep2in",
	"ep3in",
	"ep4in",
	"ep5out",
	"ep6out",
	"ep7out",
	"ep8out",
};
#define ep0name		ep_names[0]

static struct sp_udc *sp_udc_arry[2] = {NULL, NULL};

#if 0
/* test mode, When test mode is enabled, sof is not detected */
static uint test_mode;
#endif

static uint dmsg = 0x3;

#define UDC_LOGE(fmt, arg...)		pr_err(fmt, ##arg)
#define UDC_LOGW(fmt, arg...)		do { if (dmsg & BIT(0)) pr_warn(fmt, ##arg); } while (0)
#define UDC_LOGL(fmt, arg...)		do { if (dmsg & BIT(1)) pr_info(fmt, ##arg); } while (0)
#define UDC_LOGI(fmt, arg...)		do { if (dmsg & BIT(2)) pr_info(fmt, ##arg); } while (0)
#define UDC_LOGD(fmt, arg...)		do { if (dmsg & BIT(3)) pr_info(fmt, ##arg); } while (0)

/* Produces a mask of set bits covering a range of a 32-bit value */
static inline uint32_t bitfield_mask(uint32_t shift, uint32_t width)
{
	return ((1 << width) - 1) << shift;
}

/* Extract the value of a bitfield found within a given register value */
static inline uint32_t bitfield_extract(uint32_t reg_val, uint32_t shift, uint32_t width)
{
	return (reg_val & bitfield_mask(shift, width)) >> shift;
}

/* Replace the value of a bitfield found within a given register value */
static inline uint32_t bitfield_replace(uint32_t reg_val, uint32_t shift, uint32_t width, uint32_t val)
{
	uint32_t mask = bitfield_mask(shift, width);

	return (reg_val & ~mask) | (val << shift);
}

/* internal `bitfield_replace' version, for register value with mask bits */
#define __bitfield_replace(value, shift, width, new_value)	\
	(bitfield_replace(value, shift, width, new_value) | bitfield_mask(shift + 16, width))

static int sp_udc_get_frame(struct usb_gadget *gadget);
static int sp_udc_pullup(struct usb_gadget *gadget, int is_on);
static int sp_udc_vbus_session(struct usb_gadget *gadget, int is_active);
static int sp_udc_start(struct usb_gadget *gadget, struct usb_gadget_driver *driver);
static int sp_udc_stop(struct usb_gadget *gadget);
#if 0
static struct usb_ep *sp_udc_match_ep(struct usb_gadget *_gadget,
					struct usb_endpoint_descriptor *desc,
						struct usb_ss_ep_comp_descriptor *ep_comp);
#endif
static int sp_udc_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc);
static int sp_udc_ep_disable(struct usb_ep *_ep);
static struct usb_request *sp_udc_alloc_request(struct usb_ep *_ep, gfp_t mem_flags);
static void sp_udc_free_request(struct usb_ep *_ep, struct usb_request *_req);
static int sp_udc_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags);
static int sp_udc_dequeue(struct usb_ep *_ep, struct usb_request *_req);
static int sp_udc_set_halt(struct usb_ep *_ep, int value);
static int sp_udc_probe(struct udevice *dev);
static int hal_udc_setup(struct sp_udc *udc, const struct usb_ctrlrequest *ctrl);

static void uphy_init(int port_num)
{
	unsigned int val, set;

#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
	if (0 == port_num) {
		/* enable clock for UPHY, USBC and OTP */
		writel(RF_MASK_V_SET(1 << 8), moon0_reg + CLOCK_ENABLE3);
		writel(RF_MASK_V_SET(1 << 13), moon0_reg + CLOCK_ENABLE3);
		writel(RF_MASK_V_SET(1 << 2), moon0_reg + CLOCK_ENABLE2);

		/* disable reset for OTP */
		writel(RF_MASK_V_CLR(1 << 2), moon0_reg + HARDWARE_RESET2);

		/* reset UPHY */
		writel(RF_MASK_V_SET(1 << 8), moon0_reg + HARDWARE_RESET3);
		mdelay(1);
		writel(RF_MASK_V_CLR(1 << 8), moon0_reg + HARDWARE_RESET3);
		mdelay(1);

		/* Default value modification */
		writel(0x08888101, uphy0_reg + GLOBAL_CONTROL0);

		/* PLL power off/on twice */
		writel(0x88, uphy0_reg + GLOBAL_CONTROL2);
		mdelay(1);
		writel(0x80, uphy0_reg + GLOBAL_CONTROL2);
		mdelay(1);
		writel(0x88, uphy0_reg + GLOBAL_CONTROL2);
		mdelay(1);
		writel(0x80, uphy0_reg + GLOBAL_CONTROL2);
		mdelay(20);
		writel(0x0, uphy0_reg + GLOBAL_CONTROL2);

		/* USBC 0 reset */
		writel(RF_MASK_V_SET(1 << 13), moon0_reg + HARDWARE_RESET3);
		mdelay(1);
		writel(RF_MASK_V_CLR(1 << 13), moon0_reg + HARDWARE_RESET3);
		mdelay(1);

		/* fix rx-active question */
		val = readl(uphy0_reg + UPHY0_CONFIGS19);
		set = val | 0xf;
		writel(set, uphy0_reg + UPHY0_CONFIGS19);

		/* OTP for USB phy tx clock invert */
		val = readl(hb_gp_reg + HB_OTP_DATA2);
		if ((val >> 1) & 1) {
			val = readl(uphy0_reg + GLOBAL_CONTROL1);
			set = val | (1 << 5);
			writel(set, uphy0_reg + GLOBAL_CONTROL1);
		}

		/* OTP for USB phy rx clock invert */
		val = readl(hb_gp_reg + HB_OTP_DATA2);
		if (val & 1) {
			val = readl(uphy0_reg + GLOBAL_CONTROL1);
			set = val | (1 << 6);
			writel(set, uphy0_reg + GLOBAL_CONTROL1);
		}

		/* OTP for USB DISC (disconnect voltage) */
		val = readl(hb_gp_reg + HB_OTP_DATA6);
		set = val & 0x1f;
		if (!set)
			set = DEFAULT_UPHY_DISC;

		val = readl(uphy0_reg + UPHY0_CONFIGS7);
		set |= (val & ~0x1f);
		writel(set, uphy0_reg + UPHY0_CONFIGS7);

		/* OTP for USB phy current source adjustment */
		writel(RF_MASK_V_CLR(1 << 5), moon3_reg + M3_CONFIGS20);

		/* OTP for RX squelch level control to APHY */
		val = readl(hb_gp_reg + HB_OTP_DATA6);
		set = (val >> 5) & 0x7;
		if (!set)
			set = DEFAULT_SQ_CT;

		val = readl(uphy0_reg + UPHY0_CONFIGS25);
		set |= (val & ~0x7);
		writel(set, uphy0_reg + UPHY0_CONFIGS25);
	}
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	if (0 == port_num) {
		/* enable clock for UPHY, USBC and OTP */
		writel(RF_MASK_V_SET(1 << 12), moon2_reg + M2_CONFIGS6);	// UPHY0_CLKEN=1
		writel(RF_MASK_V_SET(1 << 15), moon2_reg + M2_CONFIGS6);	// USBC0_CLKEN=1
		writel(RF_MASK_V_SET(1 << 13), moon2_reg + M2_CONFIGS5);

		/* disable reset for OTP */
		writel(RF_MASK_V_CLR(1 << 9), moon0_reg + HARDWARE_RESET0);	// RBUS_BLOCKB_RESET=0
		mdelay(1);
		writel(RF_MASK_V_CLR(1 << 13), moon0_reg + HARDWARE_RESET4);	// OTPRX_RESET=0
		mdelay(1);

		/* reset UPHY0 */
		/* UPHY0_RESET : 1->0 */
		writel(RF_MASK_V_SET(1 << 12), moon0_reg + HARDWARE_RESET5);
		mdelay(1);
		writel(RF_MASK_V_CLR(1 << 12), moon0_reg + HARDWARE_RESET5);
		mdelay(1);

		/* Default value modification */
		/* G149.28 uphy0_gctr0 */
		writel(0x08888101, uphy0_reg + GLOBAL_CONTROL0);

		/* PLL power off/on twice */
		/* G149.30 uphy0_gctrl2 */
		writel(0x88, uphy0_reg + GLOBAL_CONTROL2);
		mdelay(1);
		writel(0x80, uphy0_reg + GLOBAL_CONTROL2);
		mdelay(1);
		writel(0x88, uphy0_reg + GLOBAL_CONTROL2);
		mdelay(1);
		writel(0x80, uphy0_reg + GLOBAL_CONTROL2);
		mdelay(1);
		writel(0x0, uphy0_reg + GLOBAL_CONTROL2);
		mdelay(20); /*	experience */

		/* USBC 0 reset */
		/* USBC0_RESET : 1->0 */
		writel(RF_MASK_V_SET(1 << 15), moon0_reg + HARDWARE_RESET5);
		mdelay(1);
		writel(RF_MASK_V_CLR(1 << 15), moon0_reg + HARDWARE_RESET5);
		mdelay(1);

		/* fix rx-active question */
		/* G149.19 */
		val = readl(uphy0_reg + UPHY0_CONFIGS19);
		set = val | 0xf;
		writel(set, uphy0_reg + UPHY0_CONFIGS19);

		/* OTP for USB phy rx clock invert */
		/* G149.29[6] */
		val = readl(hb_gp_reg + HB_OTP_DATA2);
		if (val & 1) {
			val = readl(uphy0_reg + GLOBAL_CONTROL1);
			set = val | (1 << 6);
			writel(set, uphy0_reg + GLOBAL_CONTROL1);
		}

		/* OTP for USB phy tx clock invert */
		/* G149.29[5] */
		val = readl(hb_gp_reg + HB_OTP_DATA2);
		if ((val >> 1) & 1) {
			val = readl(uphy0_reg + GLOBAL_CONTROL1);
			set = val | (1 << 5);
			writel(set, uphy0_reg + GLOBAL_CONTROL1);
		}

		/* OTP for USB DISC (disconnect voltage) */
		/* G149.7[4:0] */
		val = readl(hb_gp_reg + HB_OTP_DATA19);
		set = val & 0x1f;
		if (!set)
			set = DEFAULT_UPHY_DISC;

		val = readl(uphy0_reg + UPHY0_CONFIGS7);
		set |= (val & ~0x1f);
		writel(set, uphy0_reg + UPHY0_CONFIGS7);
	}
#endif
}

static void usb_power_init(int is_host, int port_num)
{
#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
	/* USB control register:		*/
	/* Host:   ctrl=1, host sel=1, type=1	*/
	/* Device  ctrl=1, host sel=0, type=0	*/
	if (is_host) {
		if (0 == port_num)
			writel(RF_MASK_V_SET(7 << 0), moon3_reg + M3_CONFIGS22);
	} else {
		if (0 == port_num) {
			writel(RF_MASK_V_SET(1 << 0), moon3_reg + M3_CONFIGS22);
			writel(RF_MASK_V_CLR(3 << 1), moon3_reg + M3_CONFIGS22);
		}
	}
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	/* USB control register:		*/
	/* Host:   ctrl=1, host sel=1, type=1	*/
	/* Device  ctrl=1, host sel=0, type=0	*/
	if (is_host) {
		writel(RF_MASK_V_SET(7 << 0), moon4_reg + M4_CONFIGS10);
	} else {
		writel(RF_MASK_V_SET(1 << 0), moon4_reg + M4_CONFIGS10);
		writel(RF_MASK_V_CLR(3 << 1), moon4_reg + M4_CONFIGS10);
	}
#endif
}

static const struct usb_gadget_ops sp_ops = {
	.get_frame			= sp_udc_get_frame,
	.wakeup				= NULL,
	.set_selfpowered		= NULL,
	.pullup				= sp_udc_pullup,
	.vbus_session			= sp_udc_vbus_session,
	.vbus_draw			= NULL,
	.udc_start			= sp_udc_start,
	.udc_stop			= sp_udc_stop,
	.match_ep			= NULL,
};

static const struct usb_ep_ops sp_ep_ops = {
	.enable		= sp_udc_ep_enable,
	.disable	= sp_udc_ep_disable,

	.alloc_request	= sp_udc_alloc_request,
	.free_request	= sp_udc_free_request,

	.queue		= sp_udc_queue,
	.dequeue	= sp_udc_dequeue,

	.set_halt	= sp_udc_set_halt,
};

static void *udc_malloc_align(dma_addr_t *pa, size_t size, size_t align)
{
	void *alloc_addr = NULL;

	alloc_addr = dma_alloc_coherent((unsigned)size, (unsigned long *)pa);
	if (!alloc_addr) {
		UDC_LOGW("%s.%d, fail\n", __func__, __LINE__);
		return NULL;
	}

	return alloc_addr;
}

static void udc_free_align(void *vaddr, dma_addr_t pa, size_t size)
{
	dma_free_coherent(&pa);
}

static int udc_ring_malloc(struct sp_udc *udc, struct udc_ring *const ring, uint8_t num_mem)
{
	if (ring->trb_va) {
		UDC_LOGW("ring already exists\n");
		return -1;
	}

	if (!num_mem) {
		UDC_LOGW("ring members are %d\n", num_mem);
		return -2;
	}

	ring->num_mem = num_mem;
	ring->trb_va = (struct trb_data *)udc_malloc_align(&ring->trb_pa, ring->num_mem * ENTRY_SIZE,
											ALIGN_64_BYTE);

	if (!ring->trb_va) {
		UDC_LOGW("malloc %d memebrs ring fail\n", num_mem);
		return -3;
	}

	ring->end_trb_va = ring->trb_va + (ring->num_mem - 1);
	ring->end_trb_pa = ring->trb_pa + ((ring->num_mem - 1) * ENTRY_SIZE);

	UDC_LOGD("ring %px[%d], s:%px,%llx e:%px,%llx\n", ring, ring->num_mem,
			ring->trb_va, ring->trb_pa, ring->end_trb_va, ring->end_trb_pa);

	return 0;
}

static void udc_ring_free(struct sp_udc *udc, struct udc_ring *const ring)
{
	udc_free_align(ring->trb_va, ring->trb_pa, ring->num_mem * ENTRY_SIZE);

	ring->trb_va = NULL;
	ring->end_trb_va = NULL;
	ring->num_mem = 0;
}

static inline struct udc_endpoint *to_sp_ep(struct usb_ep *ep)
{
	return container_of(ep, struct udc_endpoint, ep);
}

static inline struct sp_udc *to_sp_udc(struct usb_gadget *gadget)
{
	return container_of(gadget, struct sp_udc, gadget);
}

static inline struct sp_request *to_sp_req(struct usb_request *req)
{
	return container_of(req, struct sp_request, req);
}

#if 0
#ifndef CONFIG_SP_USB_PHY
static u8 sp_phy_disc_off[3] = {6, 6, 5};
static u8 sp_phy_disc_shift[3] = {0, 8, 24};
#define ORIG_UPHY_DISC      0x7
#define DEFAULT_UPHY_DISC   0x8

static void sp_usb_config_phy_otp(u32 usb_no)
{
	struct uphy_rn_regs *phy = UPHY_RN_REG(usb_no);
	u32 val;

	/* Select Clock Edge Control Signal */
	if (readl(&OTP_REG->hb_otp_data[2]) & BIT(usb_no)) {
		pr_info("uphy%d rx clk inv\n", usb_no);
		val = readl(&phy->gctrl[1]);
		val |= BIT(6);
		writel(val, &phy->gctrl[1]);
	}

	/* control disconnect voltage */
	val = readl(&OTP_REG->hb_otp_data[sp_phy_disc_off[usb_no]]);
	val = bitfield_extract(val, sp_phy_disc_shift[usb_no], 5);
	if (!val)
		val = DEFAULT_UPHY_DISC;
	else if (val <= ORIG_UPHY_DISC)
		val++;
	writel(bitfield_replace(readl(&phy->cfg[7]), 0, 5, val), &phy->cfg[7]);
}

static void usb_controller_phy_init(u32 usb_no)
{
	struct uphy_rn_regs *phy = UPHY_RN_REG(usb_no);
	u32 val;

	/* 1. reset UPHY */
	writel(RF_MASK_V_SET(BIT(usb_no + 1)), &MOON2_REG->reset[4]);
	mdelay(1);
	writel(RF_MASK_V_CLR(BIT(usb_no + 1)), &MOON2_REG->reset[4]);
	mdelay(1);

	/* 2. Default value modification */
	writel(0x08888100 | BIT(usb_no + 1), &phy->gctrl[0]);

	/* 3. PLL power off/on twice */
	writel(0x88, &phy->gctrl[2]);
	mdelay(1);
	writel(0x80, &phy->gctrl[2]);
	mdelay(1);
	writel(0x88, &phy->gctrl[2]);
	mdelay(1);
	writel(0x80, &phy->gctrl[2]);
	mdelay(1);
	writel(0x00, &phy->gctrl[2]);
	mdelay(20); // experience

	/* 4. UPHY 0&1 internal register modification */
	// Register default value: 0x87
	//writel(0x87, &phy->cfg[7]);

	/* 5. USB controller reset */
	writel(RF_MASK_V_SET(BIT(usb_no + 4)), &MOON2_REG->reset[4]);
	mdelay(1);
	writel(RF_MASK_V_CLR(BIT(usb_no + 4)), &MOON2_REG->reset[4]);
	mdelay(1);

	/* 6. fix rx-active question */
	val = readl(&phy->cfg[19]);
	val |= 0x0f;
	writel(val, &phy->cfg[19]);

	sp_usb_config_phy_otp(usb_no);
}

static void udc_usb_switch(struct sp_udc *udc, bool device)
{
	struct uphy_rn_regs *phy = UPHY_RN_REG(udc->port_num);

	if (device) {
		/* 2. Default value modification */
		writel(phy->gctrl[0] | BIT(8), &phy->gctrl[0]);

		if (udc->port_num == 0)
			MOON3_REG->sft_cfg[21] = RF_MASK_V(0x7 << 9, 0x01 << 9);
		else if (udc->port_num == 1)
			MOON3_REG->sft_cfg[22] = RF_MASK_V(0x7 << 0, 0x1 << 0);

		/* connect */
		udc->vbus_active = true;
		usb_udc_vbus_handler(&udc->gadget, udc->vbus_active);
		UDC_LOGL("host to device\n");
	} else {
		udc->vbus_active = false;
		usb_udc_vbus_handler(&udc->gadget, udc->vbus_active);
		if (udc->port_num == 0)
			MOON3_REG->sft_cfg[21] = RF_MASK_V(0x7 << 9, 0x07 << 9);
		else if (udc->port_num == 1)
			MOON3_REG->sft_cfg[22] = RF_MASK_V(0x7 << 0, 0x7 << 0);
		UDC_LOGL("device to host\n");
	}
}

int32_t usb_switch(uint8_t port, bool device)
{
	struct sp_udc *udc;

	if (port >= 2) {
		UDC_LOGE("usb switch has no port %d\n", port);
		return -EINVAL;
	}

	udc = sp_udc_arry[port];
	if (udc) {
		UDC_LOGL("USB port udc[%d]->%d switch\n", udc->port_num, port);
		udc_usb_switch(udc, device);
		return 0;
	}
	UDC_LOGW("USB port %d,UDC not ready\n", port);
	return -EINVAL;
}
EXPORT_SYMBOL(usb_switch);
#endif
#endif

void init_ep_spin(struct sp_udc *udc)
{
	int i;

	for (i = 0; i < UDC_MAX_ENDPOINT_NUM; i++)
		spin_lock_init(&udc->ep_data[i].lock);
}

static void sp_udc_done(struct udc_endpoint *ep, struct sp_request *req, int status)
{
	if (likely(req->req.status == -EINPROGRESS))
		req->req.status = status;

	req->req.complete(&ep->ep, &req->req);
}

static void sp_udc_nuke(struct sp_udc *udc, struct udc_endpoint *ep, int status)
{
	struct sp_request *req;
	unsigned long flags;

	UDC_LOGD("%s.%d\n", __func__, __LINE__);

	if (ep == NULL)
		return;

	spin_lock_irqsave(&ep->lock, flags);
	while (!list_empty (&ep->queue)) {
		req = list_entry (ep->queue.next, struct sp_request, queue);

#ifndef PIO_MODE
		usb_gadget_unmap_request(&udc->gadget, &req->req, EP_DIR(ep->bEndpointAddress));
#endif

		list_del(&req->queue);
		spin_unlock(&ep->lock);
		sp_udc_done(ep, req, status);
		spin_lock(&ep->lock);
	}

	spin_unlock_irqrestore(&ep->lock, flags);
}

#if 0
static void udc_sof_polling(struct timer_list *t)
{
	uint32_t new_frame_num = 0;
	struct sp_udc *udc = from_timer(udc, t, sof_polling_timer);
	volatile struct udc_reg *USBx = udc->reg;

	new_frame_num = USBx->USBD_FRNUM_ADDR & FRNUM;
	if (udc->frame_num == new_frame_num) {
		UDC_LOGI("sof timer,device disconnect\n");
		udc->bus_reset_finish = false;
		usb_phy_notify_disconnect(udc->usb_phy, udc->gadget.speed);
		udc->frame_num = 0;
		return;
	}
	udc->frame_num = new_frame_num;
	mod_timer(&udc->sof_polling_timer, jiffies + HZ / 20);
}

static void udc_before_sof_polling(struct timer_list *t)
{
	uint32_t new_frame_num = 0;
	struct sp_udc *udc = from_timer(udc, t, before_sof_polling_timer);
	volatile struct udc_reg *USBx = udc->reg;

	if (udc->bus_reset_finish) {
		UDC_LOGI("before sof timer,bus reset finish\n");
		mod_timer(&udc->sof_polling_timer, jiffies + HZ / 20);
		usb_phy_notify_connect(udc->usb_phy, udc->gadget.speed);
		return;
	}

	new_frame_num = USBx->USBD_FRNUM_ADDR & FRNUM;
	if (udc->frame_num == new_frame_num) {
		UDC_LOGE("bus reset err\n");
		udc->bus_reset_finish = false;
		usb_phy_notify_disconnect(udc->usb_phy, udc->gadget.speed);
		udc->frame_num = 0;
		return;
	}

	udc->frame_num = new_frame_num;
	mod_timer(&udc->before_sof_polling_timer, jiffies + 1 * HZ);
}
#endif

static uint32_t check_trb_status(struct trb_data *t_trb)
{
	uint32_t trb_status;
	uint32_t ret = 0;

	trb_status = ETRB_CC(t_trb->entry2);
	switch (trb_status) {
	case INVALID_TRB:
		UDC_LOGD("invaild trb\n");
		break;
	case SUCCESS_TRB:
		ret = SUCCESS_TRB;
		break;
	case DATA_BUF_ERR:
		UDC_LOGD("data buf err\n");
		break;
	case BABBLE_ERROR:
		UDC_LOGD("babble error\n");
		ret = BABBLE_ERROR;
		break;
	case TRANS_ERR:
		UDC_LOGD("trans err,%p,%x,%x,%x,%x\n", t_trb, t_trb->entry0, t_trb->entry1, 
									t_trb->entry2, t_trb->entry3);
		ret = TRANS_ERR;
		break;
	case TRB_ERR:
		UDC_LOGD("trb err\n");
		break;
	case RESOURCE_ERR:
		UDC_LOGD("resource err\n");
		break;
	case SHORT_PACKET:
		UDC_LOGD("short packet\n");
		ret = SHORT_PACKET;
		break;
	case EVENT_RING_FULL:
		UDC_LOGD("event ring full\n");
		ret = EVENT_RING_FULL;
		break;

	/* UDC evnet */
	case UDC_STOPED:
		ret = UDC_STOPED;
		break;
	case UDC_RESET:
		ret = UDC_RESET;
		break;
	case UDC_SUSPEND:
		ret = UDC_SUSPEND;
		break;
	case UDC_RESUME:
		ret = UDC_RESUME;
		break;
	default:
		UDC_LOGE("%s.%d,trb_status:%d\n", __func__, __LINE__, trb_status);
		ret = -EPERM;
		break;
	}

	return ret;
}

static inline void hal_udc_sw_stop_handle(struct sp_udc *udc)
{
	volatile struct udc_reg *USBx = udc->reg;

	sp_udc_nuke(udc, &udc->ep_data[0], -ESHUTDOWN);
	hal_udc_endpoint_unconfigure(udc, 0x00);
	USBx->GL_CS |= 1 << 31;
}

static uint32_t hal_udc_check_trb_type(struct trb_data *t_trb)
{
	uint32_t trb_type = -1;
	uint32_t index;

	index = LTRB_TYPE((uint32_t)t_trb->entry3);
	switch (index) {
	case NORMAL_TRB:
		trb_type = NORMAL_TRB;
		UDC_LOGD("NORMAL_TRB WHY?\n");
		break;
	case SETUP_TRB:
		trb_type = SETUP_TRB;
		UDC_LOGD("SETUP_TRB\n");
		break;
	case LINK_TRB:
		trb_type = LINK_TRB;
		UDC_LOGD("LINK_TRB WHY?\n");
		break;
	case TRANS_EVENT_TRB:
		trb_type = TRANS_EVENT_TRB;
		UDC_LOGD("TRANS_EVENT_TRB\n");
		break;
	case DEV_EVENT_TRB:
		trb_type = DEV_EVENT_TRB;
		break;
	case SOF_EVENT_TRB:
		trb_type = SOF_EVENT_TRB;
		break;
	default:
		UDC_LOGD("%s.%d, index:%d\n", __func__, __LINE__, index);
		break;
	}

	return trb_type;
}

static void hal_udc_transfer_event_handle(struct transfer_event_trb *transfer_evnet, struct sp_udc *udc)
{
	struct normal_trb *ep_trb = NULL;
	struct udc_endpoint *ep = NULL;
	struct sp_request *req;
	uint32_t aligned_len;
	uint32_t trans_len = 0;
#ifdef PIO_MODE
	uint8_t *data_buf;
#else
	uint32_t *data_buf;
#endif
	uint8_t ep_num;
	int8_t count = TRANSFER_RING_COUNT;
	unsigned long flags;

	ep_num = transfer_evnet->eid;
	ep = &udc->ep_data[ep_num];

	spin_lock_irqsave(&ep->lock, flags);
	if (!ep->ep_trb_ring_dq) {
		spin_unlock_irqrestore(&ep->lock, flags);
		UDC_LOGW(" ep%d not configure\n", ep->num);

		return ;
	}

	ep_trb = (struct normal_trb *)(ep->ep_transfer_ring.trb_va +
		((transfer_evnet->trbp - ep->ep_transfer_ring.trb_pa) / sizeof(struct trb_data)));

#ifdef PIO_MODE
	data_buf = (uint8_t *)phys_to_virt(ep_trb->ptr);
#else
	data_buf = (uint32_t *)phys_to_virt(ep_trb->ptr);
#endif

	trans_len = ep_trb->tlen;

	UDC_LOGD("ep %x[%c],trb:%px,%x len %d - %d\n", ep_num, ep_trb->dir ? 'I' : 'O', ep_trb,
						transfer_evnet->trbp, trans_len, transfer_evnet->len);

	if (!ep->num && !trans_len && list_empty (&ep->queue)) {
		spin_unlock_irqrestore(&ep->lock, flags);
		UDC_LOGD("+%s.%d ep0 zero\n", __func__, __LINE__);

		return;
	}

#ifdef PIO_MODE
	if (trans_len) {
		aligned_len = roundup(trans_len, ARCH_DMA_MINALIGN);

		if ((ep->is_in && (ep_num == EP0)) || (!ep->is_in && (ep_num != EP0))) {
			req = list_entry (ep->queue.next, struct sp_request, queue);
			memcpy((uint8_t *)req->req.buf, data_buf, aligned_len);
			invalidate_dcache_range((unsigned long)ep_trb->ptr,
							(unsigned long)ep_trb->ptr + aligned_len);
		}
	}
#else
	if (!ep->is_in && trans_len) {
		aligned_len = roundup(trans_len, ARCH_DMA_MINALIGN);
		invalidate_dcache_range((unsigned long)ep_trb->ptr,
						(unsigned long)ep_trb->ptr + aligned_len);
	}
#endif

	while (!list_empty (&ep->queue)) {
		req = list_entry (ep->queue.next, struct sp_request, queue);

		UDC_LOGD("find ep%x,req:%px,req_trb:%px->%px\n", ep_num, req,
									req->transfer_trb, ep_trb);

		if (req->transfer_trb == (struct trb_data *)ep_trb) {
			req->req.actual = trans_len - transfer_evnet->len;
			req->transfer_trb = NULL;

#ifndef PIO_MODE
			usb_gadget_unmap_request(&udc->gadget, &req->req, EP_DIR(ep->bEndpointAddress));
#endif

			list_del(&req->queue);

#ifdef PIO_MODE
			kfree(req->buffer);
			req->buffer = NULL;
#endif

			spin_unlock_irqrestore(&ep->lock, flags);
			sp_udc_done(ep, req, 0);

			return ;
		}

		if (list_is_last(&req->queue, &ep->queue))
			count = -2;

		count--;
		if (count < 0)
			break;
	}

	if (count) {
		spin_unlock_irqrestore(&ep->lock, flags);
		UDC_LOGD("ep%x ep_queue not req %d\n", ep_num, count);
	}
}

static void hal_udc_analysis_event_trb(struct trb_data *event_trb, struct sp_udc *udc)
{
	volatile struct udc_reg *USBx = udc->reg;
	struct usb_ctrlrequest crq;
	uint32_t trb_type;
	uint32_t ret;

	/* handle trb err situation */
	trb_type = hal_udc_check_trb_type(event_trb);

	ret = check_trb_status(event_trb);
	if (!ret) {
		UDC_LOGE("error,check_trb_status fail,ret : %d\n", ret);
		return;
	}

	switch (trb_type) {
	case SETUP_TRB:
		if (!udc->bus_reset_finish) {
			switch (USBx->USBD_DEBUG_PP & 0xFF) {
			case PP_FULL_SPEED:
				udc->gadget.speed = USB_SPEED_FULL;
				UDC_LOGL("Full speed\n");
				break;
			case PP_HIGH_SPEED:
				udc->gadget.speed = USB_SPEED_HIGH;
				UDC_LOGL("High speed\n");
				break;
			default:
				UDC_LOGE("error, Unknown speed\n");
				udc->gadget.speed = udc->def_run_full_speed ? USB_SPEED_FULL : USB_SPEED_HIGH;
				break;
			}
		}

		/* enable device auto sof */
		if (!(readl(&USBx->GL_CS) & BIT(9)))
			writel(bitfield_replace(readl(&USBx->GL_CS), 9, 1, 1), &USBx->GL_CS);

		udc->bus_reset_finish = true;
		memcpy(&crq, event_trb, 8);

		UDC_LOGD("s:%x,%x,%x,%x,%x\n", crq.bRequestType, crq.bRequest, crq.wValue,
									crq.wIndex, crq.wLength);

		hal_udc_setup(udc, &crq);
		break;
	case TRANS_EVENT_TRB:
		hal_udc_transfer_event_handle((struct transfer_event_trb *)event_trb, udc);
		break;
	case DEV_EVENT_TRB:
		/* disable device auto sof
		 * Nont:
		 * Prevent automatically generated SOF
		 * from affecting detection of device disconnection
		 */
		writel(bitfield_replace(readl(&USBx->GL_CS), 9, 1, 0), &USBx->GL_CS);

		switch (ret) {
		case UDC_RESET:
			USBx->EP0_CS &= ~EP_EN;		/* dislabe ep0 */
			if (udc->driver->reset)
				udc->driver->reset(&udc->gadget);

#if 0
			if (!test_mode)
				mod_timer(&udc->before_sof_polling_timer, jiffies + 1 * HZ);
#endif

			UDC_LOGL("bus reset\n");
			break;
		case UDC_SUSPEND:
			UDC_LOGL("udc suspend\n");
			break;
		case UDC_RESUME:
			UDC_LOGL("udc resume\n");
			break;
		case UDC_STOPED:
			UDC_LOGL("udc stoped\n");

			if (udc->usb_test_mode)
				writel(bitfield_replace(readl(&USBx->GL_CS), 12, 4, 0), &USBx->GL_CS);

			hal_udc_sw_stop_handle(udc);
			break;
		default:
			UDC_LOGE("not support:ret = %d\n", ret);
			return;
		}

		break;
	case SOF_EVENT_TRB:
		UDC_LOGL("sof event trb\n");
		break;
	}
}

static void handle_event(struct sp_udc *udc)
{
	volatile struct udc_reg *USBx = udc->reg;
	struct udc_ring *tmp_event_ring = NULL;
	struct trb_data *event_ring_dq = NULL;
	struct trb_data *end_trb;
	uint32_t temp_event_sg_count = udc->current_event_ring_seg;
	uint32_t valid_event_count = 0;
	uint32_t aligned_len;
	uint32_t erdp_reg = 0;
	uint32_t trb_cc = 0;
	uint8_t temp_ccs = udc->event_ccs;
	unsigned long flags;
	bool found = false;

	spin_lock_irqsave(&udc->lock, flags);
	if (!udc->event_ring || !udc->driver) {
		UDC_LOGD("handle_event return\n");
		spin_unlock_irqrestore(&udc->lock, flags);
		return;
	}

	tmp_event_ring = &udc->event_ring[temp_event_sg_count];
	end_trb = tmp_event_ring->end_trb_va;

	/* event ring dq */
	event_ring_dq = udc->event_ring_dq;
	aligned_len = roundup(8 * sizeof(struct trb_data), ARCH_DMA_MINALIGN);
	invalidate_dcache_range((unsigned long)event_ring_dq, aligned_len);

	/* Count the valid events this time */
	while (1) {
		trb_cc = ETRB_C(event_ring_dq->entry3);
		if (trb_cc != temp_ccs) {	/* invaild event trb */
			if (found)
				event_ring_dq--;

			break;
		} else {
			found = true;
		}

		/* switch event segment */
		if (end_trb == event_ring_dq) {
			if (temp_event_sg_count == (udc->event_ring_seg_total - 1)) {
				temp_event_sg_count = 0;
				/* The last segment, flip ccs */
				temp_ccs = (~temp_ccs) & 0x1;
			} else {
				temp_event_sg_count++;
			}

			event_ring_dq = udc->event_ring[temp_event_sg_count].trb_va;
			end_trb = udc->event_ring[temp_event_sg_count].end_trb_va;
		} else {
			event_ring_dq++;
		}

		valid_event_count++;
	}

	if (valid_event_count > 0) {
		UDC_LOGD("------ valid event %d -------\n", valid_event_count);

		/* reacquire ring dq */
		event_ring_dq = udc->event_ring_dq;
	} else {
		UDC_LOGD("------ no event %p -------\n", udc->event_ring_dq);
	}
	spin_unlock_irqrestore(&udc->lock, flags);

	while (valid_event_count > 0) {
		trb_cc = ETRB_C(event_ring_dq->entry3);

		/* trb_cc is used or not */
		if (trb_cc != udc->event_ccs) {
			break;
		} else {
			UDC_LOGD("------ event %d -------\n", valid_event_count);

			hal_udc_analysis_event_trb(event_ring_dq, udc);
			if (end_trb == event_ring_dq) {
				if (udc->current_event_ring_seg == (udc->event_ring_seg_total - 1)) {
					udc->current_event_ring_seg = 0;
					udc->event_ccs = (~udc->event_ccs) & 0x1;
				} else {
					udc->current_event_ring_seg++;
				}
				event_ring_dq = udc->event_ring[udc->current_event_ring_seg].trb_va;
				end_trb = udc->event_ring[temp_event_sg_count].end_trb_va;
			} else {
				event_ring_dq++;
			}

			valid_event_count--;
		}
	}

	spin_lock_irqsave(&udc->lock, flags);
	udc->event_ring_dq = event_ring_dq;
	erdp_reg = USBx->DEVC_ERDP & DESI;
	USBx->DEVC_ERDP = (uint32_t)(udc->event_ring[udc->current_event_ring_seg].trb_pa +
					((event_ring_dq - udc->event_ring[udc->current_event_ring_seg].trb_va) *
						sizeof(struct trb_data))) | erdp_reg;

	UDC_LOGD("ERDP_reg %x, %px\n", USBx->DEVC_ERDP, event_ring_dq);

	USBx->DEVC_ERDP |= EHB;
	spin_unlock_irqrestore(&udc->lock, flags);
}

#if 0
static void hal_dump_event_ring(struct sp_udc *udc)
{
	struct udc_ring *tmp_event_ring = NULL;
	volatile struct udc_reg *USBx = udc->reg;
	uint32_t seg = 0;
	uint32_t trb_index = 0;
	struct trb_data *trb;

	pr_notice("udc event ccs:%d\n", udc->event_ccs);
	pr_notice("udc event seg table: %d\n", udc->event_ring_seg_total);
	pr_notice("udc event ring dq:%px,%x\n", udc->event_ring_dq, USBx->DEVC_ERDP);

	for (seg = 0; seg < udc->event_ring_seg_total; seg++) {
		tmp_event_ring = &udc->event_ring[seg];
		if (!tmp_event_ring) {
			pr_info("Not evnet ring %d\n", seg);

			continue;
		}

		pr_notice("event ring:%d\n", seg);
		pr_notice("len:%d\n", tmp_event_ring->num_mem);
		pr_notice("start:%px,%llx\n", tmp_event_ring->trb_va, tmp_event_ring->trb_pa);
		pr_notice("end:%px,%llx\n", tmp_event_ring->end_trb_va, tmp_event_ring->end_trb_pa);
		pr_notice("va[pa]:index | entry0,entry1,entry2,entry3 |t:type,ccs:ccs\n");

		for (trb_index = 0; trb_index < tmp_event_ring->num_mem; trb_index++) {
			trb = &tmp_event_ring->trb_va[trb_index];
			pr_notice("%px[%llx]:%2d | %8x,%8x,%8x,%8x |t:%4d,ccs:%d\n", trb,
			tmp_event_ring->trb_pa + (trb_index * ENTRY_SIZE), trb_index,
			trb->entry0, trb->entry1, trb->entry2, trb->entry3,
			LTRB_TYPE((uint32_t)trb->entry3),
			ETRB_C(trb->entry3));
		}
	}
}

static void hal_dump_ep_desc(struct sp_udc *udc)
{
	volatile struct udc_reg *USBx = udc->reg;
	uint32_t ep_index = 0;
	struct sp_desc *ep_desc;

	pr_notice("\n\nep count:%d\n", UDC_MAX_ENDPOINT_NUM);
	pr_notice("ep desc %px,%llx\n", udc->ep_desc, udc->ep_desc_pa);
	pr_notice("DEVC_ADDR:%x\n", USBx->DEVC_ADDR);
	pr_notice("va[pa]:index | entry0,entry1,entry2,entry3 |reg:\n");

	for (ep_index = 0; ep_index < UDC_MAX_ENDPOINT_NUM; ep_index++) {
		ep_desc = &udc->ep_desc[ep_index];

		pr_notice("%px[%llx]:%2d | %8x,%8x,%8x,%8x |reg:%x\n", ep_desc,
		udc->ep_desc_pa + (ep_index * ENTRY_SIZE), ep_index,
		ep_desc->entry0, ep_desc->entry1, ep_desc->entry2, ep_desc->entry3,
		ep_index ? USBx->EPN_CS[ep_index - 1] : USBx->EP0_CS);
	}
}

static void hal_dump_ep_ring(struct sp_udc *udc, uint8_t ep_num)
{
	struct udc_ring *tmp_trb_ring = NULL;
	uint32_t trb_index = 0;
	struct trb_data *trb;
	struct udc_endpoint *ep = NULL;
	unsigned long flags;

	hal_dump_ep_desc(udc);
	ep = &udc->ep_data[ep_num];

	spin_lock_irqsave(&ep->lock, flags);

	if (!ep->ep_trb_ring_dq) {
		pr_notice("EP%d not config\n", ep_num);
		spin_unlock_irqrestore(&ep->lock, flags);

		return;
	}
	tmp_trb_ring = &ep->ep_transfer_ring;

	pr_notice("\n\nEP%d[%c] t:%d MPS:%d dq_trb:%px\n", ep_num, ep->is_in ? 'I' : 'O',
				ep->type, ep->maxpacket, ep->ep_trb_ring_dq);
	pr_notice("start:%px,%llx\n", tmp_trb_ring->trb_va, tmp_trb_ring->trb_pa);
	pr_notice("end:%px,%llx\n", tmp_trb_ring->end_trb_va, tmp_trb_ring->end_trb_pa);
	pr_notice("len:%d", tmp_trb_ring->num_mem);
	pr_notice("va[pa]:index | entry0,entry1,entry2,entry3 |t:type\n");

	for (trb_index = 0; trb_index < tmp_trb_ring->num_mem; trb_index++) {
		trb = &tmp_trb_ring->trb_va[trb_index];
		pr_notice("%px[%llx]:%2d | %8x,%8x,%8x,%8x |t:%4d\n", trb,
		tmp_trb_ring->trb_pa + (trb_index * ENTRY_SIZE), trb_index,
		trb->entry0, trb->entry1, trb->entry2, trb->entry3,
		LTRB_TYPE((uint32_t)trb->entry3));
	}

	spin_unlock_irqrestore(&ep->lock, flags);
}
#endif

static int sp_udc_irq(struct sp_udc *udc)
{
	volatile struct udc_reg *USBx;

	USBx = udc->reg;

	spin_lock(&udc->lock);
	if ((USBx->DEVC_STS & EINT) && (USBx->DEVC_IMAN & DEVC_INTR_PENDING)) {
		USBx->DEVC_STS |= EINT;
		USBx->DEVC_IMAN |= DEVC_INTR_PENDING;

		handle_event(udc);
	}
	spin_unlock(&udc->lock);

	return 1;
}

int dm_usb_gadget_handle_interrupts(struct udevice *dev)
{
	struct sp_udc *udc = dev_get_priv(dev);

	return sp_udc_irq(udc);
}

static void fill_link_trb(struct trb_data *t_trb, dma_addr_t ring)
{
	t_trb->entry0 = ring;
	t_trb->entry3 |= (LINK_TRB<<10);

	if (t_trb->entry3 & TRB_C)
		t_trb->entry3 &= ~TRB_C;
	else
		t_trb->entry3 |= TRB_C;		/* toggle cycle bit */

	UDC_LOGD("--- fill link trb ---\n");
}

static void hal_udc_fill_transfer_trb(struct trb_data *t_trb, struct udc_endpoint *ep, uint32_t ioc)
{
	struct normal_trb *tmp_trb = (struct normal_trb *)t_trb;
	uint32_t aligned_len;

	tmp_trb->tlen = ep->transfer_len;

#ifdef PIO_MODE
	tmp_trb->ptr = virt_to_phys(ep->transfer_buff);
#else
	tmp_trb->ptr = (uint32_t)ep->transfer_buff_pa;
#endif

	/* TRB function setting */
	aligned_len = roundup(tmp_trb->tlen, ARCH_DMA_MINALIGN);

	if (!ep->is_in) {
		tmp_trb->dir = 0;	/* 0 is out */

		if (ep->transfer_len)
			invalidate_dcache_range((unsigned long)tmp_trb->ptr,
							(unsigned long)tmp_trb->ptr + aligned_len);
	} else {
		tmp_trb->dir = 1;	/* 1 is IN */

		if (ep->transfer_len)
			flush_dcache_range((unsigned long)tmp_trb->ptr,
						(unsigned long)tmp_trb->ptr + aligned_len);
	}

	if (ep->type == UDC_EP_TYPE_ISOC)
		tmp_trb->sia = 1;	/* start ISO ASAP, modify */

	tmp_trb->isp = 1;

	if (ioc)
		tmp_trb->ioc = 1;	/* create event trb */
	else
		tmp_trb->ioc = 0;

	tmp_trb->type = 1;		/* trb type */

	/* valid trb */
	if (tmp_trb->cycbit == 1)
		tmp_trb->cycbit = 0;	/* set cycle bit 0 */
	else
		tmp_trb->cycbit = 1;

	aligned_len = roundup(sizeof(struct normal_trb), ARCH_DMA_MINALIGN);
	flush_dcache_range((unsigned long)tmp_trb, (unsigned long)tmp_trb + aligned_len);
}

static void hal_udc_fill_ep_desc(struct sp_udc *udc, struct udc_endpoint *ep)
{
	volatile struct udc_reg *USBx = udc->reg;
	struct endpoint0_desc *tmp_ep0_desc = NULL;
	struct endpointn_desc *tmp_epn_desc = NULL;
	uint32_t aligned_len;

	if (ep->num == 0) {
		tmp_ep0_desc = (struct endpoint0_desc *)udc->ep_desc;
		UDC_LOGD("%s.%d ep%d tmp_ep0_desc = %px\n", __func__, __LINE__, ep->num, tmp_ep0_desc);
		tmp_ep0_desc->cfgs = AUTO_RESPONSE;			/* auto response configure setting */
		tmp_ep0_desc->cfgm = AUTO_RESPONSE;			/* auto response configure setting */
		tmp_ep0_desc->speed = udc->def_run_full_speed ? UDC_FULL_SPEED : UDC_HIGH_SPEED; /* high speed */
		tmp_ep0_desc->aset = AUTO_SET_CONF | AUTO_SET_INF;	/* not auto setting config & interface*/
		tmp_ep0_desc->dcs = udc->event_ccs;			/* set cycle bit 1 */
		tmp_ep0_desc->sofic = 0;
		tmp_ep0_desc->dptr = SHIFT_LEFT_BIT4(ep->ep_transfer_ring.trb_pa);

		UDC_LOGD("ep0, dptr:0x%x\n", tmp_ep0_desc->dptr);
	} else {
		tmp_epn_desc = (struct endpointn_desc *)(udc->ep_desc + ep->num);

		UDC_LOGD("%s.%d ep%d tmp_epn_desc = %px\n", __func__, __LINE__, ep->num, tmp_epn_desc);

		tmp_epn_desc->ifm = AUTO_RESPONSE;			/* auto response interface setting */
		tmp_epn_desc->altm = AUTO_RESPONSE;			/* auto response alternated setting */
		tmp_epn_desc->num = ep->num;
		tmp_epn_desc->type = ep->type;

		/* If endpoint type is ISO or INT */
		if ((ep->type == UDC_EP_TYPE_ISOC) || (ep->type == UDC_EP_TYPE_INTR))
			tmp_epn_desc->mult = FRAME_TRANSFER_NUM_3;

		tmp_epn_desc->dcs = 1;					/* set cycle bit 1 */
		tmp_epn_desc->mps = ep->maxpacket;
		tmp_epn_desc->dptr = SHIFT_LEFT_BIT4(ep->ep_transfer_ring.trb_pa);

		UDC_LOGD("%s.%d ep%d, dptr:0x%x\n", __func__, __LINE__,
							ep->num, tmp_epn_desc->dptr);
	}
	wmb();

	if (ep->num == 0)
		USBx->EP0_CS |= RDP_EN;					/* Endpoint register enable,RDP enable */
	else
		USBx->EPN_CS[ep->num - 1] |= RDP_EN;			/* Endpoint register enable,RDP enable */

	if (ep->num == 0)
		aligned_len = roundup(sizeof(struct endpoint0_desc), ARCH_DMA_MINALIGN);
	else
		aligned_len = roundup(sizeof(struct endpointn_desc), ARCH_DMA_MINALIGN);

	flush_dcache_range(((unsigned long)udc->ep_desc + ep->num),
					((unsigned long)udc->ep_desc + ep->num) + aligned_len);
}

static struct trb_data *hal_udc_fill_trb(struct sp_udc	*udc, struct udc_endpoint *ep, uint32_t zero)
{
	struct trb_data *end_trb;
	struct trb_data *fill_trb = NULL;
	uint32_t aligned_len;

	end_trb = ep->ep_transfer_ring.end_trb_va;

	if (end_trb == ep->ep_trb_ring_dq) {
		fill_link_trb(ep->ep_trb_ring_dq, ep->ep_transfer_ring.trb_pa);
		ep->ep_trb_ring_dq->entry3 |= LTRB_TC;			/* toggle cycle bit */

		aligned_len = roundup(sizeof(struct trb_data), ARCH_DMA_MINALIGN);
		flush_dcache_range((unsigned long)ep->ep_trb_ring_dq,
					(unsigned long)ep->ep_trb_ring_dq + aligned_len);

		ep->ep_trb_ring_dq = ep->ep_transfer_ring.trb_va;
	}

	hal_udc_fill_transfer_trb(ep->ep_trb_ring_dq, ep, 1);
	fill_trb = ep->ep_trb_ring_dq;
	ep->ep_trb_ring_dq++;

	/* EP 0 send/receive zero packet */
	if (0 == ep->num && ep->transfer_len > 0) {
		if (end_trb == ep->ep_trb_ring_dq) {
			fill_link_trb(ep->ep_trb_ring_dq, ep->ep_transfer_ring.trb_pa);
			ep->ep_trb_ring_dq->entry3 |= LTRB_TC;		/* toggle cycle bit */

			aligned_len = roundup(sizeof(struct trb_data), ARCH_DMA_MINALIGN);
			flush_dcache_range((unsigned long)ep->ep_trb_ring_dq,
						(unsigned long)ep->ep_trb_ring_dq + aligned_len);

			ep->ep_trb_ring_dq = ep->ep_transfer_ring.trb_va;
		}

		/* len == MPS and len < setup len */
		if (zero && (ep->transfer_len == ep->maxpacket)) {
			ep->transfer_buff = NULL;
			ep->transfer_len = 0;
			hal_udc_fill_transfer_trb(ep->ep_trb_ring_dq, ep, 0);
			ep->ep_trb_ring_dq++;

			if (end_trb == ep->ep_trb_ring_dq) {
				fill_link_trb(ep->ep_trb_ring_dq, ep->ep_transfer_ring.trb_pa);
				ep->ep_trb_ring_dq->entry3 |= LTRB_TC;	/* toggle cycle bit */

				aligned_len = roundup(sizeof(struct trb_data), ARCH_DMA_MINALIGN);
				flush_dcache_range((unsigned long)ep->ep_trb_ring_dq,
							(unsigned long)ep->ep_trb_ring_dq + aligned_len);

				ep->ep_trb_ring_dq = ep->ep_transfer_ring.trb_va;
			}
		}

		/* ACK handshake */
		if (ep->is_in)
			ep->is_in = false;				/* ep0 rx zero packet */
		else
			ep->is_in = true;				/* ep0 tx zero packet */

		ep->transfer_buff = NULL;
		ep->transfer_len = 0;
		hal_udc_fill_transfer_trb(ep->ep_trb_ring_dq, ep, 0);
		ep->ep_trb_ring_dq++;
	}

	if (zero && ep->type == UDC_EP_TYPE_BULK && ep->is_in && ep->transfer_len > 0 &&
							(ep->transfer_len % ep->maxpacket) == 0) {
		if (end_trb == ep->ep_trb_ring_dq) {
			fill_link_trb(ep->ep_trb_ring_dq, ep->ep_transfer_ring.trb_pa);
			ep->ep_trb_ring_dq->entry3 |= LTRB_TC;		/* toggle cycle bit */

			aligned_len = roundup(sizeof(struct trb_data), ARCH_DMA_MINALIGN);
			flush_dcache_range((unsigned long)ep->ep_trb_ring_dq,
						(unsigned long)ep->ep_trb_ring_dq + aligned_len);

			ep->ep_trb_ring_dq = ep->ep_transfer_ring.trb_va;
		}

		/* send/receive zero packet */
		ep->transfer_buff = NULL;
		ep->transfer_len = 0;
		hal_udc_fill_transfer_trb(ep->ep_trb_ring_dq, ep, 0);
		ep->ep_trb_ring_dq++;
	}

	wmb();
	if (ep->num == 0)
		udc->reg->EP0_CS |= EP_EN;
	else
		udc->reg->EPN_CS[ep->num-1] |= EP_EN;

	return fill_trb;
}

int32_t hal_udc_endpoint_transfer(struct sp_udc	*udc, struct sp_request *req, uint8_t ep_addr,
					uint8_t *data, dma_addr_t data_pa, uint32_t length, uint32_t zero)
{
	struct udc_endpoint *ep;
#ifdef PIO_MODE
	uint32_t aligned_len;
#endif

	if (EP_NUM(ep_addr) > UDC_MAX_ENDPOINT_NUM) {
		UDC_LOGE("ep_addr parameter error, max endpoint num is %d.\n", UDC_MAX_ENDPOINT_NUM);
		return -EINVAL;
	}

	ep = &udc->ep_data[EP_NUM(ep_addr)];
	if (0 == ep->type && EP_NUM(ep_addr) != 0) {
		UDC_LOGE("ep%d not configure!\n", EP_NUM(ep_addr));
		return -EINVAL;
	}

	if (!ep->ep_trb_ring_dq) {
		UDC_LOGE("ep%d not configure2!\n", EP_NUM(ep_addr));
		return -EINVAL;
	}

	ep->is_in = EP_DIR(ep_addr);
	ep->transfer_len = length;

#ifdef PIO_MODE
	if (length != 0) {
		if (EP_NUM(ep_addr) == EP0)
			req->buffer = (uint8_t *)kmalloc(USB_COMP_EP0_BUFSIZ, GFP_KERNEL);
		else
			req->buffer = (uint8_t *)kmalloc(length, GFP_KERNEL);

		if (ep->is_in) {
			aligned_len = roundup(length, ARCH_DMA_MINALIGN);
			flush_dcache_range((unsigned long)virt_to_phys(req->buffer),
						(unsigned long)virt_to_phys(req->buffer) + aligned_len);

			if (EP_NUM(ep_addr) == EP0)
				memcpy(req->buffer, (uint8_t *)data, USB_COMP_EP0_BUFSIZ);
			else
				memcpy(req->buffer, (uint8_t *)data, length);
		}

		ep->transfer_buff = req->buffer;
		ep->transfer_buff_pa = 0;
	} else {
		ep->transfer_buff = NULL;
		ep->transfer_buff_pa = 0;
	}
#else
	if (length != 0) {
		ep->transfer_buff = data;
		ep->transfer_buff_pa = data_pa;
	} else {
		ep->transfer_buff = NULL;
		ep->transfer_buff_pa = 0;
	}
#endif

	/* Controller automatically responds to set config and set interface,
	   So there is no need to fill in trb.*/
	if ((0 == ep->num) && udc->aset_flag) {
		UDC_LOGD("auto set\n");
		udc->aset_flag = false;
		req->transfer_trb = NULL;

#ifndef PIO_MODE
		usb_gadget_unmap_request(&udc->gadget, &req->req, ep->is_in);
#endif

		list_del(&req->queue);

		return 0;
	}

	req->transfer_trb = hal_udc_fill_trb(udc, ep, zero);

	return 0;
}

int32_t hal_udc_endpoint_stall(struct sp_udc *udc, uint8_t ep_addr, bool stall)
{
	volatile struct udc_reg *USBx = udc->reg;
	struct udc_endpoint *ep = NULL;

	UDC_LOGD("%s.%d\n", __func__, __LINE__);

	ep = &udc->ep_data[EP_NUM(ep_addr)];

	/* Set stall */
	#if 0
	if ((stall) && (ep->is_in == true) && (!list_empty (&ep->queue)) &&
					((USBx->EPN_CS[ep->num-1] & EP_EN) == EP_EN))
		return -EAGAIN;
	#else
	mdelay(1);
	#endif

	if (stall) {
		if (ep->num == 0)
			USBx->EP0_CS |= EP_STALL;
		else
			USBx->EPN_CS[ep->num-1] |= EP_STALL;

		ep->status = ENDPOINT_HALT;
	} else {
		/* Clear stall */
		if (ep->num == 0)
			USBx->EP0_CS &= ~EP_STALL;
		else
			USBx->EPN_CS[ep->num-1] &= ~EP_STALL;

		ep->status = ENDPOINT_READY;
	}

	return 0;
}

int32_t hal_udc_device_connect(struct sp_udc *udc)
{
	volatile struct udc_reg *USBx = udc->reg;
	struct udc_ring *tmp_ring = NULL;
	uint32_t seg_count = 0;
	uint32_t aligned_len;

	UDC_LOGI("%s.%d %x\n", __func__, __LINE__, USBx->DEVC_CS);

	hal_udc_power_control(udc, UDC_POWER_OFF);

	for (seg_count = 0; seg_count < udc->event_ring_seg_total; seg_count++) {
		tmp_ring = &udc->event_ring[seg_count];
		if (tmp_ring->trb_va) {
			memset(tmp_ring->trb_va, 0, tmp_ring->num_mem * ENTRY_SIZE);

			aligned_len = roundup(tmp_ring->num_mem * ENTRY_SIZE, ARCH_DMA_MINALIGN);
			flush_dcache_range((unsigned long)tmp_ring->trb_va,
						(unsigned long)tmp_ring->trb_va + aligned_len);
		}
	}

	udc->event_ccs = 1;

	USBx->DEVC_STS = CLEAR_INT_VBUS;
	USBx->DEVC_CTRL = VBUS_DIS;

	/* fill register */
	/* event ring segnmet unmber */
	USBx->DEVC_ERSTSZ = udc->event_ring_seg_total;

	/* event ring dequeue pointers  */
	USBx->DEVC_ERDP = udc->event_ring[0].trb_pa;
	udc->event_ring_dq = udc->event_ring[0].trb_va;

	/* event ring segment table base address */
	USBx->DEVC_ERSTBA = udc->event_seg_table_pa;

	/* set interrupt moderation */
	USBx->DEVC_IMOD = 0;

	/* init step j */
	USBx->DEVC_ADDR = udc->ep_desc_pa;

	/* configure ep0 */
	hal_udc_endpoint_configure(udc, 0x00, UDC_EP_TYPE_CTRL, 0x40);

	/* enable interrupt */
	hal_udc_power_control(udc, UDC_POWER_FULL);

	/* run controller and reload ep0 */
	USBx->DEVC_CS = (UDC_RUN | EP_EN);

	return 0;
}

int32_t hal_udc_device_disconnect(struct sp_udc *udc)
{
	volatile struct udc_reg *USBx = udc->reg;

	USBx->DEVC_CS = 0;
	UDC_LOGI("%s.%d %x\n", __func__, __LINE__, USBx->DEVC_CS);

	return 0;
}

int32_t hal_udc_init(struct sp_udc *udc)
{
	struct trb_data *tmp_segment_table = NULL;
	struct udc_ring *tmp_ring = NULL;
	struct udc_endpoint *ep = NULL;
	uint32_t aligned_len;
	uint8_t seg_count = 0;
	int32_t i;

	UDC_LOGI("version = 0x%x, offsets = 0x%x, parameter = 0x%x\n",
					udc->reg->DEVC_VER, udc->reg->DEVC_MMR, udc->reg->DEVC_PARAM);

	udc->event_ring_seg_total = EVENT_SEG_COUNT;
	udc->event_ccs = 1;
	udc->current_event_ring_seg = 0;

	/* malloc event ring segment teable */
	udc->event_seg_table = (struct segment_table *)udc_malloc_align(&udc->event_seg_table_pa,
							udc->event_ring_seg_total * ENTRY_SIZE, ALIGN_64_BYTE);

	if (!udc->event_seg_table) {
		UDC_LOGE("mem_alloc event_ring_seg_table fail\n");
		goto event_seg_table_malloc_fail;
	}

	UDC_LOGD("event segment table %px,%llx,%d\n", udc->event_seg_table, udc->event_seg_table_pa, udc->event_ring_seg_total);

	/* malloc event ring pointer */
	udc->event_ring = (struct udc_ring *)udc_malloc_align(&udc->event_ring_pa,
						udc->event_ring_seg_total * sizeof(struct udc_ring), ALIGN_64_BYTE);

	udc->event_ring->trb_va = NULL;
	udc->event_ring->end_trb_va = NULL;
	if (!udc->event_ring) {
		UDC_LOGE("mem_alloc evnet_ring fail\n");

		goto event_ring_malloc_fail;
	}

	UDC_LOGD("event ring %px,%llx\n", udc->event_ring, udc->event_ring_pa);

	/* malloc evnet ring and fill the event ring segment table */
	for (seg_count = 0; seg_count < udc->event_ring_seg_total; seg_count++) {
		tmp_segment_table = (struct trb_data *)((udc->event_seg_table) + seg_count);
		tmp_ring = &udc->event_ring[seg_count];
		udc_ring_malloc(udc, tmp_ring, EVENT_RING_COUNT);

		if (!tmp_ring->trb_va) {
			UDC_LOGE("mem_alloc event_ring %d fail\n", seg_count);

			goto event_ring_malloc_fail;
		}
		/* fill the event ring segment table */
		tmp_segment_table->entry0 = tmp_ring->trb_pa;
		tmp_segment_table->entry2 = tmp_ring->num_mem;

		aligned_len = roundup(sizeof(struct segment_table) * udc->event_ring_seg_total, ARCH_DMA_MINALIGN);
		flush_dcache_range((unsigned long)udc->event_seg_table,
					(unsigned long)udc->event_seg_table + aligned_len);

		UDC_LOGD("Event_ring[%d]:%px,%llx -> %px,%llx\n", seg_count, udc->event_ring[seg_count].trb_va,
						udc->event_ring[seg_count].trb_pa, tmp_ring->trb_va, tmp_ring->trb_pa);
	}

	/* malloc ep description */
	udc->ep_desc = (struct sp_desc *)udc_malloc_align(&udc->ep_desc_pa,
								ENTRY_SIZE * UDC_MAX_ENDPOINT_NUM, ALIGN_32_BYTE);
	if (!udc->ep_desc) {
		UDC_LOGE("udc_malloc_align device desc fail\n");

		goto fail;
	}

	UDC_LOGD("device descriptor addr:%px,%llx\n", udc->ep_desc, udc->ep_desc_pa);

	/* endpoint init */
	for (i = 0U; i < UDC_MAX_ENDPOINT_NUM; i++) {
		ep = &udc->ep_data[i];
		ep->is_in = false;
		ep->num = i;

		/* Control until ep is activated */
		ep->type = 0;
		ep->maxpacket = 0U;
		ep->transfer_buff = NULL;
		ep->transfer_len = 0U;
		ep->ep_transfer_ring.trb_va = NULL;
		ep->ep_transfer_ring.num_mem = TRANSFER_RING_COUNT;
		ep->ep_trb_ring_dq = NULL;

#ifndef EP_DYNAMIC_ALLOC
		udc_ring_malloc(udc, &ep->ep_transfer_ring, TRANSFER_RING_COUNT);

		if (!ep->ep_transfer_ring.trb_va) {
			UDC_LOGE("ep%d malloc trb fail\n", i);

			ep->ep_transfer_ring.trb_va = NULL;
		}
#endif

		UDC_LOGD("ep[%d]:%px\n", i, ep);
	}

	return 0;

fail:
event_ring_malloc_fail:
	if (udc->event_ring) {
		for (i = 0; i < udc->event_ring_seg_total; i++) {
			if (udc->event_ring[i].trb_va) {
				udc_free_align(udc->event_ring[i].trb_va, udc->event_ring[i].trb_pa,
								udc->event_ring[i].num_mem * ENTRY_SIZE);
			}
		}

		udc_free_align(udc->event_ring, udc->event_ring_pa,
					udc->event_ring_seg_total * sizeof(struct udc_ring));
	}

event_seg_table_malloc_fail:
	if (udc->event_seg_table) {
		udc_free_align(udc->event_seg_table, udc->event_seg_table_pa,
					udc->event_ring_seg_total * ENTRY_SIZE);
	}

	return -EPERM;
}

int32_t hal_udc_deinit(struct sp_udc *udc)
{
	struct udc_endpoint *ep;
	int32_t i;

	UDC_LOGI("udc %d deinit\n", udc->port_num);

	if (udc->ep_desc) {
		udc_free_align(udc->ep_desc, udc->ep_desc_pa, ENTRY_SIZE*UDC_MAX_ENDPOINT_NUM);
		udc->ep_desc = NULL;
	}

	if (udc->event_ring) {
		for (i = 0; i < udc->event_ring_seg_total; i++) {
			if (udc->event_ring[i].trb_va)
				udc_ring_free(udc, &udc->event_ring[i]);
		}

		udc_free_align(udc->event_ring, udc->event_ring_pa,
					udc->event_ring_seg_total * sizeof(struct udc_ring));
		udc->event_ring = NULL;
	}

	if (udc->event_seg_table) {
		udc_free_align(udc->event_seg_table, udc->event_seg_table_pa,
					udc->event_ring_seg_total * ENTRY_SIZE);
		udc->event_seg_table = NULL;
	}

	/* endpoint init */
	for (i = 0U; i < UDC_MAX_ENDPOINT_NUM; i++) {
		ep = &udc->ep_data[i];
		ep->ep_trb_ring_dq = NULL;

		if (ep->ep_transfer_ring.trb_va)
			udc_ring_free(udc, &ep->ep_transfer_ring);
	}

	return 0;
}

int32_t hal_udc_endpoint_unconfigure(struct sp_udc *udc, uint8_t ep_addr)
{
	volatile struct udc_reg *USBx = udc->reg;
	struct udc_endpoint *ep;
	uint8_t ep_num;

	UDC_LOGI("unconfig EP %x\n", ep_addr);

	ep_num = EP_NUM(ep_addr);
	ep = &udc->ep_data[ep_num];

	spin_lock(&ep->lock);

	ep->maxpacket = 0U;
	ep->is_in = false;
	ep->num   = 0U;
	ep->type = 0U;
	ep->ep_trb_ring_dq = NULL;

	if (ep_num == 0)
		USBx->EP0_CS &= ~(RDP_EN | EP_EN); 	/* disable ep */
	else
		USBx->EPN_CS[ep_num - 1] &= ~(RDP_EN | EP_EN);

#ifdef	EP_DYNAMIC_ALLOC
	if (ep->ep_transfer_ring.trb_va != NULL)
		udc_ring_free(udc, &ep->ep_transfer_ring);
#endif

	if (!list_empty(&ep->queue))
		UDC_LOGW("ep%d list not empty\n", ep->num);

	spin_unlock(&ep->lock);

	return 0;
}

int32_t hal_udc_endpoint_configure(struct sp_udc *udc, uint8_t ep_addr,
					uint8_t ep_type, uint16_t ep_max_packet_size)
{
	struct udc_endpoint *ep;
	uint8_t ep_num = EP_NUM(ep_addr);

	UDC_LOGI("ep%x,%x type:%x mps:%x config\n", ep_addr, ep_num, ep_type, ep_max_packet_size);

	if (ep_num >= UDC_MAX_ENDPOINT_NUM) {
		UDC_LOGE("ep_addr parameter error, max endpoint num is %d,%d\n",
								UDC_MAX_ENDPOINT_NUM, ep_num);

		return -EINVAL;
	}

	if (ep_type != UDC_EP_TYPE_CTRL && ep_type != UDC_EP_TYPE_ISOC
		&& ep_type != UDC_EP_TYPE_BULK && ep_type != UDC_EP_TYPE_INTR) {
		UDC_LOGE("ep_type parameter error, unsupported type!\n");

		return -EINVAL;
	}

	ep = &udc->ep_data[ep_num];
	spin_lock(&ep->lock);

	if (!list_empty(&ep->queue)) {
		UDC_LOGW("ep%d queue not empty\n", ep_num);

		spin_unlock(&ep->lock);

		return -EINVAL;
	}

	if (ep->ep_trb_ring_dq) {
		UDC_LOGE("ep%d repeated initialization fail\n", ep_num);

		spin_unlock(&ep->lock);

		return -EINVAL;
	}

	ep->maxpacket = ep_max_packet_size;
	ep->is_in = EP_DIR(ep_addr);
	ep->num = ep_num;
	ep->type = ep_type;

#ifdef EP_DYNAMIC_ALLOC
	/* init step h */
	udc_ring_malloc(udc, &ep->ep_transfer_ring, TRANSFER_RING_COUNT);
	UDC_LOGD("%s.%d\n", __func__, __LINE__);
#endif

	if (!ep->ep_transfer_ring.trb_va) {
		UDC_LOGE("udc_malloc_align ep_transfer_ring %d fail\n", ep_num);
		spin_unlock(&ep->lock);

		return -ENOMEM;
	}

	memset(ep->ep_transfer_ring.trb_va, 0, ep->ep_transfer_ring.num_mem * ENTRY_SIZE);
	INIT_LIST_HEAD(&ep->queue);

	ep->ep_trb_ring_dq = ep->ep_transfer_ring.trb_va;
	UDC_LOGD("ep_transfer_ring[%d]:%px,%llx\n", ep_num, ep->ep_transfer_ring.trb_va,
									ep->ep_transfer_ring.trb_pa);
	hal_udc_fill_ep_desc(udc, ep);

	spin_unlock(&ep->lock);

	return 0;
}

/*
   enable udc interrupt
   UDC_POWER_OFF: disable interrupt
   UDC_POWER_FULL: enable interrupt
*/
int32_t hal_udc_power_control(struct sp_udc *udc, enum udc_power_state power_state)
{
	volatile struct udc_reg *usbx = udc->reg;

	UDC_LOGD("%s.%d\n", __func__, __LINE__);

	if ((power_state != UDC_POWER_OFF) && (power_state != UDC_POWER_FULL)) {
		UDC_LOGE("power state error\n");
		return -EINVAL;
	}

	switch (power_state) {
	case UDC_POWER_OFF:
		usbx->DEVC_IMOD = IMOD;			/* interrupt interval */
		usbx->DEVC_IMAN &= ~DEVC_INTR_ENABLE;	/* Disable interrupt */
		break;
	case UDC_POWER_FULL:
		usbx->DEVC_IMAN |= DEVC_INTR_ENABLE;	/* Enable interrupt */
		break;
	case UDC_POWER_LOW:
		UDC_LOGE("no support\n");
		return -EINVAL;
	default:
		return -EINVAL;
	}

	return 0;
}

static void hal_udc_setup_complete(struct usb_ep *ep, struct usb_request *req)
{
	volatile struct udc_reg *USBx;
	struct sp_udc *udc = NULL;

	if (req->status || req->actual != req->length)
		UDC_LOGD("hal udc setup fail\n");

	if (!req->context)
		return;

	udc = req->context;
	USBx = udc->reg;

#if 0
	if (udc->usb_test_mode) {
		switch (udc->usb_test_mode) {
		case USB_TEST_J:
			UDC_LOGL("TEST_J\n");
			writel(bitfield_replace(readl(&USBx->GL_CS), 12, 4, 0x1), &USBx->GL_CS);
			break;
		case USB_TEST_K:
			UDC_LOGL("TEST_K\n");
			writel(bitfield_replace(readl(&USBx->GL_CS), 12, 4, 0x2), &USBx->GL_CS);
			break;
		case USB_TEST_SE0_NAK:
			UDC_LOGL("TEST_SE0_NAK\n");
			writel(bitfield_replace(readl(&USBx->GL_CS), 12, 4, 0x3), &USBx->GL_CS);
			break;
		case USB_TEST_PACKET:
			UDC_LOGL("TEST_PACKET\n");
			writel(bitfield_replace(readl(&USBx->GL_CS), 12, 4, 0x4), &USBx->GL_CS);
			break;
		case USB_TEST_FORCE_ENABLE:
			UDC_LOGL("TEST_FORCE_ENABLE\n");
			writel(bitfield_replace(readl(&USBx->GL_CS), 12, 4, 0x5), &USBx->GL_CS);
			break;
		default:
			UDC_LOGL("Unsupport teset\n");
		}
	}
#endif
}

static int hal_udc_setup(struct sp_udc *udc, const struct usb_ctrlrequest *ctrl)
{
	struct udc_endpoint *ep_0 = &(udc->ep_data[0]);
	struct udc_endpoint *ep = NULL;
	struct usb_gadget *gadget = &udc->gadget;
	struct usb_composite_dev *cdev = get_gadget_data(gadget);
	struct usb_request *req = cdev->req;
	int value = -EOPNOTSUPP;
	u16 w_index = le16_to_cpu(ctrl->wIndex);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 w_length = le16_to_cpu(ctrl->wLength);
	uint8_t ep_num = 0;

	udc->usb_test_mode = 0;

	if (ctrl->bRequestType & USB_DIR_IN) {
		ep_0->bEndpointAddress |= USB_DIR_IN;
	} else {
		if (ctrl->wLength)
			ep_0->bEndpointAddress = USB_DIR_OUT;
		else
			ep_0->bEndpointAddress |= USB_DIR_IN;
	}

	/* enable auto set flag */
	udc->aset_flag = false;

	if ((USB_REQ_SET_CONFIGURATION == ctrl->bRequest
		&& (USB_DIR_OUT | USB_TYPE_STANDARD | USB_RECIP_DEVICE) == ctrl->bRequestType)
		|| (USB_REQ_SET_INTERFACE == ctrl->bRequest
			&& (USB_DIR_OUT | USB_RECIP_INTERFACE) == ctrl->bRequestType)
		|| (USB_REQ_SET_ADDRESS == ctrl->bRequest && (USB_DIR_OUT) == ctrl->bRequestType)) {
		udc->aset_flag = true;
	}

	value = udc->driver->setup(gadget, ctrl);
	if (value >= 0)
		goto done;

	req->zero = 0;
	req->context = udc;
	req->complete = hal_udc_setup_complete;
	req->length = 0;

	/* request get status */
	switch (ctrl->bRequest) {
	case USB_REQ_GET_STATUS:
		/* request device status */
		if (ctrl->bRequestType == (USB_DIR_IN | USB_RECIP_DEVICE)) {
			value = 2;
			put_unaligned_le16(0, req->buf);
		}

		/* interface status */
		if (ctrl->bRequestType == (USB_DIR_IN | USB_RECIP_INTERFACE)) {
			value = 2;
			put_unaligned_le16(0, req->buf);
		}

		/* endpoint status */
		if (ctrl->bRequestType == (USB_DIR_IN | USB_RECIP_ENDPOINT)) {
			ep_num = EP_NUM(w_index);
			if (ep_num < UDC_MAX_ENDPOINT_NUM) {
				ep = &(udc->ep_data[ep_num]);
				if (ep) {
					value = 2;
					UDC_LOGD("get EP%d status %d\n", ep_num, ep->status);
					put_unaligned_le16(ep->status, req->buf);
				}
			}
		}
		break;
	/* request set feature */
	case USB_REQ_SET_FEATURE:
		/* request set feature */
		if (ctrl->bRequestType == (USB_DIR_OUT | USB_RECIP_DEVICE)) {
			if (w_value != USB_DEVICE_TEST_MODE && w_value != USB_DEVICE_REMOTE_WAKEUP)
				break;

#if 0
			if (w_value == USB_DEVICE_TEST_MODE) {
				switch (w_index >> 8) {
				case USB_TEST_J:
					UDC_LOGL("TEST_J\n");
					udc->usb_test_mode = USB_TEST_J;
					value = 0;
					break;
				case USB_TEST_K:
					UDC_LOGL("TEST_K\n");
					udc->usb_test_mode = USB_TEST_K;
					value = 0;
					break;
				case USB_TEST_SE0_NAK:
					UDC_LOGL("TEST_SE0_NAK\n");
					udc->usb_test_mode = USB_TEST_SE0_NAK;
					value = 0;
					break;
				case USB_TEST_PACKET:
					UDC_LOGL("TEST_PACKET\n");
					udc->usb_test_mode = USB_TEST_PACKET;
					value = 0;
					break;
				case USB_TEST_FORCE_ENABLE:
					UDC_LOGL("TEST_FORCE_ENABLE\n");
					udc->usb_test_mode = USB_TEST_FORCE_ENABLE;
					break;
				default:
					UDC_LOGL("Unsupport teset\n");
				}

				if (value)
					break;
			}
#endif
		}

		/* interface status */
		if (ctrl->bRequestType == (USB_DIR_OUT | USB_RECIP_INTERFACE))
			break;

		/* endpoint status */
		if (ctrl->bRequestType == (USB_DIR_OUT | USB_RECIP_ENDPOINT)) {
			if (w_value != USB_ENDPOINT_HALT)
				break;

			ep_num = EP_NUM(w_index);
			if (ep_num < UDC_MAX_ENDPOINT_NUM) {
				ep = &(udc->ep_data[ep_num]);
				if (ep)
					sp_udc_set_halt(&ep->ep, 1);
			}
		}
		value = 0;
		break;

	/* request clear feature */
	case USB_REQ_CLEAR_FEATURE:
		/* request clear feature */
		if (ctrl->bRequestType == (USB_DIR_OUT | USB_RECIP_DEVICE)) {
			if (w_value != USB_DEVICE_TEST_MODE || w_value != USB_DEVICE_REMOTE_WAKEUP)
				break;
		}

		/* interface status */
		if (ctrl->bRequestType == (USB_DIR_OUT | USB_RECIP_INTERFACE))
			break;

		/* endpoint status */
		if (ctrl->bRequestType == (USB_DIR_OUT | USB_RECIP_ENDPOINT)) {
			if (w_value != USB_ENDPOINT_HALT)
				break;

			ep_num = EP_NUM(w_index);
			if (ep_num < UDC_MAX_ENDPOINT_NUM) {
				ep = &(udc->ep_data[ep_num]);
				if (ep)
					sp_udc_set_halt(&ep->ep, 0);
			}
		}
		value = 0;
		break;

	/* request set address */
	case USB_REQ_SET_ADDRESS:
		value = 0;
		break;
	}

	if (value >= 0) {
		req->length = value;
		req->context = udc;
		req->zero = value < w_length;
		value = usb_ep_queue(cdev->gadget->ep0, req, GFP_ATOMIC);

		if (value < 0) {
			UDC_LOGD("ep_queue --> %d\n", value);
			req->status = 0;
			hal_udc_setup_complete(gadget->ep0, req);
		}

		goto done;
	}

	UDC_LOGD("Unsupport request,return value %d\n", value);
	sp_udc_set_halt(&ep_0->ep, 1);

done:
	return value;
}

static void cfg_udc_ep(struct sp_udc *udc)
{
	struct udc_endpoint *udc_ep = NULL;
	int ep_id = 0;

	UDC_LOGD("%s.%d\n", __func__, __LINE__);

	for (ep_id = 0; ep_id < UDC_MAX_ENDPOINT_NUM; ep_id++) {
		udc_ep = &udc->ep_data[ep_id];
		udc_ep->num = ep_id;

		udc_ep->ep.name = ep_names[ep_id];
		udc_ep->ep.ops = &sp_ep_ops;
		udc_ep->dev = udc;
	}

	usb_ep_set_maxpacket_limit(&udc->ep_data[0].ep, EP_FIFO_SIZE);
	usb_ep_set_maxpacket_limit(&udc->ep_data[1].ep, ISO_MPS_SIZE);
	usb_ep_set_maxpacket_limit(&udc->ep_data[2].ep, ISO_MPS_SIZE);
	usb_ep_set_maxpacket_limit(&udc->ep_data[3].ep, ISO_MPS_SIZE);
	usb_ep_set_maxpacket_limit(&udc->ep_data[4].ep, ISO_MPS_SIZE);
	usb_ep_set_maxpacket_limit(&udc->ep_data[5].ep, ISO_MPS_SIZE);
	usb_ep_set_maxpacket_limit(&udc->ep_data[6].ep, ISO_MPS_SIZE);
	usb_ep_set_maxpacket_limit(&udc->ep_data[7].ep, ISO_MPS_SIZE);
	usb_ep_set_maxpacket_limit(&udc->ep_data[8].ep, ISO_MPS_SIZE);

	udc->ep_data[1].bEndpointAddress = USB_DIR_IN | EP1;
	udc->ep_data[1].bmAttributes = USB_ENDPOINT_XFER_BULK;

	udc->ep_data[2].bEndpointAddress = USB_DIR_IN | EP2;
	udc->ep_data[2].bmAttributes = USB_ENDPOINT_XFER_BULK;

	udc->ep_data[3].bEndpointAddress = USB_DIR_IN | EP3;
	udc->ep_data[3].bmAttributes = USB_ENDPOINT_XFER_INT;

	udc->ep_data[4].bEndpointAddress = USB_DIR_IN | EP4;
	udc->ep_data[4].bmAttributes = USB_ENDPOINT_XFER_ISOC;

	udc->ep_data[5].bEndpointAddress = USB_DIR_OUT | EP5;
	udc->ep_data[5].bmAttributes = USB_ENDPOINT_XFER_BULK;

	udc->ep_data[6].bEndpointAddress = USB_DIR_OUT | EP6;
	udc->ep_data[6].bmAttributes = USB_ENDPOINT_XFER_BULK;

	udc->ep_data[7].bEndpointAddress = USB_DIR_OUT | EP7;
	udc->ep_data[7].bmAttributes = USB_ENDPOINT_XFER_INT;

	udc->ep_data[8].bEndpointAddress = USB_DIR_OUT | EP8;
	udc->ep_data[8].bmAttributes = USB_ENDPOINT_XFER_ISOC;
}

static void sp_udc_ep_init(struct sp_udc *udc)
{
	struct udc_endpoint *ep;
	unsigned long flags;
	u32 i;

	UDC_LOGI("udc %d ep_init\n", udc->port_num);

	local_irq_save(flags);

	/* device/ep0 records init */
	INIT_LIST_HEAD(&udc->gadget.ep_list);
	INIT_LIST_HEAD(&udc->gadget.ep0->ep_list);

	for (i = 0; i < UDC_MAX_ENDPOINT_NUM; i++) {
		ep = &udc->ep_data[i];
		if (i != 0)
			list_add_tail (&ep->ep.ep_list, &udc->gadget.ep_list);

		ep->dev = udc;
		INIT_LIST_HEAD(&ep->queue);
	}

	local_irq_restore(flags);
}

static int sp_udc_ep_enable(struct usb_ep *_ep, const struct usb_endpoint_descriptor *desc)
{
	struct udc_endpoint *ep;
	struct sp_udc *udc;
	int max;

	ep = to_sp_ep(_ep);
	if (!_ep || !desc || desc->bDescriptorType != USB_DT_ENDPOINT) {
		UDC_LOGE("%s.%d,EINVAL\n", __func__, __LINE__);
		return -EINVAL;
	}

	udc = ep->dev;
	if (!udc->driver) {
		UDC_LOGE("%s.%d,%p,%x\n", __func__, __LINE__, udc->driver, udc->gadget.speed);
		return -ESHUTDOWN;
	}

	if (ep->ep_trb_ring_dq) {
		UDC_LOGW("ep%x already enable\n", ep->num);
		return -EINVAL;
	}

	ep->ep.desc = desc;

	max = usb_endpoint_maxp(desc) & 0x1fff;
	hal_udc_endpoint_configure(udc, desc->bEndpointAddress, desc->bmAttributes, max & 0x7ff);

	return 0;
}

static int sp_udc_ep_disable(struct usb_ep *_ep)
{
	struct udc_endpoint *ep = NULL;
	struct sp_udc *udc;

	UDC_LOGI("disable ep:%s\n", _ep->name);

	if (!_ep) {
		UDC_LOGE("%s.%d,EINVAL\n", __func__, __LINE__);
		return -EINVAL;
	}

	ep = to_sp_ep(_ep);
	if (!ep) {
		UDC_LOGE("%s.%d,EINVAL\n", __func__, __LINE__);
		return -EINVAL;
	}

	ep->ep.desc = NULL;
	udc = ep->dev;
	sp_udc_nuke(udc, ep, -ESHUTDOWN);
	hal_udc_endpoint_unconfigure(udc, ep->bEndpointAddress);

	return 0;
}

static struct usb_request *sp_udc_alloc_request(struct usb_ep *_ep, gfp_t mem_flags)
{
	struct sp_request *req;

	if (!_ep) {
		UDC_LOGE("%s.%d,_ep is null\n", __func__, __LINE__);
		return NULL;
	}

	req = kzalloc (sizeof(struct sp_request), mem_flags);
	if (!req) {
		UDC_LOGE("%s.%d,req is null\n", __func__, __LINE__);
		return NULL;
	}

	req->req.dma = DMA_ADDR_INVALID;
	INIT_LIST_HEAD (&req->queue);

	return &req->req;
}

static void sp_udc_free_request(struct usb_ep *_ep, struct usb_request *_req)
{
	struct sp_request *req = NULL;

	if (!_ep || !_req) {
		UDC_LOGE("%s.%d,error\n", __func__, __LINE__);
		return;
	}

	req = to_sp_req(_req);

#ifdef PIO_MODE
	kfree(req->buffer);
	req->buffer = NULL;
#endif

	kfree(req);
}

static int sp_udc_queue(struct usb_ep *_ep, struct usb_request *_req, gfp_t gfp_flags)
{
	struct sp_request *req = to_sp_req(_req);
	struct udc_endpoint *ep = to_sp_ep(_ep);
	struct sp_udc *udc;
	unsigned long flags;
	int32_t ret;

	if (unlikely (!_ep)) {
		UDC_LOGE("%s: invalid args\n", __func__);
		return -EINVAL;
	}

	if (likely (!req)) {
		UDC_LOGE("%s: req is null Why?\n", __func__);
		return -EINVAL;
	}

	udc = ep->dev;
	if (unlikely (!udc->driver)) {
		UDC_LOGE("%s.%d,%p,%x\n", __func__, __LINE__, udc->driver, udc->gadget.speed);
		return -ESHUTDOWN;
	}

	if (unlikely(!_req || !_req->complete || !_req->buf)) {
		if (!_req)
			UDC_LOGE("%s: 1 X X X\n", __func__);
		else
			UDC_LOGE("%s: 0 %p %p\n", __func__, _req->complete, _req->buf);

		return -EINVAL;
	}

	_req->status = -EINPROGRESS;
	_req->actual = 0;

	if (_req->zero)
		UDC_LOGD("EP%d queue zero %d\n", ep->num, _req->zero);

	spin_lock_irqsave(&ep->lock, flags);
	if (!ep->ep_trb_ring_dq) {
		UDC_LOGE("ep%d is not configure why??\n", ep->num);

		spin_unlock_irqrestore(&ep->lock, flags);

		return -EINVAL;
	}

#ifndef PIO_MODE
	ret = usb_gadget_map_request(&udc->gadget, _req, EP_DIR(ep->bEndpointAddress));
	if (ret)
		return ret;
#endif

	list_add_tail(&req->queue, &ep->queue);

	ret = hal_udc_endpoint_transfer(udc, req, ep->bEndpointAddress, _req->buf, _req->dma,
									_req->length, _req->zero);
	UDC_LOGD("%s: ep:%x len %d, req:%px,trb:%px\n", __func__, ep->bEndpointAddress,
								_req->length, req, req->transfer_trb);
	if (ret) {
		UDC_LOGE("%s: transfer err\n", __func__);

#ifndef PIO_MODE
		usb_gadget_unmap_request(&udc->gadget, _req, EP_DIR(ep->bEndpointAddress));
#endif

		list_del(&req->queue);
		spin_unlock_irqrestore(&ep->lock, flags);

		return -EINVAL;
	}
	spin_unlock_irqrestore(&ep->lock, flags);

	return 0;
}

static int sp_udc_dequeue(struct usb_ep *_ep, struct usb_request *_req)
{
	struct udc_endpoint *ep = to_sp_ep(_ep);
	struct sp_request *req = NULL;
	struct sp_udc *udc;
	int retval = -EINVAL;

	UDC_LOGD("%s(%px,%px)\n", __func__, _ep, _req);

	if (!_ep || !_req) {
		UDC_LOGE("%s.%d,%px,%px\n", __func__, __LINE__, _ep, _req);
		return retval;
	}

	udc = ep->dev;
	if (!udc->driver) {
		UDC_LOGE("%s.%d,dev->driver is null\n", __func__, __LINE__);
		return -ESHUTDOWN;
	}

	req = to_sp_req(_req);

	spin_lock(&ep->lock);
	if (!list_empty(&ep->queue)) {
		UDC_LOGD("dequeued req %px from %s, len %d buf %px,sp_req %px,%px\n",
					req, _ep->name, _req->length, _req->buf, req, req->transfer_trb);

#ifndef PIO_MODE
		usb_gadget_unmap_request(&udc->gadget, _req, ep->is_in);
#endif

		retval = 0;
		list_del(&req->queue);
		spin_unlock(&ep->lock);
		sp_udc_done(ep, req, -ECONNRESET);
	} else {
		spin_unlock(&ep->lock);
	}

	UDC_LOGD("dequeue done\n");

	return retval;
}

static int sp_udc_set_halt(struct usb_ep *_ep, int value)
{
	struct udc_endpoint *ep = to_sp_ep(_ep);
	struct sp_udc *udc;
	unsigned long flags;
	int retval;

	if (unlikely (!_ep || (!_ep->desc && _ep->name != ep0name))) {
		UDC_LOGE("%s: inval 2\n", __func__);
		return -EINVAL;
	}

	udc = ep->dev;
	UDC_LOGI("set EP%x halt value:%x\n", ep->num, value);

	local_irq_save (flags);
	retval = hal_udc_endpoint_stall(udc, ep->bEndpointAddress, value);
	local_irq_restore (flags);

	return retval;
}

/* gadget ops */
#if 0
static struct usb_ep *sp_udc_match_ep(struct usb_gadget *_gadget,
					struct usb_endpoint_descriptor *desc,
						struct usb_ss_ep_comp_descriptor *ep_comp)
{
	struct usb_ep *ep = NULL;

	if (usb_endpoint_type(desc) == USB_ENDPOINT_XFER_INT) {
		gadget_for_each_ep(ep, _gadget) {
			usb_ep_set_maxpacket_limit(ep, INT_MPS_SIZE);
			if (ep->caps.type_int && usb_gadget_ep_match_desc(_gadget, ep, desc, ep_comp))
				return ep;

			usb_ep_set_maxpacket_limit(ep, ISO_MPS_SIZE);
		}
	}

	return NULL;
}
#endif

static int sp_udc_get_frame(struct usb_gadget *gadget)
{
	struct sp_udc *udc = to_sp_udc(gadget);
	volatile struct udc_reg *USBx = udc->reg;
	uint32_t frame_num;

	frame_num = USBx->USBD_FRNUM_ADDR & FRNUM;
	UDC_LOGD("%s.%d,frame_num:%x\n", __func__, __LINE__, frame_num);

	return frame_num;
}

static int sp_udc_pullup(struct usb_gadget *gadget, int is_on)
{
	struct sp_udc *udc = to_sp_udc(gadget);

	UDC_LOGD("+%s.%d,is_on:%x\n", __func__, __LINE__, is_on);

	udc->vbus_active = 1;

	if (is_on) {
		if (udc->vbus_active && udc->driver) {
			/* run controller pullup D+ */
			hal_udc_device_connect(udc);
		} else {
			return 1;
		}
	} else {
		hal_udc_device_disconnect(udc);
	}

	return 0;
}

static int sp_udc_vbus_session(struct usb_gadget *gadget, int is_active)
{
	struct sp_udc *udc = to_sp_udc(gadget);

	udc->vbus_active = is_active;
	//usb_udc_vbus_handler(gadget, udc->vbus_active);
	UDC_LOGI("sp_udc vbus SW %d\n", is_active);

	return 0;
}

static int sp_udc_start(struct usb_gadget *gadget, struct usb_gadget_driver *driver)
{
	struct sp_udc *udc = to_sp_udc(gadget);

	UDC_LOGI("%s start\n", __func__);

	/* Sanity checks */
	if (!udc) {
		UDC_LOGE("%s.%d,dev is null\n", __func__, __LINE__);
		return -ENODEV;
	}

	if (udc->driver) {
		UDC_LOGE("%s.%d,busy\n", __func__, __LINE__);
		return -EBUSY;
	}

	/* Hook the driver */
	udc->driver = driver;

	/* initialize udc controller */
	hal_udc_init(udc);

	UDC_LOGI("%s success\n", __func__);

	return 0;
}

static int sp_udc_stop(struct usb_gadget *gadget)
{
	struct sp_udc *udc = to_sp_udc(gadget);

	UDC_LOGI("%s start\n", __func__);

	if (!udc) {
		UDC_LOGE("%s.%d,dev is null\n", __func__, __LINE__);

		return -ENODEV;
	}

	udc->driver = NULL;

	/* disable interrupt */
	hal_udc_power_control(udc, UDC_POWER_OFF);

	/* free udc */
	hal_udc_deinit(udc);

	UDC_LOGI("%s success\n", __func__);

	return 0;
}

static int sp_udc_probe(struct udevice *udev)
{
	struct sp_udc *udc = dev_get_priv(udev);
	fdt_addr_t base;

	base = dev_read_addr_index(udev, 0);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	udc->reg = ioremap(base, sizeof(struct udc_reg));
	if (!udc->reg)
		return -ENOMEM;

#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
	base = dev_read_addr_index(udev, 1);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	moon3_reg = ioremap(base, 128);
	if (!moon3_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 2);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	moon0_reg = ioremap(base, 128);
	if (!moon0_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 3);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	uphy0_reg = ioremap(base, 128);
	if (!uphy0_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 4);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	hb_gp_reg = ioremap(base, 128);
	if (!hb_gp_reg)
		return -ENOMEM;
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	base = dev_read_addr_index(udev, 1);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	moon4_reg = ioremap(base, 128);
	if (!moon4_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 2);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	moon0_reg = ioremap(base, 128);
	if (!moon0_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 3);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	moon1_reg = ioremap(base, 128);
	if (!moon1_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 4);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	moon2_reg = ioremap(base, 128);
	if (!moon2_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 5);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	uphy0_reg = ioremap(base, 128);
	if (!uphy0_reg)
		return -ENOMEM;

	base = dev_read_addr_index(udev, 6);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	hb_gp_reg = ioremap(base, 128);
	if (!hb_gp_reg)
		return -ENOMEM;
#endif

	cfg_udc_ep(udc);
	udc->port_num = 0;
	udc->gadget.ops = &sp_ops;
	udc->gadget.ep0 = &(udc->ep_data[0].ep);
	udc->gadget.name = DRIVER_NAME;
	udc->gadget.max_speed = USB_SPEED_HIGH;

#if 1	/* High Speed */
	udc->def_run_full_speed = false;
#else	/* Full Speed */
	udc->def_run_full_speed = true;
#endif

	sp_udc_ep_init(udc);
	spin_lock_init (&udc->lock);
	init_ep_spin(udc);

	udc->bus_reset_finish = false;
	udc->frame_num = 0;
	udc->vbus_active = false;
	udc->dev = udev;

	usb_add_gadget_udc((struct device *)udev, &udc->gadget);

	sp_udc_arry[udc->port_num] = udc;

	/* phy configurations */
	uphy_init(udev->seq_);

	/* set USB device mode */
	usb_power_init(0, udev->seq_);

	UDC_LOGL("%s succeeds\n", __func__);

	return 0;
}

static int sp_udc_remove(struct udevice *udev)
{
	struct sp_udc *udc = dev_get_priv(udev);

	usb_del_gadget_udc(&udc->gadget);
	usb_power_init(1, udev->seq_);

	return 0;
}

static const struct udevice_id sp_udc_ids[] = {
#if defined(CONFIG_TARGET_PENTAGRAM_Q645)
	{ .compatible = "sunplus,q645-usb-udc" },
#elif defined(CONFIG_TARGET_PENTAGRAM_SP7350)
	{ .compatible = "sunplus,sp7350-usb-udc" },
#endif
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

