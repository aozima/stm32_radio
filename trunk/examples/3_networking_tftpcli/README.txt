First step: open tftp server on PC, I am using "Tftpd32 by Ph.Jounin"
Second step: execute tftp_put or tftp_get command under finsh shell as belowing

-----------------------------------------------------------------
|	finsh>>tftp_put("192.168.1.5","/","radio.pls")		|
|	######done						|
|	        0, 0x00000000					|
|								|	
|	finsh>>tftp_get("192.168.1.5","/","frompc.txt")		|	
|	######done						|
|	        0, 0x00000000					|
-----------------------------------------------------------------