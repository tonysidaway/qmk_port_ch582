#include "host_driver.h"
#include "host.h"
#include "usb_device_state.h"
#include "print.h"
#include "CH58x_common.h"
#include "keycode_config.h"
#ifdef USB_ENABLE
#include "usb_main.h"
#include "printf.h"
#endif
#ifdef BLE_ENABLE
#include "ble.h"
#endif
#ifdef ESB_ENABLE
#include "esb.h"
#endif

uint8_t keyboard_led_state;
extern volatile uint8_t kbd_protocol_type;

uint8_t keyboard_leds()
{
    return keyboard_led_state;
}

void send_keyboard(report_keyboard_t *report)
{
#ifdef NKRO_ENABLE
    if (keymap_config.nkro) {
#ifdef USB_ENABLE
        if (kbd_protocol_type == kbd_protocol_usb) {
            hid_nkro_keyboard_send_report((uint8_t *)&report->nkro, EXKEY_IN_EP_SIZE);
        }
#endif
#ifdef BLE_ENABLE
        if (kbd_protocol_type == kbd_protocol_ble) {
            // there is a report id we don't want to have
            wireless_ringbuffer_write(KEYBOARD_REPORT_BITS + 1, BLE_REPORT_ID_KEYBOARD, (uint8_t *)report + 1);
        }
#endif
#ifdef ESB_ENABLE
        if (kbd_protocol_type == kbd_protocol_esb) {
            wireless_ringbuffer_write(EXKEY_IN_EP_SIZE, REPORT_ID_NKRO, (uint8_t *)&report->nkro);
        }
#endif
    } else
#endif
    {
#ifdef USB_ENABLE
        if (kbd_protocol_type == kbd_protocol_usb) {
            hid_bios_keyboard_send_report((uint8_t *)report, KEYBOARD_REPORT_SIZE);
        }
#endif
#ifdef BLE_ENABLE
        if (kbd_protocol_type == kbd_protocol_ble) {
            // we don't have the corresponding interface
        }
#endif
#ifdef ESB_ENABLE
        if (kbd_protocol_type == kbd_protocol_esb) {
            wireless_ringbuffer_write(KEYBOARD_REPORT_SIZE, REPORT_ID_KEYBOARD, (uint8_t *)report);
        }
#endif
    }
}

void send_mouse(report_mouse_t *report)
{
#ifdef MOUSE_ENABLE
#ifdef USB_ENABLE
    if (kbd_protocol_type == kbd_protocol_usb) {
        uint8_t report_to_send[6];

        report_to_send[0] = REPORT_ID_MOUSE;
        memcpy(report_to_send + 1, report, 5);
        hid_exkey_send_report(report_to_send, 6);
    }
#endif
#ifdef BLE_ENABLE
    if (kbd_protocol_type == kbd_protocol_ble) {
        wireless_ringbuffer_write(5, BLE_REPORT_ID_MOUSE, (uint8_t *)report);
    }
#endif
#ifdef ESB_ENABLE
    if (kbd_protocol_type == kbd_protocol_esb) {
        wireless_ringbuffer_write(5, REPORT_ID_MOUSE, (uint8_t *)report);
    }
#endif
#endif
}

void send_system(uint16_t data)
{
#ifdef USB_ENABLE
    if (kbd_protocol_type == kbd_protocol_usb) {
        uint8_t report_to_send[3];

        report_to_send[0] = REPORT_ID_SYSTEM;
        memcpy(report_to_send + 1, &data, 2);
        hid_exkey_send_report(report_to_send, 3);
    }
#endif
#ifdef BLE_ENABLE
    if (kbd_protocol_type == kbd_protocol_ble) {
        uint16_t data_to_send = data;

        wireless_ringbuffer_write(2, BLE_REPORT_ID_SYSTEM, (uint8_t *)&data_to_send);
    }
#endif
#ifdef ESB_ENABLE
    if (kbd_protocol_type == kbd_protocol_esb) {
        uint16_t data_to_send = data;

        wireless_ringbuffer_write(2, REPORT_ID_SYSTEM, (uint8_t *)&data_to_send);
    }
#endif
}

void send_consumer(uint16_t data)
{
#ifdef USB_ENABLE
    if (kbd_protocol_type == kbd_protocol_usb) {
        uint8_t report_to_send[3];

        report_to_send[0] = REPORT_ID_CONSUMER;
        memcpy(report_to_send + 1, &data, 2);
        hid_exkey_send_report(report_to_send, 3);
    }
#endif
#ifdef BLE_ENABLE
    if (kbd_protocol_type == kbd_protocol_ble) {
        uint16_t data_to_send = data;

        wireless_ringbuffer_write(2, BLE_REPORT_ID_CONSUMER, (uint8_t *)&data_to_send);
    }
#endif
#ifdef ESB_ENABLE
    if (kbd_protocol_type == kbd_protocol_esb) {
        uint16_t data_to_send = data;

        wireless_ringbuffer_write(2, REPORT_ID_CONSUMER, (uint8_t *)&data_to_send);
    }
#endif
}

void send_programmable_button(uint32_t data)
{
}

void send_digitizer(report_digitizer_t *report)
{
}

host_driver_t ch582_driver = { keyboard_leds, send_keyboard, send_mouse, send_system, send_consumer, send_programmable_button };

void raw_hid_send(uint8_t *data, uint8_t length)
{
#ifdef USB_ENABLE
    if (kbd_protocol_type == kbd_protocol_usb) {
        hid_custom_send_report(data, length);
    }
#endif
#ifdef BLE_ENABLE
    if (kbd_protocol_type == kbd_protocol_ble) {
        wireless_ringbuffer_write(length, BLE_REPORT_ID_CUSTOM, data);
    }
#endif
#ifdef ESB_ENABLE
    if (kbd_protocol_type == kbd_protocol_esb) {
        wireless_ringbuffer_write(length, REPORT_ID_CUSTOM, data);
    }
#endif
}

void protocol_setup()
{
    usb_device_state_init();
}

void protocol_pre_init()
{
#ifdef USB_ENABLE
    if (kbd_protocol_type == kbd_protocol_usb) {
        init_usb_driver();
    }
#endif
#ifdef BLE_ENABLE
    if (kbd_protocol_type == kbd_protocol_ble) {
        HidDev_Init();
        hogp_init();
    }
#endif
#ifdef ESB_ENABLE
    if (kbd_protocol_type == kbd_protocol_esb) {
    }
#endif
}

void protocol_post_init()
{
    host_set_driver(&ch582_driver);
}

// void protocol_pre_task()
// {
// }

// void protocol_post_task()
// {
// }

__HIGH_CODE void protocol_task()
{
    extern void keyboard_task();

#ifdef USB_ENABLE
    if (kbd_protocol_type == kbd_protocol_usb) {
        keyboard_task();
    }
#endif
#ifdef BLE_ENABLE
    if (kbd_protocol_type == kbd_protocol_ble) {
        TMOS_SystemProcess();
    }
#endif
#ifdef ESB_ENABLE
    if (kbd_protocol_type == kbd_protocol_esb) {
        TMOS_SystemProcess();
    }
#endif
}

void keyboard_post_init_user(void)
{
#ifdef DEBUG
    print("Set log output for QMK.\n");
#endif
}