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
import uk.ac.ox.cs.chaste.fc.mgmt.UserManager;
import de.binfalse.bflog.LOGGER;

public class Admin extends WebModule
{
	private static final long	serialVersionUID	= 8100907290557329579L;

	public Admin () throws NamingException, SQLException
	{
		super ();
	}

	@Override
	protected String answerWebRequest (HttpServletRequest request, HttpServletResponse response, PageHeader header, DatabaseConnector db,
		Notifications notifications, User user, HttpSession session)
	{
		if (!user.isAuthorized () || !user.getRole ().equals ("ADMIN"))
			return errorPage (request, response, null);
		
		header.addScripts (new PageHeaderScript ("res/js/admin.js", "text/javascript", "UTF-8", null));

		request.setAttribute ("Users", UserManager.getUsers (db, notifications));
		
		return "Admin.jsp";
	}

	@SuppressWarnings("unchecked")
	@Override
	protected JSONObject answerApiRequest (HttpServletRequest request, 
		HttpServletResponse response, DatabaseConnector db,
		Notifications notifications, JSONObject querry, User user, HttpSession session) throws IOException
	{
		if (!user.isAuthorized () || !user.getRole ().equals ("ADMIN"))
			throw new IOException ("not allowed.");
		
		JSONObject answer = new JSONObject();
		
		
		Object task = querry.get ("task");
		if (task == null)
		{
			response.setStatus (HttpServletResponse.SC_BAD_REQUEST);
			throw new IOException ("nothing to do.");
		}
		
		if (task.equals ("updateUserRole"))
		{
			int id = -1;
			try
			{
				id = Integer.parseInt (querry.get ("user").toString ());
			}
			catch (NumberFormatException | NullPointerException e)
			{
				e.printStackTrace ();
				LOGGER.warn ("cannot parse user id from user provided string: " + querry.get ("user"));
				throw new IOException ("user not found");
			}
			
			if (id == user.getId ())
				throw new IOException ("not allowed to change your own rule");
			
			Object role = querry.get ("role");
			if (role.equals (UserManager.ROLE_ADMIN))
				UserManager.updateUserRole (id, UserManager.ROLE_ADMIN, db, notifications);
			else if (role.equals (UserManager.ROLE_MODELER))
				UserManager.updateUserRole (id, UserManager.ROLE_MODELER, db, notifications);
			else if (role.equals (UserManager.ROLE_GUEST))
				UserManager.updateUserRole (id, UserManager.ROLE_GUEST, db, notifications);
			else
				throw new IOException ("unknown rule");
			
			JSONObject obj = new JSONObject ();
			obj.put ("response", true);
			obj.put ("responseText", "updated user");
			answer.put ("updateUserRole", obj);
		}
		
		return answer;
	}

}
