/*
 * Header de la pagination
 *
 */

#ifndef PAGING_H
#define PAGING_H

/*========================================================================
 * Includes
 *========================================================================*/

#include <types.h>
#include "const.h"


/*========================================================================
 * Constantes
 *========================================================================*/

#define PAGING_ENTRIES      1024
#define PAGING_DIRSHIFT     22
#define PAGING_TBLSHIFT     12
#define PAGING_TBLMASK      0x3FF
#define PAGING_OFFMASK      0xFFF
#define PAGING_BASESHIFT    12

#define PAGING_SELFMAP      0x3FF


/*========================================================================
 * Structures
 *========================================================================*/

/* Page directory entry */

PUBLIC struct pde
{
  u32_t present   :1  ;  /* Page presente en memoire  */
  u32_t rw        :1  ;  /* Page en lecture/ecriture  */
  u32_t user      :1  ;  /* Privilege User/Supervisor */
  u32_t pwt       :1  ;  /* Write through             */
  u32_t pcd       :1  ;  /* Cache disable             */
  u32_t accessed  :1  ;  /* Page accedee (ro ou rw)   */ 
  u32_t zero      :1  ;  /* Zero                      */
  u32_t pagesize  :1  ;  /* Taille des pages          */
  u32_t global    :1  ;  /* Page globale              */
  u32_t available :3  ;  /* Bits disponibles          */ 
  u32_t baseaddr  :20 ;  /* Adresse physique          */
}__attribute__ ((packed));


/* Page table entry */

PUBLIC struct pte
{
  u32_t present   :1  ;
  u32_t rw        :1  ;
  u32_t user      :1  ;
  u32_t pwt       :1  ;
  u32_t pcd       :1  ;
  u32_t accessed  :1  ;
  u32_t dirty     :1  ;  /* Page ecrite */
  u32_t zero      :1  ;
  u32_t global    :1  ;
  u32_t available :3  ;
  u32_t baseaddr  :20 ;
}__attribute__ ((packed));


/*========================================================================
 * Macros
 *========================================================================*/

/* Directory d'une adresse virtuelle */
#define PAGING_GET_PDE(__addr)				\
  ( (__addr) >> PAGING_DIRSHIFT )

/* Table d'une adresse virtuelle */
#define PAGING_GET_PTE(__addr)				\
  ( ((__addr) >> PAGING_TBLSHIFT)&PAGING_TBLMASK )

/* Alignement inferieur sur CONST_PAGE_SIZE */
#define PAGING_ALIGN_INF(__addr)			\
  ( ((__addr) >> CONST_PAGE_SHIFT) << CONST_PAGE_SHIFT )

/* Alignement superieur sur CONST_PAGE_SIZE */
#define PAGING_ALIGN_SUP(__addr)						\
  ( (((__addr)&0xFFFFF000) == (__addr))?((__addr) >> CONST_PAGE_SHIFT) << CONST_PAGE_SHIFT:(((__addr) >> CONST_PAGE_SHIFT)+1) << CONST_PAGE_SHIFT )

/* Self Mapping: Page Directory courant */
#define PAGING_GET_PD()							\
  ( (virtaddr_t)(PAGING_SELFMAP<<PAGING_DIRSHIFT) + (virtaddr_t)(PAGING_SELFMAP<<PAGING_TBLSHIFT) )

/* Self Mapping: Page Table i du PD courant */
#define PAGING_GET_PT(__i)						\
  ( (virtaddr_t)(PAGING_SELFMAP<<PAGING_DIRSHIFT) + (virtaddr_t)((__i)<<PAGING_TBLSHIFT) )


/*========================================================================
 * Prototypes
 *========================================================================*/

/* PD Noyau */
PUBLIC struct pde* kern_PD;

PUBLIC u8_t paging_init(void);
PUBLIC u8_t paging_map(virtaddr_t vaddr, physaddr_t paddr, u8_t super);
PUBLIC u8_t paging_unmap(virtaddr_t vaddr);
PUBLIC physaddr_t paging_virt2phys(virtaddr_t vaddr);

#endif
