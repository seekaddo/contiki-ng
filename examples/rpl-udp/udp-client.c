#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include <stdint.h>
#include <inttypes.h>
#include "contiki-lib.h"
#include "contiki-net.h"
#include "net/ipv6/uip-icmp6.h"

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

#define SERVER_IP               "fd00::302:304:506:708"
#define PING_TIMEOUT              (CLOCK_SECOND / 4)
#define SEND_INTERVAL		  (10 * CLOCK_SECOND)

//static struct simple_udp_connection udp_conn;
static struct uip_udp_conn *conn;
//static uint32_t rx_count = 0;

static uip_ipaddr_t server_addr;
static uint8_t echo_received;
static struct etimer timer;
static uint16_t packet_counter;
static char buf[255];
static struct uip_icmp6_echo_reply_notification icmp_notification;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
void
icmp_reply_handler(uip_ipaddr_t *source, uint8_t ttl,
                   uint8_t *data, uint16_t datalen)
{
  if(uip_ip6addr_cmp(source, &server_addr)) {
    printf("echo response received\n");
    echo_received = 1;
  }
}

static void
timeout_handler(void)
{
  sprintf(buf, "Hello server %04u!", packet_counter);
  printf("send message: <%s>\n", buf);
  uip_udp_packet_send(conn, buf, strlen(buf));
  packet_counter++;
}
#if 0
static void
udp_rx_callback(struct simple_udp_connection *c,
         const uip_ipaddr_t *sender_addr,
         uint16_t sender_port,
         const uip_ipaddr_t *receiver_addr,
         uint16_t receiver_port,
         const uint8_t *data,
         uint16_t datalen)
{

  LOG_INFO("Received response '%.*s' from ", datalen, (char *) data);
  LOG_INFO_6ADDR(sender_addr);
#if LLSEC802154_CONF_ENABLED
  LOG_INFO_(" LLSEC LV:%d", uipbuf_get_attr(UIPBUF_ATTR_LLSEC_LEVEL));
#endif
  LOG_INFO_("\n");
  rx_count++;
}
#endif
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  //static char str[32];
  //uip_ipaddr_t dest_ipaddr;
  //static uint32_t tx_count;
  //static uint32_t missed_tx_count;

  PROCESS_BEGIN();

  LOG_INFO("IPv6- UDP creation \n");

  uiplib_ipaddrconv(SERVER_IP, &server_addr);
  uip_icmp6_echo_reply_callback_add(&icmp_notification, icmp_reply_handler);

  LOG_INFO("pinging the IPv6-over-BLE server\n");

  do {
    etimer_set(&timer, PING_TIMEOUT);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    uip_icmp6_send(&server_addr, ICMP6_ECHO_REQUEST, 0, 20);
    LOG_INFO("Timeout no response\n");
  } while(!echo_received);

  LOG_INFO("Server Active now!! --> bind\n");
  conn = udp_new(&server_addr, UIP_HTONS(UDP_SERVER_PORT), NULL);
  udp_bind(conn, UIP_HTONS(UDP_CLIENT_PORT));

  /* Initialize UDP connection */
//  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, &server_addr,
//                      UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    //PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));
    PROCESS_YIELD();
    if((ev == PROCESS_EVENT_TIMER) && (data == &periodic_timer)) {
      timeout_handler();
      /* Add some jitter */
      etimer_set(&periodic_timer, SEND_INTERVAL
                                      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
    } else if(ev == tcpip_event) {
      LOG_INFO("Recive request %"PRIu32" to ", 888);
      //tcpip_handler();
    }
    else
    {
      LOG_INFO("Unknown event triggered \n");
    }

//    if(NETSTACK_ROUTING.node_is_reachable() &&
//        NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {
#if 0
      /* Print statistics every 10th TX */
      if(tx_count % 2 == 0) {
        LOG_INFO("Tx/Rx/MissedTx: %" PRIu32 "/%" PRIu32 "/%" PRIu32 "\n",
                 tx_count, rx_count, missed_tx_count);
      }

      /* Send to DAG root */
      LOG_INFO("Sending request %"PRIu32" to ", tx_count);
      LOG_INFO_6ADDR(&server_addr);
      LOG_INFO_("\n");
      snprintf(str, sizeof(str), "hello %" PRIu32 "", tx_count);
      simple_udp_sendto(&udp_conn, str, strlen(str), &server_addr);
      tx_count++;
#endif
//    } else {
//      uip_ipaddr_t root_ipaddr;
//      NETSTACK_ROUTING.get_root_ipaddr(&root_ipaddr);
//      LOG_INFO("Not reachable yet\n");
//      LOG_INFO_6ADDR(&dest_ipaddr);
//      LOG_INFO("\n");
//      if(tx_count > 0) {
//        missed_tx_count++;
//      }
//    }


  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
