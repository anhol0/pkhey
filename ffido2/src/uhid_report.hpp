#ifndef UHID_REPORT_HPP
#define UHID_REPORT_HPP

#include <cstdint>
#include <vector>
class UHIDReport {
public:
    uint8_t report_id;  
    uint32_t cid; 
    uint8_t cmd;
    uint16_t length;
    std::vector<uint8_t> payload;
    bool is_init_frame;
};

#endif
