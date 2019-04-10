package client.controller;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Array;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;
import java.util.Random;
import java.util.regex.Pattern;

import utils.Utils;

public class Test {

	public static void main(String[] args) throws Exception {
		int a=5;
		InputStream inputStream =new FileInputStream("data/validate.txt");
		byte[] re=new byte[4];
		inputStream.read(re, 0, 4);
		System.out.println(Utils.byteArrayToInt(re, 0));
	}

}
