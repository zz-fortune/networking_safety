package server;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

import utils.Utils;

/**
 * 这是用以与一个客户端通信的类
 * 
 * @author zz
 *
 */
public class ClientSession extends Thread {
	
	//	缓存区
	private final int BUF_SIZE = 65535;

	private Socket clientSocket;	//	与客户端连接的 socket
	private byte buf[];	//	数据缓存区
	private boolean authenticated = false;	//	用户是否已经认证

	/**
	 * 构造器
	 * 
	 * @param socket 与客户端连接的 socket
	 */
	public ClientSession(Socket socket) {
		System.out.println("新用户请求 " + socket.getInetAddress().getHostAddress() + ": " + socket.getPort());
		this.clientSocket = socket;
		this.buf = new byte[BUF_SIZE];

	}

	@Override
	public void run() {
		InputStream inputStream = null;
		OutputStream outputStream = null;
		try {
			
			//	获取输入输出流
			inputStream = this.clientSocket.getInputStream();
			outputStream = this.clientSocket.getOutputStream();
			
			//	循环接收客户端的消息
			while (inputStream.read(buf, 0, 1) > 0) {
				char code = (char) buf[0];
				
				//	根据客户端请求的类型调用不同的方法去处理
				if (code == 0x0) {
					System.out.println("开始认证...");
					this.authenticated = Utils.authenticate(inputStream, outputStream, buf);
				} else if (!authenticated) {
					System.out.println("未认证");
					break;
				}
			}
		} catch (Exception e) {
			currentThread().interrupt();
		} finally {
			try {
				this.clientSocket.close();
				inputStream.close();
				outputStream.close();
			} catch (Exception e2) {
				e2.printStackTrace();
			}
		}

	}

}
