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
 * @file hello_fpga.c
 * @brief A code sample illustrates the basic usage of the OPAE C API.
 *
 * The sample is a host application that demonstrates the basic steps of
 * interacting with FPGA using the OPAE library. These steps include:
 *
 *  - FPGA enumeration
 *  - Resource acquiring and releasing
 *  - Managing shared memory buffer
 *  - MMIO read and write
 *
 * The sample also demonstrates OPAE's object model, such as tokens, handles,
 * and properties.
 *
 * The sample requires a native loopback mode (NLB) test image to be loaded on
 * the FPGA. Refer to
 * <a href="https://opae.github.io/docs/fpga_api/quick_start/readme.html">Quick
 * Start Guide</a> for full instructions on building, configuring, and running
 * this code sample.
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <uuid/uuid.h>
#include <opae/fpga.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdint.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "safe_string/safe_string.h"



/* NLB0 AFU_ID */
#define NLB0_AFUID "D8424DC4-A4A3-C413-F89E-433683F9040B"


/*
 * macro to check return codes, print error message, and goto cleanup label
 * NOTE: this changes the program flow (uses goto)!
 */
#define ON_ERR_GOTO(res, label, desc)              \
	do {                                       \
		if ((res) != FPGA_OK) {            \
			print_err((desc), (res));  \
			goto label;                \
		}                                  \
	} while (0)

/* Type definitions */
typedef struct {
	uint32_t uint[16];
} cache_line;

void print_err(const char *s, fpga_result res)
{
	fprintf(stderr, "Error %s: %s\n", s, fpgaErrStr(res));
}

/*
 * Global configuration of bus, set during parse_args()
 * */
struct config {
	struct target {
		int bus;
		int open_flags;
	} target;
}

config = {
	.target = {
		.bus = -1,
		.open_flags = 0
	}
};

#define GETOPT_STRING "B:s"
fpga_result parse_args(int argc, char *argv[])
{
	struct option longopts[] = {
		{"bus",	   required_argument, NULL, 'B'},
		{"shared", no_argument,       NULL, 's'},
		{NULL,     0,                 NULL,  0 },
	};
	
	int getopt_ret;
	int option_index;
	char *endptr = NULL;

	while (-1 != (getopt_ret = getopt_long(argc, argv, GETOPT_STRING,
						longopts, &option_index))) {
		const char *tmp_optarg = optarg;
		/* Checks to see if optarg is null and if not it goes to value of optarg */
		if ((optarg) && ('=' == *tmp_optarg)){
			++tmp_optarg;
		}

		switch (getopt_ret){
		case 'B': /* bus */
			if (NULL == tmp_optarg){
				return FPGA_EXCEPTION;
			}
			endptr = NULL;
			config.target.bus = (int) strtoul(tmp_optarg, &endptr, 0);
			if (endptr != tmp_optarg + strnlen(tmp_optarg, 100)) {
				fprintf(stderr, "invalid bus: %s\n", tmp_optarg);
				return FPGA_EXCEPTION;
			}
			break;
		case 's':
			config.target.open_flags |= FPGA_OPEN_SHARED;
			break;
		
		default: /* invalid option */
			fprintf(stderr, "Invalid cmdline option \n");
			return FPGA_EXCEPTION;
		}
	}
	
	return FPGA_OK;
}




	
/* function to get the bus number when there are multiple buses */
/* TODO: add device and function information */
struct bdf_info {
	uint8_t bus;
};

fpga_result get_bus_info(fpga_token tok, struct bdf_info *finfo){
	fpga_result res = FPGA_OK;
	fpga_properties props;
	res = fpgaGetProperties(tok, &props);
	ON_ERR_GOTO(res, out, "reading properties from Token");

	res = fpgaPropertiesGetBus(props, &finfo->bus);
	ON_ERR_GOTO(res, out_destroy, "Reading bus from properties");

out_destroy: 
	res = fpgaDestroyProperties(&props);
	ON_ERR_GOTO(res, out, "fpgaDestroyProps");
out:
	return res;
}

void print_bus_info(struct bdf_info *info){
	printf("Running on bus 0x%02X. \n", info->bus);
}


