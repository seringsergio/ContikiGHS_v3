/*-------------------------------------------------------------------*/
/*---------------- INCLUDES -----------------------------------------*/
/*-------------------------------------------------------------------*/
#include "ghs_co_i.h"
/*-------------------------------------------------------------------*/
/*---------------- FUNCIONES-----------------------------------------*/
/*-------------------------------------------------------------------*/

node nd; //nd es node....n es neighbor


/* FUNCION que imprime el resultado final final
*/

void print_final_result()
{

    edges *e_aux;
    char string[] = "END";
    for(e_aux = e_list_head_g; e_aux != NULL; e_aux = e_aux->next) // Recorrer toda la lista
    {
        if(linkaddr_cmp(&e_aux->addr, &nd.parent)) //Solo muestro mi padre
        {
            printf("%s %d %d %d.%02d %d %d.%02d \n",
            string,
            linkaddr_node_addr.u8[0],
            nd.parent.u8[0],
            (int)(e_aux->weight / SEQNO_EWMA_UNITY),
            (int)(((100UL * e_aux->weight) / SEQNO_EWMA_UNITY) % 100),
            e_aux->state,
            (int)(nd.f.name / SEQNO_EWMA_UNITY),
            (int)(((100UL * nd.f.name) / SEQNO_EWMA_UNITY) % 100));
       }
   }
}


/* Toma la informacion de la lista de vecinos (neighbors_list) del master_neighbor_discovery
*  y copia la informacion de interes en una nueva lista de edges edges_list.
*/
void fill_edges_list(list_t edges_list, struct memb *edges_memb, struct neighbor *n_list_head)
{
    struct neighbor *n_aux;
    edges *e;
    for(n_aux = n_list_head; n_aux != NULL; n_aux = list_item_next(n_aux)) // Recorrer toda la lista
    {
        e = memb_alloc(edges_memb);        // we allocate a new struct edges from the edges_memb memory pool.

        e -> state  = BASIC; //Todos los edges inician con estado BASIC
        linkaddr_copy(&e->addr,  &n_aux->addr);
        e -> weight = n_aux -> avg_seqno_gap;

        list_add(edges_list, e); //Agregarlo a la lista
    }

}


/* Imprime la lista de edges
*/
void print_edges_list(edges *e_list_head, char *string,  const linkaddr_t *node_addr)
{
    edges *e_aux = NULL;

    for(e_aux = e_list_head; e_aux != NULL; e_aux = list_item_next(e_aux)) // Recorrer toda la lista
    {
        printf("%s %d %d %d.%02d %d \n",
              string,
              node_addr->u8[0],
              e_aux->addr.u8[0],
              (int)(e_aux->weight / SEQNO_EWMA_UNITY),
              (int)(((100UL * e_aux->weight) / SEQNO_EWMA_UNITY) % 100),
              e_aux->state
              );
    }
}

/* Un edge pasa de estado BASIC a BRANCH.
*  Become_branch = Vuelve branch un edge
*/
void become_branch(edges *e_list_head, const linkaddr_t *node_addr)
{
    edges *e_aux;
    for(e_aux = e_list_head; e_aux != NULL; e_aux = list_item_next(e_aux)) // Recorrer toda la lista
    {
        if(linkaddr_cmp(&e_aux->addr, node_addr)) //Entra si las direcciones son iguales
        {
            e_aux->state = BRANCH;
            //nd.num_branches = nd.num_branches + 1;
            break;
        }
    }
}

/* Funcion que devuelve el numero de hijos
*/
uint8_t num_hijos(edges *e_list_head)
{
    uint8_t numero_hijos = 0;

    edges *e_aux;
    for(e_aux = e_list_head; e_aux != NULL; e_aux = list_item_next(e_aux)) // Recorrer toda la lista
    {
        if(e_aux->state == BRANCH) //Entra si las direcciones son iguales
        {
            numero_hijos = numero_hijos + 1;
        }
    }

    if(nd.flags & CORE_NODE)
    {
        return (numero_hijos); //aca el padre tambien es hijo al mismo tiempo
    }else
    {
        return (numero_hijos - 1); //menos la branch del padre
    }
}

