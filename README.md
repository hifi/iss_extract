InstallShield Setup Extract
===========================

Extracts InstallShield Setup executables with embedded data files. `unshield` is required to further extract the actual data cabinets. Note that there are very little sanity checks or version checks in yet so it will possibly show you random data if you feed it an invalid or unsupported executable.

Some tested installers that can be unpacked with this tool (inc. sha1):

    8c10c0d20f15912a9450b6fbe028f31928b29893  GTA2.exe (Rockstar Classics build - currently defunct)

Usage
-----
`iss_extract l setup.exe` lists all files in the installer  
`iss_extract x setup.exe` extracts all files from the installer  
`iss_extract x setup.exe data1.hdr data1.cab` extracts data1.hdr and data1.cab from the installer  

Example
-------
    $ ./iss_extract l GTA2.exe
    InstallShield Setup Extract / Copyright (c) 2014 Toni Spets <toni.spets@iki.fi>
    
    Length     Version          Name             Path
    ----------------------------------------------------------------------
        290816 1.0.0.2          Autorun.exe      Disk1\Autorun.exe
            43 0.0.0.0          autorun.inf      Disk1\autorun.inf
        582812 0.0.0.0          data1.cab        Disk1\data1.cab
         71292 0.0.0.0          data1.hdr        Disk1\data1.hdr
     359762427 0.0.0.0          data2.cab        Disk1\data2.cab
        418296 0.0.0.0          engine32.cab     Disk1\engine32.cab
           766 0.0.0.0          icon1.ico        Disk1\icon1.ico
           570 0.0.0.0          layout.bin       Disk1\layout.bin
          5516 0.0.0.0          readme.txt       Disk1\readme.txt
        244678 0.0.0.0          setup.bmp        Disk1\setup.bmp
        328885 0.0.0.0          setup.boot       Disk1\setup.boot
        107512 7.1.100.1248     setup.exe        Disk1\setup.exe
           398 0.0.0.0          setup.ini        Disk1\setup.ini
        170039 0.0.0.0          setup.inx        Disk1\setup.inx
        243858 0.0.0.0          setup.skin       Disk1\setup.skin
    ----------------------------------------------------------------------
     362227908                                   15 files

