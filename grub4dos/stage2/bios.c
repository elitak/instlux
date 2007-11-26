/* bios.c - implement C part of low-level BIOS disk input and output */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2003,2004  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "shared.h"
#include "iso9660.h"


/* These are defined in asm.S, and never be used elsewhere, so declare the
   prototypes here.  */
extern int biosdisk_standard (int ah, int drive,
			      int coff, int hoff, int soff,
			      int nsec, int segment);
extern int get_diskinfo_standard (int drive,
				  unsigned long *cylinders,
				  unsigned long *heads,
				  unsigned long *sectors);
#if 0
extern int get_diskinfo_floppy (int drive,
				unsigned long *cylinders,
				unsigned long *heads,
				unsigned long *sectors);
#endif


/* Read/write NSEC sectors starting from SECTOR in DRIVE disk with GEOMETRY
   from/into SEGMENT segment. If READ is BIOSDISK_READ, then read it,
   else if READ is BIOSDISK_WRITE, then write it. If an geometry error
   occurs, return BIOSDISK_ERROR_GEOMETRY, and if other error occurs, then
   return the error number. Otherwise, return 0.  */
int
biosdisk (int read, int drive, struct geometry *geometry,
	  int sector, int nsec, int segment)
{
  int err;
  unsigned long base;
  
  /* first, use EBIOS if possible */
  if (geometry->flags & BIOSDISK_FLAG_LBA_EXTENSION)
    {
      struct disk_address_packet
      {
	unsigned char length;
	unsigned char reserved;
	unsigned short blocks;
	unsigned long buffer;
	unsigned long long block;
	
	/* This structure is passed in the stack. A buggy BIOS could write
	 * garbage data to the tail of the struct and hang the machine. So
	 * we need this protection. - Tinybit
	 */
	unsigned char dummy[16];
      } __attribute__ ((packed)) *dap;

      /* Even the above protection is not enough to avoid stupid actions by
       * buggy BIOSes. So we do it in the 0040:0000 segment. - Tinybit
       */
      dap = (struct disk_address_packet *)0x580;

      if (drive == 0xffff || (drive == ram_drive && rd_base != 0xffffffff))
      {
	err = 0;
	
	base = (drive == 0xffff) ? 0 : rd_base;
	
	if (nsec > 0 && nsec < 0x80)
	{
		/* the mem address is (segment << 4), the 'disk' LBA is
		 * (sector), so its address is (sector << 9) */
	  if (read)
	    /* read == 1, this really means write to 'disk' */
	    grub_memmove ((char*)((sector << 9) + base), (char*)(segment << 4), nsec << 9);
	  else
	    /* read == 0, this really means read from 'disk' */
	    grub_memmove ((char*)(segment << 4), (char*)((sector << 9) + base), nsec << 9);
		
	}else{
		err = 1;
	}
	
	return err;
      }

      /* XXX: Don't check the geometry by default, because some buggy
	 BIOSes don't return the number of total sectors correctly,
	 even if they have working LBA support. Hell.  */
#ifdef NO_BUGGY_BIOS_IN_THE_WORLD
      if (sector >= geometry->total_sectors)
	return BIOSDISK_ERROR_GEOMETRY;
#endif /* NO_BUGGY_BIOS_IN_THE_WORLD */

      /* FIXME: sizeof (DAP) must be 0x10. Should assert that the compiler
	 can't add any padding.  */
      //dap.length = sizeof (dap);
      dap->length = 0x10;	/* Yes. This should be safe. - Tinybit */
      dap->block = sector;
      dap->blocks = nsec;
      dap->reserved = 0;
      /* This is undocumented part. The address is formated in
	 SEGMENT:ADDRESS.  */
      dap->buffer = segment << 16;
      
      err = biosdisk_int13_extensions ((read + 0x42) << 8, drive, dap);

      /* return on success */
      if (! err)
	return err;

      /* bootable CD-ROM specification has no standard CHS-mode call */
      if (geometry->flags & BIOSDISK_FLAG_CDROM)
      {
#ifndef STAGE1_5
	if (debug > 1)
	  grub_printf ("biosdisk_int13_extensions read=%d, drive=0x%x, dap=%x, err=0x%x\n", read, drive, dap, err);
#endif
	return err;
      }
	  
/* #undef NO_INT13_FALLBACK */
#ifdef NO_INT13_FALLBACK
      return err;
#else
#if 0
	  geometry->flags &= ~BIOSDISK_FLAG_LBA_EXTENSION;
	  geometry->total_sectors = (geometry->cylinders
				     * geometry->heads
				     * geometry->sectors);
	  return biosdisk (read, drive, geometry, sector, nsec, segment);
#else
      /* A failure on EBIOS shouldn't deny all subsequent EBIOS accesses.
       * So we keep the BIOSDISK_FLAG_LBA_EXTENSION flag and simply try
       * standard BIOS call. -- Tinybit
       */

     /* nothing need to be done but fall through! */
#endif
#endif /* ! NO_INT13_FALLBACK */
      
    } /* if (geometry->flags & BIOSDISK_FLAG_LBA_EXTENSION) */

   /* try the standard CHS mode */
  
    {
      int cylinder_offset, head_offset, sector_offset;
      int head;

      /* SECTOR_OFFSET is counted from one, while HEAD_OFFSET and
	 CYLINDER_OFFSET are counted from zero.  */
      sector_offset = sector % geometry->sectors + 1;
      head = sector / geometry->sectors;
      head_offset = head % geometry->heads;
      cylinder_offset = head / geometry->heads;
      
      if (cylinder_offset >= geometry->cylinders)
	return BIOSDISK_ERROR_GEOMETRY;

      err = biosdisk_standard (read + 0x02, drive,
			       cylinder_offset, head_offset, sector_offset,
			       nsec, segment);
    }

  return err;
}

