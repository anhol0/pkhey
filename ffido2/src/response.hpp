#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include "uhid_report.hpp"
#include <tinycbor/cbor.h>

#define MASK 0x80

enum {
    CTAPHID_MSG = 0x03,
    CTAPHID_CBOR = 0x10,
    CTAPHID_INIT = 0x06,
    CTAPHID_PING = 0x01,
    CTAPHID_WINK = 0x08,
    CTAPHID_LOCK = 0x04,
    CTAPHID_CANCEL = 0x11,
    CTAPHID_ERROR = 0x3f,
};

class CTAPFrame {
public:
    uint32_t cid;
    uint8_t  cmd;
    uint16_t len;
    std::vector<uint8_t> payload;
    std::vector<uint8_t> stringify();
};

inline uint32_t gen_cid() {
    uint32_t cid;
    do {
        arc4random_buf(&cid, sizeof(cid));    
    } while (cid == 0xffffffff);
    return cid;
}

inline std::vector<uint8_t> CTAPFrame::stringify() {
    std::vector<uint8_t> out_v;
    out_v.push_back(cid >> 24 & 0xFF);
    out_v.push_back(cid >> 16 & 0xFF);
    out_v.push_back(cid >>  8 & 0xFF);
    out_v.push_back(cid >>  0 & 0xFF);
    out_v.push_back(cmd);
    out_v.push_back(len >> 8 & 0xFF);
    out_v.push_back(len >> 0 & 0xFF);
    out_v.insert(out_v.end(), payload.begin(), payload.end()); 
    while(out_v.size() < 64) {
        out_v.push_back(0x00);
    }
    printf("\x1b[1;32mOut data: \x1b[0m");
    for(auto &a : out_v) {
         printf("%02x", a);
    }
    printf("\n");
    return out_v; 
}

#include <tinycbor/cbor.h>

inline std::vector<uint8_t> build_getinfo_response() {
    uint8_t buffer[256];

    CborEncoder encoder;
    CborEncoder map;
    CborEncoder versions;
    CborEncoder options;

    cbor_encoder_init(&encoder, buffer, sizeof(buffer), 0);

    // map with 3 entries
    cbor_encoder_create_map(&encoder, &map, 3);

    // 1: ["FIDO_2_0"]
    cbor_encode_uint(&map, 1);

    cbor_encoder_create_array(&map, &versions, 1);
    cbor_encode_text_stringz(&versions, "FIDO_2_0");
    cbor_encoder_close_container(&map, &versions);

    // 3: aaguid (16 zero bytes)
    cbor_encode_uint(&map, 3);

    uint8_t aaguid[16] = {0};
    cbor_encode_byte_string(&map, aaguid, 16);

    // 4: options
    cbor_encode_uint(&map, 4);

    cbor_encoder_create_map(&map, &options, 2);

    cbor_encode_text_stringz(&options, "rk");
    cbor_encode_boolean(&options, true);

    cbor_encode_text_stringz(&options, "up");
    cbor_encode_boolean(&options, true);

    // cbor_encode_text_stringz(&options, "uv");
    // cbor_encode_boolean(&options, true);

    cbor_encoder_close_container(&map, &options);

    cbor_encoder_close_container(&encoder, &map);

    size_t len = cbor_encoder_get_buffer_size(&encoder, buffer);

    std::vector<uint8_t> out;

    // CTAP success byte
    out.push_back(0x00);

    out.insert(out.end(), buffer, buffer + len);

    return out;
}

inline CTAPFrame respond(UHIDReport r) {
    CTAPFrame frame;
    switch(r.cmd) {
        case CTAPHID_INIT: {
            // --- PAYLOAD STRUCTURE --- 
            // Echoed Nonce (8 Bytes) 
            // New Channel ID (4 bytes) 
            // Protocol version identifier (1 Byte) (02)
            // Major device version number (1 Byte) 
            // Minor device version number (i Byte) 
            // Build number (1 Byte)
            // Capabilities (1 Byte)
            uint32_t new_cid = gen_cid();
            std::vector<uint8_t> payload;
            if(r.payload.size() < 8) {
                std::perror("Broken INIT packet arrived");
                break;
            }
            payload.insert(payload.end(), r.payload.begin(), r.payload.begin() + 8); 
            payload.push_back((new_cid >> 24) & 0xFF);
            payload.push_back((new_cid >> 16) & 0xFF);
            payload.push_back((new_cid >>  8) & 0xFF);
            payload.push_back((new_cid >>  0) & 0xFF);
            payload.push_back(0x02);
            payload.push_back(0x01);
            payload.push_back(0x00);
            payload.push_back(0x00);
            payload.push_back(0x05);
            frame.cid = 0xffffffff;
            frame.cmd = CTAPHID_INIT | MASK;
            frame.len = (uint16_t)payload.size();
            frame.payload = payload;
            break;
        }
        case CTAPHID_CBOR: {
            std::vector<uint8_t> payload;
            // Payload generation 
            if(r.payload[0] == 0x04) {
                // CBOR 
                auto cbor = build_getinfo_response();
                // Encoding JSON in CBOR
                payload.insert(payload.end(), cbor.begin(), cbor.end());
            }
            frame.cid = r.cid;
            frame.cmd = CTAPHID_CBOR | MASK;
            frame.len = (uint16_t)payload.size();
            frame.payload = payload;
            break;
        }
        case CTAPHID_MSG: 
        case CTAPHID_CANCEL:
        case CTAPHID_PING:
        case CTAPHID_WINK:
        case CTAPHID_LOCK:
        case CTAPHID_ERROR:
            break;
    }
    return frame;
}

#endif
