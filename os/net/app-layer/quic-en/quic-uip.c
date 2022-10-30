

/**
 * \file
 *         QUIC transport implementation for uIPv6
 */

/**
 * \addtogroup quic-transport
 * @{
 *
 * \defgroup quic-uip QUIC transport implementation for uIP
 * @{
 *
 * This is an implementation of CoAP transport and CoAP endpoint over uIP
 * with DTLS support.
 */

#include "contiki.h"
#include "net/ipv6/uip-udp-packet.h"
#include "net/ipv6/uiplib.h"
#include "quic-constants.h"
#include "quic-endpoint.h"
#include "quic-transport.h"
#include <assert.h>

/* Log configuration */
//#include "quic-log.h"
#include "sys/log.h"
#define LOG_MODULE "quic-eng"
#define LOG_LEVEL  LOG_LEVEL_MAIN


/* sanity check for configured values */
#if QUIC_MAX_PACKET_SIZE > (UIP_BUFSIZE - UIP_IPH_LEN - UIP_UDPH_LEN)
#error "UIP_CONF_BUFFER_SIZE too small for quic_MAX_CHUNK_SIZE"
#endif

#define SERVER_LISTEN_PORT        UIP_HTONS(QUIC_DEFAULT_PORT)


PROCESS(quic_engine, "QUIC Engine");

static struct uip_udp_conn *udp_conn = NULL;
static quic_udp_callback qreceive = NULL;
//static uint8_t syncReq = 0;

/*---------------------------------------------------------------------------*/
void
quic_endpoint_log(const quic_endpoint_t *ep)
{
  if(ep == NULL) {
    LOG_OUTPUT("(NULL EP)");
    return;
  }
  if(quic_endpoint_is_secure(ep)) {
    LOG_OUTPUT("quic:[");
  } else {
    LOG_OUTPUT("quic:[");
  }
  log_6addr(&ep->ipaddr);
  LOG_OUTPUT("]:%u", uip_ntohs(ep->port));
}
/*---------------------------------------------------------------------------*/
void
quic_endpoint_print(const quic_endpoint_t *ep)
{
  if(ep == NULL) {
    printf("(NULL EP)");
    return;
  }
  if(quic_endpoint_is_secure(ep) ) {
    printf("quic:[");
  } else {
    printf("quic:[");
  }
  uiplib_ipaddr_print(&ep->ipaddr);
  printf("]:%u", uip_ntohs(ep->port));
}
/*---------------------------------------------------------------------------*/
int
quic_endpoint_snprint(char *buf, size_t size, const quic_endpoint_t *ep)
{
  int n;
  if(buf == NULL || size == 0) {
    return 0;
  }
  if(ep == NULL) {
    n = snprintf(buf, size - 1, "(NULL EP)");
  } else {
    if(quic_endpoint_is_secure(ep)) {
      n = snprintf(buf, size - 1, "quic:[");
    } else {
      n = snprintf(buf, size - 1, "quic:[");
    }
    if(n < size - 1) {
      n += uiplib_ipaddr_snprint(&buf[n], size - n - 1, &ep->ipaddr);
    }
    if(n < size - 1) {
      n += snprintf(&buf[n], size -n - 1, "]:%u", uip_ntohs(ep->port));
    }
  }
  if(n >= size - 1) {
    buf[size - 1] = '\0';
  }
  return n;
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
static const quic_endpoint_t *
get_src_endpoint(uint8_t secure)
{
  static quic_endpoint_t src;
  uip_ipaddr_copy(&src.ipaddr, &UIP_IP_BUF->srcipaddr);
  src.port = UIP_UDP_BUF->srcport;
  src.secure = 0;
  return &src;
}
/*---------------------------------------------------------------------------*/
int
quic_endpoint_is_secure(const quic_endpoint_t *ep)
{
  return ep->secure;
}
/*---------------------------------------------------------------------------*/
int
quic_endpoint_is_connected(const quic_endpoint_t *ep)
{
#ifndef CONTIKI_TARGET_NATIVE
  if(!uip_is_addr_linklocal(&ep->ipaddr)
    && NETSTACK_ROUTING.node_is_reachable() == 0) {
    return 0;
  }
#endif

  /* Assume connected on native */
  return 1;
}
/*---------------------------------------------------------------------------*/
int
quic_endpoint_connect(quic_endpoint_t *ep)
{
  if(quic_endpoint_is_secure(ep) == 0) {
    LOG_DBG("connect to ");
    //LOG_DBG_QUIC_EP(ep);
    LOG_DBG_("\n");
    return 1;
  }

  return 0;
}
/*---------------------------------------------------------------------------*/
void
quic_endpoint_disconnect(quic_endpoint_t *ep)
{
  LOG_DBG("Disconnect from ");
  //LOG_DBG_QUIC_EP(ep);
  LOG_DBG_("\n");

}
/*---------------------------------------------------------------------------*/
uint8_t *
quic_databuf(void)
{
  return uip_appdata;
}
/*---------------------------------------------------------------------------*/
void
quic_transport_init(void)
{
  process_start(&quic_engine, NULL);

}
int quic_register(struct uip_udp_conn *udp_cn, const uint16_t port, quic_udp_callback callback)
{
  qreceive = callback;
  assert(udp_conn != NULL);
  udp_cn->lport = udp_conn->lport;
  udp_cn->ripaddr = udp_conn->ripaddr;
  udp_cn->rport = udp_conn->rport;
  udp_cn->appstate = udp_conn->appstate;
  //todo: check how to change the port

  return 1;
}

/*---------------------------------------------------------------------------*/
static void
process_data(void)
{

  LOG_INFO("receiving UDP datagram from [");
  LOG_INFO_6ADDR(&UIP_IP_BUF->srcipaddr);
  LOG_INFO_("]:%u\n", uip_ntohs(UIP_UDP_BUF->srcport));
  LOG_INFO("  Length: %u ttl: %d \n", uip_datalen(), udp_conn->ttl);

  if(qreceive != NULL)
  {
    qreceive(get_src_endpoint(0), quic_databuf(), uip_datalen());
  } else
  {
    LOG_INFO("No Receive Handler registered. drop quic packet\n");
  }

}
/*---------------------------------------------------------------------------*/
int
quic_sendto(const quic_endpoint_t *ep, const uint8_t *data, uint16_t length)
{
  if(ep == NULL) {
    LOG_WARN("failed to send - no endpoint\n");
    return -1;
  }

  if(!quic_endpoint_is_connected(ep)) {
    LOG_WARN("endpoint ");
    //LOG_WARN_QUIC_EP(ep);
    LOG_WARN_(" not connected - dropping packet\n");
    return -1;
  }


  uip_udp_packet_sendto(udp_conn, data, length, &ep->ipaddr, ep->port);
  LOG_INFO("sent to ");
  //LOG_WARN_QUIC_EP(ep);
  LOG_INFO_(" %u bytes\n", length);
  return length;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(quic_engine, ev, data)
{
  PROCESS_BEGIN();
  //assert(qreceive != NULL);

  /* new connection with remote host */
  udp_conn = udp_new(NULL, 0, NULL);
  udp_bind(udp_conn, SERVER_LISTEN_PORT);
  LOG_INFO("QUIC Listening on port %u\n", uip_ntohs(udp_conn->lport));
  LOG_INFO("QUIC ipv6 MTU: %d ttl:%d \n", UIP_LINK_MTU, udp_conn->ttl);


  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event) {
      if(uip_newdata()) {
        process_data();
      }
    }
  } /* while (1) */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/** @} */
/** @} */