/* Devuelve un apuntador al basic edge (su addr) que tenga menor peso.
* Least_basic_edge = Encuentra el basic edge de menor peso.
* (Lista ya ordenada en master_neighbor_discovery)
*/
linkaddr_t* least_basic_edge(edges *e_list_head)
{
    edges *e_aux;
    for(e_aux = e_list_head; e_aux != NULL; e_aux = list_item_next(e_aux)) // Recorrer toda la lista
    {
        //tener en cuenta que la lista de edges ya esta
        //ordenada de menor a mayor desde el proceso
        //master_neighbor_discovery. Entonces solo necesito
        //evaluar que el edge sea basic
        if(e_aux->state == BASIC)
        {
            break;
        }
    }
    return &e_aux->addr;
}

/*Funcion para retornar el peso del edge
*/
uint32_t weight_with_edge(const linkaddr_t *addr,  edges *e_list_head)
{
    edges *e_aux;
    for(e_aux = e_list_head; e_aux != NULL; e_aux = list_item_next(e_aux)) // Recorrer toda la lista
    {
        if(linkaddr_cmp(addr, &e_aux->addr )) //Entra si las direcciones son iguales
        {
            break;
        }
    }
    return (e_aux->weight);
}
/*Funcion para saber si el estado de un edge es branch. Se busca por addr
*/
uint8_t state_is_branch(const linkaddr_t *addr,  edges *e_list_head)
{
    edges *e_aux;
    for(e_aux = e_list_head; e_aux != NULL; e_aux = list_item_next(e_aux)) // Recorrer toda la lista
    {
        if(linkaddr_cmp(addr, &e_aux->addr )) //Entra si las direcciones son iguales
        {
            break;
        }
    }
    if(e_aux->state == BRANCH)
    {
        return 1;
    }else
    {
        return 0;
    }
}

/* Hace la inicializacion del proceso master_co_i
*/
void init_master_co_i(struct neighbor *n_list_head, struct memb *edges_memb, list_t edges_list)
{
    printf("Process Init: master_co_i \n");

    //Variables locales
    linkaddr_t *lwoe_init; //LWOE inicial. Es el edge con menor weight
    char string[] = "READ";

    //Inicializacion de Variables globales
    nd.flags = 0;
    nd.f.name = 0;
    nd.f.level = 0;
    nd.lwoe.node.weight = INFINITO;
    nd.lwoe.children.weight = INFINITO;
    nd.num_branches = 0;
    linkaddr_copy(&nd.otro_core_node, &linkaddr_node_addr); //otro core node soy YO

    //Tomar info de master_neighbor_discovery
    fill_edges_list(edges_list, edges_memb, n_list_head );

    // llenar la variable global con la cabeza de la lista
    e_list_head_g = list_head(edges_list);

    // Vuelve Branch el basic edge con menor peso
    lwoe_init = least_basic_edge(list_head(edges_list));

    //Setear LWOE del nodo
    linkaddr_copy(&nd.lwoe.node.neighbor, lwoe_init);
    nd.lwoe.node.weight = return_weight( list_head(edges_list), lwoe_init);
    /*nd.flags |= ND_LWOE; //Ya encontre el ND_LWOE
    process_post(e_LWOE, PROCESS_EVENT_CONTINUE, NULL);*/

    //imprimir la info que tome de fill_edges_list y guarde en edges_list
    print_edges_list(list_head(edges_list), string, &linkaddr_node_addr);

}
/* LLena un msg de initiate con los valores parametros
*/
void llenar_initiate_msg(initiate_msg *i_msg, uint32_t name,
                        uint8_t level, uint8_t state, const linkaddr_t *dest, uint8_t flags)
{
    i_msg->f.name     = name;
    i_msg->f.level    = level;
    i_msg->nd_state   = state;
    linkaddr_copy(&i_msg->destination , dest);

    if( !(flags & BECOME_CORE_NODE))
    {
        i_msg->flags     &= ~BECOME_CORE_NODE;
    }else
    if(flags & BECOME_CORE_NODE)
    {
        i_msg->flags     |= BECOME_CORE_NODE;
    }
}
/* LLena un msg de connect con los valores parametros
*/
void llenar_connect_msg (connect_msg *msg, uint8_t level, linkaddr_t *destination)
{
    msg->level = level;
    linkaddr_copy(&msg->destination,  destination);
}

