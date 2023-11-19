
#include "system.h"
#include "noc_if.h"

#ifndef NOC_VC_H
#define NOC_VC_H

// maximum NoC virtual channel buffer size
#define NOC_VC_BUF_N_BITS 3
#define NOC_VC_BUF_SIZE (1 << NOC_VC_BUF_N_BITS)
#define NOC_VC_PTR_MASK (NOC_VC_BUF_SIZE - 1)

struct noc_vc_fifo_t {
    noc_data_t data;
    noc_link_ctrl_t link_ctrl;
    noc_dir_e pkt_dir;
};

/**
 * Virtual channel FIFO buffer in a NoC router.
 */
class noc_vc {

    public:

        /** Constructor. */
        noc_vc(bool is_dummy = true);

        /** Determine whether the VC is full. */
        bool is_full();

        /**
         * Add an element to the queue.
         *
         * @param data      The data packet.
         * @param link_ctrl The link control packet.
         * @param pkt_dir   The desired direction of the packet.
         * @retval Whether the item was added. Returns false if the FIFO is full.
         */
        bool enqueue(noc_data_t data, noc_link_ctrl_t link_ctrl, noc_dir_e pkt_dir);

        /**
         * Remove an element from the queue.
         *
         * @param data      The data packet.
         * @param link_ctrl The link control packet.
         * @param pkt_dir   The desired direction of the packet.
         * @retval Whether an item was read. Returns false if the FIFO is empty.
         */
        bool dequeue(noc_data_t& data, noc_link_ctrl_t& link_ctrl, noc_dir_e& pkt_dir);

        /**
         * Peek an element from the queue.
         *
         * @param data      The data packet.
         * @param link_ctrl The link control packet.
         * @param pkt_dir   The desired direction of the packet.
         * @retval Whether an item was read. Returns false if the FIFO is empty.
         */
        bool peek(noc_data_t& data, noc_link_ctrl_t& link_ctrl, noc_dir_e& pkt_dir);

    private:

        /** FIFO buffer. */
        noc_vc_fifo_t *_fifo_buf;

        /** FIFO pointers. */
        int32_t _head;
        int32_t _tail;

};

#endif
