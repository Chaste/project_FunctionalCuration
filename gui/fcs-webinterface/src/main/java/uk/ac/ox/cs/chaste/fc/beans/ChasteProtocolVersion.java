/**
 * 
 */
package uk.ac.ox.cs.chaste.fc.beans;

import java.sql.Timestamp;
import java.util.Vector;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

import uk.ac.ox.cs.chaste.fc.mgmt.Tools;


/**
 * @author martin
 *
 */
public class ChasteProtocolVersion
{
	private ChasteProtocol protocol;
	private int id;
	private String version;
	private String author;
	private String filePath;
	private String url;
	private Timestamp created;
	private int numFiles;
	private Vector<ChasteFile> files;
	
	public ChasteProtocolVersion (ChasteProtocol model, int id, String version, String author, String filePath, Timestamp created, int numFiles)
	{
		this.protocol = model;
		this.id = id;
		this.version = version;
		this.url = Tools.convertForURL (version);
		this.created = created;
		this.author = author;
		this.filePath = filePath;
		this.numFiles = numFiles;
		files = new Vector<ChasteFile> ();
	}
	
	public ChasteFile getFileById (int id)
	{
		for (ChasteFile cf : files)
			if (cf.getId () == id)
				return cf;
		return null;
	}
	
	
	public String getUrl ()
	{
		return url;
	}
	
	
	public String getFilePath ()
	{
		return filePath;
	}

	public int getId ()
	{
		return id;
	}

	
	public void setFiles (Vector<ChasteFile> files)
	{
		this.files = files;
	}

	
	public Vector<ChasteFile> getFiles ()
	{
		return files;
	}
	
	
	
	public void addFile (ChasteFile file)
	{
		this.files.add (file);
	}
	
	
	public int getNumFiles ()
	{
		return numFiles;
	}



	public String getAuthor ()
	{
		return author;
	}


	
	public String getCreated ()
	{
		return Tools.formatTimeStamp (created);
	}


	public ChasteProtocol getProtocol()
	{
		return protocol;
	}

	
	public String getVersion ()
	{
		return version;
	}


	public void debug ()
	{
		System.out.println ("\t" + getVersion ());
	}


	@SuppressWarnings("unchecked")
	public JSONObject toJson ()
	{
		JSONObject json = new JSONObject ();

		json.put ("version", version);
		json.put ("created", getCreated ());
		json.put ("author", getAuthor ());
		json.put ("numFiles", numFiles);
		json.put ("id", id);
		
		JSONArray f = new JSONArray ();
		for (ChasteFile cf : files)
			f.add (cf.toJson ());
		json.put ("files", f);
		
		return json;
	}
	
	
}
