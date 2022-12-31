//
// Created by seekaddo on 9/13/22.
//

#ifndef CONTIKI_NG_QUIC_IETF_H
#define CONTIKI_NG_QUIC_IETF_H
#include "quic-transport.h"
#include "quant/quant.h"

extern void quic_transx(const char * const req, const char * const peer, quic_udp_callback callback );
#endif // CONTIKI_NG_QUIC_IETF_H
