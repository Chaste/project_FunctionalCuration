package uk.ac.ox.cs.chaste.fc.mgmt;

import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

import javax.naming.Context;
import javax.naming.InitialContext;
import javax.naming.NamingException;
import javax.sql.DataSource;

import uk.ac.ox.cs.chaste.fc.beans.Notifications;
import de.binfalse.bflog.LOGGER;



public class DatabaseConnector
{
	
	private Connection	connection;
	private Notifications note;
	
	
	public DatabaseConnector (Notifications notifications) throws RuntimeException
	{
		this.note = notifications;
		
		try
		{
			Context initCtx = new InitialContext ();
			Context envCtx = (Context) initCtx.lookup ("java:comp/env");
			DataSource ds = (DataSource) envCtx.lookup ("jdbc/CHASTE");
			
			connection = ds.getConnection ();
		}
		catch (NamingException | SQLException e)
		{
			e.printStackTrace ();
			note.addError ("could not connect to DB: " + e.getMessage ());
			throw new RuntimeException ("Error establishing DB connection");
		}
	}
	
	public PreparedStatement prepareStatement (String query)
	{
		try
		{
			return connection.prepareStatement (query, Statement.RETURN_GENERATED_KEYS);
		}
		catch (SQLException e)
		{
			LOGGER.error ("SQLException: cannot prepare statement", e);
			note.addError ("SQLException: cannot prepare statement");
			e.printStackTrace();
		}
		return null;
	}
	
	public void closeConnection ()
	{
		try
		{
			if (connection != null)
			{
				connection.close ();
				connection = null;
			}
		}
		catch (SQLException e)
		{
			e.printStackTrace ();
		}
	}
	
	
	protected void finalize () throws Throwable
	{
		try
		{
			closeConnection ();
		}
		finally
		{
			super.finalize ();
		}
	}
	
	public void closeRes (Statement s)
	{
		if (s == null)
			return;
		try
		{
			s.close ();
		}
		catch (SQLException e)
		{
			LOGGER.error ("error closing statement", e);
			e.printStackTrace();
		}
	}
	
	public void closeRes (ResultSet rs)
	{
		if (rs == null)
			return;
		try
		{
			rs.close ();
		}
		catch (SQLException e)
		{
			LOGGER.error ("error closing resultset", e);
			e.printStackTrace();
		}
	}
}
