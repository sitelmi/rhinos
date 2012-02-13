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
#include "thread.h"
#include "sched.h"
#include "irq.h"
#include "pit.h"
#include <ipc.h>

void titi(char c)
{
  char t[2];
  struct ipc_message m;

  t[0]=c;
  t[1]=0;

  m.len = 2;
  m.data = "o";

  while(1)
    {
      u32_t i=0;
      ipc_send(2,&m);
      //klib_bochs_print(t);
      while(i < (1<<19))
	{
	  i++;
	}
      
    }

  return;
}

void toto(char c)
{
  char t[2];
  struct ipc_message* m;

  t[0]=c;
  t[1]=0;

  while(1)
    {
      u32_t i=0;
      //klib_bochs_print(t);
      ipc_receive(IPC_ANY,&m);
      klib_bochs_print("Message len: %d",m->len);
      klib_bochs_print("Message data: ");
      klib_bochs_print((char*)m->data);
      while(i < (1<<19))
	{
	  i++;
	}
    }

  return;
}


void tata(char c)
{
  char t[2];
  int j=1048;
  struct ipc_message m;

  t[0]=c;
  t[1]=0;

  m.len = 2;
  m.data = "a";

  while(j)
    {
      u32_t i=0;
      ipc_send(2, &m);
      //klib_bochs_print(t);
      while(i < (1<<19))
	{
	  i++;
	}
      j--;
    }
  klib_bochs_print(" [Quit....] ");
  return;
}


/*========================================================================
 * Fonction principale 
 *========================================================================*/


PUBLIC int main()
{
  struct thread* thread_idle;

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
  klib_bochs_print("Kernel Contexts initialized\n");

  /* Initialisation des thread */
  thread_init();
  klib_bochs_print("Kernel Threads initialized\n");

  /* Initialisation de l ordonannceur */
  sched_init();
  klib_bochs_print("Scheduler initialized\n");


  /* Idle Thread */
  thread_idle = thread_create("[Idle]",THREAD_ID_DEFAULT,(virtaddr_t)klib_idle,NULL,THREAD_STACK_SIZE,THREAD_NICE_TOP,THREAD_QUANTUM_DEFAULT);
  ASSERT_FATAL( thread_idle != NULL );
  klib_bochs_print("Idle thread initialized\n");

  /* Tests threads */

  struct thread* to;
  struct thread* ti;
  struct thread* ta;


  to = thread_create("Toto_thread",THREAD_ID_DEFAULT,(virtaddr_t)toto,(void*)'1',THREAD_STACK_SIZE,THREAD_NICE_DEFAULT-5,THREAD_QUANTUM_DEFAULT);
  ti = thread_create("Titi_thread",THREAD_ID_DEFAULT,(virtaddr_t)titi,(void*)'2',THREAD_STACK_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);
  ta = thread_create("Tata_thread",THREAD_ID_DEFAULT,(virtaddr_t)tata,(void*)'3',THREAD_STACK_SIZE,THREAD_NICE_DEFAULT,THREAD_QUANTUM_DEFAULT);

  /* Simule un ordonnancement */
  sched_dequeue(SCHED_READY_QUEUE,ta);
  sched_enqueue(SCHED_RUNNING_QUEUE,ta);
  

  /* Initialisation du gestionnaire des IRQ */
  irq_init();
  
  /* Initialisation Horloge */
  pit_init();
  klib_bochs_print("Clock initialized (100Hz)\n");

   /* On ne doit plus arriver ici (sauf DEBUG) */
  while(1)
    {
    }

  return EXIT_SUCCESS;
}
