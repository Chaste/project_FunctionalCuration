package uk.ac.ox.cs.chaste.fc.web;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

import javax.naming.NamingException;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;
import javax.servlet.annotation.MultipartConfig;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import javax.servlet.http.Part;

import org.json.simple.JSONObject;

import uk.ac.ox.cs.chaste.fc.beans.ChasteFile;
import uk.ac.ox.cs.chaste.fc.beans.ChasteProtocol;
import uk.ac.ox.cs.chaste.fc.beans.ChasteProtocolVersion;
import uk.ac.ox.cs.chaste.fc.beans.ComputationalModelVersion;
import uk.ac.ox.cs.chaste.fc.beans.Notifications;
import uk.ac.ox.cs.chaste.fc.beans.PageHeader;
import uk.ac.ox.cs.chaste.fc.beans.User;
import uk.ac.ox.cs.chaste.fc.mgmt.ChasteFileManager;
import uk.ac.ox.cs.chaste.fc.mgmt.DatabaseConnector;
import uk.ac.ox.cs.chaste.fc.mgmt.ModelManager;
import uk.ac.ox.cs.chaste.fc.mgmt.ProtocolManager;
import uk.ac.ox.cs.chaste.fc.mgmt.Tools;
import de.binfalse.bflog.LOGGER;

@MultipartConfig
public class FileTransfer extends WebModule
{
  //private static final int BUFSIZE = 4096;
  private static final int DEFAULT_BUFFER_SIZE = 10240; // 10KB.
	

	public FileTransfer () throws NamingException, SQLException
	{
		super ();
		fileTransfer = true;
	}

	@Override
	protected String answerWebRequest (HttpServletRequest request, HttpServletResponse response, PageHeader header, DatabaseConnector db,
		Notifications notifications, User user, HttpSession session)
	{
		// TODO unauthed users can see some file. and authed users cannot see all files -> better rights management
		//if (!user.isAuthorized ())
			//return errorPage (request, response, null);
		
			// req[2] = m=model p=protocol e=experiment
			// req[3] = entity name
			// req[4] = entity id
			// req[5] = file id
			// req[6] = file name
		String[] req =  request.getRequestURI().substring(request.getContextPath().length()).split ("/");
		
		if (req.length != 7)
			return errorPage (request, response, null);
		
		int entityId = -1;
		int fileId = -1;
		try
		{
			entityId = Integer.parseInt (req[4]);
			fileId = Integer.parseInt (req[5]);
		}
		catch (NullPointerException | NumberFormatException e)
		{
			LOGGER.warn ("user provided unparsebale ids. download impossible: " + req[4] + " / " + req[5]);
			return errorPage (request, response, null);
		}
		if (entityId < 0 || fileId < 0)
			return errorPage (request, response, null);
		
		
		File file = null;
		String fileName = null;
		
		if (req[2].equals ("m"))
		{
			// get file from model
			ModelManager modelMgmt = new ModelManager (db, notifications);
			ComputationalModelVersion version = modelMgmt.getVersionById (entityId);
			if (version == null)
				return errorPage (request, response, null);
			ChasteFileManager fileMgmt = new ChasteFileManager (db, notifications);
			fileMgmt.getFiles (version);
			ChasteFile f = version.getFileById (fileId);
			if (f == null)
				return errorPage (request, response, null);
			
			fileName = f.getName ();
			file = new File (Tools.getModelStorageDir () + Tools.FILESEP + version.getFilePath () + Tools.FILESEP + f.getName ());
		}
		else if (req[2].equals ("p"))
		{
			// get file from protocol
			ProtocolManager protocolMgmt = new ProtocolManager (db, notifications);
			ChasteProtocolVersion version = protocolMgmt.getVersionById (entityId);
			if (version == null)
				return errorPage (request, response, null);
			ChasteFileManager fileMgmt = new ChasteFileManager (db, notifications);
			fileMgmt.getFiles (version);
			ChasteFile f = version.getFileById (fileId);
			if (f == null)
				return errorPage (request, response, null);
			
			fileName = f.getName ();
			file = new File (Tools.getProtocolStorageDir () + Tools.FILESEP + version.getFilePath () + Tools.FILESEP + f.getName ());
		}
		else if (req[2].equals ("e"))
		{
			// get file from experiment
		}
		else
			return errorPage (request, response, null);
		
		if (file == null || !file.exists () || !file.canRead () || !file.isFile ())
		{
			// whoops, that's our fault. shouldn't happen. hopefully.
			LOGGER.error ("unable to find file " + fileId + " in " + file + " (at least not in an expected form)");
			return errorPage (request, response, null);
		}
		
		
		try
		{
	    //int length   = 0;
	    //ServletOutputStream outStream = response.getOutputStream();
	    ServletContext context  = getServletConfig().getServletContext();
	    String mimetype = context.getMimeType(file.getName ());
	    
	    System.out.println ("mime: " + mimetype);
	    
	    if (mimetype == null)
	        mimetype = "application/octet-stream";
	    
	    String contentType = getServletContext().getMimeType(file.getName());

      // If content type is unknown, then set the default value.
      // For all content types, see: http://www.w3schools.com/media/media_mimeref.asp
      // To add new content types, add new mime-mapping entry in web.xml.
      if (contentType == null) {
          contentType = "application/octet-stream";
      }
	    
	    
      response.reset();
      response.setBufferSize(DEFAULT_BUFFER_SIZE);
      response.setContentType(contentType);
      response.setHeader("Content-Length", String.valueOf(file.length()));
      response.setHeader("Content-Disposition", "attachment; filename=\"" + file.getName() + "\"");
      /*response.setBufferSize(DEFAULT_BUFFER_SIZE);
	    response.setContentType(mimetype);
	    response.setContentLength((int)file.length());
	    response.setHeader("Content-Disposition", "attachment; filename=\"" + fileName + "\"");*/
	    
	    BufferedInputStream input = new BufferedInputStream(new FileInputStream(file), DEFAULT_BUFFER_SIZE);
	    BufferedOutputStream output = new BufferedOutputStream(response.getOutputStream(), DEFAULT_BUFFER_SIZE);

      // Write file contents to response.
      byte[] buffer = new byte[DEFAULT_BUFFER_SIZE];
      int length;
      while ((length = input.read(buffer)) > 0)
      {
          output.write(buffer, 0, length);
      }
	    
      
	    
	    /*byte[] byteBuffer = new byte[BUFSIZE];
	    DataInputStream in = new DataInputStream(new FileInputStream(file));
	    
			//response.setStatus (HttpServletResponse.SC_OK);
	    while ((in != null) && ((length = in.read(byteBuffer)) != -1))
	        outStream.write(byteBuffer,0,length);
	    */
	    input.close();
	    output.close();
	    
			return null;
		}
		catch (IOException e)
		{
			// whoops, that's our fault. shouldn't happen. hopefully.
			e.printStackTrace ();
			LOGGER.error ("unable to dump file " + fileId + " in " + file + " (at least not in an expected form)");
		}
		return errorPage (request, response, null);
	}

