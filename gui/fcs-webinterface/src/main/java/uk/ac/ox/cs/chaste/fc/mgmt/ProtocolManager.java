/**
 * 
 */
package uk.ac.ox.cs.chaste.fc.mgmt;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.TreeSet;

import uk.ac.ox.cs.chaste.fc.beans.ChasteProtocol;
import uk.ac.ox.cs.chaste.fc.beans.ChasteProtocolVersion;
import uk.ac.ox.cs.chaste.fc.beans.Notifications;
import uk.ac.ox.cs.chaste.fc.beans.User;
import de.binfalse.bflog.LOGGER;


/**
 * @author martin
 *
 * TODO: optimize (hashtables etc)
 * 
 * TODO: is current user allowed to see the stuff?
 */
public class ProtocolManager
{
	private DatabaseConnector db;
	private HashMap<Integer, ChasteProtocol> knownProtocols;
	private HashMap<Integer, ChasteProtocolVersion> knownVersions;
	private Notifications note;
	
	private static final String SQL_SELECT_BEGIN =  
		"SELECT u.acronym AS versionauthor,"
		+ " m.id AS versionid,"
		+ " m.version AS versionname,"
		+ " m.created AS versioncreated,"
		+ " m.filepath AS versionfilepath,"
		+ " u2.acronym AS protocolauthor,"
		+ " mo.id AS protocolid,"
		+ " mo.name AS protocolname,"
		+ " mo.created AS protocolcreated,"
		+ " COUNT(mf.file) AS numfiles"
		+ " FROM       `protocolversions` m"
		+ " INNER JOIN `user` u on m.author = u.id"
		+ " INNER JOIN `protocol_files` mf on mf.protocol = m.id"
		+ " INNER JOIN `protocols` mo on m.protocol=mo.id"
		+ " INNER JOIN `user` u2 on mo.author = u2.id";
	private static final String SQL_SELECT_END = 
		" GROUP BY m.id"
		+ " ORDER BY mo.name, m.version";
	
	public ProtocolManager (DatabaseConnector db, Notifications note)
	{
		this.db = db;
		knownProtocols = new HashMap<Integer, ChasteProtocol> ();
		knownVersions = new HashMap<Integer, ChasteProtocolVersion> ();
		this.note = note;
	}
	
	private TreeSet<ChasteProtocol> evaluateResult (ResultSet rs) throws SQLException
	{
		TreeSet<ChasteProtocol> res = new TreeSet<ChasteProtocol> (new ChasteProtocol.SortByName ());
		while (rs != null && rs.next ())
		{
			int vid = rs.getInt ("versionid");
			int mid = rs.getInt ("protocolid");
			
			ChasteProtocol cur = null;
			if (knownProtocols.get (mid) != null)
			{
				cur = knownProtocols.get (mid);
				res.add (cur);
			}
			else
			{
				cur = new ChasteProtocol (
					mid,
					rs.getString ("protocolname"),
					rs.getString ("protocolauthor"),
					rs.getTimestamp ("protocolcreated")
					);
				res.add (cur);
				knownProtocols.put (mid, cur);
			}
			
			if (knownVersions.get (vid) != null)
				cur.addVersion (knownVersions.get (vid));
			else
			{
				ChasteProtocolVersion neu = new ChasteProtocolVersion (
					cur,
					vid,
					rs.getString ("versionname"),
					rs.getString ("versionauthor"),
					rs.getString ("versionfilepath"),
					rs.getTimestamp ("versioncreated"),
					rs.getInt ("numfiles")
				);
				cur.addVersion (neu);
				knownVersions.put (vid, neu);
			}
		}
		return res;
	}

