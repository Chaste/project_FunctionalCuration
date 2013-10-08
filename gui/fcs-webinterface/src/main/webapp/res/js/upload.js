

function alreadyExists (uploaded, name)
{
	for (var i = 0; i < uploaded.length; i++)
		if (uploaded[i].fileName == name)
			return true;
	return false;
}

function fileNotifiation (good, msg)
{
	var elem = document.getElementById("");
	if (good)
		elem.innerHTML = "";
	else
		elem.innerHTML = "<img src='"+contextPath+"/res/img/failed.png' alt='error' /> " + msg;
}

function sendFile (uploaded, file, name, types)
{
	if (alreadyExists (uploaded, name))
	{
		addNotification ("there is already a file with the same name. please remove that first.", "error");
		return;
	}
	
	var list = document.getElementById("uploadedfiles");
	var neu = document.createElement("li");
	var neuName = document.createElement("code");
	var neuRm = document.createElement("a");
	var neuRmPic = document.createElement("img");
	var neuSize = document.createElement("small");
	var neuSizeCode = document.createElement("code");
	var neuAction = document.createElement("small");
	neuName.appendChild(document.createTextNode(name));
	neuRmPic.src = contextPath+"/res/img/failed.png";
	neuRmPic.alt = "remove from list";
	neuRm.appendChild(neuRmPic);
	neu.appendChild(neuName);
	neu.appendChild (neuRm);
	neuSizeCode.appendChild (document.createTextNode(" "+humanReadableBytes(file.size)+" "));
	neuSize.appendChild (neuSizeCode);
	neu.appendChild(neuSize);
	neu.appendChild(neuAction);
	list.appendChild(neu);
	
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
    xmlhttp.addEventListener('progress', function(e)
        {
            var done = e.position || e.loaded;
            var total = e.totalSize || e.total;
        	neuAction.innerHTML = (Math.floor(done/total*1000)/10) + "%";
        }, false);
    if ( xmlhttp.upload )    {
        xmlhttp.upload.onprogress = function(e)    {
            var done = e.position || e.loaded;
            var total = e.totalSize || e.total;
            //console.log('xmlhttp.upload progress: ' + done + ' / ' + total + ' = ' + (Math.floor(done/total*1000)/10) + '%');
            neuAction.innerHTML = (Math.floor(done/total*1000)/10) + '%';
        };
    }
    
    xmlhttp.onreadystatechange = function(e)    {

        if(xmlhttp.readyState != 4)
        	return;
    	console.log (xmlhttp.responseText);
    	var json = JSON.parse(xmlhttp.responseText);
    	console.log (json);
    	if (json)
    		displayNotifications (json);
        if (xmlhttp.status == 200 && json.upload && json.upload.response)
        {
        	var array = {
        		fileName: name,
        		tmpName: json.upload.tmpName,
        		fileType: "unknown"
        	};
        	var type = document.createElement("select");
        	for (var i = 0; i < types.length; i++)
        	{
        		var opt = document.createElement("option");
        		opt.value = types[i];
        		opt.appendChild (document.createTextNode(types[i]));
        		type.appendChild(opt);
        	}
        	type.addEventListener("click", function () {
        		array.fileType = type.options[type.selectedIndex].value;
        	}, true);

        	neuName.setAttribute("class", "success");
        	removeChildren (neuAction);
        	neuAction.appendChild (document.createTextNode("file type: "));
        	neuAction.appendChild (type);
        	uploaded.push (array);
        }
        else
        {
        	neuName.setAttribute("class", "failed");
        	neuAction.innerHTML = "failed, try again";
        }
    };

	var fd = new FormData;
	fd.append('file', file);
	fd.append('other_data', 'foo bar');
	
	console.log (name);
	
	neuAction.innerHTML = "uploading";
	xmlhttp.open('post', contextPath + "/upload.html", true);
	xmlhttp.send(fd);
	
	neuRm.addEventListener("click", function () {
		if (xmlhttp)
		{
			xmlhttp.onreadystatechange = function () {/* need this cause some browsers will throw a 'done' which we cannot interpret otherwise */};
			xmlhttp.abort();
		}
		list.removeChild(neu);
		for (var i = 0; i < uploaded.length; i++)
			if (uploaded[i].fileName == name)
				uploaded.splice(i, 1);
	}, true);
}

function handleFileSelect(evt) {
    evt.stopPropagation();
    evt.preventDefault();

    var files = evt.dataTransfer.files;
    for (var i = 0, f; f = files[i]; i++) {
			sendFile (f, f.name);
    }
  }

  function handleDragOver(evt) {
    evt.stopPropagation();
    evt.preventDefault();
    evt.dataTransfer.dropEffect = 'copy';
  }

function initUpload(uploaded, types)
{
	var inp = document.getElementById('fileupload');
	var dropZone = document.getElementById('dropbox');
	dropZone.addEventListener('dragover', handleDragOver, false);
	dropZone.addEventListener('drop', handleFileSelect, false);
	dropZone.addEventListener("click", 
	        function (event)
	        {
	        	inp.click ();
	        }, 
	        false);
	inp.addEventListener('change', function(e) {
	        var file = this.files[0];
			var fullPath = inp.value;
			var startIndex = (fullPath.indexOf('\\') >= 0 ? fullPath.lastIndexOf('\\') : fullPath.lastIndexOf('/'));
			var filename = fullPath.substring(startIndex+1);
			sendFile (uploaded, file, filename, types);
	    }, false);
}