	@SuppressWarnings("unchecked")
	@Override
	protected JSONObject answerApiRequest (HttpServletRequest request, 
		HttpServletResponse response, DatabaseConnector db,
		Notifications notifications, JSONObject querry, User user, HttpSession session) throws IOException
	{
		if (!user.isAuthorized ())
			throw new IOException ("not allowed.");
		
		// TODO: delete
		if (Math.random () > 0.5)
			throw new IOException ("you just ran into a random fail");
		
		JSONObject answer = new JSONObject();
		try
		{
			Part filePart = request.getPart("file");
			// String filename = extractFileName (filePart);
			File tmpDir = new File (Tools.getTempDir ());
			if (!tmpDir.exists ())
				if (!tmpDir.mkdirs ())
					throw new IOException ("cannot create temp dir for file upload");
			
			File tmpFile = null;
			String tmpName = null;
			while (true)
			{
				tmpName = UUID.randomUUID().toString();
				tmpFile = new File (tmpDir.getAbsolutePath () + Tools.FILESEP + tmpName);
				if (!tmpFile.exists ())
					break;
			}
			
	    filePart.write (tmpFile.getAbsolutePath ());
	    
	    if (tmpFile.exists ())
	    {
				JSONObject res = new JSONObject ();
				res.put ("response", true);
				res.put ("tmpName", tmpName);
				answer.put ("upload", res);
	    }
	    	
		}
		catch (ServletException e)
		{
			e.printStackTrace();
			LOGGER.error ("Error storing uploaded file", e);
			throw new IOException ("file cannot be uploaded.");
		}
		
		return answer;
	}
	
	
	protected static final String extractFileName (Part part)
	{
		String[] items = part.getHeader ("content-disposition").split (";");
		for (String s : items)
			if (s.trim ().startsWith ("filename"))
				return s.substring (s.indexOf ("=") + 2, s.length () - 1);
		return "";
	}

	public static File getTempFile (String tmpName)
	{
		File tmpDir = new File (Tools.getTempDir () + Tools.FILESEP + tmpName);
		if (tmpDir.exists () && tmpDir.isFile () && tmpDir.canRead ())
			return tmpDir;
		return null;
	}
	
	public static void copyFile (NewFile sourceFile, File targetDirectory) throws IOException
	{
		if (sourceFile.name.contains ("/") || sourceFile.name.contains ("\\"))
			throw new IOException ("'/' and '\\' are not allowed in file names.");
		
		if (!targetDirectory.exists ())
			targetDirectory.mkdirs ();
		
		InputStream in = new FileInputStream (sourceFile.tmpFile);
    OutputStream out = new FileOutputStream (targetDirectory + Tools.FILESEP + sourceFile.name);
    
    byte[] buf = new byte[1024];
    int len;
    while ((len = in.read(buf)) > 0)
    {
        out.write(buf, 0, len);
    }
    in.close();
    out.close();
	}

	
	static public class NewFile
	{
		File tmpFile;
		String name;
		String type;
		int dbId;
		public NewFile (File tmpFile, String name, String type)
		{
			this.tmpFile = tmpFile;
			this.name = name;
			this.type = type;
			dbId = -1;
		}
	}
	
	public static boolean ambiguous (HashMap<String, NewFile> files)
	{
		// the number of files should always be quite small, otherwise we need some other technique to find duplicates..
		List<String> f = new ArrayList<String> (files.keySet ());
		for (int i = 0; i < f.size (); i++)
			for (int j = i + 1; j < f.size (); j++)
			{
				NewFile a = files.get (f.get (i));
				NewFile b = files.get (f.get (j));
				if (a.name.equals (b.name))
					return true;
				if (a.tmpFile.equals (b.tmpFile))
					return true;
			}
		return false;
	}
}
