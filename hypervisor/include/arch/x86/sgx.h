#ifndef SGX_H
#define SGX_H

#define SGX_LEAF_FUNCTION_ECREATE (0U)
#define SGX_ENCLS_EXITING_ECREATE (1UL << SGX_LEAF_FUNCTION_ECREATE)

/**
 * struct sgx_pageinfo - an enclave page descriptor
 * @linaddr:	linear address of the enclave
 * @srcpge:	effective address of the page where contents are located
 * @metadata:	effective address of the SECINFO or PCMD
 * @secs:	effective address of EPC slot that currently contains the SECS
 */
struct sgx_pageinfo {
	uint64_t linaddr;
	uint64_t srcpge;
	uint64_t metadata;
	uint64_t secs;
} __packed __aligned(32);

extern bool is_sgx_supported;

int32_t sgx_init(void);
int32_t sgx_encls_vmexit_handler(struct acrn_vcpu *vcpu);
bool is_vm_sgx_supported(uint16_t vm_id);

#endif