/*---------------------------------------------------------------------------*/
/* Funcion que recibe un mensaje de runicast: Guarda en history_list los vecinos que
* han enviado msg y su seq. Si el avg_seqno_gap del vecino es
*  mayor, entonces reemplazo mi avg_seqno_gap.
*/
void ghs_co_i_recv_ruc(void *msg, const linkaddr_t *from,
                    struct memb *history_mem, list_t history_list, uint8_t seqno,
                    list_t co_list, struct memb *co_mem, struct process *evaluar_msg_co,
                    list_t i_list, struct memb *i_mem, struct process *evaluar_msg_i,
                    struct process *evaluar_msg_test)

{
    // OPTIONAL: Sender history
    struct history_entry *e = NULL;

    for(e = list_head(history_list); e != NULL; e = e->next) {
      if(linkaddr_cmp(&e->addr, from)) { // Si las dir son iguales entra
        break;
      }
    }
    if(e == NULL) {
      // Create new history entry
      e = memb_alloc(history_mem);
      if(e == NULL) {
        e = list_chop(history_list); /* Remove oldest at full history */
      }
      linkaddr_copy(&e->addr, from);
      e->seq = seqno;
      list_push(history_list, e);
    } else {
      // Detect duplicate callback
      if(e->seq == seqno) {
        printf("runicast message received from %d.%d, seqno %d (DUPLICATE)\n",
  	     from->u8[0], from->u8[1], seqno);
        return;
      }
      // Update existing history entry
      e->seq = seqno;
    }

        //Leer el packet buffer attribute: Especificamente el tipo de mensaje
        packetbuf_attr_t msg_type = packetbuf_attr(PACKETBUF_ATTR_PACKET_GHS_TYPE_MSG);

        // Evaluo el tipo de msg que llego
        if(msg_type == CONNECT)
        {
            connect_list *co_list_p;

            co_list_p = memb_alloc(co_mem); //Alocar memoria
            if(co_list_p == NULL)
            {
                printf("ERROR: La lista de msg de connect esta llena\n");
            }else
            {
                co_list_p->co_msg = *((connect_msg *)msg); //msg le hago cast.Luego cojo todo el msg
                linkaddr_copy(&co_list_p->from, from);
                list_push(co_list, co_list_p); //Add an item to the start of the list.
                process_post(evaluar_msg_co, PROCESS_EVENT_CONTINUE, NULL);
            }

        }else
        if(msg_type == INITIATE)
        {

            initiate_list *i_list_p;
            i_list_p = memb_alloc(i_mem); //Alocar memoria
            if(i_list_p == NULL)
            {
               printf("ERROR: La lista de msg de initiate esta llena\n");
            }else
            {
               i_list_p->i_msg = *((initiate_msg *)msg); //msg le hago cast.Luego cojo todo el msg
               linkaddr_copy(&i_list_p->from, from);
               list_push(i_list, i_list_p); //Add an item to the start of the list.
               process_post(evaluar_msg_i, PROCESS_EVENT_CONTINUE, NULL);

               //LLamar al proceso para que evalue el pospone agregado o actualizado
               // Se hace aca porque el INITIATE es quien cambia el level del fragmento
               process_post(evaluar_msg_co, PROCESS_EVENT_CONTINUE, NULL);
               process_post(evaluar_msg_test, PROCESS_EVENT_CONTINUE, NULL ) ;
            }

        } //END if msg es INITIATE




} //END runicas receive
