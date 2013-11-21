#!/usr/bin/env python
import cgi
import cgitb
import os
import tempfile
import time
import random

import fcws_utils

password="Ohchej7mo_Fohh:u1ohw"

temporaryDir="/tmp/"
temporaryFilePrefix="chasteFile"

cgitb.enable(logdir=os.path.join(temporaryDir, 'python-webservice-cgitb'))


def WriteFile(source, destination):
    """Save a file submitted by HTTP POST to disk at the given path."""
    fout = open(destination, 'wb')
    while 1:
        chunk = source.read(10240)
        if not chunk: break
        fout.write(chunk)
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
    
    # Save the submitted COMBINE archives to disk in a temporary folder
    temp_dir = tempfile.mkdtemp()
    model_path = os.path.join(temp_dir, 'model.zip')
    proto_path = os.path.join(temp_dir, 'protocol.zip')
    WriteFile(model.file, model_path)
    WriteFile(protocol.file, proto_path)
    
    # Unpack the model & protocol
    main_model_path = fcws_utils.UnpackArchive(model_path, temp_dir, 'model')
    main_proto_path = fcws_utils.UnpackArchive(proto_path, temp_dir, 'proto')
    
    # Check whether their interfaces are compatible
    missing_terms = fcws_utils.DetermineCompatibility(main_proto_path, main_model_path)
    if missing_terms:
        print signature.value, "failed to run protocol - required ontology terms are not present in the model."
        print "Missing terms are:"
        for term in missing_terms:
            print "   ", term
    else:
        # call the chaste handler via batch -> it will be executed if load average drops below 1.5
        # seems to be the most convinient mech, to not blow the machine...
        # but may be replaced by a submit to sge or other scheduling workarounds
        os.system("batch <<< '/var/www/cgi-bin/processthefiles.py "+callBack.value+" "+signature.value
                  +" "+main_model_path+" "+main_proto_path+" "+temp_dir+"'")

        # print success to calling script -> tell web interface that the call was successful
        print signature.value, "succ"


