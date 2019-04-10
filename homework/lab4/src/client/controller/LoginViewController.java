package client.controller;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.security.MessageDigest;
import java.util.Random;
import java.util.regex.Pattern;

import javafx.fxml.FXML;
import javafx.scene.control.Label;
import javafx.scene.control.PasswordField;
import javafx.scene.control.TextField;
import javafx.scene.paint.Color;
import utils.SessionPackage;
import utils.Utils;

/**
 * 这是登陆界面的控制器
 * 
 * @author zz
 *
 */
public class LoginViewController {
	
	//	获取界面中的控件
	@FXML
	private TextField username;

	@FXML
	private PasswordField password;

	@FXML
	private Label tips;

	// 保存主类的实例，用以登陆成功后通知主类跳转页面
	private ClientApp clientApp;
	
	//	一些参数
	private final int BUF_SIZE = 65535;
	private final String FILENAME = "data/validate.txt";

	@FXML
	private void initialize() {
	}

	/**
	 * 触发的登录事件
	 */
	@FXML
	private void login() {
		String usr, passwd;	//	保存用户名和密码
		usr = this.username.getText();
		passwd = this.password.getText();
		
		//	检查输入合法性
		if (!checkInput(usr, passwd)) {
			return;
		}
		MessageDigest md5;
		Socket socket = null;
		InputStream inputStream = null;
		OutputStream outputStream = null;
		byte[] buf = new byte[BUF_SIZE];
		try {
			
			//	初始化md5、socket等
			md5 = MessageDigest.getInstance("md5");
			socket = new Socket("127.0.0.1", 8080);
			inputStream = socket.getInputStream();
			outputStream = socket.getOutputStream();
			
			//	用md5将用户名和密码散列得到散列值1
			byte[] key = md5.digest((usr + passwd).getBytes("utf-8"));
			
			//	获取一个随机认证码
			byte[] validateCode = randomCode();
			
			//	将散列值1和认证码一起用md5散列，得到散列值2
			byte[] hash2 = new byte[20];
			System.arraycopy(validateCode, 0, hash2, 0, 4);
			System.arraycopy(key, 0, hash2, 4, key.length);
			hash2 = md5.digest(hash2);
			
			//	将散列值2、认证码明文、用户名明文发送给服务器
			byte[] usrbyte = usr.getBytes("utf-8");
			byte[] content = new byte[usrbyte.length + 20];
			System.arraycopy(hash2, 0, content, 0, 16);
			System.arraycopy(validateCode, 0, content, 16, 4);
			System.arraycopy(usrbyte, 0, content, 20, usrbyte.length);
			SessionPackage package1 = new SessionPackage((char) 0x0, usrbyte.length + 25);
			package1.setContent(content);
			package1.getBytes(buf);
			outputStream.write(buf, 0, package1.getLength());
			
			//	获取服务器返回消息
			SessionPackage package2 = null;
			if (inputStream.read(buf, 0, 1) > 0) {
				package2 = Utils.readPackage(inputStream, buf);
			}
			
			//	若是认证通过，显示登录成功
			if (package2 != null && package2.getCode() == 0x01) {
				this.tips.setTextFill(Color.BLUE);
				this.tips.setText("登录成功");
				
				//	将服务器返回的加密的认证码解密后存入文件
				byte[] re = Utils.decrypt(package2.getContent(), key);
				int code =Utils.byteArrayToInt(re, 0);
				String string=Integer.toString(code);
				re=string.getBytes();
				
				System.out.println(code);
				OutputStream fileStream = new FileOutputStream(FILENAME);
				fileStream.write(re, 0, re.length);
				fileStream.close();
			} else {	//	认证失败，显示提示信息
				System.out.println("登录失败");
				this.tips.setTextFill(Color.RED);
				this.tips.setText("用户名或密码错误");
			}

			inputStream.close();
			outputStream.close();
			socket.close();
		} catch (Exception e) {
			e.printStackTrace();
		}

	}

	/**
	 * 检查用户名个密码的合法性
	 * 
	 * @param usr 用户名
	 * @param passwd 密码
	 * @return true 当且仅当密码和用户名均和法，否则返回 false
	 */
	private boolean checkInput(String usr, String passwd) {
		Pattern pattern = Pattern.compile("^[1-9a-zA-Z\\-]+$");
		usr = this.username.getText();
		passwd = this.password.getText();
		if (usr.equals("")) {
			this.tips.setTextFill(Color.RED);
			this.tips.setText("用户名不能为空");
			return false;
		}

		if (passwd.equals("")) {
			this.tips.setTextFill(Color.RED);
			this.tips.setText("密码不能为空");
			return false;
		}

		if (!pattern.matcher(usr).matches() || !pattern.matcher(passwd).matches()) {
			this.tips.setTextFill(Color.RED);
			this.tips.setText("密码或用户名有误");
			return false;
		}
		return true;
	}

	/**
	 * 获取主类的实例
	 * 
	 * @param clientApp 主类的实例
	 */
	public void setClientApp(ClientApp clientApp) {
		this.clientApp = clientApp;
	}

	/**
	 * 随机生成一个认证码。并将认证码转为 byte 数组返回
	 * 
	 * @return 认证码的 byte 数组
	 */
	private byte[] randomCode() {
		Random random = new Random(System.currentTimeMillis());
		int i = random.nextInt();
		System.out.println(i);
		return Utils.intToByteArray(i);
	}

}
