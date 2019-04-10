package utils;

/**
 * 这个类是对传输的数据包的抽象
 * 
 * @author zz
 *
 */
public class SessionPackage {

	private char code;	//	消息类型
	private int length;	//	消息总长度
	private byte[] content=null;	//	消息内容

	/**
	 * 构造器
	 * 
	 * @param code 消息类型
	 * @param length 消息总长度
	 */
	public SessionPackage(char code, int length) {
		this.code = code;
		this.length = length;
	}

	public char getCode() {
		return code;
	}

	public void setCode(char code) {
		this.code = code;
	}

	public int getLength() {
		return length;
	}

	public void setLength(int length) {
		this.length = length;
	}

	public void setContent(String str) {
		this.content = str.getBytes();
	}

	public byte[] getContent() {
		return content;
	}

	public void setContent(byte[] content) {
		this.content = new byte[this.length-5];
		for (int i = 0; i < this.length-5; i++) {
			this.content[i] = content[i];
		}
	}
	
	public void getBytes(byte[] buf) {
		buf[0] = (byte)this.code;
		buf[1]=(byte)((this.length>>24)&0xff);
		buf[2]=(byte)((this.length>>16)&0xff);
		buf[3]=(byte)((this.length>>8)&0xff);
		buf[4]=(byte)(this.length&0xff);
		for (int i = 5; i < this.length; i++) {
			buf[i]=this.content[i-5];
		}
	}

}
