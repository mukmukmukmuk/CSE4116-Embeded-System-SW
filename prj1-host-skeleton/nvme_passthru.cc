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

    return nvme_passthru(NVME_CMD_WRITE,const_cast<uint8_t*>(buf.data()),buf.size(),0,buf.size()/PAGE_SIZE);
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
    return nvme_passthru(NVME_CMD_READ,buf.data(),size,0, size / PAGE_SIZE);
}

int Embedded::Proj1::Hello() {
    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * 
     * Return 0 on success, or a negative error code on failure.
     * ------------------------------------------------------------------ */

    return -1; // placeholder
}

int Embedded::Proj1::nvme_passthru(
    /* define parameters here */
    uint8_t opcode, 
    void *addr,
    uint32_t data_len,
    uint64_t slba,
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
    struct nvme_passthru_cmd cmd={};
    cmd.opcode=opcode;
    cmd.nsid=NSID;
    cmd.addr=(uint64_t)(char*)addr;
    cmd.data_len=data_len;
    cmd.cdw10=(uint32_t)(slba&0xFFFFFFFF);
	cmd.cdw11=(uint32_t)(slba>>32);
	cmd.cdw12=(uint64_t)(blocks_cnt&0x0000FFFF);
    int ret= ioctl(fd_,NVME_IOCTL_IO_CMD,&cmd);
    return ret;
}

