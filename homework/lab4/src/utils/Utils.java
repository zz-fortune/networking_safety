package utils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.security.MessageDigest;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.Statement;
import java.util.Arrays;

import javax.crypto.Cipher;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESKeySpec;

import server.Server;

/**
 * 这是一个工具类
 * 
 * @author zz
 *
 */
public class Utils {

	/**
	 * 从输入流中读取一整个数据包
	 * 
	 * @param in 输入流
	 * @param buf 缓存区
	 * @return 一个 SessionPackage 的实例
	 * @throws IOException
	 */
	public static SessionPackage readPackage(InputStream in, byte[] buf) throws IOException {
		
		System.out.println("接收到新的包...");
		
		//	读取包的头部，获取包的总长度
		int offset = 1;
		in.read(buf, offset, 4);
		int length = byteArrayToInt(buf, offset);
		
		//	利用头部信息创建SessionPackage实例
		SessionPackage pkg = new SessionPackage((char) buf[0], length);
		offset += 4;
		
		//	读取剩余信息
		while (offset < length) {
			int size = in.read(buf, offset, length - offset);
			offset += size;
		}
		byte[] content=new byte[pkg.getLength()-5];
		System.arraycopy(buf, 5, content, 0, length-5);
		pkg.setContent(content);
		return pkg;
	}

	/**
	 * 处理一个用户的认证请求
	 * 
	 * @param in 输入流
	 * @param out 输出流
	 * @param buf 缓存区
	 * @return true 当且仅当认证成功，否则 false
	 * @throws IOException
	 */
	public synchronized static boolean authenticate(InputStream in, OutputStream out, byte[] buf) throws IOException {
		
		//	读取认证包
		SessionPackage pkg = readPackage(in, buf);
		pkg.getBytes(buf);
		
		//	获取客户端计算的散列值
		byte[] recievedHash = new byte[16];
		System.arraycopy(buf, 5, recievedHash, 0, 16);
		
		//	获得明文的认证码
		byte[] storedHash = new byte[20];
		System.arraycopy(buf, 21, storedHash, 0, 4);
		
		//	获得明文的用户名
		byte[] usrtmp = new byte[pkg.getLength() - 25];
		System.arraycopy(buf, 25, usrtmp, 0, usrtmp.length);
		String usr = new String(usrtmp);
		byte[] cipher = null;

		try {
			
			System.out.println("获取用户密码...");
			
			//	读取数据库，查询对应用户的密码
			Class.forName(Server.JDBC_DRIVER);
			Connection connection = DriverManager.getConnection(Server.DB_URL, Server.USER, Server.PAWD);
			Statement statement = connection.createStatement();
			ResultSet resultSet = statement.executeQuery("SELECT * FROM account WHERE username=\"" + usr+"\"");
			if (resultSet.next()) {
				usr += resultSet.getString("passwd");
			} else {
				usr = "";
			}
			resultSet.close();
			statement.close();
			connection.close();
			
			System.out.println("处理密码...");
			
			//	将用户名和密码一起用 md5 散列，得到散列值1
			MessageDigest md5 = MessageDigest.getInstance("md5");
			cipher = md5.digest(usr.getBytes("utf-8"));
			
			//	将散列值1和认证码一起用 md5 散列，得到散列值2
			System.arraycopy(cipher, 0, storedHash, 4, cipher.length);
			storedHash = md5.digest(storedHash);

		} catch (Exception e) {
			e.printStackTrace();
		}

		//	比较服务器计算出的散列值和客户端传过来的散列值，如果相等，则认证成功
		if (storedHash != null && Arrays.equals(recievedHash, storedHash)) {
			
			System.out.println("验证成功...");
			try {
				
				//	将客户端传过来的认证码用散列值2作为对称密钥，以DES算法加密
				byte[] validateCode = new byte[4];
				System.arraycopy(buf, 21, validateCode, 0, 4);
				byte[] re = encrypt(validateCode, cipher);
				
				//	将加密后的数据传给客户端
				SessionPackage package1 = new SessionPackage((char) 0x01, re.length + 5);
				package1.setContent(re);
				package1.getBytes(buf);
				out.write(buf, 0, package1.getLength());
			} catch (Exception e) {
				e.printStackTrace();
			}
			return true;

		} else {	//	认证失败，通知客户端失败
			System.out.println("验证失败...");
			SessionPackage package1 = new SessionPackage((char) 0x0, 5);
			package1.getBytes(buf);
			out.write(buf, 0, 5);
			return false;
		}
	}

	/**
	 * 将一个byte数组中的某四位，转换为一个 int 类型的数。起始位置由 offset 指定
	 * 
	 * @param b byte 数组
	 * @param offset 起始位置
	 * @return {@code b}数组中某四字节对应的 int 类型的数
	 */
	public static int byteArrayToInt(byte[] b, int offset) {
		return b[offset + 3] & 0xFF | (b[offset + 2] & 0xFF) << 8 | (b[offset + 1] & 0xFF) << 16
				| (b[offset + 0] & 0xFF) << 24;
	}
	
	/**
	 * 将一个 int 类型的数，转换为一个 byte 数组
	 * @param i int 类型的数
	 * @return {@code i} 对应的 byte 数组
	 */
	public static byte[] intToByteArray(int i) {
		byte[] result = new byte[4];
		// 由高位到低位
		result[0] = (byte) ((i >> 24) & 0xFF);
		result[1] = (byte) ((i >> 16) & 0xFF);
		result[2] = (byte) ((i >> 8) & 0xFF);
		result[3] = (byte) (i & 0xFF);
		return result;
	}

	/**
	 * 利用给定的 key，将文本加密
	 * 
	 * @param plaintext 待加密的信息
	 * @param key 对称密钥
	 * @return 加密后的信息
	 * @throws Exception
	 */
	public static byte[] encrypt(byte[] plaintext, byte[] key) throws Exception {
		DESKeySpec keySpec = new DESKeySpec(key);
		SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("des");
		SecretKey secretKey = keyFactory.generateSecret(keySpec);
		Cipher cipher = Cipher.getInstance("des");
		cipher.init(Cipher.ENCRYPT_MODE, secretKey);
		return cipher.doFinal(plaintext);
	}
	
	/**
	 * 利用给定的 key，将加密的文本解密。
	 * 
	 * @param cipherData 加密的文本
	 * @param key 对称密钥
	 * @return 加密后的信息
	 * @throws Exception
	 */
	public static byte[] decrypt(byte[] cipherData, byte[] key) throws Exception{
		DESKeySpec keySpec = new DESKeySpec(key);
		SecretKeyFactory keyFactory = SecretKeyFactory.getInstance("des");
		SecretKey secretKey = keyFactory.generateSecret(keySpec);
		Cipher cipher = Cipher.getInstance("des");
		cipher.init(Cipher.DECRYPT_MODE, secretKey);
		return cipher.doFinal(cipherData);
	}

}
