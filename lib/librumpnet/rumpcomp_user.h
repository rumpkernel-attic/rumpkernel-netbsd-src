#ifndef __NETINET_RUMPCOMP_USER__
#define __NETINET_RUMPCOMP_USER__

int rumpcomp_librumpnet_hive_request_port(
        uint16_t port, int32_t *p_bind_result,
        int netbsd_kernel_protocol);

#endif
