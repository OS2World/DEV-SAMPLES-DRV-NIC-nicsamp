
A Sample OS/2 IBM 16-bit Token-ring Driver Notes:
-------------------------------------------------
Furnished by DWB Associates, Inc. Copyright (c) 1993.



The IBM 16 Tokenring adapter is a standard ISA BUS 16 bit network card 
compatible with Tokenring topology networks. The Driver is designed to 
operate under Microsoft LANMAN 2.1 for OS2 1.3 operating system.  IBM Lan
IBM Lan Server running under OS2 2.0 are supported. These notes cover 
certain details of operation for building the executable from sratch
as well as installing and using the driver software on the above 
mentioned system.



About this Product:
-------------------

Three files exist in the \IBM16\PRODUCT directory that can be used to install
the Sample OS/2 IBM 16 bit Token-ring driver immediately without any further 
development procedures.  The three files are the IBM16.OS2, IBM16.NIF, and 
PROTOCOL.INI files.  The section titled "Installation Disk Notes:", which 
follows the next section, will help you to get started on installing and 
testing this sample package right away.

If you are interested in further development of the source files and using 
the MAKEFILE associated with this package, the following section titled
"Source Build Notes:" should be of interest.  This section will help you
to develop your own driver for the IBM TR 16 card.  See also the README2.DOC
file found in the directory \IBM16 for more information about this and using 
the MAKEFILE for custom development.


Source Build Notes:
-------------------

Source code for the driver is included for those developers interested in
modification for their own purposes or to see how a driver is structured
for the IBM 16 Tokenring architecture.  The source code is found in the 
directory titled \IBM16\SOURCE.

The MAKEFILE, found in the \IBM16 directory, can be used to build the
driver.  You can run the MAKEFILE by typing NMAKE at the command line 
in the same directory.  The standard Microsoft Toolkit files are assumed 
for building the driver.  The following is a list of environment variables
used by MAKEFILE.


ENVIRONMENT VARIABLES:

SRCPATH = path to where the source code resides
OBJPATH = path to the object files 
PRODPATH= path to the product files: ibm16.os2, ibm16.nif, and protocal.ini
INCPATH = path to include files
ERRPATH = path to the warnings/error files created during compilation

TYPE THE COMMAND: 

NMAKE    - At the command line prompt.  This makes the OS2 version of 
           the Sample OS/2 IBM 16 bit Token-ring driver.



Installation Disk Notes:
------------------------


INSTALLING THE DRIVER:

There are four different changes neccesary to be made in order to install
the Sample OS/2 IBM 16 bit Token-ring driver.  Three of these involve putting
new files into place and the other one involves making changes to an existing
file.

The three files that need to be put into directories on the hard disk of
the machine you will be using are the IBM16.OS2 file (the driver), the  
IBM16.NIF file (the network information file), and the PROTOCOL.INI file
(the protocol file).  All three of these files can be found with this sample
driver package in the \IBM16\PRODUCT directory on the diskette that has this 
README.DOC file.  The existing file, which is found in the root directory on 
the system you will be using to test, that needs to be updated is the 
CONFIG.SYS file (the system configuration file).  Use the instructions that
follow to make these changes:

1.   The IBM16.OS2 driver should be placed in a subdirectory that you create  
     to the \LANMAN\DRIVERS\TOKENRNG directory on the system you want to use  
     to test the driver.  Whatever name you give to the subdirectory, be sure
     to remember it.  You will need the path of the driver to list it as a 
     device in the CONFIG.SYS file.  The change to CONFIG.SYS is discussed 
     in step 4. below.

2.   The IBM16.NIF file is placed in the \LANMAN\DRIVERS\NIF directory.

3.   The PROTOCOL.INI file is placed in the \LANMAN directory.  You may want
     to take care to save any existing PROTOCOL.INI file that may already   
     exist in the \LANMAN directory to use in another application. 

4.   Find the CONFIG.SYS file in the root directory of the system you will 
     be doing your driver testing on.  This file needs to have a change made 
     to it in order to run the driver.  The following line needs to be added:
  
     DEVICE=C:\LANMAN\DRIVERS\TOKENRNG\<your subdirectory>\IBM16.OS2 

     Take note of a couple of things.  First of all, if the drive your
     machine normally boots up on is not the C: drive, put in the name of 
     the drive in place of the C: shown above.  Secondly, be sure to type 
     in the name of the subdirectory you created to place the driver in where 
     it says <your subdirectory>.  This line should be added towards the end 
     of the contents of the file CONFIG.SYS.

5.   Reboot the system with the IBM 16 Token-ring card installed. 
