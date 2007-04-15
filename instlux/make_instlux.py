#!/usr/bin/python

import os
import sys

def remove_svn_dirs( dirs ):
  dirs_without_svn = []
  for dir in dirs:
    if dir.find('.svn')==-1:
      dirs_without_svn.append( dir )
  
  return dirs_without_svn

#kernels = [ {"distribution":"Linkat","version":"1.0","media":"CDROM","kernel":"LINUX2","drivers":"INITRD2","append":"root=/dev/hdc devfs=mount,dall ramdisk_size=65536"}]
kernels = [ {"distribution":"OpenSuSE","version":"10.3","media":"NET","kernel":"linux","drivers":"initrd","kernel_append":"devfs=mount,dall ramdisk_size=65536 install=http:pub/opensuse/distribution/SL-OSS-Factory/inst-source server=195.135.221.134"},{"distribution":"OpenSuSE","version":"10.3","media":"CDROM","kernel":"linux","drivers":"initrd","kernel_append":"devfs=mount,dall ramdisk_size=65536"}]
kernels = [ {"distribution":"OpenSuSE_x86_64_","version":"10.3","media":"NET","kernel":"linux","drivers":"initrd","kernel_append":"devfs=mount,dall ramdisk_size=65536 install=http:pub/opensuse/distribution/SL-OSS-Factory/inst-source server=195.135.221.134"},{"distribution":"OpenSuSE_x86_64_","version":"10.3","media":"CDROM","kernel":"linux","drivers":"initrd","kernel_append":"devfs=mount,dall ramdisk_size=65536"}]
languages = []
languages = remove_svn_dirs( os.listdir("translations")) 
#languages = ["english"]

#2007-01-07
list_of_contributors = "Greg Johnston\nMichael\nMarc Herbert\nPiarres beobide\nsesammases\nbalu_kalla\nnumatrix\nnotable\nJimmyGoon\nfra1027\nrev pete moss\nRajesh\nHeath\nAMCDeathKnight\nRobin Patt-Corner\ncarsten\nTonA\ntomtenberge\nJack Chen\nKevin\nTim\nDaveW\nBhasadu\nMarcoAurelio\nEl Paco\nkkkkk\nDan\nAas\nmetallicgreenb\nGrymyrk\nATB\nEmanuel Levy\nindygo\npetrbok\nVati-Khan\noffdutyBorg\nalicia_sb\nSam Johnston\nHarryhe\ntotro2\nHenrik Brink"

build = "build"
#nsis_bin = '"c:\Program Files\NSIS\makensis.exe"'
nsis_bin = '"/usr/bin/makensis"'
instlux_ico = "instlux.ico"
instlux_logo = "instlux_logo.bmp"
grub4dos = ["grldr","grub.exe"]

def copy_from_src_to_build( build, file ):
  listdir = remove_svn_dirs( os.listdir( build ) )
  if file not in listdir:
    input = open("src"+os.sep+file, "rb")
    output = open( build+os.sep+file, "wb")
    output.write( input.read() )
    output.flush()
    output.close()
    input.close()

def create_build_dirs( build, languages, instlux_ico, instlux_logo, grub4dos):
    listdirs = remove_svn_dirs( os.listdir(".") )
    if "bin" not in listdirs:
        os.mkdir( "bin" )
    listdirs = remove_svn_dirs( os.listdir(".") )
    if build not in listdirs:
        os.mkdir( build )

    listdirs = remove_svn_dirs( os.listdir( build ) )
    if "translations" not in listdirs:
        os.mkdir( build+os.sep+"translations" )

    if "bin" not in listdirs:
        os.mkdir( build+os.sep+"bin" )

    listdirs = remove_svn_dirs( os.listdir( build+os.sep+"translations" ) )
    for language in languages:
        if language not in listdirs:
            os.mkdir( build+os.sep+"translations"+os.sep+language )
    copy_from_src_to_build( build, instlux_ico )
    copy_from_src_to_build( build, instlux_logo )
    for grub in grub4dos:
      copy_from_src_to_build( build, grub )
  

def get_linuxes( kernels ):
  linuxes=[];
  for kernel in kernels:
    version = kernel["version"].replace(".","_")
    name = kernel["media"]+kernel["distribution"]+version+"_en"
    linuxes = linuxes+[name.replace("_en","")]
  return linuxes;

