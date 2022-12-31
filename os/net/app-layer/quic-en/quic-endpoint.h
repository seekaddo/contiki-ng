

/**
 * \file
 *         API to address QUIC-IETF endpoints
 */

/**
 * \addtogroup QUIC-transport
 * @{
 */

#ifndef QUIC_ENDPOINT_H_
#define QUIC_ENDPOINT_H_

#include "contiki.h"
#include <stdlib.h>

#ifndef quic_ENDPOINT_CUSTOM
#include "net/ipv6/uip.h"

typedef struct {
  uip_ipaddr_t ipaddr;
  uint16_t port;
  uint8_t secure;
  uint16_t af;
} quic_endpoint_t;
#endif /* QUIC_ENDPOINT_CUSTOM */

/** Simple UDP Callback function type. */
typedef void (* quic_udp_callback)(const quic_endpoint_t *src,
                                  uint8_t *payload, uint16_t payload_length);

/**
 * \brief      Print a QUIC endpoint via the logging module.
 *
 * \param ep   A pointer to the QUIC endpoint to log.
 */
void quic_endpoint_log(const quic_endpoint_t *ep);

uint8_t quic_udp_active();
struct uip_udp_conn *quic_udp_con();

uint32_t recvfr(uint8_t *buf, uint32_t len);
const char * wi_ntop(const uip_ipaddr_t *add,  char * dst);

void set_endpoint(quic_endpoint_t *ep);
uint16_t get_ttl();
//extern uip_ipaddr_t uip_hostaddr;

#define GetHAddr(a) uip_ipaddr_copy((a), &uip_hostaddr)

/**
 * \brief      Print a QUIC endpoint.
 *
 * \param ep   A pointer to the QUIC endpoint to print.
 */
void quic_endpoint_print(const quic_endpoint_t *ep);

/**
 * \brief      Print a QUIC endpoint to a string. The output is always
 *             null-terminated unless size is zero.
 *
 * \param str  The string to write to.
 * \param size The max number of characters to write.
 * \param ep   A pointer to the QUIC endpoint to print.
 * \return     Returns the number of characters needed for the output
 *             excluding the ending null-terminator or negative if an
 *             error occurred.
 */
int  quic_endpoint_snprint(char *str, size_t size,
                           const quic_endpoint_t *ep);



/**
 * \brief      Check if a QUIC endpoint is secure (encrypted).
 *
 * \param ep   A pointer to a QUIC endpoint.
 * \return     Returns non-zero if the endpoint is secure and zero otherwise.
 */
int quic_endpoint_is_secure(const quic_endpoint_t *ep);

/**
 * \brief      Check if a QUIC endpoint is connected.
 *
 * \param ep   A pointer to a QUIC endpoint.
 * \return     Returns non-zero if the endpoint is connected and zero otherwise.
 */
int quic_endpoint_is_connected(const quic_endpoint_t *ep);

/**
 * \brief      Request a connection to a QUIC endpoint.
 *
 * \param ep   A pointer to a QUIC endpoint.
 * \return     Returns zero if an error occured and non-zero otherwise.
 */
int quic_endpoint_connect(quic_endpoint_t *ep);

/**
 * \brief      Request that any connection to a QUIC endpoint is discontinued.
 *
 * \param ep   A pointer to a QUIC endpoint.
 */
void quic_endpoint_disconnect(quic_endpoint_t *ep);

#endif /* quic_ENDPOINT_H_ */
/** @} */
