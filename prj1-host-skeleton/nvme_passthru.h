#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Embedded {
    enum nvme_opcode {
        NVME_CMD_WRITE                  = 0x01,
        NVME_CMD_READ                   = 0x02,
        NVME_CMD_HELLO                   = 0x10,
        /* Additional opcodes may be defined here */ 
    };

    class Proj1 {
        public:
            Proj1() : fd_(-1) {}
            int Open(const std::string &dev);
            int ImageWrite(const std::vector<uint8_t> &buf);
            int ImageRead(std::vector<uint8_t> &buf, size_t size);
            int Hello();
        private:
            int fd_;
            int nvme_passthru(
                /* define parameters here */
                uint8_t opcode, 
                void *addr,
                uint32_t data_len,
                uint64_t lba,
                uint32_t blocks_cnt
            );
    };
}
