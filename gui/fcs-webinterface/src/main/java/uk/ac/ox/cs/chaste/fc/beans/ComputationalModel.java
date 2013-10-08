/**
 * 
 */
package uk.ac.ox.cs.chaste.fc.beans;

import java.sql.Timestamp;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

import uk.ac.ox.cs.chaste.fc.mgmt.Tools;


/**
 * @author martin
 *
 */
public class ComputationalModel
{
	private int id;
	private String name;
	private String author;
	private String url;
	private Timestamp created;
	private HashMap<Integer, ComputationalModelVersion> versions;
	
	public static class SortByName implements Comparator<ComputationalModel>
	{
		@Override
		public int compare (ComputationalModel a, ComputationalModel b)
		{
			return a.name.compareTo (b.name);
		}
	}
	
	
	public ComputationalModel (int id, String name, String author, Timestamp created)
	{
		this.id = id;
		this.name = name;
		this.url = Tools.convertForURL (name);
		this.created = created;
		this.author = author;
		versions = new HashMap<Integer, ComputationalModelVersion> ();
	}
	
	public void debug ()
	{
		System.out.println ("model " + name);
		for (ComputationalModelVersion v : versions.values ())
			if (v.getVersion ().equals (name))
				v.debug ();
	}
	
	public ComputationalModelVersion getVersion (String name)
	{
		for (ComputationalModelVersion v : versions.values ())
			if (v.getVersion ().equals (name))
				return v;
		return null;
	}
	
	
	public String getUrl ()
	{
		return url;
	}
	
	public int getId ()
	{
		return id;
	}

	
	public Map<Integer, ComputationalModelVersion> getVersions ()
	{
		return versions;
	}
	
	
	
	public void addVersion (ComputationalModelVersion version)
	{
		this.versions.put (version.getId (), version);
	}



	public String getAuthor ()
	{
		return author;
	}


	
	public String getCreated ()
	{
		return Tools.formatTimeStamp (created);
	}


	public String getName ()
	{
		return name;
	}

	
	
}
