/**
 * 
 */
package uk.ac.ox.cs.chaste.fc.beans;

import java.sql.Timestamp;

import org.json.simple.JSONObject;

import uk.ac.ox.cs.chaste.fc.mgmt.Tools;


/**
 * @author martin
 *
 */
public class ChasteFile
{
	
	public int getId ()
	{
		return id;
	}


	
	public String getName ()
	{
		return name;
	}


	
	public Timestamp getFilecreated ()
	{
		return filecreated;
	}


	
	public String getFilevis ()
	{
		return filevis;
	}


	
	public String getFiletype ()
	{
		return filetype;
	}


	
	public String getAuthor ()
	{
		return author;
	}


	
	public long getSize ()
	{
		return size;
	}




	private int id;
	private String name;
	private Timestamp filecreated;
	private String filevis;
	private String filetype;
	private String author;
	private String url;
	private long size;

	public ChasteFile (int id, String name, Timestamp filecreated,
		String filevis, String filetype, long size, String author)
	{
		super ();
		this.id = id;
		this.name = name;
		this.filecreated = filecreated;
		this.filevis = filevis;
		this.filetype = filetype;
		this.author = author;
		this.url = Tools.convertForURL (name);
		this.size = size;
	}
	
	
	/**
	 * Returns an URL aware name of this file. That's not the URL to download the file!
	 *
	 * @return the url
	 */
	public String getUrl ()
	{
		return url;
	}
	


	
	public String getCreated ()
	{
		return Tools.formatTimeStamp (filecreated);
	}




	@SuppressWarnings("unchecked")
	public JSONObject toJson ()
	{
		JSONObject json = new JSONObject ();

		json.put ("id", id);
		json.put ("created", getCreated ());
		json.put ("filetype", filetype);
		json.put ("author", author);
		json.put ("size", size);
		json.put ("name", name);
		
		return json;
	}
}
