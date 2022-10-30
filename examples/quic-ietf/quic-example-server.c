

/**
 * \file
 *
 * \author
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "contiki.h"
#include "quic-ietf.h"

/* Log configuration */
//#include "sys/log.h"
#include "quic-log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP
#define UDP_SERVER_PORT	4432

static struct uip_udp_conn udp_conn;
/*
 * Resources to be activated need to be imported through the extern keyword.
 * The build system automatically compiles the resources in the corresponding sub-directory.
 */

static void
udp_rx_callback(const quic_endpoint_t *src,
                uint8_t *payload, uint16_t payload_length)
{
  LOG_INFO("Received request '%.*s' from \n", payload_length, (char *) payload);
  //LOG_INFO_6ADDR(src);
  //LOG_INFO_(" Port: %u \n", sender_port);
  /* send back the same string to the client as an echo reply */
  LOG_INFO("Sending response.\n");
  char bug[] = "Hello World from QUIC server";

  quic_sendto(src, (uint8_t *)&bug[0], sizeof(bug));

}


PROCESS(er_example_server, "QUIC Example Server");
AUTOSTART_PROCESSES(&er_example_server);

PROCESS_THREAD(er_example_server, ev, data)
{
  PROCESS_BEGIN();

  //PROCESS_PAUSE();

  LOG_INFO("Starting QUIC Example Server\n");
  quic_register(&udp_conn,UDP_SERVER_PORT, udp_rx_callback);



  /* Define application-specific events here. */
  while(1) {
    PROCESS_WAIT_EVENT();

  }                             /* while (1) */

  PROCESS_END();
}
