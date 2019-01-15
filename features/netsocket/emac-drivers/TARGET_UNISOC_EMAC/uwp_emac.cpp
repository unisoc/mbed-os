#include <stdlib.h>

#include "mbed_shared_queues.h"
#include "netsocket/nsapi_types.h"
#include "lwip/arch.h"
#include "debug.h"
#include "uwp_emac.h"
#include "uwp_wifi_main.h"
#include "uwp_sipc.h"
#include "uwp_ipi.h"
#include "uwp_sys_wrapper.h"


// TODO:need confirm
#define UWP_HWADDR_SIZE         (6)
#define UWP_ETH_MTU_SIZE        1500
#define UWP_ETH_IF_NAME         "st"

#define RX_PRIORITY   (osPriorityNormal)
#define TX_PRIORITY   (osPriorityNormal)
#define PHY_PRIORITY  (osPriorityNormal)

extern void *packet_rx_queue;

UWP_EMAC::UWP_EMAC()
{
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the memory buffer chain that is passed to the function.
 *
 * @param buf the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return true if the packet could be sent
 *         false value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
bool UWP_EMAC::link_out(emac_mem_buf_t *buf)
{
#if 1
    emac_mem_buf_t *q, *p = buf;

    u32_t actual_txlen = 0;
    u8_t *data;
	//struct pbuf *p_init = (struct pbuf *)buf;
    LWIP_DEBUGF(NETIF_DEBUG, ("low_level_output enter, p:%08x\n", p));

#if ETH_PAD_SIZE
    //pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    data = (u8_t *)malloc(1530);
    //printf("TXBUFF:%p\r\n",data);

    if(data == NULL){
        LWIP_DEBUGF(NETIF_DEBUG, ("rda91h_low_level_output, no PKT buf\r\n"));
		memory_manager->free(buf);//SYS_ARCH_DEC(p_init->ref, 1);
        return false;
    }

    for(q = p; q != NULL; q = memory_manager->get_next(q))
    {
        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
        memcpy(data+16, memory_manager->get_ptr(q), memory_manager->get_len(q));//reserve wid header length
        actual_txlen += memory_manager->get_len(q);
        if(actual_txlen > 1514 || actual_txlen > memory_manager->get_total_len(p))
        {
            LWIP_DEBUGF(NETIF_DEBUG, ("low_level_output err, actual_txlen:%d, tot_len%d\n", actual_txlen, memory_manager->get_total_len(p)));
			memory_manager->free(buf);//SYS_ARCH_DEC(p_init->ref, 1);
			return false;
        }
    }

    /* Signal rda5996 that packet should be sent */
    if(actual_txlen == memory_manager->get_total_len(p))
    {
        uwp_mgmt_tx(data,actual_txlen);
#if ETH_PAD_SIZE
        //pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

        //LINK_STATS_INC(link.xmit);
	    memory_manager->free(buf);//SYS_ARCH_DEC(p_init->ref, 1);
        return true;
    }

    LWIP_DEBUGF(NETIF_DEBUG, ("low_level_output pkt len mismatch, actual_txlen:%d, tot_len%d\n",
            actual_txlen, memory_manager->get_total_len(p)));

	memory_manager->free(buf);//SYS_ARCH_DEC(p_init->ref, 1);
#endif
    return false;
}

/**
 * Should allocate a contiguous memory buffer and transfer the bytes of the incoming
 * packet to the buffer.
 *
 * @param buf If a frame was received and the memory buffer allocation was successful, a memory
 *            buffer filled with the received packet (including MAC header)
 * @return negative value when no more frames,
 *         zero when frame is received
 */
emac_mem_buf_t * UWP_EMAC::low_level_input(uint8_t *data, int len)
{
    emac_mem_buf_t *p, *q;
    u16_t index = 0;

    LWIP_DEBUGF(NETIF_DEBUG, ("low_level_input enter, rxlen:%d\n", len));

    /* Obtain the size of the packet and put it into the "len"
     variable. */
    if(!len)
    {
        return NULL;
    }

#if ETH_PAD_SIZE
    //len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

    /* We allocate a pbuf chain of pbufs from the pool. */
    p = memory_manager->alloc_pool(len, 0);
    if (p != NULL)
    {

#if ETH_PAD_SIZE
        //pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

        /* We iterate over the pbuf chain until we have read the entire
         * packet into the pbuf. */
        for(q = p; q != NULL; q = memory_manager->get_next(q))
        {
            /* Read enough bytes to fill this pbuf in the chain. The
            * available data in the pbuf is given by the q->len
            * variable.
            * This does not necessarily have to be a memcpy, you can also preallocate
            * pbufs for a DMA-enabled MAC and after receiving truncate it to the
            * actually received size. In this case, ensure the tot_len member of the
            * pbuf is the sum of the chained pbuf len members.
            */
            /* load rx data from 96 to local mem_pool */
            memcpy(memory_manager->get_ptr(q), &data[index], memory_manager->get_len(q));
            index += memory_manager->get_len(q);

            if(index >= len)
            {
                break;
            }
        }

        /* acknowledge rda5996 that packet has been read(); */
        //Nothing to do here

#if ETH_PAD_SIZE
        //pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

        //LINK_STATS_INC(link.recv);
    }
    else
    {
        /* Drop this packet */
        LWIP_DEBUGF(NETIF_DEBUG, ("low_level_input pbuf_alloc fail, rxlen:%d\n", len));
        //LINK_STATS_INC(link.memerr);
        //LINK_STATS_INC(link.drop);

        return NULL;
    }
    return p;
}


/** \brief  Attempt to read a packet from the EMAC interface.
 *
 */
void UWP_EMAC::packet_rx()
{
    //printf("%s\r\n",__func__);
#if 1
    uwp_wifi_msg_t msg = NULL;
    packet_rx_queue = k_msg_create(10);
    /* move received packet into a new buf */
    while (1) {
        emac_mem_buf_t *p = NULL;
        k_msg_get(packet_rx_queue, &msg, osWaitForever);
        //printf("rx type:%d\r\n",msg->type);
        switch(msg->type)
        {       
            case 0:
                p = low_level_input((unsigned char*)msg->arg1, msg->arg2);
                if (p == NULL) {
                    //rda_sem_release((void*)msg->arg3);
                    break;
                }
                //rda_sem_release((void*)msg->arg3);
                if (p) {
                    //printf("free RXBUFF:%p\r\n",msg->arg1-msg->arg3);
                    free((void *)(msg->arg1 - msg->arg3));
                    emac_link_input_cb(p);
                }
                break;
            case 1:
                emac_link_state_cb(msg->arg1);
                break;
            default:
                break;
        }
        free(msg);
    }
#endif
}

void UWP_EMAC::thread_function(void *pvParameters)
{
    printf("%s\r\n",__func__);
#if 1
    static struct UWP_EMAC *uwp_enet = static_cast<UWP_EMAC *>(pvParameters);
    uwp_enet->packet_rx();
#endif
}

bool UWP_EMAC::power_up()
{
    int ret;
    printf("%s\r\n",__func__);
	ret = uwp_cp_init();
    k_thread_create("packet_rx", UWP_EMAC::thread_function, this, NULL, DEFAULT_THREAD_STACKSIZE*5, PHY_PRIORITY);
    return (ret == 0);
}

uint32_t UWP_EMAC::get_mtu_size() const
{
    return UWP_ETH_MTU_SIZE;
}

uint32_t UWP_EMAC::get_align_preference() const
{
    return 0;
}

void UWP_EMAC::get_ifname(char *name, uint8_t size) const
{
    memcpy(name, UWP_ETH_IF_NAME, (size < sizeof(UWP_ETH_IF_NAME)) ? size : sizeof(UWP_ETH_IF_NAME));
}

uint8_t UWP_EMAC::get_hwaddr_size() const
{
    return UWP_HWADDR_SIZE;
}

bool UWP_EMAC::get_hwaddr(uint8_t *addr) const
{
    uwp_mgmt_getmac(addr);
    return true;
}

void UWP_EMAC::set_hwaddr(const uint8_t *addr)
{
    /* No-op at this stage */
}

void UWP_EMAC::set_link_input_cb(emac_link_input_cb_t input_cb)
{
    emac_link_input_cb = input_cb;
}

void UWP_EMAC::set_link_state_cb(emac_link_state_change_cb_t state_cb)
{
    emac_link_state_cb = state_cb;
}

void UWP_EMAC::add_multicast_group(const uint8_t *addr)
{
    /* No-op at this stage */
}

void UWP_EMAC::remove_multicast_group(const uint8_t *addr)
{
    /* No-op at this stage */
}

void UWP_EMAC::set_all_multicast(bool all)
{
    /* No-op at this stage */
}

void UWP_EMAC::power_down()
{
    /* No-op at this stage */
}

void UWP_EMAC::set_memory_manager(EMACMemoryManager &mem_mngr)
{
    memory_manager = &mem_mngr;
}

UWP_EMAC &UWP_EMAC::get_instance()
{
    static UWP_EMAC emac;
    return emac;
}

// Weak so a module can override
MBED_WEAK EMAC &EMAC::get_default_instance()
{
    return UWP_EMAC::get_instance();
}

