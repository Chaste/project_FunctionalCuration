#!/usr/bin/env python

import Queue
import SocketServer

import os
import shutil
import subprocess
import sys
import tempfile
import threading

# Configuration
HOST, PORT = "localhost", 7777
MAX_QUEUE_SIZE = 50
EXE_PATH = '/home/tom/eclipse/workspace/Chaste/projects/CellModelTests/apps/src/FunctionalCuration'
OUTPUT_ROOT = '/var/lib/tomcat6/CellModelTestOutput'
INPUT_ROOT = '/var/lib/tomcat6/CellModelTestInput'
PID_FILE = os.path.join(os.getcwd(), 'functional_curation_queue.pid')

# Daemonize
import daemon
daemon.UMASK = 0002
daemon.createDaemon()
try:
    fp = open(PID_FILE, 'w')
    fp.write(str(os.getpid()) + '\n')
    fp.close()
    os.chmod(PID_FILE, 0600)
except IOError:
    pass


class FunctionalCurationServer(SocketServer.TCPServer):
    """
    A simple server for running the functional curation system.
    
    TCP connections should be made to the configured host and port, and 3 lines of text sent.
    These specify the relative paths to the model, protocol, and the output folder, respectively.
    A short text response will be sent indicating if the job has been submitted successfully.
    Jobs are executed asynchronously; on completion (whether successful or otherwise), provided
    the output folder can be created writeable, an empty file named 'done' will be created there.
    """
    def __init__(self, *args, **kwargs):
        """
        Initialise the listening server object, our job queue, and worker thread.
        Note that we don't actually start listening for connections until
        self.serve_forever is called.
        """
        SocketServer.TCPServer.__init__(self, *args, **kwargs)
        self.queue = Queue.Queue(MAX_QUEUE_SIZE)
        self.worker = FunctionalCurationWorker(self.queue)
        self.worker.start()

class FunctionalCurationWorker(threading.Thread):
    """
    A simple worker class that launches the functional curation executable
    with the first job in the queue, and waits for it to complete, then repeats.
    """
    def __init__(self, queue):
        self.queue = queue
        threading.Thread.__init__(self)
        
    def _run_job(self):
        job = self.queue.get()
        model, proto, output_dir = job
        args = [EXE_PATH, model, proto, output_dir]
        stdout_fd, stdout_path = tempfile.mkstemp(text=True)
        try:
            run = subprocess.Popen(args, stderr=subprocess.STDOUT, stdout=stdout_fd)
            rc = run.wait()
        finally:
            try:
                os.fdopen(stdout_fd).close()
            except:
                pass
            try:
                os.makedirs(output_dir)
            except:
                pass
            stdout_dest = os.path.join(output_dir, 'stdout.txt')
            os.rename(stdout_path, stdout_dest)
            if os.path.exists(stdout_path):
                os.remove(stdout_path)
            try:
                os.chmod(stdout_dest, 0664)
            except:
                pass
            open(os.path.join(output_dir, 'done'), 'w').close()
    
    def run(self):
        while 1:
            try:
                self._run_job()
            except:
                pass

class FunctionalCurationRequestHandler(SocketServer.StreamRequestHandler):
    """
    Handle requests to run Functional Curation protocols.
    """

    def handle(self):
        """Handle a job submission."""
        model = self._sanitise(INPUT_ROOT, self.rfile.readline().strip())
        proto = self._sanitise(INPUT_ROOT, self.rfile.readline().strip())
        output_dir = self._sanitise(OUTPUT_ROOT, self.rfile.readline().strip())
        #self.wfile.write('Received arguments: model=%s, protocol=%s, output folder=%s\n'
        #                 % (model, proto, output_dir))
        if not os.path.isfile(model):
            self.wfile.write('Model file "%s" does not exist.\n' % model)
            return
        if not os.path.isfile(proto):
            self.wfile.write('Protocol file "%s" does not exist.\n' % proto)
            return
        try:
            self.server.queue.put((model, proto, output_dir), False)
            try:
                shutil.rmtree(output_dir)
            except:
                pass
            self.wfile.write('Job submitted successfully.\nApproximate queue length is now %d.\n'
                             % self.server.queue.qsize())
        except Queue.Full:
            self.wfile.write('Job queue is full; try again later.\n')
    
    def _sanitise(self, rootDir, relativePath):
        """
        Make sure relativePath doesn't contain .. etc,
        and return an absolute path interpreting it relative to rootDir.
        """
        bad = os.pardir + os.sep
        return os.path.join(rootDir, relativePath.replace(bad, ''))

if __name__ == "__main__":
    # Create the server
    server = FunctionalCurationServer((HOST, PORT), FunctionalCurationRequestHandler)

    # Activate the server; this will keep running until interrupted/killed
    server.serve_forever()
