/*
 * Code de demarrage en C 
 * Recharge GDT et IDT 
 *
 */


/*========================================================================
 * Includes 
 *========================================================================*/


#include "klib.h"
#include "tables.h"
#include "start.h"


/*========================================================================
 * Macros 
 *========================================================================*/


#define SET_VALUE(_i,_x)                            \
    {                                               \
        frame_array[(_i)>>1] &= 0xF0 >> (4*((_i)&0x1));      \
        frame_array[(_i)>>1] |= (_x) << (4*((_i)&0x1));     \
    }
    
#define GET_VALUE(_i)                            \
        ( ((_i)&0x1) ? frame_array[(_i)>>1]>>4 : frame_array[(_i)>>1] & 0xF )



/*========================================================================
 * Declarations Private 
 *========================================================================*/


PRIVATE u8_t frame_array[START_FRAME_MAX>>1];
PRIVATE u8_t start_e820_check(struct boot_info* bootinfo);
PRIVATE u8_t start_e820_sanitize(struct boot_info* bootinfo);
PRIVATE u8_t start_e801_generate(struct boot_info* bootinfo);
PRIVATE u8_t start_e88_generate(struct boot_info* bootinfo);



/*========================================================================
 * Fonction start_main
 *========================================================================*/


PUBLIC void start_main(struct boot_info* binfo)
{ 
  /* Recopie les informations de demarrage */
  bootinfo = binfo;

  /* Genere ou corrige un memory map */
  if (bootinfo->mem_map_count)
    {
      
      if (start_e820_check(bootinfo) != EXIT_SUCCESS)
	{
	  goto err_mem;
	}

    }
  else if ( (bootinfo->mem_upper)&&(bootinfo->mem_lower) )
    {      
      if (start_e801_generate(bootinfo) != EXIT_SUCCESS)
	{
	  goto err_mem;
	}
    }
  else if ( (bootinfo->mem_0x0)&&(bootinfo->mem_0x100000) )
    {
      if (start_e88_generate(bootinfo) != EXIT_SUCCESS)
	{
	  goto err_mem;
	}
    }
  else
    {
      goto err_mem;
    }


  /* Initialise les tables du mode protege */
  if ( (gdt_init() != EXIT_SUCCESS)||(idt_init() != EXIT_SUCCESS) ) 
    {
      goto err_tables;
    }
  klib_bochs_print("GDT & IDT initialized\n");

  return;

 err_mem:
  klib_bochs_print("Memory Error ! Aborting...\n");

 err_tables:
  klib_bochs_print("GDT & IDT Error\n");

  while(1){}
  return;
}



/************************************************
 * Verifie le memory map e820 (limites, overlap) 
 ************************************************/


PRIVATE u8_t start_e820_check(struct boot_info* bootinfo)
{
  u16_t i,j;
  u64_t mem=0;
  u8_t overlap = 0;
  struct boot_mmap_e820* entry;
  struct boot_mmap_e820* next_entry;

  /* Calcul la taille de la memoire  */
  for(entry=(struct boot_mmap_e820*)bootinfo->mem_map_addr,i=0;i<bootinfo->mem_map_count;i++,entry++)
    {
      /* Quitte si 64bits */
      if (entry->addr_h || entry->size_h)
        {
	  return EXIT_FAILURE;
        }
      
      mem += entry->size_l;
    }
  
  /* Retourne selon la taille memoire totale */
  if ( mem > (u32_t)(-1) )
    {
      return EXIT_FAILURE;
    }
  
  /* Affecte la memoire calculee */
  bootinfo->mem_total = (u32_t)mem;

  /* Detection d'overlap */
  for(entry=(struct boot_mmap_e820*)bootinfo->mem_map_addr,i=0;(i<bootinfo->mem_map_count-1)&&(!overlap);i++,entry++)
    {
      for(next_entry=entry+1,j=i+1;j<bootinfo->mem_map_count;j++,next_entry++)
	{
	  if ( ((next_entry->addr_l < entry->addr_l)&&(entry->addr_l < next_entry->addr_l+next_entry->size_l))
	       || ((next_entry->addr_l < entry->addr_l)&&(entry->addr_l < next_entry->addr_l+next_entry->size_l)) )
	    {
	      overlap = 1;
	      break;
	    }
	}
    }
  
  return  (overlap ? start_e820_sanitize(bootinfo) : EXIT_SUCCESS);
}

    
/******************************
 * Assainit le memory map e820 
 ******************************/
   
  
