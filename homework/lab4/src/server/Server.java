package server;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * 这是服务器的主类
 * 
 * @author zz
 *
 */
public class Server {

	//	数据库的一些参数
	public static final String JDBC_DRIVER = "com.mysql.cj.jdbc.Driver";
	public static final String DB_URL = "jdbc:mysql://localhost:3306/STU_MANAGEMENT";
	public static final String USER = "root";
	public static final String PAWD = "zz1160300620";

	//	服务器监听的端口
	private int port;

	/**
	 * 构造器
	 * 
	 * @param port 服务器监听的端口
	 */
	public Server(int port) {
		this.port = port;
	}

	/**
	 * 启动服务器
	 */
	public void start() {
		ServerSocket serverSocket = null;
		try {
			
			//	创建服务器端的 socket 并监听指定端口
			serverSocket = new ServerSocket(this.port);

			System.out.println("启动服务器...");
			System.out.println(
					"服务器运行在 " + serverSocket.getInetAddress().getHostAddress() + ": " + serverSocket.getLocalPort());
			Socket socket;
			while((socket=serverSocket.accept())!=null) {
				
				//	接收到新的用户连接，创建一个 ClientSession，在新线程中处理
				ClientSession session = new ClientSession(socket);
				session.start();
			}
			
		} catch (IOException e) {
			e.printStackTrace();
		} finally {
			try {
				if (serverSocket != null) {
					serverSocket.close();
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}

	public static void main(String[] args) {
		Server server = new Server(8080);
		server.start();
	}
}
