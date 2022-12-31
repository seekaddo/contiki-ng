//
// Created by Dennis Kwame Addo on 12/20/22.
//


#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <limits.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "contiki.h"
#include "quic-constants.h"
#include "quic-endpoint.h"
#include "quic-transport.h"
#include <assert.h>


#include "sys/log.h"
#define LOG_MODULE "quic-eng"
#define LOG_LEVEL  LOG_LEVEL_MAIN

#include "quic-ietf.h"

#define AF_INT6 10

typedef struct {

  struct w_engine *w;
  const char *  req;
  const char *  peer;
  struct q_conn *  c;
  quic_endpoint_t  p_addr;
  uint16_t qrcnt;
  uint16_t clean;
  quic_udp_callback cl_callback;
} qmcon;


PROCESS(quic_etranx, "QUIC Transac");

// XXX: change "flash" to 0 to disable 0-RTT:
static const struct q_conf qc = {0, "flash", 0, 0,
                                 0, 0, 0, 20, false};
static qmcon tranx_conn;


void quic_transx(const char * const req, const char * const peer, quic_udp_callback callback )
{

  tranx_conn.peer = peer;
  tranx_conn.qrcnt = 1;
  tranx_conn.req = req;
  tranx_conn.p_addr.af = AF_INT6;
  tranx_conn.p_addr.port = UIP_HTONS(4432); // default server port
  uiplib_ipaddrconv(peer, &tranx_conn.p_addr.ipaddr);
  tranx_conn.cl_callback = callback;
  tranx_conn.clean = 0;


}

PROCESS_THREAD(quic_etranx, ev, data)
{
  PROCESS_BEGIN();
  LOG_INFO("  QUIC Transx Started  \n");
  //Wait until we have all contiki-ng net stuffs done with uIP
  PROCESS_WAIT_EVENT_UNTIL(quic_udp_active());

  tranx_conn.w = q_init("uIP", &qc);
  PROCESS_WAIT_EVENT_UNTIL(tranx_conn.qrcnt); // for client request

  LOG_INFO("quic_etranx:  q_init done and ready for quic conn \n");
  struct w_iov_sq o = w_iov_sq_initializer(o);

  struct q_stream * s = 0;

  //uint8_t cnkt = 4;

  while(1) {
    PROCESS_YIELD();
    uint8_t qwait = 0;


    if (tranx_conn.c == NULL) {
      q_alloc(tranx_conn.w, &o, 0, AF_INT6, 512); // af family is ipv6
      struct w_iov * const v = sq_first(&o);
      v->len = sprintf((char *)v->buf, "GET %s\r\n", tranx_conn.req);

      static const struct q_conn_conf qcc = {
          30, 0, 0, 0, 0,
          0, 0, 0, 0,
          0, 0xff000000 + DRAFT_VERSION};
      tranx_conn.c  = q_connect(tranx_conn.w, &tranx_conn.p_addr, tranx_conn.peer, &o, &s, true,
                    "hq-" DRAFT_VERSION_STRING, &qcc);

      qwait = 1; // wait for response
    }

    if (tranx_conn.c ) {
      if(qwait) {
        warn(DBG, "==============Get Response================================");
        struct w_iov_sq i = w_iov_sq_initializer(i);
        q_read_stream(s, &i, true);
        const uint16_t len = w_iov_sq_len(&i);
        warn(DBG, "retrieved %" PRIu32 " bytes", len);
        struct w_iov *const sv = sq_first(&i);
        tranx_conn.qrcnt = 0;
        tranx_conn.cl_callback(&sv->saddr, sv->buf, sv->len);
        // warn(DBG, "Payload %d bytes->\n%s", sv->len, sv->buf);

        warn(DBG, "==============Get Done================================");
        qwait = 0;
        q_free(&i);
        q_free_stream(s);
      }

      if(tranx_conn.qrcnt) {
        // todo: here we sent a fresh new request, Using old connection but new stream
        s = q_rsv_stream(tranx_conn.c, true); // request a new stream
        q_alloc(tranx_conn.w, &o, 0, AF_INT6,
                512); // allocate memory for the stream
        struct w_iov *const vv = sq_first(&o);
        vv->len = sprintf((char *)vv->buf, "GET %s\r\n", tranx_conn.req);
        q_write(s, &o, true); // send new request on the same stream 0
        qwait = 1; // wait for response
      }


    }
    else {
      struct w_iov * const vb = sq_first(&o);
      warn(DBG, "could not retrieve %s", vb->buf);
    }

    if(tranx_conn.clean) {
      // free the memory here
      q_free(&o);
      q_close(tranx_conn.c, 0, "No connection");
      q_cleanup(tranx_conn.w);
    }

  } /* while (1) */

  PROCESS_END();
}

void quic_transx_init(void)
{
  process_start(&quic_etranx, NULL);

}




