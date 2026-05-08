// MIT License
// 
// Copyright (c) 2026 Efim Belorusets
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <cstdint>
#include <iostream>
#include <linux/uhid.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdint.h>

#include "response.hpp"
#include "uhid_report.hpp"

// --- REQUEST STRUCTURE ---
// Channel ID (4 Bytes)
// CMD (1 Byte)
// Payload length (2 Bytes)
// Nonce (randomly generated, N Bytes)
// Padding (0 everything until 64 bytes)

// --- RESPONSE STRUCTURE ---
// Same report id (1 Byte)
// Same broadcast channel (4 Bytes) 
// Response CMD (1 Byte)
// Payload length (2 Bytes)
// Payload (N Bytes)
// Padding (zero everything until size of 64 bytes)

// -- Payload example for INIT--
// Echoed Nonce (8 Bytes) 
// New Channel ID (4 bytes) 
// Protocol version identifier (1 Byte) (02)
// Major device version number (1 Byte) 
// Minor device version number (i Byte) 
// Build number (1 Byte)
// Capabilities (1 Byte)


#define MAKE_U16(high, low) (((uint16_t)high << 8) | ((uint16_t)(low)))
#define MAKE_U32(high, low) (((uint32_t)high << 16) | ((uint32_t)(low)))
#define MAKE_U64(high, low) (((uint64_t)high << 32) | ((uint64_t)(low)))


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
        ssize_t n = read(fd, &ev, sizeof(ev));
        if(n <= 0) {
            continue;
        }
        // If input from the browser 
        if (ev.type == UHID_OUTPUT) {
            UHIDReport report;
            printf("\x1b[1;31mGot data: \x1b[0m");
            uint8_t *data = ev.u.output.data;
            for(int i = 1; i < ev.u.output.size; i++) {
                printf("%02x", data[i]);
            }
            std::cout << "\n";
            // Channel ID (4 bytes)
            report.cid = ((uint32_t)data[1] << 24) |
                         ((uint32_t)data[2] << 16) |
                         ((uint32_t)data[3] << 8 ) |
                         ((uint32_t)data[4]);
            // Command (1 byte) 
            report.cmd = data[5];
            // Check if the frame is initialization frame
            report.is_init_frame = (report.cmd & 0x80);
            report.cmd = report.cmd & 0x7F;
            if(report.is_init_frame) {
                // Handle init frame 
            } else {
                // Handle continuation frame
            }
            // Length of the nonce (2 bytes)
            report.length = MAKE_U16(data[6], data[7]); 
            // Nonce itself (variable length, set by the length)
            bool respd = false;
            if(report.cmd == CTAPHID_INIT) {
                for(int i = 0; i < report.length; i++) {
                    report.payload.push_back(data[8+i]); 
                }             
                respd = true;
            } else if(report.cmd == CTAPHID_CBOR) {
                report.payload.push_back(data[8]);
                respd = true;
            } 

            if(respd) {
                // Respond based on the CMD
                struct uhid_event resp;
                memset(&resp, 0, sizeof(resp));
                resp.type = UHID_INPUT2; 
                CTAPFrame frame = respond(report);
                std::vector<uint8_t> response = frame.stringify();
                memcpy(resp.u.input2.data, response.data(), response.size());
                resp.u.input2.size = response.size(); 
                write(fd, &resp, sizeof(resp));
            }
        }
    }

    return 0;
}
