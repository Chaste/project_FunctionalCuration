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
import uk.ac.ox.cs.chaste.fc.beans.ComputationalModel;
import uk.ac.ox.cs.chaste.fc.beans.ComputationalModelVersion;
import uk.ac.ox.cs.chaste.fc.beans.Notifications;
import uk.ac.ox.cs.chaste.fc.beans.PageHeader;
import uk.ac.ox.cs.chaste.fc.beans.PageHeaderScript;
import uk.ac.ox.cs.chaste.fc.beans.User;
import uk.ac.ox.cs.chaste.fc.mgmt.ChasteFileManager;
import uk.ac.ox.cs.chaste.fc.mgmt.DatabaseConnector;
import uk.ac.ox.cs.chaste.fc.mgmt.ModelManager;
import uk.ac.ox.cs.chaste.fc.mgmt.Tools;
import uk.ac.ox.cs.chaste.fc.web.FileTransfer.NewFile;
import de.binfalse.bflog.LOGGER;

public class ModelView extends WebModule
{

	public ModelView () throws NamingException, SQLException
	{
		super ();
	}

	@Override
	protected String answerWebRequest (HttpServletRequest request, HttpServletResponse response, PageHeader header, DatabaseConnector db,
		Notifications notifications, User user, HttpSession session)
	{
		// req[2] = model name
		// req[3] = model id
		// req[4] = version
		// req[5] = version id
		// req[6] = file
		// req[7] = action
		String[] req =  request.getRequestURI().substring(request.getContextPath().length()).split ("/");
		
		if (req == null || req.length < 3)
			return errorPage (request, response, null);
		
		if (req[2].equals ("createnew"))
		{
			if (user.isAuthorized ())
			{
				header.addScripts (new PageHeaderScript ("res/js/upload.js", "text/javascript", "UTF-8", null));
				header.addScripts (new PageHeaderScript ("res/js/newmodel.js", "text/javascript", "UTF-8", null));
				//System.out.println (request.getParameter("newmodelname"));
				if (request.getParameter("newmodelname") != null)
				{
					ModelManager modelMgmt = new ModelManager (db, notifications);
					int modelId = -1;
					try
					{
						modelId = Integer.parseInt (request.getParameter("newmodelname"));
						ComputationalModel m = modelMgmt.getModelById (modelId);
						request.setAttribute ("newmodelname", m.getName ());
					}
					catch (NumberFormatException | NullPointerException e)
					{
						e.printStackTrace ();
						notifications.addError ("couldn't find desired model");
						LOGGER.warn ("user requested model id " + request.getParameter("newmodelname") + " is unparseable.");
					}
				}
				return "ModelNew.jsp";
			}
			else
				return errorPage (request, response, null);
		}
		
		if (req.length < 4)
			return errorPage (request, response, null);
		
		
		
		try
		{
			int modelID = Integer.parseInt (req[3]);
			ModelManager modelMgmt = new ModelManager (db, notifications);
			ComputationalModel model = modelMgmt.getModelById (modelID);
			if (model == null)
				return errorPage (request, response, null);
			request.setAttribute ("compModel", model);
			header.addScripts (new PageHeaderScript ("res/js/model.js", "text/javascript", "UTF-8", null));
		}
		catch (NumberFormatException e)
		{
			e.printStackTrace ();
			notifications.addError ("cannot find model");
			LOGGER.warn ("user requested model id " + req[3] + " is unparseable.");
			return errorPage (request, response, null);
		}
		
		// version/file view/action will be done on client side
		
		return "Model.jsp";
	}