int main(int argc, char *argv[])
{

	char               library_version[FPGA_VERSION_STR_MAX];
	char               library_build[FPGA_BUILD_STR_MAX];
	fpga_token         fpga_token;
	fpga_handle        fpga_handle;
	fpga_guid          guid;
	uint32_t           num_matches_fpgas = 0;
 	uint64_t           num_metrics = 0;

	fpga_result     res = FPGA_OK;
	struct bdf_info info;


  fpga_properties filter = NULL;

	/* Print version information of the underlying library */
	fpgaGetOPAECVersionString(library_version, sizeof(library_version));
	fpgaGetOPAECBuildString(library_build, sizeof(library_build));
	printf("Using OPAE C library version '%s' build '%s'\n", library_version,
	       library_build);

	res = parse_args(argc, argv);
	ON_ERR_GOTO(res, out_exit, "parsing arguments");

	if (uuid_parse(NLB0_AFUID, guid) < 0) {
		res = FPGA_EXCEPTION;
	}
	ON_ERR_GOTO(res, out_exit, "parsing guid");


	/* Get number of FPGAs in system */
	res = fpgaGetProperties(NULL, &filter);
	ON_ERR_GOTO(res, out_exit, "creating properties object");

	res = fpgaPropertiesSetObjectType(filter, FPGA_DEVICE);
	ON_ERR_GOTO(res, out_destroy, "setting object type");


  
	if (-1 != config.target.bus) {
		res = fpgaPropertiesSetBus(filter, config.target.bus);
		ON_ERR_GOTO(res, out_destroy, "setting bus");
	}

    
	res = fpgaEnumerate(&filter, 1, &fpga_token, 1, &num_matches_fpgas);
	ON_ERR_GOTO(res, out_destroy, "enumerating fpga");


  if (num_matches_fpgas <= 0) {
		res = FPGA_NOT_FOUND;
	}
	ON_ERR_GOTO(res, out_exit, "no matching fpga");

	if (num_matches_fpgas > 1) {
		fprintf(stderr, "Found more than one suitable fpga. ");
		res = get_bus_info(fpga_token, &info);
		ON_ERR_GOTO(res, out_exit, "getting bus num");
		printf("Running on bus 0x%02X. \n", info.bus);
	}

	/* Open fpga  */
	res = fpgaOpen(fpga_token, &fpga_handle, config.target.open_flags);
	ON_ERR_GOTO(res, out_destroy_tok, "opening fpga");
 

	uint64_t i = 0;
	fpgaGetNumMetrics(fpga_handle, &num_metrics);
	printf(" num_metrics =%ld \n", num_metrics);
	
	struct fpga_metric_t  *metric = calloc(sizeof(struct fpga_metric_t), num_metrics);

	fpgaGetMetricsInfo(fpga_handle, metric, num_metrics);
	for (i = 0; i < num_metrics; i++){

		printf("%-20ld  | %-30s  | %-20s  | %-30s  | %-20s \n",
						metric[i].mertic_info.metric_id,
						metric[i].mertic_info.qualifier_name,
						metric[i].mertic_info.group_name,
						metric[i].mertic_info.metric_name,
						metric[i].mertic_info.metric_units);

	}


	uint64_t id_array[] = { 1, 5, 30, 35, 10 };

	struct fpga_metric_t  *metric_array = calloc(sizeof(struct fpga_metric_t), 5);

	fpgaGetMetricsByIds(fpga_handle, id_array,5, metric_array);

	for (i = 0; i < 5; i++){

		printf("%-20ld  | %-30s  | %-20s  | %-30s  | %-20s \n",
			metric_array[i].mertic_info.metric_id,
			metric_array[i].mertic_info.qualifier_name,
			metric_array[i].mertic_info.group_name,
			metric_array[i].mertic_info.metric_name,
			metric_array[i].mertic_info.metric_units);
		printf("value   = %ld \n", metric_array[i].value.ivalue);

	}
	
	char* metric_string[] = { "power_mgmt:consumed","performance:fabric:mmio_read" };
	uint64_t  array_size = 2;

	struct fpga_metric_t  *metric_array_serach = calloc(sizeof(struct fpga_metric_t), array_size);


	 fpgaGetMetricsByStrings(fpga_handle,
		metric_string,
		array_size,
		metric_array_serach);

	
	for (i = 0; i < array_size; i++){

		printf("%-20ld  | %-30s  | %-20s  | %-30s  | %-20s \n",
			metric_array[i].mertic_info.metric_id,
			metric_array[i].mertic_info.qualifier_name,
			metric_array[i].mertic_info.group_name,
			metric_array[i].mertic_info.metric_name,
			metric_array[i].mertic_info.metric_units);
		printf("value   = %ld \n", metric_array[i].value.ivalue);


	}

	/* Release fpga */
//out_close:
	res = fpgaClose(fpga_handle);
	ON_ERR_GOTO(res, out_destroy_tok, "closing fpga");

	/* Destroy token */
out_destroy_tok:
	res = fpgaDestroyToken(&fpga_token);
	ON_ERR_GOTO(res, out_exit, "destroying token");

out_destroy:
	res = fpgaDestroyProperties(&filter);
	ON_ERR_GOTO(res, out_exit, "destroying properties object");
 
out_exit:
	return (res == FPGA_OK) ? 0 : 1;
}