/* Check bootable CD-ROM emulation status.  
 * Return 0 on failure. 
 */
int
get_cdinfo (int drive, struct geometry *geometry)
{
  int err;
  struct iso_spec_packet
  {
    unsigned char size;
    unsigned char media_type;
    unsigned char drive_no;
    unsigned char controller_no;
    unsigned long image_lba;
    unsigned short device_spec;
    unsigned short cache_seg;
    unsigned short load_seg;
    unsigned short length_sec512;
    unsigned char cylinders;
    unsigned char sectors;
    unsigned char heads;
    
    unsigned char dummy[16];
  } __attribute__ ((packed));
  
  struct iso_spec_packet *cdrp;
  
  cdrp = (struct iso_spec_packet *)0x580;
  grub_memset (cdrp, 0, sizeof (struct iso_spec_packet));
  cdrp->size = sizeof (struct iso_spec_packet) - 16;
#ifndef STAGE1_5
  if (debug > 1)
	grub_printf (" int13/4B01(%X),", drive);
#endif
  err = biosdisk_int13_extensions (0x4B01, drive, cdrp);
#ifndef STAGE1_5
  if (debug > 1)
	grub_printf ("err=%X,drive=%X, ", err, drive);
#endif

  if (drive == 0x7F && drive < cdrp->drive_no)
	drive = cdrp->drive_no;

  if (! err && cdrp->drive_no == drive && !(cdrp->media_type & 0x0F))
    {
//    if ((cdrp.media_type & 0x0F) == 0)
//	{
          /* No-emulation mode bootable CD-ROM */
          geometry->flags = BIOSDISK_FLAG_LBA_EXTENSION | BIOSDISK_FLAG_CDROM;
          geometry->cylinders = 65536; // 0;
          geometry->heads = 255; //1;
          geometry->sectors = 15;
          geometry->sector_size = 2048;
          geometry->total_sectors = 65536 * 255 * 15; //MAXINT;
	  return drive;
//	}
//    else
//	{
//	  /* Floppy or hard-disk emulation */
//	  geometry->cylinders
//	    = ((unsigned int) cdrp.cylinders
//	       + (((unsigned int) (cdrp.sectors & 0xC0)) << 2));
//	  geometry->heads = cdrp.heads;
//	  geometry->sectors = cdrp.sectors & 0x3F;
//	  geometry->sector_size = SECTOR_SIZE;
//	  geometry->total_sectors = (geometry->cylinders
//				     * geometry->heads
//				     * geometry->sectors);
//	  return -1;
//	}
    }
  return 0;	/* failure */
}

/* Return the geometry of DRIVE in GEOMETRY. If an error occurs, return
   non-zero, otherwise zero.  */
