package client.model;

/**
 * 这是 account 的模型
 * 
 * @author zz
 *
 */
public class Account {
	
	private String username;
	private String password;
	
	public Account(String usr, String pawd) {
		this.username = usr;
		this.password = pawd;
	}
	
	public String getUsername() {
		return username;
	}
	
	public void setUsername(String username) {
		this.username = username;
	}
	
	public String getPassword() {
		return password;
	}
	
	public void setPassword(String password) {
		this.password = password;
	}

}
