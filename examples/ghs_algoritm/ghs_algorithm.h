#ifndef GHS_ALGORITHM_H
#define GHS_ALGORITHM_H

/*------------------------------------------------------------------- */
/*----------- INCLUDES ------------------------------------------------ */
/*------------------------------------------------------------------- */

#include "contiki.h"
#include "lib/list.h"

/*------------------------------------------------------------------- */
/*-----------VARIABLES GLOBALES-------------------------------------------------*/
/*------------------------------------------------------------------- */


/*------------------------------------------------------------------- */
/*-----------TYPEDEF-------------------------------------------------*/
/*------------------------------------------------------------------- */
typedef struct wait s_wait;
/*------------------------------------------------------------------- */
/*-----------PROCESOS-------------------------------------------------*/
/*------------------------------------------------------------------- */

//Procesos generales
PROCESS_NAME(wait);

//Procesos de neighbor_discovery
PROCESS_NAME(n_broadcast_neighbor_discovery);
PROCESS_NAME(n_link_weight_worst_case);
PROCESS_NAME(master_neighbor_discovery);
PROCESS_NAME(master_co_i);

//Procesos de test ar
PROCESS_NAME(send_message_co_i);
PROCESS_NAME(e_pospone_connect);
PROCESS_NAME(master_test_ar);
PROCESS_NAME(e_test);


/*------------------------------------------------------------------- */
/*----------EVENTOS -------- -----------------------------------------*/
/*------------------------------------------------------------------- */

//Comunes a todos los procesos
extern process_event_t e_wait_stabilization;
extern process_event_t e_infinite_wait;

//neighbor discovery
extern process_event_t e_discovery_broadcast;
extern process_event_t e_weight_worst;
extern process_event_t e_init_master_co_i;

// master find found
    //estados
extern process_event_t e_found;
extern process_event_t e_find;
    //msg
extern process_event_t e_msg_connect;
extern process_event_t e_msg_initiate;
extern process_event_t e_msg_test;
extern process_event_t e_msg_reject;
extern process_event_t e_msg_accept;
extern process_event_t e_msg_report;
extern process_event_t e_msg_change_root;

//master_test_ar
extern process_event_t e_init_master_test_ar;
extern process_event_t e_evaluate_test;

/*-------------------------------------------------------------------*/
/*---------------- Estructuras---------------------------------------*/
/*-------------------------------------------------------------------*/

struct wait
{
    uint8_t seconds;
    struct process *return_process;
};


#endif /* GHS_ALGORITHM_H */