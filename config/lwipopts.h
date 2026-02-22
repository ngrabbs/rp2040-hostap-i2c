/*
 * lwIP Options Configuration for RP2040 HostAP I2C Project
 */

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

/* Platform specific locking */
#define NO_SYS                          0
#define LWIP_SOCKET                     0
#define LWIP_NETCONN                    0

/* Memory options */
#define MEM_LIBC_MALLOC                 0
#define MEMP_MEM_MALLOC                 0
#define MEM_ALIGNMENT                   4
#define MEM_SIZE                        (16 * 1024)
#define MEMP_NUM_PBUF                   24
#define MEMP_NUM_UDP_PCB                8
#define MEMP_NUM_TCP_PCB                8
#define MEMP_NUM_TCP_PCB_LISTEN         4
#define MEMP_NUM_TCP_SEG                32
#define MEMP_NUM_NETBUF                 8
#define MEMP_NUM_NETCONN                8
#define MEMP_NUM_SYS_TIMEOUT            16

#define PBUF_POOL_SIZE                  24
#define PBUF_POOL_BUFSIZE               1536

/* Core options */
#define LWIP_ARP                        1
#define LWIP_ETHERNET                   1
#define LWIP_ICMP                       1
#define LWIP_RAW                        1
#define LWIP_CHKSUM_ALGORITHM           3

/* IPv4 options */
#define LWIP_IPV4                       1
#define IP_FORWARD                      0
#define IP_OPTIONS_ALLOWED              1
#define IP_REASSEMBLY                   1
#define IP_FRAG                         1
#define IP_REASS_MAXAGE                 3
#define IP_REASS_MAX_PBUFS              10
#define IP_DEFAULT_TTL                  255

/* DHCP options */
#define LWIP_DHCP                       1
#define DHCP_DOES_ARP_CHECK             0

/* UDP options */
#define LWIP_UDP                        1
#define UDP_TTL                         255

/* TCP options */
#define LWIP_TCP                        1
#define TCP_TTL                         255
#define TCP_WND                         (8 * TCP_MSS)
#define TCP_MAXRTX                      12
#define TCP_SYNMAXRTX                   6
#define TCP_QUEUE_OOSEQ                 1
#define TCP_MSS                         1460
#define TCP_SND_BUF                     (8 * TCP_MSS)
#define TCP_SND_QUEUELEN                ((4 * (TCP_SND_BUF) + (TCP_MSS - 1)) / (TCP_MSS))
#define TCP_LISTEN_BACKLOG              1

/* Network interface options */
#define LWIP_NETIF_STATUS_CALLBACK      1
#define LWIP_NETIF_LINK_CALLBACK        1
#define LWIP_NETIF_HOSTNAME             1
#define LWIP_NETIF_API                  0

/* LoopIF - required for DHCP server */
#define LWIP_HAVE_LOOPIF                0

/* DNS options */
#define LWIP_DNS                        1
#define DNS_TABLE_SIZE                  4
#define DNS_MAX_NAME_LENGTH             256

/* Statistics options */
#define LWIP_STATS                      0
#define LWIP_STATS_DISPLAY              0

/* Debugging options */
#define LWIP_DEBUG                      0
#define LWIP_DBG_MIN_LEVEL              LWIP_DBG_LEVEL_ALL
#define LWIP_DBG_TYPES_ON               LWIP_DBG_ON

/* HTTP Server options */
#define LWIP_HTTPD_CGI                  1
#define LWIP_HTTPD_SSI                  1
#define LWIP_HTTPD_SSI_INCLUDE_TAG      0
#define LWIP_HTTPD_MAX_TAG_NAME_LEN     16
#define LWIP_HTTPD_MAX_TAG_INSERT_LEN   192
#define LWIP_HTTPD_SUPPORT_POST         0
#define LWIP_HTTPD_DYNAMIC_HEADERS      1
#define LWIP_HTTPD_DYNAMIC_FILE_READ    0
#define LWIP_HTTPD_CUSTOM_FILES         1
#define HTTPD_PRECALCULATED_CHECKSUM    0

/* Thread options for FreeRTOS integration */
#define TCPIP_THREAD_NAME               "tcpip"
#define TCPIP_THREAD_STACKSIZE          2048
#define TCPIP_THREAD_PRIO               (configMAX_PRIORITIES - 2)
#define TCPIP_MBOX_SIZE                 8

#define DEFAULT_THREAD_STACKSIZE        1024
#define DEFAULT_THREAD_PRIO             (tskIDLE_PRIORITY + 1)
#define DEFAULT_RAW_RECVMBOX_SIZE       8
#define DEFAULT_UDP_RECVMBOX_SIZE       8
#define DEFAULT_TCP_RECVMBOX_SIZE       8
#define DEFAULT_ACCEPTMBOX_SIZE         8

/* Required for FreeRTOS */
#define LWIP_FREERTOS_CHECK_CORE_LOCKING 0
#define LWIP_ASSERT_CORE_LOCKED()       do { } while(0)

/* Checksum options - use hardware acceleration if available */
#define CHECKSUM_GEN_IP                 1
#define CHECKSUM_GEN_UDP                1
#define CHECKSUM_GEN_TCP                1
#define CHECKSUM_GEN_ICMP               1
#define CHECKSUM_CHECK_IP               1
#define CHECKSUM_CHECK_UDP              1
#define CHECKSUM_CHECK_TCP              1
#define CHECKSUM_CHECK_ICMP             1

#endif /* LWIPOPTS_H */
