#!/usr/bin/env python
import cgi
import cgitb
import os
import tempfile
import time

password="Ohchej7mo_Fohh:u1ohw"

temporaryDir="/tmp/"
temporaryFilePrefix="chasteFile"

cgitb.enable()


# function to copy sent files to FS
def writeFile (source, destination):
	fout = open (destination, 'wb')
	while 1:
		chunk = source.read(10240)
		if not chunk: break
		fout.write (chunk)
	fout.flush()
	os.fsync(fout)
        fout.close()


# parse sent objects
form = cgi.FieldStorage()
if (not form.has_key("password")) or (form["password"].value != password) or (not form.has_key("callBack")) or (not form.has_key("signature")) or (not form.has_key("model")) or (not form.has_key("protocol")):
	print "Content-Type: text/html\n\n";
	print '''
        <html><head><title>ChastePermissionError</title></head><body>
		looks like you're not allowed to do that
        </body></html>
        '''
else:
	print "Content-Type: text/plain\n\n"
	callBack = form["callBack"]
	signature = form["signature"]
	model = form["model"]
	protocol = form["protocol"]

	temp_dir = tempfile.mkdtemp()
	modelFile = os.path.join(temp_dir, 'model.zip')
	protocolFile = os.path.join(temp_dir, 'protocol.zip')

	writeFile (model.file, modelFile)
	writeFile (protocol.file, protocolFile)
	
	# call the chaste handler via batch -> it will be executed if load average drops below 1.5
	# seems to be the most convinient mech, to not blow the machine...
	# but may be replaced by a submit to sge or other scheduling workarounds
	os.system("batch <<< '/var/www/cgi-bin/processthefiles.py "+callBack.value+" "+signature.value+" "+modelFile+" "+protocolFile+"'")
	
	# print success to calling script -> tell web interface that the call was successful
	print signature.value, "succ"


