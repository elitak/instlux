/* common.c - miscellaneous shared variables and routines */
/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2004  Free Software Foundation, Inc.
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

#include <shared.h>
#include <iso9660.h>

#ifdef SUPPORT_DISKLESS
# define GRUB	1
# include <etherboot.h>
#endif

/*
 *  Shared BIOS/boot data.
 */

struct multiboot_info mbi;
unsigned long saved_drive;
unsigned long saved_partition;
char saved_dir[256];
//unsigned long cdrom_drives[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
unsigned long cdrom_drive = GRUB_INVALID_DRIVE;
unsigned long force_cdrom_as_boot_device = 1;
unsigned long ram_drive;
unsigned long rd_base = 0;	/* Note the rd_base value of -1 invalidates the ram drive. */
unsigned long rd_size = 0;	/* The rd_size 0 stands for 4GB, not for length of 0. */
#ifndef STAGE1_5
unsigned long saved_mem_upper;

/* This saves the maximum size of extended memory (in KB).  */
unsigned long extended_memory;
#endif
int is64bit = 0;
int errorcheck = 1;

/*
 *  Error code stuff.
 */

grub_error_t errnum = ERR_NONE;

#ifndef STAGE1_5

char *err_list[] =
{
  [ERR_NONE] = 0,
  [ERR_BAD_ARGUMENT] = "Invalid argument",
  [ERR_BAD_FILENAME] = "Filename must be either an absolute pathname or blocklist",
  [ERR_BAD_FILETYPE] = "Bad file or directory type",
  [ERR_BAD_GZIP_DATA] = "Bad or corrupt data while decompressing file",
  [ERR_BAD_GZIP_HEADER] = "Bad or incompatible header in compressed file",
  [ERR_BAD_PART_TABLE] = "Partition table invalid or corrupt",
  [ERR_BAD_VERSION] = "Mismatched or corrupt version of stage1/stage2",
  [ERR_BELOW_1MB] = "Loading below 1MB is not supported",
  [ERR_BOOT_COMMAND] = "Kernel must be loaded before booting",
  [ERR_BOOT_FAILURE] = "Unknown boot failure",
  [ERR_BOOT_FEATURES] = "Unsupported Multiboot features requested",
  [ERR_DEV_FORMAT] = "Unrecognized device string",
  [ERR_DEV_NEED_INIT] = "Device not initialized yet",
  [ERR_DEV_VALUES] = "Invalid device requested",
  [ERR_EXEC_FORMAT] = "Invalid or unsupported executable format",
  [ERR_FILELENGTH] = "Filesystem compatibility error, cannot read whole file",
  [ERR_FILE_NOT_FOUND] = "File not found",
  [ERR_FSYS_CORRUPT] = "Inconsistent filesystem structure",
  [ERR_FSYS_MOUNT] = "Cannot mount selected partition",
  [ERR_GEOM] = "Selected cylinder exceeds maximum supported by BIOS",
  [ERR_HD_VOL_START_0] = "The BPB hidden_sectors should not be zero for a hard-disk partition boot sector",
  [ERR_IN_SITU_FLOPPY] = "Only hard drives could be mapped in situ.",
  [ERR_IN_SITU_MEM] = "Should not use --mem together with --in-situ.",
  [ERR_NEED_LX_KERNEL] = "Linux kernel must be loaded before initrd",
  [ERR_NEED_MB_KERNEL] = "Multiboot kernel must be loaded before modules",
  [ERR_NO_DISK] = "Selected disk does not exist",
  [ERR_NO_DISK_SPACE] = "No spare sectors on the disk",
  [ERR_NO_PART] = "No such partition",
  [ERR_NO_HEADS] = "The number of heads must be specified. The `--heads=0' option tells map to choose a value(but maybe unsuitable) for you",
  [ERR_NO_SECTORS] = "The number of sectors per track must be specified. The `--sectors-per-track=0' option tells map to choose a value(but maybe unsuitable) for you",
  [ERR_NON_CONTIGUOUS] = "File for drive emulation must be in one contiguous disk area",
  [ERR_NUMBER_OVERFLOW] = "Overflow while parsing number",
  [ERR_NUMBER_PARSING] = "Error while parsing number",
  [ERR_OUTSIDE_PART] = "Attempt to access block outside partition",
  [ERR_PRIVILEGED] = "Must be authenticated",
  [ERR_READ] = "Disk read error",
  [ERR_SYMLINK_LOOP] = "Too many symbolic links",
  [ERR_UNALIGNED] = "File is not sector aligned",
  [ERR_UNRECOGNIZED] = "Unrecognized command",
  [ERR_WONT_FIT] = "Selected item cannot fit into memory",
  [ERR_WRITE] = "Disk write error",
  [ERR_INT13_ON_HOOK] = "The int13 handler already on hook",
  [ERR_INT13_OFF_HOOK] = "The int13 handler not yet on hook",
  [ERR_NO_DRIVE_MAPPED] = "Refuse to hook int13 because of empty drive map table",
  [ERR_INVALID_HEADS] = "Invalid heads. Should be between 0 and 256(0 means auto)",
  [ERR_INVALID_SECTORS] = "Invalid sectors. Should be between 0 and 63(0 means auto)",
  [ERR_SPECIFY_GEOM] = "Should not specify geometry when mapping a whole drive or when emulating a hard disk with a logical partition",
  [ERR_EXTENDED_PARTITION] = "Extended partition table is invalid, or its CHS values conflict with the BPB in a logical partition",
  [ERR_DEL_MEM_DRIVE] = "You should delete other mem drive first, or use `--mem' option to force the deletion",
  [ERR_SPECIFY_MEM] = "Should not specify `--mem' when mapping a whole drive",
  [ERR_INVALID_FLOPPIES] = "Invalid floppies. Should be between 0 and 2",
  [ERR_INVALID_HARDDRIVES] = "Invalid harddrives. Should be between 0 and 127",
  [ERR_INVALID_LOAD_SEGMENT] = "Invalid load segment. Should be between 0 and 0x9FFF",
  [ERR_INVALID_LOAD_OFFSET] = "Invalid load offset. Should be between 0 and 0xFFFF",
  [ERR_INVALID_LOAD_LENGTH] = "Invalid load length. Should be between 512 and 0xA0000",
  [ERR_INVALID_SKIP_LENGTH] = "Invalid skip length. Should be non-negative and less than the file size",
  [ERR_INVALID_BOOT_CS] = "Invalid boot CS. Should be between 0 and 0xFFFF",
  [ERR_INVALID_BOOT_IP] = "Invalid boot IP. Should be between 0 and 0xFFFF",
  [ERR_INVALID_RAM_DRIVE] = "Invalid ram_drive. Should be between 0 and 254",
//  [ERR_INVALID_RD_BASE] = "Invalid rd_base. Should not be 0xffffffff",
//  [ERR_INVALID_RD_SIZE] = "Invalid rd_size. Should not be 0",
  [ERR_MD_BASE] = "When mapping whole mem device at a fixed location, you must specify --mem to a value > 0.",
  [ERR_RD_BASE] = "RD_BASE must be sector-aligned and non-zero for mapping at a fixed location",
  [ERR_DOS_BACKUP] = "GRUB was not booted from DOS, or the backup copy of DOS at physical\naddress 0x200000 is corrupt",
  [ERR_ENABLE_A20] = "Failed to turn on Gate A20!",
  [ERR_DISABLE_A20] = "Failed to turn off Gate A20!",
  [ERR_DEFAULT_FILE] = "Invalid DEFAULT file format. Please copy a valid DEFAULT file from the grub4dos release and try again. Also note that the DEFAULT file must be uncompressed.",
  [ERR_PARTITION_TABLE_FULL] = "Cannot use --in-situ because the partition table is full(i.e., all the 4 entries are in use).",

};