int
get_diskinfo (int drive, struct geometry *geometry)
{
  int err;
  int version;
  unsigned long long total_sectors = 0, tmp = 0;
  unsigned long flags;
      
  struct drive_parameters *drp;

  drp = (struct drive_parameters *)0x580;

  if (drive == 0xffff)	/* memory disk */
    {
      unsigned long long total_mem_bytes;

      total_mem_bytes = 0;

      if (mbi.flags & MB_INFO_MEM_MAP)
        {
          struct AddrRangeDesc *map = (struct AddrRangeDesc *) mbi.mmap_addr;
          unsigned long end_addr = mbi.mmap_addr + mbi.mmap_length;

          for (; end_addr > (unsigned long) map; map = (struct AddrRangeDesc *) (((int) map) + 4 + map->size))
	    {
	      unsigned long long top_end;

	      if (map->Type != MB_ARD_MEMORY)
		  continue;
	      top_end =  map->BaseAddr + map->Length;
	      if (top_end > 0x100000000ULL)
		  top_end = 0x100000000ULL;
	      if (total_mem_bytes < top_end)
		  total_mem_bytes = top_end;

	    }
        }
      else
	  grub_printf ("Address Map BIOS Interface is not activated.\n");

      if (total_mem_bytes)
      {
	geometry->flags = BIOSDISK_FLAG_LBA_EXTENSION;
	geometry->sector_size = SECTOR_SIZE;
	geometry->total_sectors = (total_mem_bytes /*+ SECTOR_SIZE - 1*/) >> SECTOR_BITS;
	geometry->heads = 255;
	geometry->sectors = 63;
	geometry->cylinders = (geometry->total_sectors + 255 * 63 -1) / (255 * 63);
	return 0;
      }
      
    } else if (drive == ram_drive)	/* ram disk device */
    {
      if (rd_base != 0xffffffff)
      {
	geometry->flags = BIOSDISK_FLAG_LBA_EXTENSION;
	geometry->sector_size = SECTOR_SIZE;
	geometry->total_sectors = (rd_size ? ((rd_size + SECTOR_SIZE - 1)>> SECTOR_BITS) : 0x800000);
	geometry->heads = 255;
	geometry->sectors = 63;
	geometry->cylinders = (geometry->total_sectors + 255 * 63 -1) / (255 * 63);
	return 0;
      }
    }

#if defined(GRUB_UTIL) || defined(STAGE1_5)
  if (drive == cdrom_drive)
#else
  if (drive == cdrom_drive || (drive >= min_cdrom_id && drive < min_cdrom_id + atapi_dev_count))
#endif
  {
	/* No-emulation mode bootable CD-ROM */
	geometry->flags = BIOSDISK_FLAG_LBA_EXTENSION | BIOSDISK_FLAG_CDROM;
	geometry->cylinders = 65536;
	geometry->heads = 255;
	geometry->sectors = 15;
	geometry->sector_size = 2048;
	geometry->total_sectors = 65536 * 255 * 15;
	return 0;
  }

  /* Clear the flags.  */
  flags = 0;
  
#ifdef GRUB_UTIL
#define FIND_DRIVES 8
#else
#define FIND_DRIVES (*((char *)0x475))
#endif
      if (drive >= 0x80 + FIND_DRIVES /* || (version && (drive & 0x80)) */ )
#undef FIND_DRIVES
	{
	  /* Possible CD-ROM - check the status.  */
	  if (get_cdinfo (drive, geometry))
	    return 0;
	}
      
#ifndef STAGE1_5
      if (debug > 1)      
	grub_printf (" int13/41(%X),", drive);
#endif
      version = check_int13_extensions (drive);
#ifndef STAGE1_5
      if (debug > 1)      
	grub_printf ("version=%X, ", version);
#endif

//      err = 1;
      
      /* It is safe to clear out DRP.  */
      grub_memset (drp, 0, sizeof (struct drive_parameters));

      /* Buggy KT133A(AWARD BIOS 6.00PG) does not return valid version.
       * So we don't check version for now. - Tinybit
       */

      /* PhoenixBIOS 4.0 Revision 6.0 for ZF Micro might understand the
	 greater buffer size for the "get drive parameters" int 13 call in 
	 its own way.  Supposedly the BIOS assumes even bigger space is
	 available and thus corrupts the stack. This is why we specify the  
	 exactly necessary size of 0x42 bytes. */
      drp->size = sizeof (struct drive_parameters) - 16;
	  
#ifndef STAGE1_5
	  if (drive & 0x80)
	  if (debug > 1)
		grub_printf (" int13/48(%X),", drive);
#endif
	  if (drive & 0x80)
	    err = biosdisk_int13_extensions (0x4800, drive, drp);
	  else
	    err = 0;
#ifndef STAGE1_5
	  if (drive & 0x80)
	  if (debug > 1)
		grub_printf ("err=%X, C/H/S=%d/%d/%d, Sector Count/Size=%d/%d, ", err, drp->cylinders, drp->heads, drp->sectors, drp->total_sectors, drp->bytes_per_sector);
#endif
	  if (! err)
	    {
	      /* Set the LBA flag.  */
	      if (version & 1) /* support functions 42h-44h, 47h-48h */
	      {
		flags = BIOSDISK_FLAG_LBA_EXTENSION;
	      
	        /* Set the CDROM flag.  */
		if (drp->bytes_per_sector == ISO_SECTOR_SIZE)
		  flags |= BIOSDISK_FLAG_CDROM;
	      }

	      /* I'm not sure if GRUB should check the bit 1 of DRP.FLAGS,
		 so I omit the check for now. - okuji  */
	      /* if (drp->flags & (1 << 1)) */
	       
	      /* FIXME: when the 2TB limit becomes critical, we must
		 change the type of TOTAL_SECTORS to unsigned long
		 long.  */

#if 1
	      //total_sectors = drp->cylinders * drp->heads * drp->sectors;
	      //if (total_sectors < drp->total_sectors)
		  total_sectors = drp->total_sectors;
#else
	      if (total_sectors < drp->total_sectors)
		  total_sectors = drp->total_sectors /* & ~0L */;
	      else
		  /* Some buggy BIOSes doesn't return the total sectors
		     correctly but returns zero. So if it is zero, compute
		     it by C/H/S returned by the LBA BIOS call.  */
		  total_sectors = drp->cylinders * drp->heads * drp->sectors;
#endif
	    }


#ifndef STAGE1_5
	if (debug > 1)
		grub_printf (" int13/08(%X),", drive);
#endif
	/* Don't pass GEOMETRY directly, but pass each element instead,
		 so that we can change the structure easily.  */
	version = get_diskinfo_standard (drive, &geometry->cylinders, &geometry->heads, &geometry->sectors);
#ifndef STAGE1_5
	if (debug > 1)
		grub_printf ("version=%X, C/H/S=%d/%d/%d, ", version, geometry->cylinders, geometry->heads, geometry->sectors);
#endif
	if (version && err)
		return err; /* When we return with ERROR, we should not change the geometry!! */
      
	geometry->flags = flags;
	geometry->sector_size = (drp->bytes_per_sector ? drp->bytes_per_sector : SECTOR_SIZE);
	if (geometry->cylinders < drp->cylinders)
	    geometry->cylinders = drp->cylinders;
	if (geometry->heads < drp->heads)
	    geometry->heads = drp->heads;
	if (geometry->sectors < drp->sectors)
	    geometry->sectors = drp->sectors;
	if (geometry->heads > 256)
	    geometry->heads = 256;
	if (geometry->sectors * geometry->sector_size > 63 * 512)
	    geometry->sectors = 63 * 512 / geometry->sector_size;
	tmp = (unsigned long long)(geometry->cylinders) *
	      (unsigned long long)(geometry->heads) *
	      (unsigned long long)(geometry->sectors);
	if (total_sectors < tmp)
	    total_sectors = tmp;
	geometry->total_sectors = total_sectors;

#ifndef STAGE1_5
	if (geometry->sector_size != SECTOR_SIZE) /* CD */
		return 0;

	/* workaround for buggy USB-bootable board QDI 848E.
	 * try a further probe in the boot sector.
	 */
#ifndef STAGE1_5
	if (debug > 1)
		grub_printf (" int13/02(%X),", drive);
#endif
	/* read the boot sector: int 13, AX=0x201, CX=1, DH=0 */
	err = biosdisk_standard (0x02, drive, 0, 0, 1, 1, SCRATCHSEG);
#ifndef STAGE1_5
	if (debug > 1)
		grub_printf ("err=%X, ", err);
#endif
	if (err)
	{
		/* try again using LBA */
		if (geometry->flags & BIOSDISK_FLAG_LBA_EXTENSION)
		{
			struct disk_address_packet
			{
				unsigned char length;
				unsigned char reserved;
				unsigned short blocks;
				unsigned long buffer;
				unsigned long long block;

				unsigned char dummy[16];
			} __attribute__ ((packed)) *dap;

			dap = (struct disk_address_packet *)0x580;

			dap->length = 0x10;
			dap->reserved = 0;
			dap->blocks = 1;
			dap->buffer = SCRATCHSEG << 16;
			dap->block = 0;

			err = biosdisk_int13_extensions (0x4200, drive, dap);
		} /* if (geometry->flags & BIOSDISK_FLAG_LBA_EXTENSION) */
	}

	if (err)
		goto failure_read_boot_sector;

	/* successfully read boot sector */

	if (drive & 0x80)
	{
		/* hard disk */
		if ((err = probe_mbr((struct master_and_dos_boot_sector *)SCRATCHADDR, 0, total_sectors, 0)))
		{
			if (debug > 0)
				grub_printf ("\nWarning: Unrecognized partition table for drive %X. Please rebuild it using\na Microsoft-compatible FDISK tool(err=%d). Current C/H/S=%d/%d/%d\n", drive, err, geometry->cylinders, geometry->heads, geometry->sectors);
			goto failure_probe_boot_sector;
		}
		err = (int)"MBR";
	}else{
		/* floppy */
		if (probe_bpb((struct master_and_dos_boot_sector *)SCRATCHADDR))
			goto failure_probe_boot_sector;
		err = (int)"BPB";
	}

	if (drive & 0x80)
	if (probed_cylinders != geometry->cylinders)
	    if (debug > 1)
		grub_printf ("\nWarning: %s cylinders(%d) is not equal to the BIOS one(%d).\n", (char *)err, probed_cylinders, geometry->cylinders);

	geometry->cylinders = probed_cylinders;

	if (probed_heads != geometry->heads)
	    if (debug > 1)
		grub_printf ("\nWarning: %s heads(%d) is not equal to the BIOS one(%d).\n", (char *)err, probed_heads, geometry->heads);

	geometry->heads	= probed_heads;

	if (probed_sectors_per_track != geometry->sectors)
	    if (debug > 1)
		grub_printf ("\nWarning: %s sectors per track(%d) is not equal to the BIOS one(%d).\n", (char *)err, probed_sectors_per_track, geometry->sectors);

	geometry->sectors = probed_sectors_per_track;

	if (probed_total_sectors > total_sectors)
	{
	    if (drive & 0x80)
	    if (debug > 1)
		grub_printf ("\nWarning: %s total sectors(%d) is greater than the BIOS one(%d).\nSome buggy BIOSes could hang when you access sectors exceeding the BIOS limit.\n", (char *)err, probed_total_sectors, total_sectors);
	    geometry->total_sectors	= probed_total_sectors;
	}

	if (drive & 0x80)
	if (probed_total_sectors < total_sectors)
	    if (debug > 1)
		grub_printf ("\nWarning: %s total sectors(%d) is less than the BIOS one(%d).\n", (char *)err, probed_total_sectors, total_sectors);

	goto failure_read_boot_sector;	/* not a real failure, but to skip the `failure_probe_boot_sector' code. */

failure_probe_boot_sector:
	/* for floppy, try INT13/AH=48h here. This could hang on some buggy USB BIOSes. */
	if ((drive & 0x80) == 0)
	{
		/* It is safe to clear out DRP.  */
		grub_memset (drp, 0, sizeof (struct drive_parameters));
		drp->size = sizeof (struct drive_parameters) - 16;

#ifndef STAGE1_5
		if (debug > 0)
			grub_printf ("\nBPB geometry probe failed, so try int13/48(%X), but this could hang on buggy USB BIOSes...", drive);
#endif
		err = biosdisk_int13_extensions (0x4800, drive, drp);
#ifndef STAGE1_5
		if (debug > 0)
			grub_printf (" err=%X, C/H/S=%d/%d/%d, Sector Count/Size=%d/%d.\n", err, drp->cylinders, drp->heads, drp->sectors, drp->total_sectors, drp->bytes_per_sector);
#endif
		if (drp->heads > 0 && drp->heads <= 256 && drp->sectors > 0 && drp->sectors <= 63)
		{
			geometry->sectors = drp->sectors;
			geometry->heads = drp->heads;
			geometry->cylinders = drp->cylinders;
			geometry->total_sectors = drp->cylinders * drp->heads * drp->sectors;
		}
		if (geometry->total_sectors < drp->total_sectors)
		    geometry->total_sectors = drp->total_sectors;

	}

failure_read_boot_sector:
	/* if C/H/S=0/0/0, use a safe default one. */
	if (geometry->sectors == 0)
	{
		if (drive & 0x80)
		{
			/* hard disk */
			geometry->sectors = 63;
		}else{
			/* floppy */
			if (geometry->total_sectors > 5760)
				geometry->sectors = 63;
			else if (geometry->total_sectors > 2880)
				geometry->sectors = 36;
			else
				geometry->sectors = 18;
		}
	}
	if (geometry->heads == 0)
	{
		if (drive & 0x80)
		{
			/* hard disk */
			geometry->heads = 255;
		}else{
			/* floppy */
			if (geometry->total_sectors > 5760)
				geometry->heads = 255;
			else
				geometry->heads = 2;
		}
	}
	if (geometry->cylinders == 0)
	{
		geometry->cylinders = (geometry->total_sectors / geometry->heads / geometry->sectors);
	}
#endif

  return 0;
}

