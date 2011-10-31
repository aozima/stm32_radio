/* 使用最小限度API实现的一个简单的HTTP/1.0服务器 */
#include <lwip/api.h>
#include <finsh.h>

/*  这是实际的web页面数据。大部分的编译器会将这些数据放在ROM里 */
const static char indexdata[] = "<html> \
	<head><title>A test page</title></head> \
	<body> \
	This is a small test page. \
	</body> \
	</html>";
const static char http_html_hdr[] = "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n";

/*  这个函数处理进入的连接 */
static void process_connection(struct netconn *conn)
{
	struct netbuf *inbuf;
	char *rq;
	rt_uint16_t len;

	/*  从这个连接读取数据到inbuf，我们假定在这个netbuf中包含完整的请求 */
	inbuf = netconn_recv(conn);

	/*  获取指向netbuf中第一个数据片断的指针，在这个数据片段里我们希望包含这个请求 */
	netbuf_data(inbuf, (void**)&rq, &len);

	/*  检查这个请求是不是HTTP "GET /\r\n"  */
	if( rq[0] == 'G' &&
		rq[1] == 'E' &&
		rq[2] == 'T' &&
		rq[3] == ' ')
	{
		/*  发送头部数据 */
		netconn_write(conn, http_html_hdr, sizeof(http_html_hdr), NETCONN_NOCOPY);
		/*  发送实际的web页面 */
		netconn_write(conn, indexdata, sizeof(indexdata), NETCONN_NOCOPY);

		/*  关闭连接 */
		netconn_close(conn);
	}
	netbuf_delete(inbuf);
}

/* 线程入口 */
void lw_thread(void* paramter)
{
	struct netconn *conn, *newconn;

	/*  建立一个新的TCP连接句柄 */
	conn = netconn_new(NETCONN_TCP);

	/*  将连接绑定在任意的本地IP地址的80端口上 */
	netconn_bind(conn, NULL, 80);

	/*  连接进入监听状态 */
	netconn_listen(conn);

	/*  循环处理 */
	while(1)
	{
		/*  接受新的连接请求 */
		newconn = netconn_accept(conn);

		/*  处理进入的连接 */
		process_connection(newconn);

		/*  删除连接句柄 */
		netconn_delete(newconn);
	}
}

void websrv()
{
	rt_thread_t tid;

	tid = rt_thread_create("websrv", lw_thread, RT_NULL,
		1024, 25, 5);
	if (tid != RT_NULL) rt_thread_startup(tid);
}
FINSH_FUNCTION_EXPORT(websrv, startup a simple web server);
