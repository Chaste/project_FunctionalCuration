function addNotification (err, list)
{
	console.log ("adding" + list);
	if (!list)
		list = "error";
	var errsList = document.getElementById(list + "list");
	var errors = document.getElementById(list);
	var item = document.createElement("li");
	item.appendChild(document.createTextNode (err));
	errsList.appendChild(item);

	if (errsList.firstChild)
		errors.setAttribute("class", "");
}

function displayNotifications (json)
{
	if (json && json.notifications)
	{
		if (json.notifications.errors)
		{
			var errs = json.notifications.errors;
			for(var i = 0; i < errs.length; i++)
				addNotification (errs[i], "error");
		}
		if (json.notifications.notes)
		{
			var errs = json.notifications.notes;
			for(var i = 0; i < errs.length; i++)
				addNotification (errs[i], "info");
		}
	}
}

function removeChildren (elem)
{
	while (elem.firstChild)
		elem.removeChild(elem.firstChild);
}

function clearNotifications (type)
{
	var list = document.getElementById(type+"list");
	removeChildren (list);
	list = document.getElementById(type);
	list.setAttribute("class", "invisible");
}

function convertForURL (str)
{
	var url = str.replace(/\W/g, '');
	if (url.length >= 5)
		return url;
	while (url.length < 7)
		url += Math.random().toString(36).substring(7);
	return url.substring (0, 5);
}

function initPage ()
{
	var dismissErrs = document.getElementById("dismisserrors");
	dismissErrs.addEventListener("click", 
	        function (event)
	        {
				clearNotifications ("error");
	        }, 
	        false);
	var dismissNotes = document.getElementById("dismissnotes");
	dismissNotes.addEventListener("click", 
	        function (event)
	        {
				clearNotifications ("info");
	        }, 
	        false);
	
	var times = document.getElementsByTagName("time");
	for (var i = 0; i < times.length; i++)
	{
		//console.log (times[i].innerHTML);
		/*var date = new XDate(times[i].innerHTML, true);
		if (date && date.valid ())
		{
			times[i].setAttribute ("datetime", times[i].innerHTML);
			times[i].innerHTML = date.toString ("MMM dS, yyyy 'at' h:mm tt");
		}*/
		var tm = times[i].innerHTML;
		if (tm)
		{
			times[i].setAttribute ("datetime", tm);
			times[i].innerHTML = beautifyTimeStamp (tm);
		}
	}
}

function beautifyTimeStamp (datestring)
{
	var date = new XDate(datestring, true);
	if (date && date.valid ())
	{
		return date.toString ("MMM d'<sup>'S'</sup>', yyyy 'at' h:mm tt");
	}
	return datestring;
}

function getYMDHMS (datestring)
{
	var date = new XDate(datestring, true);
	if (date && date.valid ())
	{
		return date.toString ("yyyy-MM-dd_HH-mm-ss");
	}
	return datestring;
}
function humanReadableBytes (bytes)
{
    var sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB'];
    if (bytes == 0)
    	return '0 Bytes';
    var i = parseInt (Math.floor(Math.log(bytes) / Math.log(1024)));
    return Math.round (bytes / Math.pow(1024, i), 2) + ' ' + sizes[i];
};

document.addEventListener("DOMContentLoaded", initPage, false);