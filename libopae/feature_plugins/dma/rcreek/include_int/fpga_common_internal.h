// Copyright(c) 2018, Intel Corporation
//
// Redistribution  and  use  in source  and  binary  forms,  with  or  without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of  source code  must retain the  above copyright notice,
//   this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
// * Neither the name  of Intel Corporation  nor the names of its contributors
//   may be used to  endorse or promote  products derived  from this  software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,  BUT NOT LIMITED TO,  THE
// IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT  SHALL THE COPYRIGHT OWNER  OR CONTRIBUTORS BE
// LIABLE  FOR  ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR
// CONSEQUENTIAL  DAMAGES  (INCLUDING,  BUT  NOT LIMITED  TO,  PROCUREMENT  OF
// SUBSTITUTE GOODS OR SERVICES;  LOSS OF USE,  DATA, OR PROFITS;  OR BUSINESS
// INTERRUPTION)  HOWEVER CAUSED  AND ON ANY THEORY  OF LIABILITY,  WHETHER IN
// CONTRACT,  STRICT LIABILITY,  OR TORT  (INCLUDING NEGLIGENCE  OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,  EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

/**
 * \fpga_dma_internal.h
 * \brief FPGA DMA BBB Internal Header
 */

#ifndef __FPGA_COMMON_INT_H__
#define __FPGA_COMMON_INT_H__

#include <opae/fpga.h>
#include "fpga_dma_types.h"
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "x86-sse2.h"

#ifdef CHECK_DELAYS
#pragma message "Compiled with -DCHECK_DELAYS.  Not to be used in production"
#endif

#define FPGA_DMA_ST_ERR(msg_str)                                               \
	fprintf(stderr, "Error %s: %s\n", __FUNCTION__, msg_str);

#define FPGA_DMA_ST_WARN(msg_str)                                              \
	fprintf(stderr, "Warning %s: %s\n", __FUNCTION__, msg_str);

#ifdef FPGA_DMA_DEBUG
#pragma message "Compiled with -DFPGA_DMA_DEBUG.  Not to be used in production"
#endif

#ifndef max
#define max(a, b)                                                              \
	({                                                                     \
		__typeof__(a) _a = (a);                                        \
		__typeof__(b) _b = (b);                                        \
		_a > _b ? _a : _b;                                             \
	})
#endif

#ifndef min
#define min(a, b)                                                              \
	({                                                                     \
		__typeof__(a) _a = (a);                                        \
		__typeof__(b) _b = (b);                                        \
		_a < _b ? _a : _b;                                             \
	})
#endif

#define DMA_SHUTDOWN_CTL_VAL (0x21)

#define FPGA_DMA_TIMEOUT_MSEC (120000)

#define QWORD_BYTES 8
#define DWORD_BYTES 4
#define IS_ALIGNED_DWORD(addr) (addr % 4 == 0)
#define IS_ALIGNED_QWORD(addr) (addr % 8 == 0)

#define M2S_DMA_UUID_H 0xfee69b442f7743ed
#define M2S_DMA_UUID_L 0x9ff49b8cf9ee6335
#define S2M_DMA_UUID_H 0xf118209ad59a4b3f
#define S2M_DMA_UUID_L 0xa66cd700a658a015

#define FPGA_DMA_UUID_H 0xef82def7f6ec40fc
#define FPGA_DMA_UUID_L 0xa9149a35bace01ea
#define FPGA_DMA_WF_MAGIC_NO 0x5772745F53796E63ULL
#define FPGA_DMA_HOST_MASK 0x2000000000000
#define FPGA_DMA_WF_HOST_MASK 0x3000000000000
#define FPGA_DMA_WF_ROM_MAGIC_NO_MASK 0x1000000000000

#define AFU_DFH_REG 0x0
#define AFU_DFH_NEXT_OFFSET 16
#define AFU_DFH_EOL_OFFSET 40
#define AFU_DFH_TYPE_OFFSET 60

// BBB Feature ID (refer CCI-P spec)
#define FPGA_DMA_BBB 0x2

// Feature ID for DMA BBB
#define FPGA_DMA_BBB_FEATURE_ID 0x765

// DMA Register offsets from base
#define FPGA_DMA_CSR 0x40
#define FPGA_DMA_DESC 0x60
#define FPGA_DMA_RESPONSE 0x80
#define FPGA_DMA_STREAMING_VALVE 0xA0

