#include "nvme_passthru.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/nvme_ioctl.h>
#include <fcntl.h>
#include <cstring>
#include <cstdint>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <cstdio>
#include <inttypes.h>
#include <algorithm>
#include <cmath>

using namespace std;

const unsigned int PAGE_SIZE = 4096;
const unsigned int MAX_BUFLEN = 16*1024*1024; /* Maximum transfer size (can be adjusted if needed) */
const unsigned int NSID = 1; /* NSID can be checked using 'sudo nvme list' */
const unsigned int DMA_CHUNK_SIZE=1024*1024;
int Embedded::Proj1::Open(const std::string &dev) {
    int err;
    err = open(dev.c_str(), O_RDONLY);
    if (err < 0)
        return -1;
    fd_ = err;

    struct stat nvme_stat;
    err = fstat(fd_, &nvme_stat);
    if (err < 0)
        return -1;
    if (!S_ISCHR(nvme_stat.st_mode) && !S_ISBLK(nvme_stat.st_mode))
        return -1;

    return 0;
}

int Embedded::Proj1::ImageWrite(const std::vector<uint8_t> &buf) {
    if (buf.empty()) return -EINVAL;
    if (buf.size() > MAX_BUFLEN) {
        cerr << "[ERROR] Image size exceeds the maximum transfer size limit." << endl;
        return -EINVAL;
    }

    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * 
     * Return 0 on success, or a negative error code on failure.
     * ------------------------------------------------------------------ */
    // lsa 옮겨주기
    // block num은 0-base
    uint64_t lba = 0;
    size_t offset = 0;
    const size_t buf_size=buf.size();
    const uint8_t *buf_data = buf.data();
    while (offset < buf_size) {
        // 자른거
        size_t chunk_len = min((size_t)(DMA_CHUNK_SIZE), buf_size - offset);
        // block_cnt를 올림 해줘서 나머지 부분들 살려줌
        uint32_t blocks_cnt = (uint32_t)((chunk_len + (PAGE_SIZE - 1)) / PAGE_SIZE);
        
        // 올림한 거 해서 토탈 보낼 수 있는거
        size_t xfer_len = (size_t)blocks_cnt * PAGE_SIZE;

        //cout<< "DMA CHUNK SIZE(write) : " <<chunk_len<<endl;
        void *addr = (void *)(buf_data + offset);
        //만약에 떨거지들이 남았을때, 이때도 결국 chunck size만큼으로 보내야하니까,
        //남는 공간을 0으로 채워서 보냄
        vector<uint8_t> padded;
        if (xfer_len > chunk_len) {
            padded.resize(xfer_len, 0);
            memcpy(padded.data(), buf_data + offset, chunk_len);
            addr = padded.data();
        }


        int res = nvme_passthru(
            NVME_CMD_WRITE, 
            addr, 
            xfer_len, 
            lba, 
            blocks_cnt
        );
        if (res < 0) return res;
        
        offset += chunk_len;
        lba += blocks_cnt;
    }

    return 0;
}

int Embedded::Proj1::ImageRead(std::vector<uint8_t> &buf, size_t size) {
    if (size == 0) return -EINVAL;
    if (size > MAX_BUFLEN) {
        cerr << "[ERROR] Requested read size exceeds the maximum transfer size limit." << endl;
        return -EINVAL;
    }

    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * 
     * Return 0 on success, or a negative error code on failure.
     * ------------------------------------------------------------------ */
    buf.resize(size);

    uint64_t lba = 0;
    size_t offset = 0;
    const size_t buf_size = buf.size();
    uint8_t *buf_data = buf.data();

    while (offset < buf_size) {
        size_t chunk_len = min((size_t)DMA_CHUNK_SIZE, buf_size - offset);
        uint32_t blocks_cnt = (uint32_t)((chunk_len + PAGE_SIZE - 1) / PAGE_SIZE);
        size_t xfer_len = (size_t)blocks_cnt * PAGE_SIZE;

        void *addr = buf_data + offset;
        //cout<< "DMA CHUNK SIZE(read) : " <<chunk_len<<endl;
        vector<uint8_t> padded;
        if (xfer_len > chunk_len) {
            padded.resize(xfer_len, 0);
            addr = padded.data();
        }

        int res = nvme_passthru(
            NVME_CMD_READ,
            addr,
            xfer_len,
            lba,
            blocks_cnt
        );
        if (res < 0) return res;

        if (xfer_len > chunk_len) {
            memcpy(buf_data + offset, padded.data(), chunk_len);
        }

        offset += chunk_len;
        lba += blocks_cnt;
    }

    return 0;
}

int Embedded::Proj1::Hello() {
    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * 
     * Return 0 on success, or a negative error code on failure.
     * ------------------------------------------------------------------ */
    int res = nvme_passthru(
                NVME_CMD_HELLO,
                0,
                0,
                0,
                1
                );
    return res;
}

int Embedded::Proj1::nvme_passthru(
    /* define parameters here */
    uint8_t opcode, 
    void *addr,
    uint32_t data_len,
    uint64_t start_lba,
    uint32_t blocks_cnt
){
    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * This function should serve as the low-level interface for issuing
     * passthru NVMe commands. Make sure to include appropriate arguments
     * (e.g., opcode, namespace ID, command dwords, buffer pointer, length,
     * and result field) so that higher-level methods (ImageWrite, ImageRead,
     * Hello) can be implemented using this helper.
     *
     * Hint: refer to the Linux nvme_ioctl.h header and the struct nvme_passthru_cmd definition.
     * - Link: https://elixir.bootlin.com/linux/v5.15/source/include/uapi/linux/nvme_ioctl.h
     * ------------------------------------------------------------------ */
    
    /*
    struct nvme_passthru_cmd {
	__u8	opcode; check 0x01이 read 0x02가 write
	__u8	flags;
	__u16	rsvd1;
	__u32	nsid; check
	__u32	cdw2;
	__u32	cdw3;
	__u64	metadata;
	__u64	addr; check
	__u32	metadata_len;
	__u32	data_len; check
	__u32	cdw10; check (시작 LBA)
	__u32	cdw11; check (시작 LBA)
	__u32	cdw12; check (NLB)
	__u32	cdw13;
	__u32	cdw14;
	__u32	cdw15;
	__u32	timeout_ms;
	__u32	result;
    };
    */
    struct nvme_passthru_cmd cmd{};
    cmd.opcode=opcode;
    cmd.nsid=NSID;
    cmd.addr=(uint64_t)(char*)addr;
    cmd.data_len=data_len;
    cmd.cdw10=(uint32_t)(start_lba&0xFFFFFFFF);
	cmd.cdw11=(uint32_t)(start_lba>>32);
	cmd.cdw12=(uint64_t)((blocks_cnt-1)&0xFFFF);
    int res= ioctl(fd_,NVME_IOCTL_IO_CMD,&cmd);
    return res;
}

