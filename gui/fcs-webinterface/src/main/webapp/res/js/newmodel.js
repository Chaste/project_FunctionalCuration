
var uploadedModels = new Array ();
//var knownTypes = ["unknown", "CellML", "CSV", "HDF5", "EPS", "PNG", "XMLPROTOCOL", "TXTPROTOCOL"];
var knownModelTypes = ["unknown", "CellML", "CSV", "HDF5", "EPS", "PNG", "XMLPROTOCOL", "TXTPROTOCOL"];

function verifyNewModel (jsonObject, elem, modelNameAction, versionNameAction, storeAction)
{
    elem.innerHTML = "<img src='"+contextPath+"/res/img/loading2-new.gif' alt='loading' />";
    
	var xmlhttp = null;
    // !IE
    if (window.XMLHttpRequest)
    {
        xmlhttp = new XMLHttpRequest();
    }
    // IE -- microsoft, we really hate you. every single day.
    else if (window.ActiveXObject)
    {
        xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
    }
    
    xmlhttp.open("POST", '', true);
    xmlhttp.setRequestHeader("Content-type", "application/json");

    xmlhttp.onreadystatechange = function()
    {
        if(xmlhttp.readyState != 4)
        	return;
        
    	var json = JSON.parse(xmlhttp.responseText);
    	console.log (json);
    	displayNotifications (json);
    	
        if(xmlhttp.status == 200)
        {
        	
        	if (json.modelName && modelNameAction)
        	{
	        	var msg = json.modelName.responseText;
	        	if (json.modelName.response)
	        		modelNameAction.innerHTML = "<img src='"+contextPath+"/res/img/check.png' alt='valid' /> " + msg;
	        	else
	        		modelNameAction.innerHTML = "<img src='"+contextPath+"/res/img/failed.png' alt='invalid' /> " + msg;
        	}
        	if (json.versionName)
        	{
	        	var msg = json.versionName.responseText;
	        	if (json.versionName.response)
	        		versionNameAction.innerHTML = "<img src='"+contextPath+"/res/img/check.png' alt='valid' /> " + msg;
	        	else
	        		versionNameAction.innerHTML = "<img src='"+contextPath+"/res/img/failed.png' alt='invalid' /> " + msg;
        	}
        	if (json.createNewModel)
        	{
	        	var msg = json.createNewModel.responseText;
	        	if (json.createNewModel.response)
	        	{
	        		var form = document.getElementById ("newmodelform");
	        		removeChildren (form);
	        		var h1 = document.createElement("h1");
	        		var img = document.createElement("img");
	        		img.src = contextPath + "/res/img/check.png";
	        		img.alt = "created model successfully";
	        		h1.appendChild(img);
	        		h1.appendChild(document.createTextNode (" Congratulations"));
	        		var p = document.createElement("p");
	        		p.appendChild(document.createTextNode ("You've just created a new model! Have a look at "));
	        		var a = document.createElement("a");
	        		a.href = contextPath + "/myfiles.html";
	        		a.appendChild(document.createTextNode ("your files"));
	        		p.appendChild(a);
	        		p.appendChild(document.createTextNode ("."));

	        		form.appendChild(h1);
	        		form.appendChild(p);
	        	}
	        	else
	        		storeAction.innerHTML = "<img src='"+contextPath+"/res/img/failed.png' alt='invalid' /> " + msg;
        	}
        }
        else
        {
        	elem.innerHTML = "<img src='"+contextPath+"/res/img/failed.png' alt='error' /> sorry, serverside error occurred.";
        }
    };
    xmlhttp.send(JSON.stringify(jsonObject));
}


function initNewModel ()
{
	var modelName = document.getElementById("modelname");
	var versionName = document.getElementById("versionname");
	var modelNameAction = document.getElementById("modelaction");
	var versionNameAction = document.getElementById("versionaction");
	var storeAction = document.getElementById("saveaction");
	var svbtn = document.getElementById('savebutton');
	
	modelName.addEventListener("blur", function( event )
	{
		verifyNewModel ({
	    	task: "verifyNewModel",
	    	modelName: modelName.value
	    }, modelNameAction, modelNameAction, versionNameAction, storeAction);
	  }, true);
	
	versionName.addEventListener("blur", function( event ) {
		verifyNewModel ({
	    	task: "verifyNewModel",
	    	modelName: modelName.value,
	    	versionName: versionName.value
	    }, versionNameAction, modelNameAction, versionNameAction, storeAction);
	  }, true);
	
	
	var insertDate = document.getElementById('dateinserter');
	insertDate.addEventListener("click", function (ev) {
		if (versionName)
		{
			versionName.focus ();
			versionName.value = getYMDHMS (new Date());
			versionName.blur ();
		}
	}, true);
	

	
	
	
	initUpload (uploadedModels, knownModelTypes);
	

	svbtn.addEventListener("click", function (ev) {
		verifyNewModel(
		{
	    	task: "createNewModel",
	    	modelName: modelName.value,
	    	versionName: versionName.value,
	    	files: uploadedModels
	    }, storeAction, modelNameAction, versionNameAction, storeAction);
	}, true);
	
}


document.addEventListener("DOMContentLoaded", initNewModel, false);