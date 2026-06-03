#ifndef KV_FTL_H_
#define KV_FTL_H_

#define KV_FTL_MAX_KEY_COUNT		(1U << 22)
#define KV_FTL_ENTRY_BYTES			8
#define KV_FTL_INDEX_TABLE_BYTES	(KV_FTL_MAX_KEY_COUNT * KV_FTL_ENTRY_BYTES)
#define KV_FTL_CMD_SLOT_COUNT		(1U << 10)

#define KV_FTL_SUCCESS				0
#define KV_FTL_NO_SUCH_KEY			1
#define KV_FTL_INVALID_KEY			2
#define KV_FTL_CAPACITY_FULL		3

void InitKvFtl(void);

unsigned int KvFtlPut(unsigned int key,
		unsigned int nlb,
		unsigned int valueLength,
		unsigned int *startLba);

unsigned int KvFtlGet(unsigned int key,
		unsigned int *startLba,
		unsigned int *nlb,
		unsigned int *valueLength);

void KvFtlSetPendingGetLength(unsigned int cmdSlotTag, unsigned int valueLength);
unsigned int KvFtlGetPendingGetLength(unsigned int cmdSlotTag);
void KvFtlClearPendingGetLength(unsigned int cmdSlotTag);

#endif /* KV_FTL_H_ */
