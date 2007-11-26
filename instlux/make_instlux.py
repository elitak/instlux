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
#kernels = [ {"distribution":"Linkat","version":"1.1","media":"CDROM","kernel":"linux","drivers":"initrd","kernel_append":"devfs=mount,dall ramdisk_size=65536"}]

kernels = [ 
	{"distribution":"openSUSE","version":"11.0","media":"NET","kernel":"linux","drivers":"initrd","boot_dir":"","boot_dl_dir":"ftp://ftp4.gwdg.de/pub/opensuse/distribution/SL-OSS-factory/inst-source/boot/$ARCH/loader","kernel_append":"devfs=mount,dall ramdisk_size=65536 install=ftp:pub/opensuse/distribution/SL-OSS-factory/inst-source/ server=ftp4.gwdg.de lang=$LangParam splash=silent","icon":"opensuse.ico","logo":""},
	{"distribution":"openSUSE","version":"11.0","media":"LOCAL","kernel":"linux","drivers":"initrd","boot_dir":"$EXEDIR\\boot\\$ARCH\\loader","boot_dl_dir":"","kernel_append":"devfs=mount,dall ramdisk_size=65536  lang=$LangParam splash=silent","icon":"opensuse.ico","logo":""}, 
#	{"distribution":"openSUSE","version":"10.3","media":"NET","kernel":"linux","drivers":"initrd","boot_dir":"","boot_dl_dir":"ftp://ftp4.gwdg.de/pub/opensuse/distribution/SL-OSS-factory/inst-source/boot/$ARCH/loader","kernel_append":"devfs=mount,dall ramdisk_size=65536 install=ftp:pub/opensuse/distribution/SL-OSS-factory/inst-source/ server=ftp4.gwdg.de lang=$LangParam splash=silent","icon":"opensuse.ico","logo":""},
#	{"distribution":"openSUSE","version":"10.3","media":"LOCAL","kernel":"linux","drivers":"initrd","boot_dir":"$EXEDIR\\boot\\$ARCH\\loader","boot_dl_dir":"","kernel_append":"devfs=mount,dall ramdisk_size=65536  lang=$LangParam splash=silent","icon":"opensuse.ico","logo":""}, 
#	{"distribution":"openSUSE","version":"10.3","media":"CDROM","kernel":"linux","drivers":"initrd","boot_dir":"","boot_dl_dir":"","kernel_append":"devfs=mount,dall ramdisk_size=65536  lang=$LangParam splash=silent","icon":"opensuse.ico","logo":""}, 
	]

#languages = ["catalan"]
#languages = ["english"]
languages = remove_svn_dirs( os.listdir("translations")) 

#2007-01-07
list_of_contributors = ""
f = open("list_of_contributors.txt", "r")
for line in f.readlines():
	list_of_contributors += line 
	
f.close()

build = "build"
#nsis_bin = '"c:\Program Files\NSIS\makensis.exe"'
nsis_bin = '"makensis"'
grub4dos = ["grldr","grldr.mbr","grub.exe"]

def copy_from_src_to_build( build, file ):
  listdir = remove_svn_dirs( os.listdir( build ) )
  if file in listdir:
	os.remove(build+os.sep+file)

  if file != "":	
	input = open("src"+os.sep+file, "rb")
	output = open( build+os.sep+file, "wb")
	output.write( input.read() )
	output.flush()
	output.close()
	input.close()

