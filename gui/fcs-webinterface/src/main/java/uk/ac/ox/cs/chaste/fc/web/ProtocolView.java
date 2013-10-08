package uk.ac.ox.cs.chaste.fc.web;

import java.io.File;
import java.io.IOException;
import java.sql.SQLException;
import java.util.HashMap;
import java.util.UUID;

import javax.naming.NamingException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;

import uk.ac.ox.cs.chaste.fc.beans.ChasteFile;
import uk.ac.ox.cs.chaste.fc.beans.ChasteProtocol;
import uk.ac.ox.cs.chaste.fc.beans.ChasteProtocolVersion;
import uk.ac.ox.cs.chaste.fc.beans.Notifications;
import uk.ac.ox.cs.chaste.fc.beans.PageHeader;
import uk.ac.ox.cs.chaste.fc.beans.PageHeaderScript;
import uk.ac.ox.cs.chaste.fc.beans.User;
import uk.ac.ox.cs.chaste.fc.mgmt.ChasteFileManager;
import uk.ac.ox.cs.chaste.fc.mgmt.DatabaseConnector;
import uk.ac.ox.cs.chaste.fc.mgmt.ProtocolManager;
import uk.ac.ox.cs.chaste.fc.mgmt.Tools;
import uk.ac.ox.cs.chaste.fc.web.FileTransfer.NewFile;
import de.binfalse.bflog.LOGGER;

public class ProtocolView extends WebModule
{

	public ProtocolView () throws NamingException, SQLException
	{
		super ();
	}

	@Override
	protected String answerWebRequest (HttpServletRequest request, HttpServletResponse response, PageHeader header, DatabaseConnector db,
		Notifications notifications, User user, HttpSession session)
	{
		String[] req =  request.getRequestURI().substring(request.getContextPath().length()).split ("/");
		
		if (req == null || req.length < 3)
			return errorPage (request, response, null);
		
		if (req[2].equals ("createnew"))
		{
			if (user.isAuthorized ())
			{
				header.addScripts (new PageHeaderScript ("res/js/upload.js", "text/javascript", "UTF-8", null));
				header.addScripts (new PageHeaderScript ("res/js/newprotocol.js", "text/javascript", "UTF-8", null));
				
				if (request.getParameter("newprotocolname") != null)
				{
					ProtocolManager protocolMgmt = new ProtocolManager (db, notifications);
					int protocolId = -1;
					try
					{
						protocolId = Integer.parseInt (request.getParameter("newprotocolname"));
						ChasteProtocol m = protocolMgmt.getProtocolById (protocolId);
						request.setAttribute ("newprotocolname", m.getName ());
					}
					catch (NumberFormatException | NullPointerException e)
					{
						e.printStackTrace ();
						notifications.addError ("couldn't find desired protocol");
						LOGGER.warn ("user requested protocol id " + request.getParameter("newprotocolname") + " is unparseable.");
					}
				}
				return "ProtocolNew.jsp";
			}
			else
				return errorPage (request, response, null);
		}
		
		if (req.length < 4)
			return errorPage (request, response, null);
		
		
		
		try
		{
			int protocolID = Integer.parseInt (req[3]);
			ProtocolManager protocolMgmt = new ProtocolManager (db, notifications);
			ChasteProtocol protocol = protocolMgmt.getProtocolById (protocolID);
			if (protocol == null)
				return errorPage (request, response, null);
			request.setAttribute ("chasteProtocol", protocol);
			header.addScripts (new PageHeaderScript ("res/js/protocol.js", "text/javascript", "UTF-8", null));
		}
		catch (NumberFormatException e)
		{
			e.printStackTrace ();
			notifications.addError ("cannot find protocol");
			LOGGER.warn ("user requested protocol id " + req[3] + " is unparseable.");
			return errorPage (request, response, null);
		}
		
		// version/file view/action will be done on client side
		
		return "Protocol.jsp";
	}

	@Override
	protected JSONObject answerApiRequest (HttpServletRequest request, 
		HttpServletResponse response, DatabaseConnector db,
		Notifications notifications, JSONObject querry, User user, HttpSession session) throws IOException
	{
		// TODO: regularly clean up:
		// uploaded protocols that were not used
		// created protocol dirs that don't exist in protocols
		
		JSONObject answer = new JSONObject();
		
		ChasteFileManager fileMgmt = new ChasteFileManager (db, notifications); 
		ProtocolManager protocolMgmt = new ProtocolManager (db, notifications);
		
		Object task = querry.get ("task");
		if (task == null)
		{
			response.setStatus (HttpServletResponse.SC_BAD_REQUEST);
			throw new IOException ("nothing to do.");
		}
		
		if (task.equals ("createNewProtocol") || task.equals ("verifyNewProtocol"))
			createNewProtocol (task, notifications, querry, user, answer, protocolMgmt, fileMgmt);
		
		if (task.equals ("getProtocolInfo"))
		{
			if (querry.get ("protocolversion") != null)
				getVersion (querry.get ("protocolversion"), task, notifications, querry, user, answer, protocolMgmt, fileMgmt);
			if (querry.get ("protocolversionfile") != null)
				getFile (querry.get ("protocolversionfile"), task, notifications, querry, user, answer, protocolMgmt, fileMgmt);
		}
		
		
		return answer;
	}
	
