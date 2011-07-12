/*
 * Virtmem_buddy.c
 * Allocateur de memoire virtuelle (>=4096)
 *
 */


#include <types.h>
#include <wmalloc.h>
#include "klib.h"
#include "start.h"
#include "physmem.h"
#include "paging.h"
#include "virtmem_buddy.h"


/***********************
 * Declarations PRIVATE
 ***********************/

/* DEBUG: WaterMArk Allocator */
PRIVATE struct virt_buddy_wm_alloc virt_wm;


/*********************************
 * Initialisation de l'allocateur
 *********************************/

PUBLIC void  virtmem_buddy_init()
{
  /* DEBUG: Initialise le WaterMark */
  WMALLOC_INIT(virt_wm,PAGING_ALIGN_SUP(PHYS_PAGE_NODE_POOL_ADDR+((bootinfo->mem_size) >> PHYS_PAGE_SHIFT)*sizeof(struct ppage_desc)),(1<<31));
  return;
}


/**************
 * Allocation
 **************/

PUBLIC void* virtmem_buddy_alloc(u32_t size, u8_t flags)
{
  virtaddr_t vaddr,vaddr2;

  /* DEBUG: Allocation via WaterMark */
  vaddr = vaddr2 = (virtaddr_t)WMALLOC_ALLOC(virt_wm,size);

  /* DEBUG: Mappage sur pages physiques */
  if (flags & VIRT_BUDDY_MAP)
    {
      u32_t i,n;
  
      /* DEBUG: Nombre de pages */
      n = ((size&0xFFFFF000) == size)?(size >> PAGING_PAGE_SHIFT) :((size >> PAGING_PAGE_SHIFT)+1);
      
      /* DEBUG: Mapping */
      for(i=0;i<n;i++)
	{
	  physaddr_t paddr;

	  paddr = (u32_t)phys_alloc(PAGING_PAGE_SIZE);
	  paging_map(vaddr2,paddr,TRUE);
	  vaddr2+=PAGING_PAGE_SIZE;

	}
      
    }

  return (void*)vaddr;

}


/*************
 * Liberation
 *************/

PUBLIC void  virtmem_buddy_free(void* addr)
{
  /* DEBUG: Liberation via WaterMark */
  WMALLOC_FREE(virt_wm,addr);
  return;
}