	@Override
	protected JSONObject answerApiRequest (HttpServletRequest request, 
		HttpServletResponse response, DatabaseConnector db,
		Notifications notifications, JSONObject querry, User user, HttpSession session) throws IOException
	{
		// TODO: regularly clean up:
		// uploaded models that were not used
		// created model dirs that don't exist in models
		
		JSONObject answer = new JSONObject();
		
		ChasteFileManager fileMgmt = new ChasteFileManager (db, notifications); 
		ModelManager modelMgmt = new ModelManager (db, notifications);
		
		Object task = querry.get ("task");
		if (task == null)
		{
			response.setStatus (HttpServletResponse.SC_BAD_REQUEST);
			throw new IOException ("nothing to do.");
		}
		
		if (task.equals ("createNewModel") || task.equals ("verifyNewModel"))
			createNewModel (task, notifications, querry, user, answer, modelMgmt, fileMgmt);
		
		if (task.equals ("getModelInfo"))
		{
			if (querry.get ("modelversion") != null)
				getVersion (querry.get ("modelversion"), task, notifications, querry, user, answer, modelMgmt, fileMgmt);
			if (querry.get ("modelversionfile") != null)
				getFile (querry.get ("modelversionfile"), task, notifications, querry, user, answer, modelMgmt, fileMgmt);
		}
		
		
		return answer;
	}
	
	@SuppressWarnings("unchecked")
	private void getVersion (Object version, Object task, Notifications notifications, JSONObject querry, User user, JSONObject answer, ModelManager modelMgmt, ChasteFileManager fileMgmt) throws IOException
	{
		try
		{
			int versionId = Integer.parseInt (version.toString ());
			ComputationalModelVersion vers = modelMgmt.getVersionById (versionId);
			if (vers == null)
				notifications.addError ("no version found");
			else
			{
				fileMgmt.getFiles (vers);
				answer.put ("modelversion", vers.toJson ());
			}
		}
		catch (NullPointerException | NumberFormatException e)
		{
			e.printStackTrace ();
			LOGGER.warn ("user provided model version id not parseable: " + version);
			throw new IOException ("version not found");
		}
	}
	
