/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef H_L2CAP_PRIV_
#define H_L2CAP_PRIV_

#include "ble_l2cap_coc_priv.h"
#include "host/ble_l2cap.h"
#include <inttypes.h>
#include "stats/stats.h"
#include "os/queue.h"
#include "os/os_mbuf.h"
#ifdef __cplusplus
extern "C" {
#endif

struct ble_hs_conn;
struct hci_data_hdr;

STATS_SECT_START(ble_l2cap_stats)
    STATS_SECT_ENTRY(chan_create)
    STATS_SECT_ENTRY(chan_delete)
    STATS_SECT_ENTRY(update_init)
    STATS_SECT_ENTRY(update_rx)
    STATS_SECT_ENTRY(update_fail)
    STATS_SECT_ENTRY(proc_timeout)
    STATS_SECT_ENTRY(sig_tx)
    STATS_SECT_ENTRY(sig_rx)
    STATS_SECT_ENTRY(sm_tx)
    STATS_SECT_ENTRY(sm_rx)
STATS_SECT_END
extern STATS_SECT_DECL(ble_l2cap_stats) ble_l2cap_stats;

extern struct os_mempool ble_l2cap_chan_pool;

#define BLE_L2CAP_CID_ATT           4
#define BLE_L2CAP_CID_SIG           5
#define BLE_L2CAP_CID_SM            6

/* This is nimble specific; packets sent to the black hole CID do not elicit
 * an "invalid CID" response.
 */
#define BLE_L2CAP_CID_BLACK_HOLE    0xffff

#define BLE_L2CAP_HDR_SZ    4

typedef uint8_t ble_l2cap_chan_flags;

typedef int ble_l2cap_rx_fn(struct ble_l2cap_chan *chan, struct os_mbuf **rxom);

struct ble_l2cap_chan {
    SLIST_ENTRY(ble_l2cap_chan) next;
    uint16_t conn_handle;
    uint16_t dcid;
    uint16_t scid;
    uint16_t my_mtu;
    uint16_t peer_mtu;      /* 0 if not exchanged. */
    ble_l2cap_chan_flags flags;

    struct os_mbuf *rx_buf;
    uint16_t rx_len;        /* Length of current reassembled rx packet. */

    ble_l2cap_rx_fn *rx_fn;

#if MYNEWT_VAL(BLE_L2CAP_COC_MAX_NUM) != 0
    uint16_t psm;
    struct ble_l2cap_coc_endpoint coc_rx;
    struct ble_l2cap_coc_endpoint coc_tx;
    ble_l2cap_event_fn *cb;
    void *cb_arg;
#endif
};

struct ble_l2cap_hdr {
    uint16_t len;
    uint16_t cid;
};

typedef int ble_l2cap_tx_fn(struct ble_hs_conn *conn,
                            struct ble_l2cap_chan *chan);

#define BLE_L2CAP_CHAN_F_TXED_MTU       0x01    /* We have sent our MTU. */

SLIST_HEAD(ble_l2cap_chan_list, ble_l2cap_chan);

int ble_l2cap_parse_hdr(struct os_mbuf *om, int off,
                        struct ble_l2cap_hdr *l2cap_hdr);
struct os_mbuf *ble_l2cap_prepend_hdr(struct os_mbuf *om, uint16_t cid,
                                      uint16_t len);

struct ble_l2cap_chan *ble_l2cap_chan_alloc(uint16_t conn_handle);
void ble_l2cap_chan_free(struct ble_l2cap_chan *chan);

bool ble_l2cap_is_mtu_req_sent(const struct ble_l2cap_chan *chan);

int ble_l2cap_rx(struct ble_hs_conn *conn,
                 struct hci_data_hdr *hci_hdr,
                 struct os_mbuf *om,
                 ble_l2cap_rx_fn **out_rx_cb,
                 struct os_mbuf **out_rx_buf,
                 int *out_reject_cid);
int ble_l2cap_tx(struct ble_hs_conn *conn, struct ble_l2cap_chan *chan,
                 struct os_mbuf *txom);

void ble_l2cap_forget_rx(struct ble_hs_conn *conn, struct ble_l2cap_chan *chan);
int ble_l2cap_init(void);

#ifdef __cplusplus
}
#endif

#endif
