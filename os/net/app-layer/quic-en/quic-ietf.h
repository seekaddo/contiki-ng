//
// Created by seekaddo on 9/13/22.
//

#ifndef CONTIKI_NG_QUIC_IETF_H
#define CONTIKI_NG_QUIC_IETF_H
#include "quant/quant.h"
#include "quic-transport.h"

extern void quic_transx(const char *const req, const char *const peer, quic_udp_callback callback);
extern void quic_streeam(const char *req, quic_udp_callback callback);
extern void qcon_state();
extern void qstream_state();
extern void qstate_handler();
#endif // CONTIKI_NG_QUIC_IETF_H
