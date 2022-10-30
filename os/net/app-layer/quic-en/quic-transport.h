

/**
 * \file
 *         API for QUIC-IETF transport
 * \author

 */

/**
 * \addtogroup QUIC-IETF
 * @{
 *
 * \defgroup QUIC-transport QUIC-IETF transport API
 * @{
 *
 * The QUIC transport API defines a common interface for sending/receiving
 * QUIC messages.
 */

#ifndef QUIC_TRANSPORT_H_
#define QUIC_TRANSPORT_H_

#include "quic-endpoint.h"

/**
 * \brief      Returns a common data buffer that can be used when
 *             generating QUIC messages for transmission. The buffer
 *             size is at least quic_MAX_PACKET_SIZE bytes.
 *
 *             In Contiki-NG, this corresponds to the uIP buffer.
 *
 * \return     A pointer to a data buffer where a QUIC message can be stored.
 */
uint8_t *quic_databuf(void);

/**
 * \brief      Send a message to the specified quic endpoint
 * \param ep   A pointer to a quic endpoint
 * \param data A pointer to data to send
 * \param len  The size of the data to send
 * \return     The number of bytes sent or negative if an error occurred.
 */
int quic_sendto(const quic_endpoint_t *ep, const uint8_t *data, uint16_t len);

int quic_register(struct uip_udp_conn *udp_cn, const uint16_t port, quic_udp_callback callback);

/**
 * \brief      Initialize the quic transport.
 *
 *             This function initializes the quic transport implementation and
 *             should only be called by the quic engine.
 */
void quic_transport_init(void);

#endif /* quic_TRANSPORT_H_ */
/** @} */
/** @} */
