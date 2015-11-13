
  Memo:  This is the readme2.doc file for Sample OS/2 IBM TR16 driver.
         Copyright (c) 1993 - DWB Associates, Inc.  All Rights Reserved.

  Brief: If you plan to do development in a different directory structure
         than the one this sample driver package is built in, you will  
         need to change the environment parameters found in MAKEFILE to
         correspond to the path(s) you want to build the object files, the
         IBM16.OS2 driver file, and any *.ERR files you may want to use.
          
         You will also need to make changes to the IBMLK file.  Inside 
         IBMLK, you need to change the paths listed for the residence of
         the *.OBJ files, the IBM16.OS2 file, and the IBM16.MAP file to
         correspond to the directory structure you want to use.	 Of course,
         the paths in IBMLK must be the same for all files as they are
         listed in the MAKEFILE.

         Below is a listing of the files associated with this driver  
         package and their respective directories.  This is the directory
         structure of the current MAKEFILE and IBMLK linking file.
  
 ____________________________________________________________________________ 
 ----------------------------------------------------------------------------
 
 \IBM16 directory:
 -----------------

       This Readme file               OS2 Compile/Link files           
      -------------------       ----------------------------------	
          README2.DOC            MAKEFILE    IBMLK     OS2DEV.DEF	

 ____________________________________________________________________________ 
 ----------------------------------------------------------------------------

 \IBM16\SOURCE directory:
 ------------------------

	  	    C-files           ASM-files
                  ----------------------------------
	  	   BUFF     C       _BUFF    ASM      
	  	   GIEXEC   C       DEBUG    ASM      
	  	   HW       C       GI       ASM      
	  	   HWINIT   C       _INIT    ASM      
	  	   INITND   C       _MISC    ASM      
	  	   INITOS   C       OPSYS    ASM      
	  	   MCAINIT  C       TICKER   ASM      
	  	   MISC     C       _UPRIMT  ASM       
	  	   NDIS     C          
	  	   PRINT    C          
	  	   STRINGS  C          
	  	   UPRIM    C          
	  	                     
 ____________________________________________________________________________ 
 ----------------------------------------------------------------------------

 \IBM16\INCLUDE directory:
 -------------------------

                 C-file Headers  ASM-file Headers  
                ----------------------------------
                   ADAPTER  H      BSEDOS   INC    
	           BUFF     H      BSEDOS16 INC    
	           DEVHDR   H      _BUFF    INC    
	           DIALOGS  H      DEVHDR   INC    
	           GIEXEC   H      DEVHLP   INC    
	           HW       H      GI       INC    
	           HWBUFF   H      _HWBUFF  INC    
	           HWINIT   H      IOCTL    INC    
	           IOCTL    H      LAYOUT   INC    
	           MCA      H      _MISC    INC    
	           MISC     H      NDIS     INC    
	           NDIS     H      OS2DEF   INC    
	           PRINT    H      OS2DEF16 INC    
	           STRINGS  H      SEGMENTS INC    
	           TYPES    H      _UPRIM   INC    
	           UPRIM    H       	       		
	           VERSION  H       	       		
  							     		    
 ____________________________________________________________________________
 ----------------------------------------------------------------------------
   				    
 \IBM16\OS2 directory:
 ---------------------
   	       
                    C- and ASM- Object files
   		---------------------------------
   		   BUFF.OBJ          _BUFF.OBJ      
   		   GIEXEC.OBJ        DEBUG.OBJ      
   		   HW.OBJ            GI.OBJ         
   		   HWINIT.OBJ        _INIT.OBJ      
   		   INITND.OBJ        _MISC.OBJ      
   		   INITOS.OBJ        OPSYS.OBJ      
   		   MCAINIT.OBJ       TICKER.OBJ     
   		   MISC.OBJ          _UPRIMT.OBJ    
          	   NDIS.OBJ             
   	           PRINT.OBJ            
   		   STRINGS.OBJ          
		   UPRIM.OBJ            
 		
 ____________________________________________________________________________
 ----------------------------------------------------------------------------

 \IBM16\ERR directory:
 ---------------------
             
                 C- and ASM- Error/Warnings files
		 --------------------------------
		   BUFF.ERR          _BUFF.ERR      
		   GIEXEC.ERR        DEBUG.ERR      
		   HW.ERR            GI.ERR         
		   HWINIT.ERR        _INIT.ERR      
		   INITND.ERR        _MISC.ERR      
		   INITOS.ERR        OPSYS.ERR      
		   MCAINIT.ERR       TICKER.ERR     
		   MISC.ERR          _UPRIMT.ERR    
		   NDIS.ERR             
		   PRINT.ERR        
	           STRINGS.ERR        
		   UPRIM.ERR        


 ____________________________________________________________________________
 ----------------------------------------------------------------------------

 \IBM16\PRODUCT directory:
 -------------------------
                          
     Sample OS/2 IBM TR16 driver package        Map and Symbol files                  
     ------------------------------------       --------------------    
     IBM16.OS2   IBM16.NIF   PROTOCOL.INI    	IBM16.MAP  IBM16.SYM
