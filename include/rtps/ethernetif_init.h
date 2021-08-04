#ifdef _RTPS_CALLBACK_H_
#define _RTPS_CALLBACK_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "lwip.h"

extern err_t rtps_ethernetif_init(struct netif *netif);

#ifdef __cplusplus
}
#endif

#endif /* _RTPS_CALLBACK_H_ */
