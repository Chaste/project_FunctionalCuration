
var pages = [ "model", "protocol", "experiment" ];



function updateUser (jsonObject, elem)
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
        	
        	if (json.updatePassword)
        	{
        		if (json.updatePassword.response)
        			elem.innerHTML = "<img src='"+contextPath+"/res/img/check.png' alt='valid' /> " + json.updatePassword.responseText;
        		else
        			elem.innerHTML = "<img src='"+contextPath+"/res/img/failed.png' alt='invalid' /> " + json.updatePassword.responseText;
        	}
        }
        else
        	elem.innerHTML = "<img src='"+contextPath+"/res/img/failed.png' alt='error' /> sorry, serverside error occurred.";
    };
    xmlhttp.send(JSON.stringify(jsonObject));
}

function initMe ()
{
	if (document.getElementById ("myaccounttable"))
	{
		// account page
		var btn = document.getElementById("pwchanger");
		btn.addEventListener("click", function (ev) {
			if (ev.which == 1)
			{
				var old = document.getElementById("oldpassword").value;
				var new1 = document.getElementById("newpassword1").value;
				var new2 = document.getElementById("newpassword2").value;
				
				if (!old || !new1 || !new2)
				{
					addNotification ("please fill in all fields.", "error");
					return;
				}
				
				if (new1 != new2)
				{
					addNotification ("the new passwords are different.", "error");
					return;
				}
				
				updateUser ({
					task: "updatePassword",
					prev: old,
					next: new1
				}, document.getElementById("changeaction"));
			}
	    	}, true);
	}
	else
	{
		// files page
		
		for (var i = 0; i < pages.length; i++)
		{
			var btn = document.getElementById(pages[i] + "chooser");
			registerSwitchPagesListener (btn, pages[i]);
		}
		switchPage ("model");
		
		/*
		var modellist = document.getElementById("modellist");
		var protocollist = document.getElementById("protocollist");
		var experimentlist = document.getElementById("experimentlist");
		
		
		var modelchooser = document.getElementById("modelchooser");
		var protocolchooser = document.getElementById("protocolchooser");
		var experimentchooser = document.getElementById("experimentchooser");
		
		modelchooser.addEventListener("click", function () {
			
		}, true);*/
		
	}
}
function switchPage (page)
{
	console.log ("switching to " + page);
	for (var i = 0; i < pages.length; i++)
	{
		if (pages[i] == page)
			document.getElementById(pages[i] + "list").style.display = "block";
		else
			document.getElementById(pages[i] + "list").style.display = "none";
	}
}
function registerSwitchPagesListener (btn, page)
{
	console.log ("register switch listener: " + page);
	btn.addEventListener("click", function () {
		console.log ("switch listener triggered " + page);
		switchPage (page);
	}, true);
}

document.addEventListener("DOMContentLoaded", initMe, false);