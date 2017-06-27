#pragma once

#if HAVE_CONFIG_H
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <errno.h>
#include <memory>

//cast done inside tl layer
struct mp_request;
struct mp_region;
struct mp_window;

typedef struct mp_request * mp_request_t;
typedef struct mp_region * mp_region_t;
typedef struct mp_window * mp_window_t;

#define MP_SUCCESS 0
#define MP_FAILURE 1
#define MP_DEFAULT 0

#define MP_CHECK(stmt)                                  \
do {                                                    \
    int result = (stmt);                                \
    if (MP_SUCCESS != result) {                         \
        fprintf(stderr, "[%s:%d] mp call failed \n",    \
         __FILE__, __LINE__);                           \
        exit(EXIT_FAILURE);                             \
    }                                                   \
    assert(0 == result);                                \
} while (0)
//oob_comm->abort(-1);

#define MP_CHECK_OOB_OBJ()																\
	if(!oob_comm) {																		\
		fprintf(stderr, "[%s:%d] OOB object not initialized \n", __FILE__, __LINE__);	\
		exit(EXIT_FAILURE);																\
	}

#define MP_CHECK_TL_OBJ()																\
	if(!tl_comm) {																		\
		fprintf(stderr, "[%s:%d] TL object not initialized \n", __FILE__, __LINE__);	\
		oob_comm->abort(-1);					                						\
		exit(EXIT_FAILURE);																\
	}

#define MP_CHECK_COMM_OBJ()		\
	MP_CHECK_OOB_OBJ();			\
	MP_CHECK_TL_OBJ();

#define MP_API_MAJOR_VERSION    3
#define MP_API_MINOR_VERSION    0
#define MP_API_VERSION          ((MP_API_MAJOR_VERSION << 16) | MP_API_MINOR_VERSION)

#define MP_API_VERSION_COMPATIBLE(v) \
    ( ((((v) & 0xffff0000U) >> 16) == MP_API_MAJOR_VERSION) &&   \
      ((((v) & 0x0000ffffU) >> 0 ) >= MP_API_MINOR_VERSION) )

//===== INFO
typedef enum mp_param {
    MP_PARAM_VERSION=0,
    MP_NUM_PARAMS,
    MP_OOB_TYPE,
    MP_TL_TYPE,
    MP_NUM_RANKS,
    MP_MY_RANK
} mp_param_t;


//===== mp.cc
int mp_query_param(mp_param_t param, int *value);
int mp_init(int argc, char *argv[], int par1);
void mp_finalize();
void mp_get_envars();
mp_request_t * mp_create_request(int number);
mp_region_t * mp_create_regions(int number);
int mp_unregister_regions(int number, mp_region_t * mp_regs);
int mp_register_region_buffer(void * addr, size_t length, mp_region_t * mp_reg);
int mp_window_create(void *addr, size_t size, mp_window_t *window_t);
int mp_window_destroy(mp_window_t *window_t);
void mp_barrier();
void mp_abort();

//===== mp_comm.cc
int mp_irecv(void * buf, size_t size, int peer, mp_region_t * mp_reg, mp_request_t * mp_req);
int mp_isend(void * buf, size_t size, int peer, mp_region_t * mp_reg, mp_request_t * mp_req);

int mp_iput(void *buf, int size, mp_region_t * mp_reg, int peer, size_t displ, mp_window_t * mp_win, mp_request_t * mp_req, int flags);
int mp_iget(void *buf, int size, mp_region_t * mp_reg, int peer, size_t displ, mp_window_t * mp_win, mp_request_t * mp_req);

int mp_wait_word(uint32_t *ptr, uint32_t value, int flags);
int mp_wait(mp_request_t * mp_req);
int mp_wait_all(int number, mp_request_t * mp_reqs);

#ifdef HAVE_GDSYNC
#include <cuda.h>
#include <cuda_runtime.h>

//===== mp_comm_async.cc
int mp_send_prepare(void * buf, size_t size, int peer, mp_region_t * mp_reg, mp_request_t * mp_req);
int mp_isend_post_async(mp_request_t * mp_req, cudaStream_t stream);
int mp_isend_post_all_async(int number, mp_request_t * mp_req, cudaStream_t stream);
int mp_isend_async(void * buf, size_t size, int peer, mp_region_t * mp_reg, mp_request_t * mp_req, cudaStream_t stream);
int mp_send_post_async(mp_request_t * mp_req, cudaStream_t stream);
int mp_send_post_all_async(int number, mp_request_t * mp_req, cudaStream_t stream);
int mp_send_async(void * buf, size_t size, int peer, mp_region_t * mp_reg, mp_request_t * mp_req, cudaStream_t stream);

int mp_put_prepare(void *buf, int size, mp_region_t * mp_reg, int peer, size_t displ, mp_window_t * mp_win, mp_request_t * mp_req, int flags);
int mp_iput_post_async(mp_request_t * mp_req, cudaStream_t stream);
int mp_iput_post_all_async(int number, mp_request_t * mp_req, cudaStream_t stream);
int mp_iput_async(void *buf, int size, mp_region_t * mp_reg, int peer, size_t displ, mp_window_t * mp_win, mp_request_t * mp_req, int flags, cudaStream_t stream);
int mp_iget_async(void *buf, int size, mp_region_t * mp_reg, int peer, size_t displ, mp_window_t * mp_win, mp_request_t * mp_req, cudaStream_t stream);

int mp_wait_word_async(uint32_t *ptr, uint32_t value, int flags, cudaStream_t stream);
int mp_wait_async(mp_request_t * mp_req, cudaStream_t stream);
int mp_wait_all_async(int number, mp_request_t * mp_reqs, cudaStream_t stream);
int mp_progress_requests(int number, mp_request_t * mp_reqs);
#endif
