#include "contiki-lib.h"
#include "contiki-net.h"
#include <inttypes.h>
#include <stdint.h>

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_DBG

#define WITH_SERVER_REPLY 1
#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

#define SERVER_IP "fd00::302:304:506:708"
#define PING_TIMEOUT (CLOCK_SECOND / 4)
#define SEND_INTERVAL (10 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static uint32_t rx_count = 0;
static uip_ipaddr_t server_addr;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void udp_rx_callback(struct simple_udp_connection *c,
                            const uip_ipaddr_t *sender_addr,
                            uint16_t sender_port,
                            const uip_ipaddr_t *receiver_addr,
                            uint16_t receiver_port, const uint8_t *data,
                            uint16_t datalen) {

  LOG_INFO("Received response '%.*s' from ", datalen, (char *)data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");
  rx_count++;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data) {
  static struct etimer periodic_timer;
  static char str[32];
  static uint32_t tx_count;

  PROCESS_BEGIN();

  LOG_INFO("IPv6- UDP creation \n");

  uiplib_ipaddrconv(SERVER_IP, &server_addr);

  /* Initialize UDP connection */
  uint16_t status =
      simple_udp_register(&udp_conn, UDP_CLIENT_PORT, &server_addr,
                          UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, SEND_INTERVAL);
  if (status) {
    LOG_INFO("UDP Client Connecton created success\n");
  }
  while (1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    LOG_INFO("UDP Client send packet %" PRIu32 " server-IP: \n", tx_count);
    LOG_INFO_6ADDR(&server_addr);
    LOG_INFO("\n");

    snprintf(str, sizeof(str), "hello %" PRIu32 "", tx_count);
    simple_udp_sendto(&udp_conn, str, strlen(str), &server_addr);
    // simple_udp_send(&udp_conn, str, strlen(str));
    tx_count++;
    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND +
                                    (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