	@SuppressWarnings("unchecked")
	private void getVersion (Object version, Object task, Notifications notifications, JSONObject querry, User user, JSONObject answer, ProtocolManager protocolMgmt, ChasteFileManager fileMgmt) throws IOException
	{
		try
		{
			int versionId = Integer.parseInt (version.toString ());
			ChasteProtocolVersion vers = protocolMgmt.getVersionById (versionId);
			if (vers == null)
				notifications.addError ("no version found");
			else
			{
				fileMgmt.getFiles (vers);
				answer.put ("protocolversion", vers.toJson ());
			}
		}
		catch (NullPointerException | NumberFormatException e)
		{
			e.printStackTrace ();
			LOGGER.warn ("user provided protocol version id not parseable: " + version);
			throw new IOException ("version not found");
		}
	}
	
	@SuppressWarnings("unchecked")
	private void getFile (Object versionFile, Object task, Notifications notifications, JSONObject querry, User user, JSONObject answer, ProtocolManager protocolMgmt, ChasteFileManager fileMgmt) throws IOException
	{
		try
		{
			int fileId = Integer.parseInt (versionFile.toString ());
			ChasteFile file = fileMgmt.getFileById (fileId);
			if (file == null)
				notifications.addError ("no file found");
			else
			{
				answer.put ("protocolversionfile", file.toJson ());
			}
		}
		catch (NullPointerException | NumberFormatException e)
		{
			e.printStackTrace ();
			LOGGER.warn ("user provided file id not parseable: " + versionFile);
			throw new IOException ("file not found");
		}
	}
	
	
	@SuppressWarnings("unchecked")
	private void createNewProtocol (Object task, Notifications notifications, JSONObject querry, User user, JSONObject answer, ProtocolManager protocolMgmt, ChasteFileManager fileMgmt) throws IOException
	{

		String protocolName = null;
		String versionName = null;
		String filePath = null;
		File protocolDir = null;
		HashMap<String, NewFile> files = new HashMap<String, NewFile> ();
		boolean createOk = task.equals ("createNewProtocol");
		ChasteProtocol protocol = null;
		if (querry.get ("protocolName") != null)
		{
			JSONObject obj = new JSONObject ();
			obj.put ("response", true);
			obj.put ("responseText", "nice name");
			answer.put ("protocolName", obj);
			
			protocolName = querry.get ("protocolName").toString ().trim ();
			protocol = protocolMgmt.getProtocolByName (protocolName);
			if (protocol == null)
			{
				if (protocolName.length () < 5)
				{
					obj.put ("response", false);
					obj.put ("responseText", "needs to be at least 5 characters in length");
					createOk = false;
				}
				// else name ok
			}
			// else protocol exists -> great, but warn
			else
			{
				obj.put ("response", true);
				obj.put ("responseText", "protocol name exists. you're going to upload a new version to an existing protocol.");
			}
		}
		else
			createOk = false;
		
		if (querry.get ("versionName") != null)
		{
			JSONObject obj = new JSONObject ();
			obj.put ("response", true);
			obj.put ("responseText", "nice version identifier");
			answer.put ("versionName", obj);
			
			versionName = querry.get ("versionName").toString ().trim ();
			
			if (versionName.length () < 5)
			{
				obj.put ("response", false);
				obj.put ("responseText", "needs to be at least 5 characters in length");
				createOk = false;
			}
			// else ok
			

			if (protocol != null)
			{
				if (protocol.getVersion (versionName) != null)
				{
					obj.put ("response", false);
					obj.put ("responseText", "this protocol already contains a version with that name");
					createOk = false;
				}
			}
			// new protocol -> dont care about version names
		}
		else
			createOk = false;
		
		if (createOk)
		{
			// do we have any files?
			// creating an empty protocol makes no sense..
			if (querry.get ("files") != null)
			{
				filePath = UUID.randomUUID ().toString ();
				protocolDir = new File (Tools.getProtocolStorageDir () + Tools.FILESEP + filePath);
				while (protocolDir.exists ())
				{
					filePath = UUID.randomUUID ().toString ();
					protocolDir = new File (Tools.getProtocolStorageDir () + Tools.FILESEP + filePath);
				}
				
				if (!protocolDir.mkdirs ())
				{
					LOGGER.error ("cannot create protocol dir: " + protocolDir);
					throw new IOException ("cannot create protocol dir");
				}
				
				JSONArray array= (JSONArray) querry.get ("files");
				for (int i = 0; i < array.size (); i++)
				{
					JSONObject file=(JSONObject) array.get (i);

					String tmpName = null;
					String name = null;
					String type = null;
					try
					{
						tmpName = file.get ("tmpName").toString ();
						name = file.get ("fileName").toString ();
						type = file.get ("fileType").toString ();
					}
					catch (NullPointerException e)
					{
						throw new IOException ("incomplete file information");
					}
					if (name == null || name.length () < 1)
					{
						LOGGER.warn ("user provided protocol file name is empty or null.");
						throw new IOException ("detected empty file name. thats not allowed.");
					}
					if (name.contains ("/") || name.contains ("\\"))
					{
						LOGGER.warn ("user provided protocol file name contains / or \\.");
						throw new IOException ("'/' and '\\' are not allowed in file names.");
					}
					if (type == null || type.length () < 1)
						type = "unknown";
					
					File tmp = FileTransfer.getTempFile (tmpName);
					if (tmp == null)
					{
						notifications.addError ("cannot find file " + name + ". please upload again.");
						createOk = false;
					}
					else
						files.put (tmpName, new NewFile (tmp, name, type));
					
					// System.out.println (file.get ("fileName") + " - " + file.get ("fileType") + " - " + file.get ("tmpName"));
				}
			}
			
			if (files.size () < 1)
			{
				createOk = false;
				notifications.addError ("no files chosen. empty protocols don't make much sense");
			}
			
			if (FileTransfer.ambiguous (files))
			{
				createOk = false;
				notifications.addError ("there was an error with the files. the provided information is ambiguous");
			}
		}
		

		if (task.equals ("createNewProtocol") && !createOk)
		{
			JSONObject res = new JSONObject ();
			res.put ("response", false);
			res.put ("responseText", "failed due to previous errors");
			answer.put ("createNewProtocol", res);
		}
		
		if (createOk)
		{
			// create a protocol if it not yet exists
			int protocolId = -1;
			if (protocol == null)
				protocolId = protocolMgmt.createProtocol (protocolName, user);
			else
				protocolId = protocol.getId ();
			
			if (protocolId < 0)
			{
				cleanUp (protocolDir, -1, files, fileMgmt, protocolMgmt);
				LOGGER.error ("error inserting/creating protocol to db");
				throw new IOException ("wasn't able to create/insert protocol to db.");
			}
			
			// create version
			int versionId = protocolMgmt.createVersion (protocolId, versionName, filePath, user);
			if (versionId < 0)
			{
				cleanUp (protocolDir, versionId, files, fileMgmt, protocolMgmt);
				LOGGER.error ("error inserting/creating protocol version to db");
				throw new IOException ("wasn't able to create/insert protocol version to db.");
			}
			
			for (NewFile f : files.values ())
			{
				//copy file
				try
				{
					FileTransfer.copyFile (f, protocolDir);
				}
				catch (IOException e)
				{
					e.printStackTrace ();
					cleanUp (protocolDir, versionId, files, fileMgmt, protocolMgmt);
					LOGGER.error ("error copying file from tmp to protocol dir", e);
					throw new IOException ("wasn't able to copy a file. sry, our fault.");
				}
				
				// insert to db
				int fileId = fileMgmt.addFile (f.name, f.type, user, f.tmpFile.length ());
				if (fileId < 0)
				{
					cleanUp (protocolDir, versionId, files, fileMgmt, protocolMgmt);
					LOGGER.error ("error inserting file to db");
					throw new IOException ("wasn't able to insert file " + f.name + " to db.");
				}
				f.dbId = fileId;
				
				// associate files+version
				if (!fileMgmt.associateFileToProtocol (fileId, versionId))
				{
					cleanUp (protocolDir, versionId, files, fileMgmt, protocolMgmt);
					LOGGER.error ("error inserting file to db");
					throw new IOException ("wasn't able to insert file " + f.name + " to db.");
				}
			}
			

			JSONObject res = new JSONObject ();
			res.put ("response", true);
			res.put ("responseText", "added version successfully");
			answer.put ("createNewProtocol", res);
			
			// TODO: remove temp files
		}
		
		if (!createOk)
		{
			cleanUp (protocolDir, -1, files, fileMgmt, protocolMgmt);
		}
	}
	
	private void cleanUp (File protocolDir, int versionId, HashMap<String, NewFile> files, ChasteFileManager fileMgmt, ProtocolManager protocolMgmt)
	{
		// delete protocol directory recursively
		if (protocolDir != null && protocolDir.exists ())
			try
			{
				Tools.delete (protocolDir, false);
			}
			catch (IOException e)
			{
				// in general we don't really care about that in this special case. but lets log it...
				e.printStackTrace();
				LOGGER.warn ("deleting of " + protocolDir + " failed.");
			}
		
		// remove protocols from db
		for (NewFile f : files.values ())
			if (f.dbId >= 0)
				fileMgmt.removeFile (f.dbId);
		
		// delete version from db
		if (versionId >= 0)
			protocolMgmt.removeVersion (versionId);
	}
}
