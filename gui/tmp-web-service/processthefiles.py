#!/usr/bin/env python
import os
import sys
import glob
import json
import datetime
import requests
import shutil
import subprocess
import tempfile
import time
import zipfile

my_output_file = tempfile.NamedTemporaryFile(prefix='python-webservice-output-', delete=False)
sys.stderr = my_output_file
sys.stdout = my_output_file

# this file is called via batch -> it is executed if there is CPU time available
# arguments:
# sys.argv[1] == callback url
# sys.argv[2] == signature
# sys.argv[3] == path to primary model file
# sys.argv[4] == path to primary protocol file
# sys.argv[5] == path to temporary folder
# so do whatever you want to create the experiment and put it in an combine archive
# send the archive to sys.argv[1], together with the signature (see below)
callback_url, signature, model_path, proto_path, temp_dir = sys.argv[1:6]

# Debug
fout = open ("/tmp/python-webservice.debug", 'a+')
fout.write ("======")
fout.write (datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
fout.write ("======\n")
fout.write (sys.argv[1] + "\n")
fout.write (sys.argv[2] + "\n")
fout.write (sys.argv[3] + "\n")
fout.write (sys.argv[4] + "\n")
fout.write (sys.argv[5] + "\n")
fout.flush()
os.fsync(fout)
fout.close()

# Call FunctionalCuration exe, writing output to the temporary folder containing inputs
# (or rather, a subfolder thereof).
# Also redirect stdout and stderr so we can debug any issues.
os.environ['LD_LIBRARY_PATH'] = '/home/bob/petsc-3.1-p8/linux-gnu-opt/lib:/home/tom/eclipse/workspace/Chaste/lib'
os.environ['CHASTE_TEST_OUTPUT'] = '/tmp/python-webservice-testoutput'
os.environ['USER'] = 'tom'
os.environ['GROUP'] = 'www-data'
os.environ['HOME'] = '/home/tom'
args = ['/home/tom/eclipse/workspace/Chaste/projects/FunctionalCuration/apps/src/FunctionalCuration',
        model_path,
        proto_path,
        os.path.join(temp_dir, 'output')
       ]
child_stdout_name = os.path.join(temp_dir, 'stdout.txt')
output_file = open(child_stdout_name, 'w')
subprocess.call(args, stdout=output_file, stderr=subprocess.STDOUT)
output_file.close()

# Zip up the outputs and post them to the callback
output_path = os.path.join(temp_dir, 'output.zip')
output_files = glob.glob(os.path.join(temp_dir, 'output', '*', '*', '*')) # Yuck!
output_zip = zipfile.ZipFile(output_path, 'w', zipfile.ZIP_DEFLATED)
output_zip.write(child_stdout_name, 'stdout.txt')
for ofile in output_files:
    if os.path.isfile(ofile):
        output_zip.write(ofile, os.path.basename(ofile))
outcome = 'success' if 'success' in output_zip.filelist else 'failed'
# Add a manifest if Chaste didn't create one
if 'manifest.xml' not in output_zip.filelist:
    manifest = open(os.path.join(temp_dir, 'manifest.xml'), 'w')
    manifest.write("""<?xml version='1.0' encoding='utf-8'?>
<omexManifest xmlns='http://identifiers.org/combine.specifications/omex-manifest'>
  <content location='%s' format='%s'/>
  <content location='%s' format='%s'/>
</omexManifest>
""" % ('manifest.xml', 'http://identifiers.org/combine.specifications/omex-manifest',
       'stdout.txt', 'text/plain'))
    manifest.close()
    output_zip.write(os.path.join(temp_dir, 'manifest.xml'), 'manifest.xml')
output_zip.close()

files = {'experiment': open(output_path, 'rb')}
payload = {'signature': sys.argv[2], 'returntype': outcome}
r = requests.post(callback_url, files=files, data=payload)

# Debug
fout = open ("/tmp/python-webservice-result.debug", 'a+')
fout.write ("======")
fout.write (datetime.datetime.now().strftime("%Y-%m-%d %H:%M"))
fout.write ("======\n")
fout.write (str(r.status_code) + "\n")
fout.write (str(r.content) + "\n")
fout.flush()
os.fsync(fout)
fout.close()

# Remove the temporary folder
shutil.rmtree(temp_dir)

