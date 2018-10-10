// Copyright(c) 2017, Intel Corporation
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
 * \file types_int.h
 * \brief Internal type definitions for FPGA API
 */

#ifndef __FPGA_TYPES_INT_H__
#define __FPGA_TYPES_INT_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <opae/types.h>
#include <opae/sysobject.h>
#include <opae/types_enum.h>

#define SYSFS_PATH_MAX 256
#define SYSFS_FPGA_CLASS_PATH "/sys/class/fpga"
#define FPGA_DEV_PATH "/dev"

#define SYSFS_AFU_PATH_FMT "/intel-fpga-dev.%d/intel-fpga-port.%d"
#define SYSFS_FME_PATH_FMT "/intel-fpga-dev.%d/intel-fpga-fme.%d"

// substring that identifies a sysfs directory as the FME device.
#define FPGA_SYSFS_FME "fme"
// substring that identifies a sysfs directory as the AFU device.
#define FPGA_SYSFS_AFU "port"
// name of the FME interface ID (GUID) sysfs node.
#define FPGA_SYSFS_FME_INTERFACE_ID "pr/interface_id"
// name of the AFU GUID sysfs node.
#define FPGA_SYSFS_AFU_GUID "afu_id"
// name of the socket id sysfs node.
#define FPGA_SYSFS_SOCKET_ID "socket_id"
// name of the number of slots sysfs node.
#define FPGA_SYSFS_NUM_SLOTS "ports_num"
// name of the bitstream id sysfs node.
#define FPGA_SYSFS_BITSTREAM_ID "bitstream_id"

// fpga device path
#define SYSFS_FPGA_FMT "/intel-fpga-dev.%d"

// FPGA device id
#define FPGA_SYSFS_DEVICEID "device/device"

// Integrated FPGA Device ID
#define FPGA_INTEGRATED_DEVICEID 0xbcc0

// Discrete FPGA Device ID
#define FPGA_DISCRETE_DEVICEID 0x09c4

#define FPGA_BBS_VER_MAJOR(i) (((i) >> 56) & 0xf)
#define FPGA_BBS_VER_MINOR(i) (((i) >> 52) & 0xf)
#define FPGA_BBS_VER_PATCH(i) (((i) >> 48) & 0xf)

#define DEV_PATH_MAX 256

// FPGA token magic (FPGATOKN)
#define FPGA_TOKEN_MAGIC 0x46504741544f4b4e
// FPGA handle magic (FPGAHNDL)
#define FPGA_HANDLE_MAGIC 0x46504741484e444c
// FPGA property magic (FPGAPROP)
#define FPGA_PROPERTY_MAGIC 0x4650474150524f50
// FPGA event handle magid (FPGAEVNT)
#define FPGA_EVENT_HANDLE_MAGIC 0x4650474145564e54
// FPGA invalid magic (FPGAINVL)
#define FPGA_INVALID_MAGIC 0x46504741494e564c

#define FEATURE_TOKEN_MAGIC 0x46504741564f4b4e

#define FEATURE_HANDLE_MAGIC 0x46504741584e444c

#define DMA_MAX_CHANNEL 32

// Register/Unregister for interrupts
#define FPGA_IRQ_ASSIGN (1 << 0)
#define FPGA_IRQ_DEASSIGN (1 << 1)

