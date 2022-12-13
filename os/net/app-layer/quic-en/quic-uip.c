

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

uint8_t syncNewData = 0;


uint8_t quic_udp_active()
{
  return udp_conn != NULL;

}

struct uip_udp_conn *quic_udp_con()
{
  return udp_conn;
}
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

void set_endpoint(quic_endpoint_t *ep)
{
  uip_ipaddr_copy(&ep->ipaddr, &UIP_IP_BUF->srcipaddr);
  ep->port = UIP_UDP_BUF->srcport;
  ep->secure = 0;
}

uint16_t get_ttl()
{
  return udp_conn->ttl;
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
  //todo: remove this later from production code and use BLE version
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
quic_databuf(void) // call uip_datalen for the len
{
  return uip_appdata;
}
/*---------------------------------------------------------------------------*/
void
quic_transport_init(void)
{
  process_start(&quic_engine, NULL);
  //todo: start another process to handle all quic request
  // A pointer to the struct data is needed to pass the whole request

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

uint32_t recvfr(uint8_t *buf, uint32_t len)
{
  if(syncNewData == 0)
    return 0;

  uint32_t akLen = uip_datalen();
  if(uip_datalen() > len)
  {
    LOG_WARN("------Not Enough buffer provided by backend");
    akLen = uip_datalen(); //
  }

  memcpy(buf,quic_databuf(),akLen);
  syncNewData = 0;
  return akLen;
}

/*---------------------------------------------------------------------------*/
static void
process_data(void)
{

  //todo:
  //set active newdata on
  syncNewData = 1;

  LOG_INFO("receiving UDP datagram from [");
  LOG_INFO_6ADDR(&UIP_IP_BUF->srcipaddr);
  LOG_INFO_("]:%u\n", uip_ntohs(UIP_UDP_BUF->srcport));
  LOG_INFO("  Length: %u ttl: %d \n", uip_datalen(), udp_conn->ttl);

  char buf[UIPLIB_IPV6_MAX_STR_LEN];
  uiplib_ipaddr_snprint(buf, sizeof(buf), &UIP_IP_BUF->srcipaddr);
  LOG_INFO("  test uIP addr: %s \n", buf);

  if(qreceive != NULL)
  {
    qreceive(get_src_endpoint(0), quic_databuf(), uip_datalen());
  } else
  {
    LOG_INFO("No Receive Handler registered. drop quic packet\n");
  }

}
const char * wi_ntop(uip_ipaddr_t *add,  char * dest)
{
  uiplib_ipaddr_snprint(dest, 40, add);
  return dest;
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
  char buf[UIPLIB_IPV6_MAX_STR_LEN];
  uiplib_ipaddr_snprint(buf, sizeof(buf), &uip_hostaddr);
  LOG_INFO("  Client uIP addr: %s \n", buf);
  LOG_INFO("QUIC Listening on port %u\n", uip_ntohs(udp_conn->lport));
  LOG_INFO("QUIC ipv6 MTU: %d ttl:%d \n", UIP_LINK_MTU, udp_conn->ttl);


  while(1) {
    PROCESS_YIELD();

    if(ev == tcpip_event) {
      if(uip_newdata()) {
        process_data(); // tood:DEE run loop_run here
      }

      //todo: DEE loop_run state-machine (new)
      //todo: create a separate thread for processing all QUIC request.
      // No AutoStart Process but started on active connection
      // Using the etimer in the thread
    }
  } /* while (1) */

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/** @} */
/** @} */
