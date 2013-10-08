package uk.ac.ox.cs.chaste.fc.mgmt;

import java.io.File;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.math.BigInteger;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Properties;
import java.util.Random;

import javax.mail.Message;
import javax.mail.MessagingException;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;

import org.apache.commons.lang3.StringEscapeUtils;


public class Tools
{
	private static Random rand = new SecureRandom ();
	
	private static String mailFrom = "noreply@chaste.cs.ox.ac.uk";
	private static String mailFromName = "Chaste";
	private static String domainName = "chaste.cs.ox.ac.uk";
	private static String tempDir = "/tmp/chasteTempDir";
	private static String storageDir = "/tmp/chasteStorageDir";
	
	public static final String FILESEP = System.getProperty("file.separator");
	public static final SimpleDateFormat dateFormater = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
	//public static final SimpleDateFormat dateFormater = new SimpleDateFormat("MMM d, yyyy 'at' h:mm a");
	
	public static final String getPassword (int min, int max)
	{
		String pw = "";
		
		while (pw.length () < max)
			pw += new BigInteger (50, rand).toString (32);
		
		return pw.substring (0, new Random ().nextInt(max - min) + min);
	}
	
	public static void delete (File f, boolean breakable) throws IOException
	{
		if (f.isDirectory()) {
	    for (File c : f.listFiles())
	      delete(c, breakable);
	  }
		if (!f.delete ())
		{
			if (breakable)
				throw new IOException ("unable to delete " + f);
		}
	}
	
	
	public static void setTempDir (String t)
	{
		tempDir = t;
	}
	
	
	public static String getTempDir ()
	{
		return tempDir;
	}
	
	
	public static void setMailFrom (String nfrom)
	{
		mailFrom = nfrom;
	}
	
	public static void setMailFromName (String nfrom)
	{
		mailFromName = nfrom;
	}
	
	public static void setDomainName (String dns)
	{
		domainName = dns;
	}
	
	public static String getDomainName ()
	{
		return domainName;
	}
	
	public static final String validataUserInput (String input)
	{
		return StringEscapeUtils.escapeHtml3(input);
	}
	
	public static final void sendMail (String to, String toName, String subject, String body) throws UnsupportedEncodingException, MessagingException
	{
		Session session = Session.getDefaultInstance(new Properties(), null);
		Message msg = new MimeMessage(session);
    msg.setFrom(new InternetAddress(mailFrom, mailFromName));
    msg.addRecipient(Message.RecipientType.TO, new InternetAddress(to, toName));
    msg.setSubject(subject);
    msg.setText(body);
    Transport.send(msg);
	}
	
	public static final String hash (String msg)
	{
		MessageDigest md;
		try
		{
			md = MessageDigest.getInstance("MD5");
	    md.update(msg.getBytes());
	
	    byte byteData[] = md.digest();
	
	    StringBuffer sb = new StringBuffer();
	    for (int i = 0; i < byteData.length; i++)
	        sb.append(Integer.toString((byteData[i] & 0xff) + 0x100, 16).substring(1));
			return sb.toString ();
		}
		catch (NoSuchAlgorithmException e)
		{
			e.printStackTrace();
			return null;
		}
	}
	
	public static final String formatTimeStamp (Timestamp date)
	{
		return dateFormater.format(date);
		/*Calendar c = Calendar.getInstance();//new Calendar ();
		c.setTime (date);
		return "" + c.getTimeInMillis ();*/
	}


	public static void setStorageDir (String tmp)
	{
		storageDir = tmp;
	}
	
	
	public static String getProtocolStorageDir ()
	{
		return storageDir + FILESEP + "protocols";
	}
	
	
	public static String getModelStorageDir ()
	{
		return storageDir + FILESEP + "models";
	}
	
	public static String convertForURL (String someStr)
	{
		String s = someStr.replaceAll("[^A-Za-z0-9]", "");
		while (s.length () < 5)
			s += getPassword (4, 6);
		return s;
	}
}
