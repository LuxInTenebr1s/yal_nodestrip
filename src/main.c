#include <logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

#include <zephyr.h>
#include <device.h>
#include <drivers/pwm.h>
#include <usb/usb_device.h>

#include <net/net_config.h>
#include <net/net_mgmt.h>
#include <net/net_if.h>

#define PWM_LED0_NODE 	DT_ALIAS(pwm_led0)

#if DT_NODE_HAS_STATUS(PWM_LED0_NODE, okay)
#define PWM_CTLR 	DT_PWMS_CTLR(PWM_LED0_NODE)
#define PWM_CHANNEL 	DT_PWMS_CHANNEL(PWM_LED0_NODE)
#define PWM_FLAGS 	DT_PWMS_FLAGS(PWM_LED0_NODE)
#else
#error "Not configured for this sample"
#define PWM_CTLR	DT_INVALID_NODE
#define PWM_CHANNEL	0
#define PWM_FLAGS	0
#endif

#define PERIOD_1MS_USEC 10
#define PERIOD_USEC 	255 * PERIOD_1MS_USEC

static struct net_mgmt_event_callback mgmt_cb;
static const struct device *pwm;

static void handler(struct net_mgmt_event_callback *cb,
		    uint32_t mgmt_event,
		    struct net_if *iface)
{
	const uint8_t *data = cb->info;
	if (mgmt_event != NET_EVENT_IPV4_ARTNET_OUT) {
		return;
	}

	LOG_INF("got data: %d @ %p", cb->info_length, cb->info);
	pwm_pin_set_usec(pwm, PWM_CHANNEL, PERIOD_USEC,
			 data[0] * PERIOD_1MS_USEC, PWM_FLAGS);
}

void main(void)
{
	net_mgmt_init_event_callback(&mgmt_cb, handler,
				     NET_EVENT_IPV4_ARTNET_OUT);
	net_mgmt_add_event_callback(&mgmt_cb);

	pwm = DEVICE_DT_GET(PWM_CTLR);
	if (!device_is_ready(pwm)) {
		LOG_ERR("Error: PWM device is not ready");
		return;
	}
	pwm_pin_set_usec(pwm, PWM_CHANNEL, PERIOD_USEC, 0, PWM_FLAGS);

#if defined(CONFIG_USB_DEVICE_STACK)
        int ret;

        ret = usb_enable(NULL);
        if (ret != 0) {
                printk("usb enable error %d\n", ret);
        }

        (void)net_config_init_app(NULL, "Initializing network");
#endif /* CONFIG_USB_DEVICE_STACK */
}