/* static for BIOS memory map fakery */
static struct AddrRangeDesc fakemap[3] =
{
  {20, 0, 0, MB_ARD_MEMORY},
  {20, 0x100000, 0, MB_ARD_MEMORY},
  {20, 0x1000000, 0, MB_ARD_MEMORY}
};

/* A big problem is that the memory areas aren't guaranteed to be:
   (1) contiguous, (2) sorted in ascending order, or (3) non-overlapping.
   Thus this kludge.  */
static unsigned long
mmap_avail_at (unsigned long bottom)
{
  unsigned long long top;
  unsigned long addr;
  int cont;
  
  top = bottom;
  do
    {
      for (cont = 0, addr = mbi.mmap_addr;
	   addr < mbi.mmap_addr + mbi.mmap_length;
	   addr += *((unsigned long *) addr) + 4)
	{
	  struct AddrRangeDesc *desc = (struct AddrRangeDesc *) addr;
	  
	  if (desc->Type == MB_ARD_MEMORY
	      && desc->BaseAddr <= top
	      && desc->BaseAddr + desc->Length > top)
	    {
	      top = desc->BaseAddr + desc->Length;
	      cont++;
	    }
	}
    }
  while (cont);

  /* For now, GRUB assumes 32bits addresses, so...  */
  if (top > 0xFFFFFFFF)
    top = 0xFFFFFFFF;
  
  return (unsigned long) top - bottom;
}
#endif /* ! STAGE1_5 */