	public boolean removeVersion (int versionId)
	{
		PreparedStatement st = db.prepareStatement ("DELETE FROM `protocolversions` WHERE `id`=?");
    ResultSet rs = null;
    boolean ok = false;
		
		try
		{
			st.setInt (1, versionId);
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
          throw new SQLException("Deleting protocol version failed, no rows affected.");
      ok = true;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err deleting protocol version: " + e.getMessage ());
			LOGGER.error ("db problem while deleting protocol version", e);
			ok = false;
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return ok;
	}
	
	public int createProtocol (String name, User u)
	{
		PreparedStatement st = db.prepareStatement ("INSERT INTO `protocols`(`name`, `author`) VALUES (?,?)");
    ResultSet rs = null;
    int id = -1;
		
		try
		{
			st.setString (1, name);
			st.setInt (2, u.getId ());
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
      {
          throw new SQLException("Creating protocol failed, no rows affected.");
      }

      rs = st.getGeneratedKeys();
      if (rs.next())
      	id = rs.getInt (1);
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err creating protocol: " + e.getMessage ());
			LOGGER.error ("db problem while creating protocol", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return id;
	}
	
	public int createVersion (int protocolid, String versionName, String filePath, User u)
	{
		PreparedStatement st = db.prepareStatement ("INSERT INTO `protocolversions`(`author`, `protocol`, `version`, `filepath`) VALUES (?,?,?,?)");
    ResultSet rs = null;
    int id = -1;
		
		try
		{
			st.setInt (1, u.getId ());
			st.setInt (2, protocolid);
			st.setString (3, versionName);
			st.setString (4, filePath);
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
      {
          throw new SQLException("Creating protocol version failed, no rows affected.");
      }

      rs = st.getGeneratedKeys();
      if (rs.next())
      	id = rs.getInt (1);
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err creating protocol version: " + e.getMessage ());
			LOGGER.error ("db problem while creating protocol version", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return id;
	}
	
	public ChasteProtocol getProtocolById (int id)
	{
		
		PreparedStatement st = db.prepareStatement (
			SQL_SELECT_BEGIN
			+ " WHERE mo.id=?"
			+ SQL_SELECT_END);
		ResultSet rs = null;
		try
		{
			st.setInt (1, id);
			st.execute ();
			rs = st.getResultSet ();
			evaluateResult (rs);
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err retrieving protocols: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving protocols", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return knownProtocols.get (id);
	}
	
	public ChasteProtocol getProtocolByName (String name)
	{
		
		PreparedStatement st = db.prepareStatement (
			SQL_SELECT_BEGIN
			+ " WHERE mo.name=?"
			+ SQL_SELECT_END);
		ResultSet rs = null;
		try
		{
			st.setString (1, name);
			st.execute ();
			rs = st.getResultSet ();
			evaluateResult (rs);
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err retrieving protocols: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving protocols", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		for (ChasteProtocol m : knownProtocols.values ())
			if (m.getName ().equals (name))
				return m;
		return null;
	}
	
	public ChasteProtocolVersion getVersionById (int id)
	{
		if (knownVersions.get (id) != null)
			return knownVersions.get (id);
		
		PreparedStatement st = db.prepareStatement (
			SQL_SELECT_BEGIN
			+ " WHERE m.id=?"
			+ SQL_SELECT_END);
		ResultSet rs = null;
		try
		{
			st.setInt (1, id);
			st.execute ();
			rs = st.getResultSet ();
			evaluateResult (rs);
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err retrieving protocol version: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving protocol version", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return knownVersions.get (id);
	}
	
	public TreeSet<ChasteProtocol> getProtocolsOfAuthor (String nick)
	{
		PreparedStatement st = db.prepareStatement (
			SQL_SELECT_BEGIN
			+ " WHERE u.acronym=?"
			+ SQL_SELECT_END);
		ResultSet rs = null;
		try
		{
			st.setString (1, nick);
			st.execute ();
			rs = st.getResultSet ();
			TreeSet<ChasteProtocol> res = evaluateResult (rs);
			db.closeRes (st);
			db.closeRes (rs);
			return res;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err retrieving protocols: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving protocols", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		return null;
	}
}