def get_customizations( kernels, build):
  customizations = []
  for kernel in kernels:
    version = kernel["version"].replace(".","_")
    name = "instlux"+kernel["media"]+kernel["distribution"]+version+"_en"
    caption = kernel["distribution"]+" "+kernel["version"]+" installer"
    list_of_files_string =""
    dir_out = name.replace("_en","")
    dir = "distros"+os.sep+dir_out
    kernel_append = kernel["kernel_append"]   
    for dirpath, dirnames, filenames in os.walk( dir ):
      if dirpath.find('.svn')==-1:
        dirpath_formated = dirpath.replace("/","\\").replace( build+os.sep,"")
        list_of_files_string = list_of_files_string+"   SetOutPath $INSTDIR\\"+dirpath_formated+"\n"
        for file in filenames:
          list_of_files_string = list_of_files_string+"   File \"..\\"+dirpath_formated+"\\"+file+"\"\n"
    customizations.append({"FILENAME":name+".nsi","NAME":name,"OUTFILE":name+".exe", "CAPTION":caption, "MENU_TITLE":caption, "KERNEL":dir_out+"/"+kernel["kernel"], "DRIVERS":dir_out+"/"+kernel["drivers"], "LIST_OF_FILES":list_of_files_string, "BOOT_TITLE":caption, "OUTPATH":dir_out, "KRNL_APPEND":kernel_append})
  return customizations;

def create_one_nsis_file_for_distro( customizations,  build ):
    template_file = open( "src"+os.sep+"instlux_template_en.nsi", "r" )
    orig_data = template_file.read()
    template_file.close()

    for item in customizations:
	data = orig_data
	f = open(  build+os.sep+item["FILENAME"], "w" )
	for customization_key, customization_value in item.iteritems():
		data = data.replace( customization_key, customization_value )

	f.write( data )
	f.flush()
	f.close()


def get_translations( languages, linuxes):
  translations = {}
  translations[ 'All' ] = []
  
  for linux in linuxes:
    translations[ 'All' ].append([linux+"_en",linux])
    mui_languages = ""
    mui_translated_licenses = ""
    for language in languages:
      mui_languages+="  !insertmacro MUI_LANGUAGE \""+language+"\"\n"    
      mui_translated_licenses += "  LicenseLangString license ${LANG_"+language.upper()+"} \"translations/"+language+"/license_"+language+".txt\"\n"
    translations[ 'All' ].append(["  !insertmacro MUI_LANGUAGE \"English\"",mui_languages])
    translations[ 'All' ].append(["  LicenseLangString license ${LANG_ENGLISH} \"translations/english/license_english.txt\"",mui_translated_licenses])
  return translations;

def translate(input_file_name, output_file_name, translations):
    input_file=open( input_file_name, "r")
    output_file=open( output_file_name, "w")
    for line in input_file.readlines():
        data = line
        for translation in translations:
            if line.find(translation[0])!=-1:
                data = line.replace(translation[0], translation[1])
        output_file.write( data )

    output_file.flush()
    output_file.close()
    input_file.close()

def create_all_nsis( translations, linuxes, build):
  for language,values in translations.iteritems():
    for linux in linuxes:
      input_file_name = build+os.sep+"instlux"+linux+"_en.nsi"
      output_file_name = build+os.sep+"instlux"+linux+"_"+language+".nsi"
      translate( input_file_name, output_file_name, values)
      
def compile_nsis( translations, linuxes, build, nsis_bin):
  for language,values in translations.iteritems():
    for linux in linuxes:
      file_name = build+os.sep+"instlux"+linux+"_"+language+".nsi"
      command = nsis_bin+" "+file_name
      os.system( command )

def create_translated_licenses ( languages, build, list_of_contributors):
  for language in languages:
    f_orig = open( "translations"+os.sep+language+os.sep+"license_"+language+".txt", "r")
    f_dest = open( build+os.sep+"translations"+os.sep+language+os.sep+"license_"+language+".txt", "w")
    f_dest.write( f_orig.read().replace( "LIST_OF_CONTRIBUTORS", list_of_contributors ) )
    f_dest.flush()
    f_dest.close()
    f_orig.close()


create_build_dirs(  build, languages, instlux_ico, instlux_logo, grub4dos )
linuxes = get_linuxes( kernels )
customizations = get_customizations( kernels, build)
translations = get_translations( languages, linuxes );
create_one_nsis_file_for_distro( customizations, build )
create_translated_licenses ( languages, build, list_of_contributors)
create_all_nsis( translations, linuxes, build)
compile_nsis( translations, linuxes, build, nsis_bin)




