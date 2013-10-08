
var uploadedProtocols = new Array ();
var knownProtocolTypes = ["unknown", "CellML", "CSV", "HDF5", "EPS", "PNG", "XMLPROTOCOL", "TXTPROTOCOL"];

function verifyNewProtocol (jsonObject, elem, protocolNameAction, versionNameAction, storeAction)
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
        	
        	if (json.protocolName && protocolNameAction)
        	{
	        	var msg = json.protocolName.responseText;
	        	if (json.protocolName.response)
	        		protocolNameAction.innerHTML = "<img src='"+contextPath+"/res/img/check.png' alt='valid' /> " + msg;
	        	else
	        		protocolNameAction.innerHTML = "<img src='"+contextPath+"/res/img/failed.png' alt='invalid' /> " + msg;
        	}
        	if (json.versionName)
        	{
	        	var msg = json.versionName.responseText;
	        	if (json.versionName.response)
	        		versionNameAction.innerHTML = "<img src='"+contextPath+"/res/img/check.png' alt='valid' /> " + msg;
	        	else
	        		versionNameAction.innerHTML = "<img src='"+contextPath+"/res/img/failed.png' alt='invalid' /> " + msg;
        	}
        	if (json.createNewProtocol)
        	{
	        	var msg = json.createNewProtocol.responseText;
	        	if (json.createNewProtocol.response)
	        	{
	        		var form = document.getElementById ("newprotocolform");
	        		removeChildren (form);
	        		var h1 = document.createElement("h1");
	        		var img = document.createElement("img");
	        		img.src = contextPath + "/res/img/check.png";
	        		img.alt = "created protocol successfully";
	        		h1.appendChild(img);
	        		h1.appendChild(document.createTextNode (" Congratulations"));
	        		var p = document.createElement("p");
	        		p.appendChild(document.createTextNode ("You've just created a new protocol! Have a look at "));
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


function initNewProtocol ()
{
	var protocolName = document.getElementById("protocolname");
	var versionName = document.getElementById("versionname");
	var protocolNameAction = document.getElementById("protocolaction");
	var versionNameAction = document.getElementById("versionaction");
	var storeAction = document.getElementById("saveaction");
	var svbtn = document.getElementById('savebutton');
	
	protocolName.addEventListener("blur", function( event )
	{
		verifyNewProtocol ({
	    	task: "verifyNewProtocol",
	    	protocolName: protocolName.value
	    }, protocolNameAction, protocolNameAction, versionNameAction, storeAction);
	  }, true);
	
	versionName.addEventListener("blur", function( event ) {
		verifyNewProtocol ({
	    	task: "verifyNewProtocol",
	    	protocolName: protocolName.value,
	    	versionName: versionName.value
	    }, versionNameAction, protocolNameAction, versionNameAction, storeAction);
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
	

	
	
	
	initUpload (uploadedProtocols, knownProtocolTypes);
	

	svbtn.addEventListener("click", function (ev) {
		verifyNewProtocol(
		{
	    	task: "createNewProtocol",
	    	protocolName: protocolName.value,
	    	versionName: versionName.value,
	    	files: uploadedProtocols
	    }, storeAction, protocolNameAction, versionNameAction, storeAction);
	}, true);
	
}


document.addEventListener("DOMContentLoaded", initNewProtocol, false);