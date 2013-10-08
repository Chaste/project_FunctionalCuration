package uk.ac.ox.cs.chaste.fc.web;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.UnsupportedEncodingException;
import java.sql.SQLException;

import javax.naming.NamingException;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

import uk.ac.ox.cs.chaste.fc.beans.Notifications;
import uk.ac.ox.cs.chaste.fc.beans.PageHeader;
import uk.ac.ox.cs.chaste.fc.beans.User;
import uk.ac.ox.cs.chaste.fc.mgmt.CookieManager;
import uk.ac.ox.cs.chaste.fc.mgmt.DatabaseConnector;
import uk.ac.ox.cs.chaste.fc.mgmt.Tools;
import de.binfalse.bflog.LOGGER;


public abstract class WebModule
extends HttpServlet
{
	public static final String JSP_PATH = "/WEB-INF/";
	public static final String JSP_ERR = "Exception.jsp";
	
	protected boolean fileTransfer;

  /**
   * @throws SQLException 
   * @throws NamingException 
   * @see HttpServlet#HttpServlet()
   */
	public WebModule () throws NamingException, SQLException
	{
		super ();
		fileTransfer = false;
	}
	
	protected abstract JSONObject answerApiRequest (HttpServletRequest request, HttpServletResponse response, DatabaseConnector db, Notifications notifications, JSONObject querry, User user, HttpSession session) throws Exception;

	protected abstract String answerWebRequest (HttpServletRequest request, HttpServletResponse response, PageHeader header, DatabaseConnector db, Notifications notifications, User user, HttpSession session);

	@SuppressWarnings("unchecked")
	private void runApiRequest (HttpServletRequest request, HttpServletResponse response, DatabaseConnector db, Notifications notifications, User user, HttpSession session)
	{
		response.setContentType ("application/json");
		response.setCharacterEncoding ("UTF-8");
		try
		{
			request.setCharacterEncoding ("UTF-8");
		}
		catch (UnsupportedEncodingException e1)
		{
			e1.printStackTrace();
			LOGGER.error ("utf8 not supported by request.");

			response.setStatus (HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
			return;
		}

		PrintWriter out = null;
		try
		{
			out = response.getWriter ();
		}
		catch (IOException e)
		{
			e.printStackTrace ();
			LOGGER.error ("cannot get writer of response.");

			response.setStatus (HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
			return;
		}
		
		
		
		
		
		
		
		JSONObject obj = null;
		try
		{
			if (!fileTransfer)
				obj = answerApiRequest (request, response, db, notifications, getPostRequest (request, response), user, session);
			else
				obj = answerApiRequest (request, response, db, notifications, null, user, session);
		}
		catch (Exception e)
		{
			response.setStatus (HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
			e.printStackTrace();
			obj = new JSONObject ();
			JSONObject n = new JSONObject ();
			JSONArray a = new JSONArray ();
			a.add (e.getMessage ());
			n.put ("errors", a);
			obj.put ("notifications", n);
			out.println (obj);
			return;
		}
		
		if (obj != null)
		{
			if (notifications.isNotifications ())
			{
				JSONObject n = new JSONObject ();
				if (notifications.isError ())
				{
					JSONArray ar = new JSONArray ();
					for (String e : notifications.getErrors ())
						ar.add (e);
					n.put ("errors", ar);
				}
				if (notifications.isInfo ())
				{
					JSONArray ar = new JSONArray ();
					for (String e : notifications.getInfos ())
						ar.add (e);
					n.put ("notes", ar);
				}
				obj.put ("notifications", n);
			}
			out.println (obj);
		}
		else
		{
			response.setStatus (HttpServletResponse.SC_NO_CONTENT);
		}
	}
	
	private String runWebRequest (HttpServletRequest request, HttpServletResponse response, DatabaseConnector db, Notifications notifications, User user, HttpSession session) throws UnsupportedEncodingException
	{
		
		

		PageHeader header = new PageHeader ();
		request.setAttribute ("PageHeader", header);
		request.setAttribute ("Notifications", notifications);
		request.setAttribute ("User", user);
		request.setAttribute ("contextPath", request.getContextPath ());
		
		String ret = answerWebRequest (request, response, header, db, notifications, user, session);
		
		if (ret != null)
		{
			response.setContentType ("text/html");
			response.setCharacterEncoding ("UTF-8");
			request.setCharacterEncoding ("UTF-8");
		}
		return ret;
	}
	
	private void configure ()
	{
		ServletContext context = getServletContext ();
		
		String tmp = context.getInitParameter("production");
		if (!Boolean.parseBoolean (tmp))
			LOGGER.addLevel (LOGGER.DEBUG | LOGGER.INFO);
		
		tmp = context.getInitParameter ("mailSender");
		if (tmp != null)
			Tools.setMailFrom (tmp);
		
		tmp = context.getInitParameter ("mailSenderName");
		if (tmp != null)
			Tools.setMailFromName (tmp);
		
		tmp = context.getInitParameter ("chasteURL");
		if (tmp != null)
			Tools.setDomainName (tmp);
		
		tmp = context.getInitParameter ("tempDir");
		if (tmp != null)
			Tools.setTempDir (tmp);
		
		tmp = context.getInitParameter ("storageDir");
		if (tmp != null)
			Tools.setStorageDir (tmp);
		
	}
	
	private void doAction (HttpServletRequest request, HttpServletResponse response, boolean post) throws ServletException, IOException
	{
		
		LOGGER.setLogFile ("/tmp/chastelog1");
		LOGGER.setLogToFile (true);
		LOGGER.addLevel (LOGGER.WARN | LOGGER.ERROR);
		
		configure ();
		
	  HttpSession session = request.getSession (true);
	  CookieManager cookieMgmt = new CookieManager (request, response);
		Notifications notifications = new Notifications ();
		DatabaseConnector db = new DatabaseConnector (notifications);
		User user = new User (db, notifications, cookieMgmt);
		user.authByRequest (session);
		
		if (post)
		{
			runApiRequest (request, response, db, notifications, user, session);
			db.closeConnection ();
		}
		else
		{
			String dispatch = JSP_PATH + JSP_ERR;
			try
			{
				dispatch = runWebRequest (request, response, db, notifications, user, session);
			}
			catch (Exception e)
			{
				e.printStackTrace ();
				notifications.addError ("Error building site: " + e.getMessage ());
				LOGGER.error ("error building web site", e);
			}
			db.closeConnection ();
			if (dispatch != null)
				request.getRequestDispatcher (JSP_PATH + dispatch).forward (request, response);
		}
	}
	
	
	/**
	 * @see HttpServlet#doGet(HttpServletRequest request, HttpServletResponse
	 *      response)
	 */
	protected void doGet (HttpServletRequest request, HttpServletResponse response)
		throws ServletException,
			IOException
	{
		doAction (request, response, false);
	}
	
	/**
	 * @see HttpServlet#doPost(HttpServletRequest request, HttpServletResponse
	 *      response)
	 */
	protected void doPost (HttpServletRequest request,
		HttpServletResponse response) throws ServletException, IOException
	{
		doAction (request, response, true);
	}
	
	private static final JSONObject getPostRequest (HttpServletRequest request, HttpServletResponse response) throws IOException
	{
		JSONObject querry = null;
		String post = null;
		
		try
		{
			post = getPostContent (request);
			//System.out.println ("post is: " + post);
			querry = (JSONObject) JSONValue.parse (post);
		}
		catch (IOException e)
		{
			LOGGER.error ("error reading content of post request", e);
			e.printStackTrace ();
			response.setStatus (HttpServletResponse.SC_INTERNAL_SERVER_ERROR);
			throw new IOException ("don't understand post content.");
		}
		catch (ClassCastException e)
		{
			LOGGER.error ("error reading content of post request. probably bad formated: " + post, e);
			e.printStackTrace ();
			response.setStatus (HttpServletResponse.SC_BAD_REQUEST);
			throw new IOException ("don't understand post content. probably bad formated: " + post);
		}
		
		if (querry == null)
		{
			response.setStatus (HttpServletResponse.SC_BAD_REQUEST);
			throw new IOException ("don't understand post content. probably bad formated: " + post);
		}
		
		return querry;
	}
	
	protected static String getPostContent (HttpServletRequest request) throws IllegalStateException, IOException
	{
		StringBuffer jb = new StringBuffer ();
		String line = null;
		BufferedReader reader = request.getReader ();
		while ( (line = reader.readLine ()) != null)
		{
			jb.append (line);
		}
		return jb.toString ();
	}
	
	protected String errorPage (HttpServletRequest request, HttpServletResponse response, String msg)
	{
		response.setStatus (HttpServletResponse.SC_NOT_FOUND);
		request.setAttribute ("exceptionMessage", msg == null ? "The page you requested doesn't exist." : msg);
		return "Exception.jsp";
	}
}