// Get file descriptor from event handle
#define FILE_DESCRIPTOR(eh) (((struct _fpga_event_handle *)eh)->fd)
#ifdef __cplusplus
extern "C" {
#endif
/** System-wide unique FPGA resource identifier */
struct _fpga_token {
	uint32_t instance;
	uint64_t magic;
	char sysfspath[SYSFS_PATH_MAX];
	char devpath[DEV_PATH_MAX];
	struct error_list *errors;
};

/** Process-wide unique FPGA handle */
struct _fpga_handle {
	pthread_mutex_t lock;
	uint64_t magic;
	fpga_token token;
	int fddev;		    // file descriptor for the device.
	int fdfpgad;		    // file descriptor for the event daemon.
	struct wsid_map *wsid_root; // wsid information (list)
	struct wsid_map *mmio_root; // MMIO information (list)
	void *umsg_virt;	    // umsg Virtual Memory pointer
	uint64_t umsg_size;	 // umsg Virtual Memory Size
	uint64_t *umsg_iova;	// umsg IOVA from driver
};

/*
 * Event handle struct to perform
 * event operations
 *
 */
struct _fpga_event_handle {
	pthread_mutex_t lock;
	uint64_t magic;
	int fd;
	uint32_t flags;
};

/*
 * Global list to store wsid/physptr/length vectors
 */
struct wsid_map {
	uint64_t wsid;
	uint64_t addr;
	uint64_t phys;
	uint64_t len;
	uint64_t offset;
	uint32_t index;
	int flags;
	struct wsid_map *next;
};

/*
 * Global list to store tokens received during enumeration
 * Since tokens as seen by the API are only void*, we need to keep the actual
 * structs somewhere.
 */
struct token_map {
	struct _fpga_token _token;
	struct token_map *next;
};

typedef enum { FPGA_SYSFS_DIR = 0, FPGA_SYSFS_FILE } fpga_sysfs_type;

struct _fpga_object {
	pthread_mutex_t lock;
	fpga_handle handle;
	fpga_sysfs_type type;
	char *path;
	char *name;
	int perm;
	size_t size;
	size_t max_size;
	uint8_t *buffer;
	fpga_object *objects;
};

typedef struct __attribute__ ((__packed__)) {
	uint64_t dfh;
	uint64_t feature_uuid_lo;
	uint64_t feature_uuid_hi;
} dfh_feature_t;

typedef union {
	uint64_t reg;
	struct {
		uint64_t feature_type:4;
		uint64_t reserved_8:8;
		uint64_t afu_minor:4;
		uint64_t reserved_7:7;
		uint64_t end_dfh:1;
		uint64_t next_dfh:24;
		uint64_t afu_major:4;
		uint64_t feature_id:12;
	} bits;
} dfh_reg_t;

/** Device-wide unique FPGA feature resource identifier */
struct _fpga_feature_token {
	uint64_t magic;
	uint32_t feature_type;
	uint64_t feature_uuid_lo;
	uint64_t feature_uuid_hi;
	fpga_token token;
	struct _fpga_feature_token *next;
};

/** Process-wide unique FPGA feature handle */
struct _fpga_feature_handle {
	fpga_handle fpga_h;
	pthread_mutex_t lock;
	uint64_t magic;
	fpga_feature_token token;
	uint32_t mmio_num;
	uint64_t mmio_offset;
	uint64_t feature_base;
	uint64_t feature_offset;
	fpga_sub_feature capability;
	fpga_event_handle *eh_root;
};

// Data structures from DMA MM implementation
typedef union {
	uint32_t reg;
	struct {
		uint32_t tx_channel:8;
		uint32_t generate_sop:1;
		uint32_t generate_eop:1;
		uint32_t park_reads:1;
		uint32_t park_writes:1;
		uint32_t end_on_eop:1;
		uint32_t eop_rvcd_irq_en:1;
		uint32_t transfer_irq_en:1;
		uint32_t early_term_irq_en:1;
		uint32_t trans_error_irq_en:8;
		uint32_t early_done_en:1;
		uint32_t wait_for_wr_rsp:1;
		uint32_t reserved_2:5;
		uint32_t go:1;
	};
} _fpga_dma_desc_ctrl_t;

typedef struct __attribute__((__packed__)) _fpga_dma_desc {
	//0x0
	uint32_t rd_address;
	//0x4
	uint32_t wr_address;
	//0x8
	uint32_t len;
	//0xC
	uint16_t seq_num;
	uint8_t rd_burst_count;
	uint8_t wr_burst_count;
	//0x10
	uint16_t rd_stride;
	uint16_t wr_stride;
	//0x14
	uint32_t rd_address_ext;
	//0x18
	uint32_t wr_address_ext;
	//0x1c
	_fpga_dma_desc_ctrl_t control;
} _fpga_dma_desc;

#define DMA_BUFFER_POOL_SIZE 8

/* Queue dispatching transfers to the hardware */
typedef struct _fpga_dma_transfer_q {
	int read_index;
	int write_index;
	fpga_dma_transfer *queue; // Transfers queue
	sem_t entries; // Counting semaphore, count represents available entries in queue
	pthread_mutex_t qmutex; // Gain exclusive access before queue operations
} _fpga_dma_transfer_q;

/* DMA specific feature information which it is stored in the handle */
struct _fpga_dma_capability {
	// Channel type
	fpga_dma_channel_type_t ch_type;

	// DMA channel information
	uint64_t cpu_affinity;
	uint64_t dma_channel;
	uint64_t ring_size;

	// CSR layout
	uint64_t dma_csr_base;
	uint64_t dma_desc_base;
	uint64_t dma_rsp_base;
	uint64_t dma_streaming_valve_base;

	// Address span extender
	uint64_t dma_ase_cntl_base;
	uint64_t dma_ase_data_base;

	// Channel-local pinned buffers
	fpga_dma_buffer buffer_pool[DMA_BUFFER_POOL_SIZE];

	// Channel-local queue of transfers
	_fpga_dma_transfer_q dma_transfer_queue;

	// Channel-local index of the next available transfer in the dispatcher queue
	uint64_t next_avail_transfer_idx;

	// Channel-local total number of unused transfer in the dispatcher queue of transfers
	// Note: Count includes the next available transfer in
	// the dispatcher queue indexed by next_avail_transfer_idx
	uint64_t unused_transfer_count;
};

#ifdef __cplusplus
}
#endif
#endif // __FPGA_TYPES_INT_H__
