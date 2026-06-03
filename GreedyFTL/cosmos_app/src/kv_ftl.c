#include <assert.h>

#include "kv_ftl.h"
#include "ftl_config.h"
#include "memory_map.h"

typedef struct _KV_FTL_ENTRY
{
	unsigned int startLba;
	unsigned int valueLength;
} KV_FTL_ENTRY, *P_KV_FTL_ENTRY;

#define KV_FTL_INVALID_LBA		0xFFFFFFFFU

static P_KV_FTL_ENTRY kvFtlIndex = (P_KV_FTL_ENTRY)KV_FTL_INDEX_TABLE_ADDR;
static unsigned int kvFtlInitialized = 0;
static unsigned int kvFtlNextLba = 0;
static unsigned int kvFtlPendingGetLength[KV_FTL_CMD_SLOT_COUNT];

void InitKvFtl(void)
{
	unsigned int key;

	if(kvFtlInitialized)
		return;

	assert(sizeof(KV_FTL_ENTRY) == KV_FTL_ENTRY_BYTES);
	assert(KV_FTL_INDEX_TABLE_END_ADDR < RESERVED1_START_ADDR);
	assert(KV_FTL_INDEX_TABLE_END_ADDR <= DRAM_END_ADDR);

	for(key = 0; key < KV_FTL_MAX_KEY_COUNT; key++)
	{
		kvFtlIndex[key].startLba = KV_FTL_INVALID_LBA;
		kvFtlIndex[key].valueLength = 0;
	}

	for(key = 0; key < KV_FTL_CMD_SLOT_COUNT; key++)
		kvFtlPendingGetLength[key] = 0;

	kvFtlNextLba = 0;
	kvFtlInitialized = 1;
}

unsigned int KvFtlPut(unsigned int key,
		unsigned int nlb,
		unsigned int valueLength,
		unsigned int *startLba)
{
	unsigned int allocatedLba;
	unsigned int blockCount;

	InitKvFtl();

	if(key >= KV_FTL_MAX_KEY_COUNT)
		return KV_FTL_INVALID_KEY;

	blockCount = nlb + 1;
	if((kvFtlNextLba + blockCount) > (storageCapacity_L / USER_CHANNELS))
		return KV_FTL_CAPACITY_FULL;

	allocatedLba = kvFtlNextLba;
	kvFtlNextLba += blockCount;

	kvFtlIndex[key].startLba = allocatedLba;
	kvFtlIndex[key].valueLength = valueLength;
	if(kvFtlIndex[key].valueLength == 0)
		kvFtlIndex[key].valueLength = blockCount * BYTES_PER_NVME_BLOCK;

	*startLba = allocatedLba;

	return KV_FTL_SUCCESS;
}

void KvFtlSetPendingGetLength(unsigned int cmdSlotTag, unsigned int valueLength)
{
	if(cmdSlotTag >= KV_FTL_CMD_SLOT_COUNT)
		return;

	kvFtlPendingGetLength[cmdSlotTag] = valueLength;
}

unsigned int KvFtlGetPendingGetLength(unsigned int cmdSlotTag)
{
	if(cmdSlotTag >= KV_FTL_CMD_SLOT_COUNT)
		return 0;

	return kvFtlPendingGetLength[cmdSlotTag];
}

void KvFtlClearPendingGetLength(unsigned int cmdSlotTag)
{
	if(cmdSlotTag >= KV_FTL_CMD_SLOT_COUNT)
		return;

	kvFtlPendingGetLength[cmdSlotTag] = 0;
}

unsigned int KvFtlGet(unsigned int key,
		unsigned int *startLba,
		unsigned int *nlb,
		unsigned int *valueLength)
{
	unsigned int length;

	InitKvFtl();

	if(key >= KV_FTL_MAX_KEY_COUNT)
		return KV_FTL_NO_SUCH_KEY;

	if(kvFtlIndex[key].startLba == KV_FTL_INVALID_LBA)
		return KV_FTL_NO_SUCH_KEY;

	length = kvFtlIndex[key].valueLength;

	*startLba = kvFtlIndex[key].startLba;
	*valueLength = length;
	*nlb = (length - 1) / BYTES_PER_NVME_BLOCK;

	return KV_FTL_SUCCESS;
}