	@SuppressWarnings("unchecked")
	private void getFile (Object versionFile, Object task, Notifications notifications, JSONObject querry, User user, JSONObject answer, ModelManager modelMgmt, ChasteFileManager fileMgmt) throws IOException
	{
		try
		{
			int fileId = Integer.parseInt (versionFile.toString ());
			ChasteFile file = fileMgmt.getFileById (fileId);
			if (file == null)
				notifications.addError ("no file found");
			else
			{
				answer.put ("modelversionfile", file.toJson ());
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
	private void createNewModel (Object task, Notifications notifications, JSONObject querry, User user, JSONObject answer, ModelManager modelMgmt, ChasteFileManager fileMgmt) throws IOException
	{

		String modelName = null;
		String versionName = null;
		String filePath = null;
		File modelDir = null;
		HashMap<String, NewFile> files = new HashMap<String, NewFile> ();
		boolean createOk = task.equals ("createNewModel");
		ComputationalModel model = null;
		if (querry.get ("modelName") != null)
		{
			JSONObject obj = new JSONObject ();
			obj.put ("response", true);
			obj.put ("responseText", "nice name");
			answer.put ("modelName", obj);
			
			modelName = querry.get ("modelName").toString ().trim ();
			model = modelMgmt.getModelByName (modelName);
			if (model == null)
			{
				if (modelName.length () < 5)
				{
					obj.put ("response", false);
					obj.put ("responseText", "needs to be at least 5 characters in length");
					createOk = false;
				}
				// else name ok
			}
			// else model exists -> great, but warn
			else
			{
				obj.put ("response", true);
				obj.put ("responseText", "model name exists. you're going to upload a new version to an existing model.");
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
			

			if (model != null)
			{
				if (model.getVersion (versionName) != null)
				{
					obj.put ("response", false);
					obj.put ("responseText", "this model already contains a version with that name");
					createOk = false;
				}
			}
			// new model -> dont care about version names
		}
		else
			createOk = false;
		
		if (createOk)
		{
			// do we have any files?
			// creating an empty model makes no sense..
			if (querry.get ("files") != null)
			{
				filePath = UUID.randomUUID ().toString ();
				modelDir = new File (Tools.getModelStorageDir () + Tools.FILESEP + filePath);
				while (modelDir.exists ())
				{
					filePath = UUID.randomUUID ().toString ();
					modelDir = new File (Tools.getModelStorageDir () + Tools.FILESEP + filePath);
				}
				
				if (!modelDir.mkdirs ())
				{
					LOGGER.error ("cannot create model dir: " + modelDir);
					throw new IOException ("cannot create model dir");
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
						LOGGER.warn ("user provided model file name is empty or null.");
						throw new IOException ("detected empty file name. thats not allowed.");
					}
					if (name.contains ("/") || name.contains ("\\"))
					{
						LOGGER.warn ("user provided model file name contains / or \\.");
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
				notifications.addError ("no files chosen. empty models don't make much sense");
			}
			
			if (FileTransfer.ambiguous (files))
			{
				createOk = false;
				notifications.addError ("there was an error with the files. the provided information is ambiguous");
			}
		}
		

		if (task.equals ("createNewModel") && !createOk)
		{
			JSONObject res = new JSONObject ();
			res.put ("response", false);
			res.put ("responseText", "failed due to previous errors");
			answer.put ("createNewModel", res);
		}
		
		if (createOk)
		{
			// create a model if it not yet exists
			int modelId = -1;
			if (model == null)
				modelId = modelMgmt.createModel (modelName, user);
			else
				modelId = model.getId ();
			
			if (modelId < 0)
			{
				cleanUp (modelDir, -1, files, fileMgmt, modelMgmt);
				LOGGER.error ("error inserting/creating model to db");
				throw new IOException ("wasn't able to create/insert model to db.");
			}
			
			// create version
			int versionId = modelMgmt.createVersion (modelId, versionName, filePath, user);
			if (versionId < 0)
			{
				cleanUp (modelDir, versionId, files, fileMgmt, modelMgmt);
				LOGGER.error ("error inserting/creating model version to db");
				throw new IOException ("wasn't able to create/insert model version to db.");
			}
			
			for (NewFile f : files.values ())
			{
				//copy file
				try
				{
					FileTransfer.copyFile (f, modelDir);
				}
				catch (IOException e)
				{
					e.printStackTrace ();
					cleanUp (modelDir, versionId, files, fileMgmt, modelMgmt);
					LOGGER.error ("error copying file from tmp to model dir", e);
					throw new IOException ("wasn't able to copy a file. sry, our fault.");
				}
				
				// insert to db
				int fileId = fileMgmt.addFile (f.name, f.type, user, f.tmpFile.length ());
				if (fileId < 0)
				{
					cleanUp (modelDir, versionId, files, fileMgmt, modelMgmt);
					LOGGER.error ("error inserting file to db");
					throw new IOException ("wasn't able to insert file " + f.name + " to db.");
				}
				f.dbId = fileId;
				
				// associate files+version
				if (!fileMgmt.associateFileToModel (fileId, versionId))
				{
					cleanUp (modelDir, versionId, files, fileMgmt, modelMgmt);
					LOGGER.error ("error inserting file to db");
					throw new IOException ("wasn't able to insert file " + f.name + " to db.");
				}
			}
			

			JSONObject res = new JSONObject ();
			res.put ("response", true);
			res.put ("responseText", "added version successfully");
			answer.put ("createNewModel", res);
			
			// TODO: remove temp files
		}
		
		if (!createOk)
		{
			cleanUp (modelDir, -1, files, fileMgmt, modelMgmt);
		}
	}
	
	private void cleanUp (File modelDir, int versionId, HashMap<String, NewFile> files, ChasteFileManager fileMgmt, ModelManager modelMgmt)
	{
		// delete model directory recursively
		if (modelDir != null && modelDir.exists ())
			try
			{
				Tools.delete (modelDir, false);
			}
			catch (IOException e)
			{
				// in general we don't really care about that in this special case. but lets log it...
				e.printStackTrace();
				LOGGER.warn ("deleting of " + modelDir + " failed.");
			}
		
		// remove models from db
		for (NewFile f : files.values ())
			if (f.dbId >= 0)
				fileMgmt.removeFile (f.dbId);
		
		// delete version from db
		if (versionId >= 0)
			modelMgmt.removeVersion (versionId);
	}
}