#define FPGA_DMA_ADDR_SPAN_EXT_CNTL 0x200
#define FPGA_DMA_ADDR_SPAN_EXT_DATA 0x1000

#define DMA_ADDR_SPAN_EXT_WINDOW (4 * 1024)
#define DMA_ADDR_SPAN_EXT_WINDOW_MASK ((uint64_t)(DMA_ADDR_SPAN_EXT_WINDOW - 1))

#define FPGA_DMA_MASK_32_BIT 0xFFFFFFFF

#define FPGA_DMA_CSR_BUSY (1 << 0)
#define FPGA_DMA_DESC_BUFFER_EMPTY 0x2
#define FPGA_DMA_DESC_BUFFER_FULL 0x4

#define FPGA_DMA_ALIGN_BYTES 64
#define IS_DMA_ALIGNED(addr) (addr % FPGA_DMA_ALIGN_BYTES == 0)

// MIN_SSE2_SIZE is the minimum size in bytes of a memcpy where SSE2 copy
// benefits over movsb
#define MIN_SSE2_SIZE 4096
#define CACHE_LINE_SIZE 64
#define ALIGN_TO_CL(x) ((uint64_t)(x) & ~(CACHE_LINE_SIZE - 1))
#define IS_CL_ALIGNED(x) (((uint64_t)(x) & (CACHE_LINE_SIZE - 1)) == 0)

#define CSR_BASE(dma_handle)                                                   \
	((uint64_t)(((handle_common *)dma_handle)->chan_desc->dma_csr_base))
#define RSP_BASE(dma_handle) ((uint64_t)dma_handle->dma_rsp_base)
#define ST_VALVE_BASE(dma_handle)                                              \
	((uint64_t)dma_handle->dma_streaming_valve_base)
#define ASE_DATA_BASE(dma_handle) ((uint64_t)dma_handle->dma_ase_data_base)
#define ASE_CNTL_BASE(dma_handle) ((uint64_t)dma_handle->dma_ase_cntl_base)
#define HOST_MMIO_32_ADDR(dma_handle, offset)                                  \
	((volatile uint32_t *)((uint64_t)(((handle_common *)(dma_handle))      \
						  ->chan_desc->mmio_va)        \
			       + (uint64_t)(offset)))
#define HOST_MMIO_64_ADDR(dma_handle, offset)                                  \
	((volatile uint64_t *)((uint64_t)(((handle_common *)(dma_handle))      \
						  ->chan_desc->mmio_va)        \
			       + (uint64_t)(offset)))
#define HOST_MMIO_32(dma_handle, offset)                                       \
	(*HOST_MMIO_32_ADDR(dma_handle, offset))
#define HOST_MMIO_64(dma_handle, offset)                                       \
	(*HOST_MMIO_64_ADDR(dma_handle, offset))

#define CSR_STATUS(dma_h) (CSR_BASE(dma_h) + offsetof(msgdma_csr_t, status))
#define CSR_CONTROL(dma_h) (CSR_BASE(dma_h) + offsetof(msgdma_csr_t, ctrl))
#define CSR_FILL_LEVEL(dma_h)                                                  \
	(CSR_BASE(dma_h) + offsetof(msgdma_csr_t, fill_level))
#define CSR_RSP_FILL_LEVEL(dma_h)                                              \
	(CSR_BASE(dma_h) + offsetof(msgdma_csr_t, rsp_level))
#define RSP_BYTES_TRANSFERRED(dma_h)                                           \
	(RSP_BASE(dma_h) + offsetof(msgdma_rsp_t, actual_bytes_tf))
#define RSP_STATUS(dma_h) (RSP_BASE(dma_h) + offsetof(msgdma_rsp_t, rsp_status))
#define ST_VALVE_CONTROL(dma_h)                                                \
	(ST_VALVE_BASE(dma_h) + offsetof(msgdma_st_valve_t, control))
#define ST_VALVE_STATUS(dma_h)                                                 \
	(ST_VALVE_BASE(dma_h) + offsetof(msgdma_st_valve_t, status))
#define FPGA_DMA_MASK_32_BIT 0xFFFFFFFF

#define FPGA_DMA_CSR_BUSY (1 << 0)
#define FPGA_DMA_DESC_BUFFER_EMPTY 0x2
#define FPGA_DMA_DESC_BUFFER_FULL 0x4

