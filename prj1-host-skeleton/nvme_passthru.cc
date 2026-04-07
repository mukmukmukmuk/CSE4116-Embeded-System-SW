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

using namespace std;

const unsigned int PAGE_SIZE = 4096;
const unsigned int MAX_BUFLEN = 16*1024*1024; /* Maximum transfer size (can be adjusted if needed) */
const unsigned int NSID = 1; /* NSID can be checked using 'sudo nvme list' */

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

    return -1; // placeholder
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

    return -1; // placeholder
}

int Embedded::Proj1::Hello() {
    /* ------------------------------------------------------------------
     * TODO: Implement this function.
     * 
     * Return 0 on success, or a negative error code on failure.
     * ------------------------------------------------------------------ */

    return -1; // placeholder
}

int Embedded::Proj1::nvme_passthru(/* define parameters here */)
{
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
    struct nvme_passthru_cmd cmd;
    cmd.opcode = ;
    

    return -1; // placeholder
}

