package uk.ac.ox.cs.chaste.fc.beans;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;

import javax.servlet.http.Cookie;
import javax.servlet.http.HttpSession;

import uk.ac.ox.cs.chaste.fc.mgmt.CookieManager;
import uk.ac.ox.cs.chaste.fc.mgmt.DatabaseConnector;
import uk.ac.ox.cs.chaste.fc.mgmt.Tools;
import de.binfalse.bflog.LOGGER;


public class User
{
	private static final String COOKIE_NAME = "ChasteUser";
	private static final String SESSION_ATTR = "ChasteUser";
	
	private String nick;
	private String mail;
	private String institution;
	private String cookie;
	private Timestamp created;
	private String role;
	private int id;
	
	private DatabaseConnector db;
	private Notifications note;
	private CookieManager cookieMgmt;
	
	public User (DatabaseConnector db, Notifications note, CookieManager cookieMgmt)
	{
		this.db = db;
		this .note = note;
		this.cookieMgmt = cookieMgmt;
	}
	
	
	/**
	 * Instantiates a new read-only user. No auth etc. Just to display.
	 */
	public User (int id, String mail, String nick, String institution,
		Timestamp created, String role)
	{
		this.id = id;
		this.mail = mail;
		this.nick = nick;
		this.institution = institution;
		this.created = created;
		this.role =role;
	}


	private void reqDB (ResultSet rs) throws SQLException
	{
		if (rs == null)
			return;
		
		while (rs != null && rs.next())
		{
			nick = rs.getString ("acronym");
			mail = rs.getString ("mail");
			institution = rs.getString ("institution");
			cookie = rs.getString ("cookie");
			role = rs.getString ("role");
			created = rs.getTimestamp ("created");
			id = rs.getInt ("id");
		}
	}
	
	public boolean isAuthorized ()
	{
		return mail != null;
	}
	
	public boolean authByForm (HttpSession session, String mail, String password, boolean remember)
	{
		PreparedStatement st = db.prepareStatement ("SELECT * FROM `user` WHERE `mail`=? AND `password`=MD5(?)");
		ResultSet rs = null;
		try
		{
			st.setString (1, mail);
			st.setString (2, password);
			st.execute ();
			rs = st.getResultSet ();
			reqDB (rs);
			if (this.mail != null)
			{
				session.setAttribute (SESSION_ATTR, this.mail);
				if (cookie != null && cookie.length () > 5 && remember)
					setCookie ();
			}
			return this.mail != null;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err: " + e.getMessage ());
			LOGGER.error ("db problem during session auth", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		return false;
	}
	
	
	public String getNick ()
	{
		return nick;
	}

	
	public String getMail ()
	{
		return mail;
	}

	
	public String getInstitution ()
	{
		return institution;
	}

	
	public String getCreated ()
	{
		return Tools.formatTimeStamp (created);
	}

	
	public String getRole ()
	{
		return role;
	}

	
	public int getId ()
	{
		return id;
	}

	
	public Notifications getNote ()
	{
		return note;
	}
	
	public void logout (HttpSession session)
	{
		mail = null;
		session.setAttribute (SESSION_ATTR, null);
		Cookie userCookie = new Cookie (COOKIE_NAME, "");
		userCookie.setMaxAge (1);
		cookieMgmt.setCookie (userCookie);
	}

	public void authByRequest (HttpSession session)
	{
		String mail = (String) session.getAttribute (SESSION_ATTR);
		//System.out.println ("mail: " + mail);
		if (mail != null)
		{
			PreparedStatement st = db.prepareStatement ("SELECT * FROM `user` WHERE `mail`=?");
			ResultSet rs = null;
			try
			{
				st.setString (1, mail);
				st.execute ();
				rs = st.getResultSet ();
				reqDB (rs);
			}
			catch (SQLException e)
			{
				e.printStackTrace();
				note.addError ("sql err: " + e.getMessage ());
				LOGGER.error ("db problem during session auth", e);
			}
			finally
			{
				db.closeRes (st);
				db.closeRes (rs);
			}
		}
		Cookie cook = cookieMgmt.getCookie (COOKIE_NAME);
		if (cook != null)
		{
			if (this.mail == null)
			{
				// not yet authed
				String val = cook.getValue ();
				PreparedStatement st = db.prepareStatement ("SELECT * FROM `user` WHERE `cookie`=?");
				ResultSet rs = null;
				try
				{
					st.setString (1, val);
					st.execute ();
					rs = st.getResultSet ();
					reqDB (rs);
					rs.close ();
				}
				catch (SQLException e)
				{
					e.printStackTrace();
					note.addError ("sql err: " + e.getMessage ());
					LOGGER.error ("db problem during session auth", e);
				}
				finally
				{
					db.closeRes (st);
					db.closeRes (rs);
				}
			}
			
			if (this.mail != null)
			{
				// we're authed -> reset cookie
				setCookie ();
			}
		}
		//System.out.println ("auth by req: " + isAuthorized ());
	}
	
	private void setCookie ()
	{
		Cookie userCookie = new Cookie (COOKIE_NAME, cookie);
		userCookie.setMaxAge (60*60*24*90);
		cookieMgmt.setCookie (userCookie);
	}
}