#define FPGA_DMA_ALIGN_BYTES 64
#define IS_DMA_ALIGNED(addr) (addr % FPGA_DMA_ALIGN_BYTES == 0)
// Granularity of DMA transfer (maximum bytes that can be packed
// in a single descriptor).This value must match configuration of
// the DMA IP. Larger transfers will be broken down into smaller
// transactions.
#define FPGA_DMA_BUF_SIZE (uint32_t)(2 * 1024 * 1024)
#define FPGA_DMA_BUF_ALIGN_SIZE FPGA_DMA_BUF_SIZE

// Convenience macros
#ifdef FPGA_DMA_DEBUG
#define debug_print(fmt, ...)                                                  \
	do {                                                                   \
		if (FPGA_DMA_DEBUG) {                                          \
			fprintf(stderr, "%s (%d) : ", __FUNCTION__, __LINE__); \
			fprintf(stderr, fmt, ##__VA_ARGS__);                   \
		}                                                              \
	} while (0)
#define error_print(fmt, ...)                                                  \
	do {                                                                   \
		fprintf(stderr, "%s (%d) : ", __FUNCTION__, __LINE__);         \
		fprintf(stderr, fmt, ##__VA_ARGS__);                           \
		err_cnt++;                                                     \
	} while (0)
#else
#define debug_print(...)
#define error_print(...)
#endif

#define FPGA_DMA_MAX_BUF 8

// Max. async transfers in progress
#define FPGA_DMA_MAX_INFLIGHT_TRANSACTIONS 100000
#define INVALID_CHANNEL (0x7fffffffffffffffULL)

typedef enum _pool_type {
	POOL_INVALID = 0,
	POOL_SEMA,
	POOL_MUTEX,
	POOL_BUFFERS,
} pool_type;

typedef struct _pool_hdr {
	pool_type type;
	uint32_t destroyed;
} pool_header;

// Semaphore / mutex pool
typedef struct _sem_pool {
	struct _sem_pool *next;
	pool_header header;
	sem_t m_semaphore;
} sem_pool_item;

typedef struct _mutex_pool {
	struct _mutex_pool *next;
	pool_header header;
	pthread_mutex_t m_mutex;
} mutex_pool_item;

typedef struct _buffer_pool {
	struct _buffer_pool *next;
	pool_header header;
	uint64_t size;
	uint64_t *dma_buf_ptr;
	uint64_t dma_buf_wsid;
	uint64_t dma_buf_iova;
} buffer_pool_item;


typedef struct _fpga_dma_transfer {
	sem_pool_item *tf_semaphore;
	mutex_pool_item *tf_mutex;
	fpga_dma_channel_type_t ch_type;
	uint64_t src;
	uint64_t dst;
	uint64_t len;
	fpga_dma_transfer_type_t transfer_type;
	fpga_dma_tx_ctrl_t tx_ctrl;
	fpga_dma_rx_ctrl_t rx_ctrl;
	fpga_dma_transfer_cb cb;
	bool eop_status;
	void *context;
	size_t rx_bytes;
	uint32_t num_buffers;
	buffer_pool_item **buffers;
	buffer_pool_item *small_buffer;
} fpga_dma_transfer_t;

typedef struct qinfo {
	int read_index;
	int write_index;
	int num_free;
	fpga_dma_transfer_t *queue[FPGA_DMA_MAX_INFLIGHT_TRANSACTIONS];
	fpga_dma_transfer_t *free_queue[FPGA_DMA_MAX_INFLIGHT_TRANSACTIONS];
	// Counting semaphore, count represents available entries
	// in queue. mutex to gain exclusive access before queue operations
	sem_pool_item *q_semaphore;
	pthread_spinlock_t q_mutex;
} qinfo_t;

#define FPGA_DMA_MAGIC_ID (0x9de448f7)
#define FPGA_DMA_TX_CHANNEL_MAGIC_ID (0x48f749de)
#define FPGA_DMA_RX_CHANNEL_MAGIC_ID (0x44e9d8f7)
#define FPGA_MSGDMA_MAGIC_ID (0xe8f7449d)
#define IS_MSGDMA_HANDLE(dma_h)                                                \
	((dma_h)->header.magic_id == FPGA_MSGDMA_MAGIC_ID)
#define IS_DMA_HANDLE(dma_h)                                                   \
	((dma_h)->main_header.magic_id == FPGA_DMA_MAGIC_ID)
#define IS_TX_CHANNEL_HANDLE(dma_h)                                            \
	((dma_h)->header.magic_id == FPGA_DMA_TX_CHANNEL_MAGIC_ID)
#define IS_RX_CHANNEL_HANDLE(dma_h)                                            \
	((dma_h)->header.magic_id == FPGA_DMA_RX_CHANNEL_MAGIC_ID)
#define IS_CHANNEL_HANDLE(dma_h)                                               \
	(IS_RX_CHANNEL_HANDLE(dma_h) || IS_TX_CHANNEL_HANDLE(dma_h)            \
	 || IS_MSGDMA_HANDLE(dma_h))

typedef struct _internal_channel_desc {
	fpga_dma_channel_desc desc;
	uint32_t mmio_num;
	uint64_t mmio_offset;
	uint64_t mmio_va;
	uint64_t dma_base;
	uint64_t dma_csr_base;
	uint64_t dma_desc_base;
} internal_channel_desc;

// *MUST* be first element of each handle type
typedef struct _handle_common {
	uint32_t magic_id;
	struct _fpga_dma_handle *dma_h;
	fpga_handle fpga_h;
	internal_channel_desc *chan_desc;
	uint64_t dma_channel;
	fpga_dma_channel_type_t ch_type;

	// Interrupt event handle
	fpga_event_handle eh;
	// Transaction queue (model as a fixed-size circular buffer)
	qinfo_t transferRequestq;
} handle_common;

typedef struct _fpga_dma_handle {
	handle_common main_header;

	uint32_t num_open_channels;
	void **open_channels;

	// For protection for sem/mutex manipulation
	// TODO: These can be globals, right?
	pthread_spinlock_t sem_mutex;
	pthread_spinlock_t mutex_mutex;
	pthread_spinlock_t buffer_mutex;

	sem_pool_item *sem_in_use_head;
	sem_pool_item *sem_free_head;
	mutex_pool_item *mutex_in_use_head;
	mutex_pool_item *mutex_free_head;
	buffer_pool_item *buffer_in_use_head;
	buffer_pool_item *buffer_free_head;

	// Descriptors for channels (array)
	internal_channel_desc *chan_descs;
	uint32_t num_avail_channels;

	pthread_t completion_thread_id;
	sem_t completion_thread_sem;

	pthread_t m2s_thread_id;
	sem_t m2s_thread_sem;
	pthread_t s2m_thread_id;
	sem_t s2m_thread_sem;
	pthread_t m2m_thread_id;
	sem_t m2m_thread_sem;
	// Transaction completion queue (model as a fixed-size circular buffer)
	qinfo_t transferCompleteq;

	struct sigaction old_action;
	volatile uint32_t *CsrControl;

#define FPGA_DMA_MAX_SMALL_BUFFERS 4
	uint32_t num_smalls;
} fpga_dma_handle_t;

typedef struct _m2s_dma_handle {
	handle_common header;
} m2s_dma_handle_t;

typedef struct _s2m_dma_handle {
	handle_common header;

	uint64_t dma_rsp_base;
	uint64_t dma_streaming_valve_base;

	// Index of the next available descriptor in the dispatcher queue
	uint64_t next_avail_desc_idx;
	// Total number of unused descriptors in the dispatcher queue
	// Leftover descriptors are reused for subsequent transfers
	// Note: Count includes the next available descriptor in
	// the dispatcher queue indexed by next_avail_desc_idx
	uint64_t unused_desc_count;
} s2m_dma_handle_t;

typedef struct _m2m_dma_handle {
	handle_common header;

	uint64_t cur_ase_page;

	uint64_t dma_ase_cntl_base;
	uint64_t dma_ase_data_base;

	// magic number buffer
	volatile uint64_t *magic_buf;
	uint64_t magic_iova;
	uint64_t magic_wsid;
} m2m_dma_handle_t;

typedef union {
	uint32_t reg;
	struct {
		uint32_t tx_channel : 8;
		uint32_t generate_sop : 1;
		uint32_t generate_eop : 1;
		uint32_t park_reads : 1;
		uint32_t park_writes : 1;
		uint32_t end_on_eop : 1;
		uint32_t eop_rvcd_irq_en : 1;
		uint32_t transfer_irq_en : 1;
		uint32_t early_term_irq_en : 1;
		uint32_t trans_error_irq_en : 8;
		uint32_t early_done_en : 1;
		uint32_t wait_for_wr_rsp : 1;
		uint32_t reserved_2 : 5;
		uint32_t go : 1;
	};
} msgdma_desc_ctrl_t;

#pragma pack(push, 1)
typedef struct {
	// 0x0
	uint32_t rd_address;
	// 0x4
	uint32_t wr_address;
	// 0x8
	uint32_t len;
	// 0xC
	uint16_t seq_num;
	uint8_t rd_burst_count;
	uint8_t wr_burst_count;
	// 0x10
	uint16_t rd_stride;
	uint16_t wr_stride;
	// 0x14
	uint32_t rd_address_ext;
	// 0x18
	uint32_t wr_address_ext;
	// 0x1c
	msgdma_desc_ctrl_t control;
} msgdma_ext_desc_t;
#pragma pack(pop)

typedef union {
	uint32_t reg;
	struct {
		uint32_t busy : 1;
		uint32_t desc_buf_empty : 1;
		uint32_t desc_buf_full : 1;
		uint32_t rsp_buf_empty : 1;
		uint32_t rsp_buf_full : 1;
		uint32_t stopped : 1;
		uint32_t resetting : 1;
		uint32_t stopped_on_errror : 1;
		uint32_t stopped_on_early_term : 1;
		uint32_t irq : 1;
		uint32_t reserved : 22;
	} st;
} msgdma_status_t;

typedef union {
	uint32_t reg;
	struct {
		uint32_t stop_dispatcher : 1;
		uint32_t reset_dispatcher : 1;
		uint32_t stop_on_error : 1;
		uint32_t stopped_on_early_term : 1;
		uint32_t global_intr_en_mask : 1;
		uint32_t stop_descriptors : 1;
		uint32_t flush_descriptors : 1;
		uint32_t flush_rd_master : 1;
		uint32_t flush_wr_master : 1;
		uint32_t rsvd : 19;
	} ct;
} msgdma_ctrl_t;

typedef union {
	uint32_t reg;
	struct {
		uint32_t rd_fill_level : 16;
		uint32_t wr_fill_level : 16;
	} fl;
} msgdma_fill_level_t;

typedef union {
	uint32_t reg;
	struct {
		uint32_t rsp_fill_level : 16;
		uint32_t rsvd : 16;
	} rsp;
} msgdma_rsp_level_t;

typedef union {
	uint32_t reg;
	struct {
		uint32_t rd_seq_num : 16;
		uint32_t wr_seq_num : 16;
	} seq;
} msgdma_seq_num_t;

#pragma pack(push, 1)
typedef struct {
	// 0x0
	msgdma_status_t status;
	// 0x4
	msgdma_ctrl_t ctrl;
	// 0x8
	msgdma_fill_level_t fill_level;
	// 0xc
	msgdma_rsp_level_t rsp_level;
	// 0x10
	msgdma_seq_num_t seq_num;
} msgdma_csr_t;
#pragma pack(pop)

typedef union {
	uint32_t reg;
	struct {
		uint32_t error : 8;
		uint32_t early_termination : 1;
		uint32_t eop_arrived : 1;
		uint32_t err_irq_mask : 8;
		uint32_t early_termination_irq_mask : 1;
		uint32_t desc_buffer_full : 1;
		uint32_t rsvd : 12;
	} rsp;
} msgdma_rsp_status_t;

#pragma pack(push, 1)
typedef struct {
	// 0x0
	uint32_t actual_bytes_tf;
	// 0x4
	msgdma_rsp_status_t rsp_status;
} msgdma_rsp_t;
#pragma pack(pop)

typedef union {
	uint32_t reg;
	struct {
		uint32_t en_data_flow : 1;
		uint32_t en_det_tf : 1;
		uint32_t en_non_det_tf : 1;
		uint32_t clr_bytes_transferred : 1;
		uint32_t rsvd : 28;
	} ct;
} msgdma_st_valve_ctrl_t;

typedef union {
	uint32_t reg;
	struct {
		uint32_t det_tf_occurred : 1;
		uint32_t non_det_tf_occurred : 1;
		uint32_t rsvd : 30;
	} st;
} msgdma_st_valve_status_t;

#pragma pack(push, 1)
typedef struct {
	// 0x0
	uint32_t bytes_transferred_l32;
	// 0x4
	uint32_t bytes_transferred_u32;
	// 0x8
	uint32_t bytes_to_transfer;
	// 0xc
	msgdma_st_valve_ctrl_t control;
	// 0x10
	msgdma_st_valve_status_t status;
} msgdma_st_valve_t;
#pragma pack(pop)

#endif // __FPGA_COMMON_INT_H__
