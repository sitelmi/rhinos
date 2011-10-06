/*
 * RhinOS Main
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/

#include <types.h>
#include "const.h"
#include <llist.h>
#include "start.h"
#include "klib.h"
#include "assert.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem.h"
#include "context_cpu.h"
#include "irq.h"
#include "pit.h"


struct context_cpu* ctx_toto;
struct context_cpu* ctx_titi;

void toto(char* s)
{
  klib_bochs_print(s);
  context_cpu_switch_to(ctx_titi);
  return;
}


void titi(char* s)
{
  klib_bochs_print(s);
  context_cpu_switch_to(kern_ctx);
  return;
}



/*========================================================================
 * Fonction principale 
 *========================================================================*/

PUBLIC int main()
{

  /* Initialisation de la memoire physique */
  phys_init();
  klib_bochs_print("Physical Memory Manager initialized\n");

  /* Initialisation de la pagination */
  paging_init();
  klib_bochs_print("Paging enabled\n");

  /* Initialisation de la memoire virtuelle */
  virt_init();
  klib_bochs_print("Virtual Memory Manager initialized\n");

  /* Initialisation des contextes */
  context_cpu_init();
  klib_bochs_print("Kernel Context initialized\n");

  /* Initialisation du gestionnaire des IRQ */
  irq_init();

  /* Initialisation Horloge */
  pit_init();
  klib_bochs_print("Clock initialized (100Hz)\n");

  
  virtaddr_t stack_toto;
  virtaddr_t stack_titi;
 

  stack_toto = (virtaddr_t)virt_alloc(4096);
  stack_titi = (virtaddr_t)virt_alloc(4096);

  ctx_toto = context_cpu_create((virtaddr_t)toto,(void*)"coucou \n",stack_toto,4096);
  ctx_titi = context_cpu_create((virtaddr_t)titi,(void*)"le monde \n",stack_titi,4096);

  context_cpu_switch_to(ctx_toto);

  klib_bochs_print("Back in main\n");

 
  /* On ne doit plus arriver ici (sauf DEBUG) */
  while(1)
    {
    }

  return EXIT_SUCCESS;
}
