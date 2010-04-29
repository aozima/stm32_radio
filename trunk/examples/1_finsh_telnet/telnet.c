#include <rtthread.h>
#include <lwip/sockets.h>

/* telnet device interface */

void telnet_task(void* parameter)
{
   char *recv_data;
   rt_uint32_t sin_size;
   int sock, connected, bytes_received;
   struct sockaddr_in server_addr, client_addr;

   recv_data = rt_malloc(1024);
   if (recv_data == RT_NULL)
   {
       rt_kprintf("No memory\n");
       return;
   }

	/* create TCP socket */
   if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
   {
       /* create failed */
       rt_kprintf("Socket error\n");

       /* release the memory */
       rt_free(recv_data);
       return;
   }

   /* init server address */
   server_addr.sin_family = AF_INET;
   server_addr.sin_port = htons(23); /* telnet srv port */
   server_addr.sin_addr.s_addr = INADDR_ANY;
   rt_memset(&(server_addr.sin_zero),8, sizeof(server_addr.sin_zero));

   /* bind socket */
   if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
       /* failed */
       rt_kprintf("Unable to bind\n");

       /* release the memory */
       rt_free(recv_data);
       return;
   }

   /* listen on socket */
   if (listen(sock, 5) == -1)
   {
       rt_kprintf("Listen error\n");

       /* release recv buffer */
       rt_free(recv_data);
       return;
   }

   rt_kprintf("\nTelnet Server Waiting for client on port 23...\n");
   while(1)
   {
       sin_size = sizeof(struct sockaddr_in);

       /* accept a client */
       connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);

       /* handle client */
       while (1)
       {
           /* receive data from client */
           bytes_received = recv(connected,recv_data, 1024, 0);
           if (bytes_received < 0)
           {
               /* failed, close this connection. */
               lwip_close(connected);
               break;
           }

           /* get message */
           recv_data[bytes_received] = '\0';

		   /* notify to shell */
       }
   }

   /* close lwip */
   lwip_close(sock);

   /* release memory */
   rt_free(recv_data);

   return ;
}

void telnet_srv()
{
	rt_thread_t tid;

	tid = rt_thread_create("telnet", telnet_task, RT_NULL,
		2048, 0x25, 5);
	if (tid != RT_NULL)
		rt_thread_startup(tid);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(telnet_srv, startup telnet server);
#endif
