package uk.ac.ox.cs.chaste.fc.web;

import java.io.IOException;
import java.sql.SQLException;

import javax.naming.NamingException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import org.json.simple.JSONObject;

import uk.ac.ox.cs.chaste.fc.beans.Notifications;
import uk.ac.ox.cs.chaste.fc.beans.PageHeader;
import uk.ac.ox.cs.chaste.fc.beans.PageHeaderScript;
import uk.ac.ox.cs.chaste.fc.beans.User;
import uk.ac.ox.cs.chaste.fc.mgmt.DatabaseConnector;

public class Login extends WebModule
{

	public Login () throws NamingException, SQLException
	{
		super ();
	}

	@Override
	protected String answerWebRequest (HttpServletRequest request, HttpServletResponse response, PageHeader header, DatabaseConnector db,
		Notifications notifications, User user, HttpSession session)
	{
		if (user.isAuthorized ())
			return "Index.jsp";
		
		header.addScripts (new PageHeaderScript ("res/js/login.js", "text/javascript", "UTF-8", null));
		return "Login.jsp";
	}

	@SuppressWarnings("unchecked")
	@Override
	protected JSONObject answerApiRequest (HttpServletRequest request, 
		HttpServletResponse response, DatabaseConnector db,
		Notifications notifications, JSONObject querry, User user, HttpSession session) throws IOException
	{
		JSONObject answer = new JSONObject();
		
		Object task = querry.get ("task");
		if (task == null || !task.equals ("logmein"))
		{
			response.setStatus (HttpServletResponse.SC_BAD_REQUEST);
			throw new IOException ("nothing to do.");
		}
		
		String mail = (String) querry.get ("mail");
		String password = (String) querry.get ("password");
		Boolean remember = (Boolean) querry.get ("remember");
		
		if (mail == null || password == null)
		{
			response.setStatus (HttpServletResponse.SC_BAD_REQUEST);
			throw new IOException ("bad request.");
		}
		
		if (remember == null)
			remember = false;
		
		user.authByForm (session, mail, password, remember);
		
		if (user.isAuthorized ())
		{
			//System.out.println ("succ auth");
			
			JSONObject res = new JSONObject ();
			res.put ("response", true);
			res.put ("responseText", "successfully logged in.");
			answer.put ("login", res);
		}
		else
		{
			System.out.println ("failed auth");
			JSONObject res = new JSONObject ();
			res.put ("response", false);
			res.put ("responseText", "failed to login.");
			answer.put ("login", res);
		}
		
		return answer;
	}

}
