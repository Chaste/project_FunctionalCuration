/**
 * 
 */
package uk.ac.ox.cs.chaste.fc.mgmt;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.Vector;

import uk.ac.ox.cs.chaste.fc.beans.ChasteFile;
import uk.ac.ox.cs.chaste.fc.beans.ChasteProtocolVersion;
import uk.ac.ox.cs.chaste.fc.beans.ComputationalModelVersion;
import uk.ac.ox.cs.chaste.fc.beans.Notifications;
import uk.ac.ox.cs.chaste.fc.beans.User;
import de.binfalse.bflog.LOGGER;


/**
 * @author martin
 *
 * TODO: is current user allowed to see the stuff?
 */
public class ChasteFileManager
{
	private DatabaseConnector db;
	private HashMap<Integer, ChasteFile> knownFiles;
	private Notifications note;
	
	public ChasteFileManager (DatabaseConnector db, Notifications note)
	{
		this.db = db;
		this.note = note;
		knownFiles = new HashMap<Integer, ChasteFile> ();
	}
	
	
	private Vector<ChasteFile> evaluateResult (ResultSet rs) throws SQLException
	{
		Vector<ChasteFile> res = new Vector<ChasteFile> ();
		while (rs != null && rs.next ())
		{
			
			int id = rs.getInt ("fileid");
			
			if (knownFiles.get (id) != null)
				res.add (knownFiles.get (id));
			else
			{
				ChasteFile file = new ChasteFile (
					id,
					rs.getString ("filepath"),
					rs.getTimestamp ("filecreated"),
					rs.getString ("filevis"),
					rs.getString ("filetype"),
					rs.getLong ("filesize"),
					rs.getString ("author")
					);
				knownFiles.put (id, file);
				res.add (file);
			}
		}
		return res;
		
	}


	public boolean removeFile (int fileId)
	{
		PreparedStatement st = db.prepareStatement ("DELETE FROM `files` WHERE `id`=?");
    ResultSet rs = null;
    boolean ok = false;
		
		try
		{
			st.setInt (1, fileId);
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
          throw new SQLException("Removing file failed, no rows affected.");
      ok = true;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err removing file: " + e.getMessage ());
			LOGGER.error ("db problem while removing file", e);
			ok = false;
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return ok;
	}
	
	public int addFile (String name, String type, User u, long size)
	{
		PreparedStatement st = db.prepareStatement ("INSERT INTO `files`(`relpath`, `type`, `author`, `size`) VALUES (?,?,?,?)");
    ResultSet rs = null;
    int id = -1;
		
		try
		{
			st.setString (1, name);
			st.setString (2, type);
			st.setInt (3, u.getId ());
			st.setLong (4, size);
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
      {
          throw new SQLException("Creating file failed, no rows affected.");
      }

      rs = st.getGeneratedKeys();
      if (rs.next())
      	id = rs.getInt (1);
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err adding file: " + e.getMessage ());
			LOGGER.error ("db problem while adding file", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return id;
	}


	public boolean associateFileToProtocol (int fileId, int versionId)
	{
		PreparedStatement st = db.prepareStatement ("INSERT INTO `protocol_files`(`protocol`, `file`) VALUES (?,?)");
    ResultSet rs = null;
    boolean ok = false;
		
		try
		{
			st.setInt (1, versionId);
			st.setInt (2, fileId);
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
          throw new SQLException("Associating file to protocol failed, no rows affected.");
      ok = true;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err associating file to protocol: " + e.getMessage ());
			LOGGER.error ("db problem while associating file to ptotocol", e);
			ok = false;
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return ok;
	}
	
	public boolean associateFileToModel (int fileId, int versionId)
	{
		PreparedStatement st = db.prepareStatement ("INSERT INTO `model_files`(`model`, `file`) VALUES (?,?)");
    ResultSet rs = null;
    boolean ok = false;
		
		try
		{
			st.setInt (1, versionId);
			st.setInt (2, fileId);
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
          throw new SQLException("Associating file to model failed, no rows affected.");
      ok = true;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err associating file to model: " + e.getMessage ());
			LOGGER.error ("db problem while associating file to model", e);
			ok = false;
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return ok;
	}


	public boolean getFiles (ChasteProtocolVersion vers)
	{
		int id = vers.getId ();
		ResultSet rs = null;

		PreparedStatement st = db.prepareStatement (
			"SELECT f.id AS fileid, f.relpath AS filepath, f.created AS filecreated, f.visibility AS filevis, f.type AS filetype, u.acronym AS author, f.size AS filesize FROM "
			+ "`files` f"
			+ " INNER JOIN `protocol_files` mf on mf.file = f.id"
			+ " INNER JOIN `user` u on f.author = u.id"
			+ " WHERE mf.protocol=?"
			+ " ORDER BY f.relpath");
		try
		{
			st.setInt (1, id);
			st.execute ();
			rs = st.getResultSet ();
			Vector<ChasteFile> res = evaluateResult (rs);
			vers.setFiles (res);
			return true;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err retrieving files: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving files", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return false;
	}
	
	
	public boolean getFiles (ComputationalModelVersion model)
	{
		int id = model.getId ();
		ResultSet rs = null;

		PreparedStatement st = db.prepareStatement (
			"SELECT f.id AS fileid, f.relpath AS filepath, f.created AS filecreated, f.visibility AS filevis, f.type AS filetype, u.acronym AS author, f.size AS filesize FROM "
			+ "`files` f"
			+ " INNER JOIN `model_files` mf on mf.file = f.id"
			+ " INNER JOIN `user` u on f.author = u.id"
			+ " WHERE mf.model=?"
			+ " ORDER BY f.relpath");
		try
		{
			st.setInt (1, id);
			st.execute ();
			rs = st.getResultSet ();
			Vector<ChasteFile> res = evaluateResult (rs);
			model.setFiles (res);
			return true;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err retrieving files: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving files", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return false;
	}


	public ChasteFile getFileById (int fileId)
	{
		return null;
	}
	
}