/* This queries for BIOS information.  */
void
init_bios_info (void)
{
#ifndef STAGE1_5
  unsigned long cont, memtmp, addr;
  int drive;
#endif

  /*
   *  Get information from BIOS on installed RAM.
   */

  mbi.mem_lower = get_memsize (0);	/* int12 --------safe enough */
  mbi.mem_upper = get_memsize (1);	/* int15/88 -----safe enough */

#ifndef STAGE1_5
  /*
   *  We need to call this somewhere before trying to put data
   *  above 1 MB, since without calling it, address line 20 will be wired
   *  to 0.  Not too desirable.
   */

#ifndef GRUB_UTIL
  debug = debug_boot + 1;
  DEBUG_SLEEP
  printf("Turning on gate A20... ");
  if (gateA20 (1))			/* int15/24 -----safe enough */
	printf("Success.\n");
  else
	printf("Failure!\n");
  DEBUG_SLEEP
#endif

  /* Store the size of extended memory in EXTENDED_MEMORY, in order to
     tell it to non-Multiboot OSes.  */
  extended_memory = mbi.mem_upper;
  
  /*
   *  The "mbi.mem_upper" variable only recognizes upper memory in the
   *  first memory region.  If there are multiple memory regions,
   *  the rest are reported to a Multiboot-compliant OS, but otherwise
   *  unused by GRUB.
   */

  addr = get_code_end ();
  mbi.mmap_addr = addr;
  mbi.mmap_length = 0;
  cont = 0;

  do
    {
      cont = get_mmap_entry ((void *) addr, cont);	/* int15/e820 ------ will write memory! */

      /* If the returned buffer's length is zero, quit. */
      if (! *((unsigned long *) addr))
	break;

      mbi.mmap_length += *((unsigned long *) addr) + 4;
      addr += *((unsigned long *) addr) + 4;
    }
  while (cont);

  if (mbi.mmap_length)
    {
      unsigned long long max_addr;
      
      /*
       *  This is to get the lower memory, and upper memory (up to the
       *  first memory hole), into the "mbi.mem_{lower,upper}"
       *  elements.  This is for OS's that don't care about the memory
       *  map, but might care about total RAM available.
       */
      mbi.mem_lower = mmap_avail_at (0) >> 10;
      mbi.mem_upper = mmap_avail_at (0x100000) >> 10;

      /* Find the maximum available address. Ignore any memory holes.  */
      for (max_addr = 0, addr = mbi.mmap_addr;
	   addr < mbi.mmap_addr + mbi.mmap_length;
	   addr += *((unsigned long *) addr) + 4)
	{
	  struct AddrRangeDesc *desc = (struct AddrRangeDesc *) addr;
	  
	  if (desc->Type == MB_ARD_MEMORY && desc->Length > 0
	      && desc->BaseAddr + desc->Length > max_addr)
	    max_addr = desc->BaseAddr + desc->Length;
	}

      extended_memory = (max_addr - 0x100000) >> 10;
    }
  else if ((memtmp = get_eisamemsize ()) != -1)		/* int15/e801 ------safe enough */
    {
      cont = memtmp & ~0xFFFF;
      memtmp = memtmp & 0xFFFF;

      if (cont != 0)
	extended_memory = (cont >> 10) + 0x3c00;
      else
	extended_memory = memtmp;
      
      if (!cont || (memtmp == 0x3c00))
	memtmp += (cont >> 10);
      else
	{
	  /* XXX should I do this at all ??? */

	  mbi.mmap_addr = (unsigned long) fakemap;
	  mbi.mmap_length = sizeof (fakemap);
	  fakemap[0].Length = (mbi.mem_lower << 10);
	  fakemap[1].Length = (memtmp << 10);
	  fakemap[2].Length = cont;
	}

      mbi.mem_upper = memtmp;
    }

  saved_mem_upper = mbi.mem_upper;

#ifndef GRUB_UTIL
  is64bit = check_64bit ();
#endif

  /* Get the drive info.  */
  /* FIXME: This should be postponed until a Multiboot kernel actually
     requires it, because this could slow down the start-up
     unreasonably.  */
  mbi.drives_length = 0;
  mbi.drives_addr = addr;

  /* For now, GRUB doesn't probe floppies, since it is trivial to map
     floppy drives to BIOS drives.  */
#ifdef GRUB_UTIL
#define FIND_DRIVES 8
#else
#define FIND_DRIVES (*((char *)0x475))
#endif
  if (debug > 1)
	grub_printf ("hard drives: %d, int13: %X, int15: %X\n", FIND_DRIVES, *(unsigned long *)0x4C, *(unsigned long *)0x54);
  DEBUG_SLEEP
//#ifndef GRUB_UTIL
//  FIND_DRIVES = 1;
//#endif
  for (drive = 0x80; drive < 0x80 + FIND_DRIVES; drive++)
//  for (drive = 0x80 + FIND_DRIVES - 1; drive >= 0x80; drive--)
    {
#undef FIND_DRIVES
//      struct geometry tmp_geom;
      struct drive_info *info = (struct drive_info *) addr;
      unsigned short *port;
      
      /* Get the geometry. This ensures that the drive is present.  */
      if (debug > 1)
	grub_printf ("get_diskinfo(%X), ", drive);
      if (get_diskinfo (drive, &tmp_geom))
	break;
      if (debug > 1)
	grub_printf (" %sC/H/S=%d/%d/%d, Sector Count/Size=%d/%d\n",
		(tmp_geom.flags & BIOSDISK_FLAG_LBA_EXTENSION) ? "LBA, " : "",
		tmp_geom.cylinders, tmp_geom.heads, tmp_geom.sectors,
		tmp_geom.total_sectors, tmp_geom.sector_size);
      
      /* Clean out the I/O map.  */
      grub_memset ((char *) io_map, 0,
		   IO_MAP_SIZE * sizeof (unsigned short));

      /* Disable to probe I/O ports temporarily, because this doesn't
	 work with some BIOSes (maybe they are too buggy).  */
#if 0
      /* Track the int13 handler.  */
      track_int13 (drive);
#endif

      /* Set the information.  */
      info->drive_number = drive;
      info->drive_mode = ((tmp_geom.flags & BIOSDISK_FLAG_LBA_EXTENSION)
			  ? MB_DI_LBA_MODE : MB_DI_CHS_MODE);
      info->drive_cylinders = tmp_geom.cylinders;
      info->drive_heads = tmp_geom.heads;
      info->drive_sectors = tmp_geom.sectors;

      addr += sizeof (struct drive_info);
      for (port = io_map; *port; port++, addr += sizeof (unsigned short))
	*((unsigned short *) addr) = *port;

      info->size = addr - (unsigned long) info;
      mbi.drives_length += info->size;
    }

  DEBUG_SLEEP
  /* Set the boot loader name.  */
  mbi.boot_loader_name = (unsigned long) "GNU GRUB " VERSION;

  /*
   *  Initialize other Multiboot Info flags.
   */

  mbi.flags = (MB_INFO_MEMORY | MB_INFO_CMDLINE | MB_INFO_BOOTDEV
	       | MB_INFO_DRIVE_INFO | MB_INFO_CONFIG_TABLE
	       | MB_INFO_BOOT_LOADER_NAME);
  
#if 0
  
  /* Only multi_boot need this. So we should not initialize it here. 
   * Moved to boot_func in builtins.c
   */
  
  /* Get the ROM configuration table by INT 15, AH=C0h.  */
  mbi.config_table = get_rom_config_table ();

  /* Get the APM BIOS table.  */
  get_apm_info ();
  if (apm_bios_info.version)
    mbi.apm_table = (unsigned long) &apm_bios_info;
  
  if (apm_bios_info.version)
    mbi.flags |= MB_INFO_APM_TABLE;

#endif
  
#endif /* STAGE1_5 */

#if !defined(STAGE1_5) && !defined(GRUB_UTIL)
  /* Set cdrom drive.  */
    
    /* Get the geometry.  */
    if (debug > 1)
	printf("boot drive=%X, ", boot_drive);
    cdrom_drive = get_cdinfo (boot_drive, &tmp_geom);
    if (! cdrom_drive || cdrom_drive != boot_drive)
	cdrom_drive = GRUB_INVALID_DRIVE;
    if (debug > 1)
	printf("%s\n", cdrom_drive == GRUB_INVALID_DRIVE ? "Not CD":"Is CD");
  DEBUG_SLEEP
#endif
  
#if !defined(STAGE1_5) && !defined(GRUB_UTIL)

  if (cdrom_drive == GRUB_INVALID_DRIVE)  
  {
    int err;
    int version;
    struct drive_parameters *drp = (struct drive_parameters *)0x600;

#ifdef GRUB_UTIL
#define FIND_DRIVES 8
#else
#define FIND_DRIVES (*((char *)0x475))
#endif
    for (drive = 0xFF; drive >= 0x7F; drive--)
    {
//    struct geometry tmp_geom;
    
      if (drive >= 0x80 && drive < 0x80 + FIND_DRIVES)
	continue;
      
      /* Get the geometry.  */
      if (debug > 1)
	grub_printf ("\rget_cdinfo(%X),", drive);
      cdrom_drive = get_cdinfo (drive, &tmp_geom);
      if (cdrom_drive)
      {
	if (drive != 0x7F)
	  break;
	drive = cdrom_drive;
	cdrom_drive = get_cdinfo (drive, &tmp_geom);
	if (cdrom_drive == drive)
	  break;
	drive = 0x7F;
      }
    
      cdrom_drive = GRUB_INVALID_DRIVE;
    
      /* Some buggy BIOSes will hang at EBIOS `Get Drive Parameters' call
       * (INT 13h function 48h). So we only do further checks for Bochs.
       */
      
      if (bios_id != 1)		/* if it is not Bochs ... */
	continue;		/* ... skip and try next drive. */
      
      /* When qemu has a cdrom attached but not booted from cdrom, its
       * `get bootable cdrom status call', the int13/ax=4B01, returns CF=0
       * but with a wrong `Bootable CD-ROM Specification Packet' as follows:
       *
       * The first byte(packet size) is 0x13, all the rest bytes are 0's.
       * 
       * So we need to call get_diskinfo() here as a workaround.
       *
       *		(The bug was reported by Jacopo Lazzari. Thanks!)
       */

      if (drive >= 0x80)
      {
//printf("check drive: 0x%X\t\t", drive);

//printf (" check_int13_extensions (0x%X) ...", drive);
	version = check_int13_extensions (drive);
//printf (" OK! version=0x%X\n", version);

	if (! (version & 1)) /* not support functions 42h-44h, 47h-48h */
	    continue;	/* failure, try next drive. */
	
	  /* PhoenixBIOS 4.0 Revision 6.0 for ZF Micro might understand 
	     the greater buffer size for the "get drive parameters" int 
	     0x13 call in its own way.  Supposedly the BIOS assumes even 
	     bigger space is available and thus corrupts the stack.  
	     This is why we specify the exactly necessary size of 0x42 
	     bytes. */
	
        /* It is safe to clear out DRP.  */
        grub_memset (drp, 0, sizeof (struct drive_parameters));
	  
	drp->size = sizeof (struct drive_parameters) - 16;
	  
//printf (" biosdisk_int13_extensions (0x4800, 0x%X) ...", drive);
	err = biosdisk_int13_extensions (0x4800, drive, drp);
//printf (" OK! err=0x%X\n", err);
	if (! err && drp->bytes_per_sector == ISO_SECTOR_SIZE)
	{
	    /* Assume it is CDROM.  */
	    cdrom_drive = drive;
	    break;
	}
//printf("not cdrom\r", drive);
      } /* if (drive >= 0x80) */
    
    } /* for (drive = 0x7F; drive < 0xff; drive++) */
  } /* if (cdrom_drive == GRUB_INVALID_DRIVE) */
#undef FIND_DRIVES
  
  if (debug > 1)
    grub_printf(" cdrom_drive == %X.\n", cdrom_drive);
  DEBUG_SLEEP
#endif /* ! STAGE1_5 && ! GRUB_UTIL */

  /* check if the no-emulation-mode bootable cdrom exists. */
  
  /* if cdrom_drive is active, we assume it is the boot device */
  if (saved_entryno == 0 && force_cdrom_as_boot_device)
	force_cdrom_as_boot_device = 0;
  if (cdrom_drive != GRUB_INVALID_DRIVE && force_cdrom_as_boot_device)
  {
    boot_drive = cdrom_drive;	/* force it to be the boot drive */
  }

  if (boot_drive == cdrom_drive)
	/* force it to be "whole drive" without partition table */
	install_partition = 0xFFFFFF;
  
  /* Set boot drive and partition.  */
  saved_drive = boot_drive;
  saved_partition = install_partition;

#ifndef GRUB_UTIL
#ifndef STAGE1_5
  debug = 1;
  min_cdrom_id = (cdrom_drive < 0xE0 && cdrom_drive >= 0xC0) ? 0xE0 : 0xC0;
  
  /* if grub.exe is booted as a Linux kernel, check the initrd disk. */

  /* the real mode zero page(only the beginning 2 sectors, the boot params) is loaded at 0xA00 */

  /* check the header signature "HdrS" (0x53726448) */

  ram_drive = 0x7f;	/* the default ram_drive is a floppy. */
  if (*(unsigned long*)(int*)(0xA00 + 0x202) == 0x53726448)
  {
	unsigned long initrd_addr;
	unsigned long initrd_size;
	
	initrd_addr = *(unsigned long*)(int*)(0xA00 + 0x218);
	initrd_size = *(unsigned long*)(int*)(0xA00 + 0x21c);
	if (initrd_addr && initrd_size)
	{
	    rd_base = initrd_addr;
	    rd_size = initrd_size;

	    /* check if there is a partition table */
	    if (*(unsigned short *)(initrd_addr + 0x40) == 0xAA55)
	    {
#if 0
	    	initrd_addr += 0x1BE;	/* here begins the partition table */
		initrd_size = 0;	/* count valid partition entries */
		  
		for (i = 0; i < 4; i++)
		{
		    if ((unsigned char)(*(char*)(initrd_addr + i*16) & 0x7F))
			break;
		    if ((*(long long *)(initrd_addr + i*16)) | (*(long long *)(initrd_addr + i*16 + 8)))
		    {
			if (! (*(long *)(initrd_addr + i*16 + 8)) || ! (*(long long *)(initrd_addr + i*16 + 12)))
			    break;
			if (! (*(char *)(initrd_addr + i*16 + 2)) & 0x3f)
			    break;
			if (! (*(char *)(initrd_addr + i*16 + 6)) & 0x3f)
			    break;
			initrd_size++;
		    }
		}
		
		if (i == 4 && initrd_size)	/* partition table is valid. */
		    ram_drive = 0xfe;		/* let it be a harddrive */
#else
		if (! probe_mbr ((struct master_and_dos_boot_sector *)initrd_addr, 0, initrd_size, 0))
		    ram_drive = 0xfe;	/* partition table is valid, so let it be a harddrive */
		else
		    grub_printf ("\nUnrecognized partition table for RAM DRIVE; assuming floppy. Please rebuild\nit using a Microsoft-compatible FDISK tool, if the INITRD is a hard-disk image.\n");
#endif
	    }
	}
  }
#endif /* ! STAGE1_5 */
#endif /* ! GRUB_UTIL */

  /* Start main routine here.  */
  
  grub_printf("Starting cmain() ... ");
  
#if !defined(STAGE1_5) && !defined(GRUB_UTIL)
  DEBUG_SLEEP
#endif /* ! STAGE1_5 && ! GRUB_UTIL */
  cmain ();
}
