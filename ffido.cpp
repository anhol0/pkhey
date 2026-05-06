#include <linux/uhid.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>

static const uint8_t fido_report_desc[] = {
    0x06, 0xD0, 0xF1,        // Usage Page (FIDO 0xF1D0)
    0x09, 0x01,              // Usage (U2F HID Authenticator Device)
    0xA1, 0x01,              // Collection (Application)

    0x09, 0x20,              // Usage (Input Report Data)
    0x15, 0x00,              // Logical Minimum (0)
    0x26, 0xFF, 0x00,        // Logical Maximum (255)
    0x75, 0x08,              // Report Size (8 bits)
    0x95, 0x40,              // Report Count (64 bytes)
    0x81, 0x02,              // Input (Data,Var,Abs)

    0x09, 0x21,              // Usage (Output Report Data)
    0x15, 0x00,
    0x26, 0xFF, 0x00,
    0x75, 0x08,
    0x95, 0x40,
    0x91, 0x02,              // Output (Data,Var,Abs)

    0xC0                     // End Collection
};

int main() {
    int fd = open("/dev/uhid", O_RDWR);
    if (fd < 0) {
        perror("open /dev/uhid");
        return 1;
    }

    struct uhid_event ev;
    memset(&ev, 0, sizeof(ev));

    // Device creation event
    ev.type = UHID_CREATE2;

    memcpy(ev.u.create2.rd_data, fido_report_desc, sizeof(fido_report_desc));
    ev.u.create2.rd_size = sizeof(fido_report_desc);

    ev.u.create2.bus = BUS_USB;
    ev.u.create2.vendor = 0x1234;
    ev.u.create2.product = 0x5678;

    write(fd, &ev, sizeof(ev));

    printf("UHID device created\n");

    while (1) {
        read(fd, &ev, sizeof(ev));

        if (ev.type == UHID_OUTPUT) {
            printf("Got HID output report (browser → device)\n");
        }
    }

    return 0;
}
