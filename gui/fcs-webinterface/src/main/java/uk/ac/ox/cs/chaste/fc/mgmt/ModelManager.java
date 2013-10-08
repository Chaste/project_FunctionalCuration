/**
 * 
 */
package uk.ac.ox.cs.chaste.fc.mgmt;

import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.TreeSet;

import uk.ac.ox.cs.chaste.fc.beans.ComputationalModel;
import uk.ac.ox.cs.chaste.fc.beans.ComputationalModelVersion;
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
public class ModelManager
{
	private DatabaseConnector db;
	private HashMap<Integer, ComputationalModel> knownModels;
	private HashMap<Integer, ComputationalModelVersion> knownVersions;
	private Notifications note;
	
	private static final String SQL_SELECT_BEGIN =  
		"SELECT u.acronym AS versionauthor,"
		+ " m.id AS versionid,"
		+ " m.version AS versionname,"
		+ " m.created AS versioncreated,"
		+ " m.filepath AS versionfilepath,"
		+ " u2.acronym AS modelauthor,"
		+ " mo.id AS modelid,"
		+ " mo.name AS modelname,"
		+ " mo.created AS modelcreated,"
		+ " COUNT(mf.file) AS numfiles"
		+ " FROM       `modelversions` m"
		+ " INNER JOIN `user` u on m.author = u.id"
		+ " INNER JOIN `model_files` mf on mf.model = m.id"
		+ " INNER JOIN `models` mo on m.model=mo.id"
		+ " INNER JOIN `user` u2 on mo.author = u2.id";
	private static final String SQL_SELECT_END = 
		" GROUP BY m.id"
		+ " ORDER BY mo.name, m.version";
	
	public ModelManager (DatabaseConnector db, Notifications note)
	{
		this.db = db;
		knownModels = new HashMap<Integer, ComputationalModel> ();
		knownVersions = new HashMap<Integer, ComputationalModelVersion> ();
		this.note = note;
	}
	
	private TreeSet<ComputationalModel> evaluateResult (ResultSet rs) throws SQLException
	{
		TreeSet<ComputationalModel> res = new TreeSet<ComputationalModel> (new ComputationalModel.SortByName ());
		while (rs != null && rs.next ())
		{
			int vid = rs.getInt ("versionid");
			int mid = rs.getInt ("modelid");
			
			ComputationalModel cur = null;
			if (knownModels.get (mid) != null)
			{
				cur = knownModels.get (mid);
				res.add (cur);
			}
			else
			{
				cur = new ComputationalModel (
					mid,
					rs.getString ("modelname"),
					rs.getString ("modelauthor"),
					rs.getTimestamp ("modelcreated")
					);
				res.add (cur);
				knownModels.put (mid, cur);
			}
			
			if (knownVersions.get (vid) != null)
				cur.addVersion (knownVersions.get (vid));
			else
			{
				ComputationalModelVersion neu = new ComputationalModelVersion (
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
		PreparedStatement st = db.prepareStatement ("DELETE FROM `modelversions` WHERE `id`=?");
    ResultSet rs = null;
    boolean ok = false;
		
		try
		{
			st.setInt (1, versionId);
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
          throw new SQLException("Deleting model version failed, no rows affected.");
      ok = true;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err deleting model version: " + e.getMessage ());
			LOGGER.error ("db problem while deleting model version", e);
			ok = false;
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return ok;
	}
	
	public int createModel (String name, User u)
	{
		PreparedStatement st = db.prepareStatement ("INSERT INTO `models`(`name`, `author`) VALUES (?,?)");
    ResultSet rs = null;
    int id = -1;
		
		try
		{
			st.setString (1, name);
			st.setInt (2, u.getId ());
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
      {
          throw new SQLException("Creating model failed, no rows affected.");
      }

      rs = st.getGeneratedKeys();
      if (rs.next())
      	id = rs.getInt (1);
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err creating model: " + e.getMessage ());
			LOGGER.error ("db problem while creating model", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return id;
	}
	
	public int createVersion (int modelId, String versionName, String filePath, User u)
	{
		PreparedStatement st = db.prepareStatement ("INSERT INTO `modelversions`(`author`, `model`, `version`, `filepath`) VALUES (?,?,?,?)");
    ResultSet rs = null;
    int id = -1;
		
		try
		{
			st.setInt (1, u.getId ());
			st.setInt (2, modelId);
			st.setString (3, versionName);
			st.setString (4, filePath);
			
			int affectedRows = st.executeUpdate();
      if (affectedRows == 0)
      {
          throw new SQLException("Creating version failed, no rows affected.");
      }

      rs = st.getGeneratedKeys();
      if (rs.next())
      	id = rs.getInt (1);
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err creating model version: " + e.getMessage ());
			LOGGER.error ("db problem while creating model version", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return id;
	}
	
	public ComputationalModel getModelById (int id)
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
			note.addError ("sql err retrieving models: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving models", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return knownModels.get (id);
	}
	
	public ComputationalModel getModelByName (String name)
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
			note.addError ("sql err retrieving models: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving models", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		for (ComputationalModel m : knownModels.values ())
			if (m.getName ().equals (name))
				return m;
		return null;
	}
	
	public ComputationalModelVersion getVersionById (int id)
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
			note.addError ("sql err retrieving model version: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving model version", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		
		return knownVersions.get (id);
	}
	
	public TreeSet<ComputationalModel> getModelsOfAuthor (String nick)
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
			TreeSet<ComputationalModel> res = evaluateResult (rs);
			db.closeRes (st);
			db.closeRes (rs);
			return res;
		}
		catch (SQLException e)
		{
			e.printStackTrace();
			note.addError ("sql err retrieving models: " + e.getMessage ());
			LOGGER.error ("db problem while retrieving models", e);
		}
		finally
		{
			db.closeRes (st);
			db.closeRes (rs);
		}
		return null;
	}
}