def create_build_dirs( build, languages, kernels, grub4dos):
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

    for kernel in kernels:
    	copy_from_src_to_build( build, kernel["icon"] )
    	copy_from_src_to_build( build, kernel["logo"] )

    for grub in grub4dos:
      copy_from_src_to_build( build, grub )

    copy_from_src_to_build( build, "advanced.ini" )	
  

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
    name_version = kernel["distribution"] + " " + kernel["version"] + " Installer"
    outfile_name = kernel["distribution"] + version
    caption = kernel["distribution"]+" "+kernel["version"]+" installer (" +  kernel["media"] + ")"
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

    CREATE_CONTAINER_ALLOWS_ARCHSELECTION = "";
    CREATE_CONTAINER = ""
    if kernel["media"] == "NET":
	CREATE_CONTAINER = "inetc::get \""+kernel["boot_dl_dir"] + "/" + kernel["drivers"] + "\" \"$c\\"+kernel["distribution"]+"\\"+kernel["drivers"] + "\"\r\n"  
        CREATE_CONTAINER = CREATE_CONTAINER + "  Pop $0\r\n";
        CREATE_CONTAINER = CREATE_CONTAINER + "  StrCmp $0 \"OK\" dlkernel\r\n";
        CREATE_CONTAINER = CREATE_CONTAINER + "  StrCmp $0 \"URL Parts Error\" dlkernel\r\n"; # FIXME: This is an evil workaround for inetc on Vista

        CREATE_CONTAINER = CREATE_CONTAINER + "    MessageBox MB_OK|MB_ICONEXCLAMATION \"Download Error initrd ( $0 ), click OK to abort installation\" /SD IDOK\r\n";
        CREATE_CONTAINER = CREATE_CONTAINER + "  Abort\r\n";
        CREATE_CONTAINER = CREATE_CONTAINER + "  dlkernel:\r\n";
	CREATE_CONTAINER = CREATE_CONTAINER + "  inetc::get \""+kernel["boot_dl_dir"] + "/" + kernel["kernel"] + "\" \"$c\\"+kernel["distribution"]+"\\"+kernel["kernel"] + "\"\r\n"  
        CREATE_CONTAINER = CREATE_CONTAINER + "  StrCmp $0 \"OK\" dlok\r\n";
        CREATE_CONTAINER = CREATE_CONTAINER + "  StrCmp $0 \"URL Parts Error\" dlok\r\n"; # FIXME: This is an evil workaround for inetc on Vista
        CREATE_CONTAINER = CREATE_CONTAINER + "    MessageBox MB_OK|MB_ICONEXCLAMATION \"Download Error kernel ( $0 ), click OK to abort installation\" /SD IDOK\r\n";
        CREATE_CONTAINER = CREATE_CONTAINER + "  Abort\r\n";
        CREATE_CONTAINER = CREATE_CONTAINER + "  dlok:\r\n";
        CREATE_CONTAINER_ALLOWS_ARCHSELECTION = "goto displayadvanceddialog";
    elif kernel["media"] != "LOCAL":
    	CREATE_CONTAINER = "IfFileExists \"$c\\"+kernel["distribution"]+"\\"+kernel["drivers"]+"\" FileExists\r\n"
    	CREATE_CONTAINER = CREATE_CONTAINER + "File /oname=$c\\"+kernel["distribution"]+"\\"+kernel["drivers"]+" ../distros/instlux"+kernel["media"]+outfile_name+"//"+kernel["drivers"]+"\r\n"
    	CREATE_CONTAINER = CREATE_CONTAINER + "File /oname=$c\\"+kernel["distribution"]+"\\"+kernel["kernel"]+" ../distros/instlux"+kernel["media"]+outfile_name+"//"+kernel["kernel"]+"\r\n"



    customizations.append({
		    "NAME_VERSION":name_version,
		    "FILENAME":name+".nsi",
		    "OUTFILE":outfile_name,
		    "CREATE_CONTAINER":CREATE_CONTAINER,
		    "CREATE_CONTAINER_ALLOWS_ARCHSELECTION":CREATE_CONTAINER_ALLOWS_ARCHSELECTION,
		    "CAPTION":caption,
		    "MENU_TITLE":caption,
		    "BOOT_TITLE":caption,
		    "KRNL_APPEND":kernel_append,
		    "DISTRO":kernel["distribution"],
		    "MEDIA":kernel["media"],
		    "BOOTDIR":kernel["boot_dir"],
		    "KERNEL":kernel["kernel"],
		    "DRIVERS":kernel["drivers"],
		    "DISTICON":kernel["icon"],
		    "LOGO":kernel["logo"]})

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
      mui_languages+="  !insertmacro MUI_LANGUAGE \""+ language[0:1].upper() + language[1:] +"\"\n"    
      mui_translated_licenses += "  LicenseLangString license ${LANG_"+language.upper()+"} \"translations/"+language+"/license_"+language+".txt\"\n"
    translations[ 'All' ].append(["  !insertmacro MUI_LANGUAGE \"English\"",mui_languages])
    translations[ 'All' ].append(["  LicenseLangString license ${LANG_ENGLISH} \"translations/english/license_english.txt\"",mui_translated_licenses])
    if (len(languages)<2):
    	translations[ 'All' ].append(["!insertmacro MUI_LANGDLL_DISPLAY","#!insertmacro MUI_LANGDLL_DISPLAY"])
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


create_build_dirs(  build, languages, kernels, grub4dos )
linuxes = get_linuxes( kernels )
customizations = get_customizations( kernels, build)
translations = get_translations( languages, linuxes );
create_one_nsis_file_for_distro( customizations, build )
create_translated_licenses ( languages, build, list_of_contributors)
create_all_nsis( translations, linuxes, build)
compile_nsis( translations, linuxes, build, nsis_bin)

