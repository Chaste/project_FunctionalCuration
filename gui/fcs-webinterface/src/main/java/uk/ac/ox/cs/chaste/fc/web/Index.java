package uk.ac.ox.cs.chaste.fc.web;

import java.sql.SQLException;

import javax.naming.NamingException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import org.json.simple.JSONObject;

import uk.ac.ox.cs.chaste.fc.beans.Notifications;
import uk.ac.ox.cs.chaste.fc.beans.PageHeader;
import uk.ac.ox.cs.chaste.fc.beans.User;
import uk.ac.ox.cs.chaste.fc.mgmt.DatabaseConnector;



/**
 * TODO: check mysql resources are closed. check twice!
 * TODO: remove all print stack traces and configure the logger
 * TODO: delete models etc (effectively make them just invisible to prevent loss of data), delete user -> unset password & cookie
			// TODO: check all these tostring things for nullpointers! probably replace `Object task = querry.get ("task");` 
 * TODO: verify rigts on different objects
 * TODO: documentation of installation
 * TODO: display readme file, if exists?
 * TODO: check model/protocol etc name -> varchar 100 enough?
 * TODO: merge model/protocol -> less duplicates
 */

public class Index
	extends WebModule
{
	
	
	public Index () throws NamingException, SQLException
	{
		super ();
	}

	public String answerWebRequest (HttpServletRequest request, HttpServletResponse response, PageHeader header, DatabaseConnector db, Notifications notifications, User user, HttpSession session)
	{
		/*LOGGER.error ("index 1");
		
		LOGGER.error ("index 2");
		
		try
		{
			-> ResultSet rs = db.execute ("SELECT * FROM user");
	    while(rs.next())
	    {
	    	System.out.println("row: " + rs.getString ("mail") + "<br>");
	    }
			rs.close();
		}
		catch (SQLException e)
		{
			LOGGER.error ("error connection querrying db", e);
			notifications.addError ("db err: " + e.getMessage ());
		}

		LOGGER.error ("index 3");
		
		notifications.addError ("test error");
		notifications.addInfo ("test info");
		


		LOGGER.error ("index 4");*/

		//notifications.addError ("test error");
		//notifications.addInfo ("test info");
		
		return "Index.jsp";
	}

	@Override
	protected JSONObject answerApiRequest (HttpServletRequest request, 
		HttpServletResponse response, DatabaseConnector db,
		Notifications notifications, JSONObject querry, User user, HttpSession session)
	{
		// TODO Auto-generated method stub
		return null;
	}
}