PRIVATE u8_t start_e820_sanitize(struct boot_info* bootinfo)
{     
  u32_t frame_size=0;
  u32_t i,j;
  struct boot_mmap_e820* entry;
  
  /* Taille des frames */
  while(frame_size<<START_SHIFT < bootinfo->mem_total)
    {
      frame_size++;
      
      /* Cas de la comparaison avec 4G */
      if (frame_size > 1048576)
        {
	  frame_size = 1048576;
	  break;
        }
      
    }
  
  /* Construit le frame_array */
  for(entry=(struct boot_mmap_e820*)bootinfo->mem_map_addr,i=0;i<bootinfo->mem_map_count;i++,entry++)
    {
      
      for(j=entry->addr_l;j<entry->addr_l+entry->size_l;j+=frame_size)
        {
	  
	  if (GET_VALUE(j/frame_size) < entry->type)
            {
	      SET_VALUE(j/frame_size,entry->type);
            }
	  
        }
      
    }
  
  /* Modifie le mmap... */

  entry=(struct boot_mmap_e820*)bootinfo->mem_map_addr;
  
  /* Initialise le premier item */
  bootinfo->mem_map_count=1;
  entry->addr_l = 0;
  entry->size_l = frame_size;
  entry->type = GET_VALUE(0);
  
  /* Parcourt le frame_array tout en creant une entree a chaque changement de type */
  for(j=1;j<START_FRAME_MAX;j++)
    {
      
      if (GET_VALUE(j)!=GET_VALUE(j-1))
	{
	  bootinfo->mem_map_count++;
	  if (bootinfo->mem_map_count > START_E820_MAX)
	    {
	      return EXIT_FAILURE;
	    }
	  
	  entry++;
	  entry->size_l = 0;
	  entry->addr_l = j*frame_size;
	  entry->type = GET_VALUE(j);
          
	}
      
      entry->size_l += frame_size;
      
    }
  
  /* Recalcul de la memoire totale */
  bootinfo->mem_total = 0;
  for(entry=(struct boot_mmap_e820*)bootinfo->mem_map_addr,i=0;i<bootinfo->mem_map_count;i++,entry++)
    {
      if (entry->type)
	{
	  bootinfo->mem_total+=entry->size_l;
	}
    }
  
  
  return EXIT_SUCCESS;
}



/************************************************
 * Genere un memory map depuis int 0x15 ax=0e801 
 ************************************************/


PRIVATE u8_t start_e801_generate(struct boot_info* bootinfo)
{
  struct boot_mmap_e820* entry;
  
  /* Taille totale */
  bootinfo->mem_total = START_MEM_SIZE_0 + (bootinfo->mem_lower << 10) + (bootinfo->mem_upper << 16);
  
  /* Cree un faux memory map */
  bootinfo->mem_map_count = 3;
  
  entry = (struct boot_mmap_e820*)bootinfo->mem_map_addr;
  entry->addr_l = 0;
  entry->size_l = CONST_ROM_AREA_START;
  entry->addr_h = 0;
  entry->size_h = 0;
  entry->type = START_E820_AVAILABLE;
  
  entry = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+sizeof(struct boot_mmap_e820));
  entry->addr_l = 0x100000;
  entry->size_l = bootinfo->mem_lower << 10;
  entry->addr_h = 0;
  entry->size_h = 0;
  entry->type = START_E820_AVAILABLE;
  
  entry = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+2*sizeof(struct boot_mmap_e820));
  entry->addr_l = 0x1000000;
  entry->size_l = bootinfo->mem_upper << 16;
  entry->addr_h = 0;
  entry->size_h = 0;
  entry->type = START_E820_AVAILABLE;
  
  return EXIT_SUCCESS;
}



/************************************************
 * Genere un memory map depuis int 0x15 ax=0e88 
 ************************************************/


PRIVATE u8_t start_e88_generate(struct boot_info* bootinfo)
{
  struct boot_mmap_e820* entry;
  
  /* Taille totale */
  bootinfo->mem_total = (bootinfo->mem_0x0 + bootinfo->mem_0x100000) << 10;
  
  /* Cree un faux memory map */
  bootinfo->mem_map_count = 2;
  
  entry = (struct boot_mmap_e820*)bootinfo->mem_map_addr;
  entry->addr_l = 0;
  entry->size_l = (bootinfo->mem_0x0 < CONST_ROM_AREA_START ? bootinfo->mem_0x0 : CONST_ROM_AREA_START) ;
  entry->addr_h = 0;
  entry->size_h = 0;
  entry->type = START_E820_AVAILABLE;
  
  entry = (struct boot_mmap_e820*)(bootinfo->mem_map_addr+sizeof(struct boot_mmap_e820));
  entry->addr_l = 0x100000;
  entry->size_l = bootinfo->mem_0x100000 << 10;
  entry->addr_h = 0;
  entry->size_h = 0;
  entry->type = START_E820_AVAILABLE;
  
  return EXIT_SUCCESS;
}
