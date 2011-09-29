package fcs.server;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

import com.google.gwt.user.server.rpc.RemoteServiceServlet;

import fcs.client.SimulationService;

@SuppressWarnings("serial")
public class SimulationServiceImpl extends RemoteServiceServlet implements SimulationService{

	@Override
	public String runSimulation(String model) {
		String response = "";
        File dir = new File(GetResultsServiceImpl.protocolsDir);
    	String[] child = dir.list();
    	
      Socket sock=null;
      PrintWriter write = null;
      BufferedReader in = null;
      for(int i=0;i<child.length;i++){
    	  System.out.println(child[i]);
      }
      
     try{
      sock = new Socket("localhost", 7777);
      write = new PrintWriter(sock.getOutputStream(), true);
      
      in = new BufferedReader(new InputStreamReader(sock.getInputStream()));
      
      System.out.println("Connected to socket!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
      
      write.println("Models/"+model+"\n"+"Protocols/"+child[0]+"\n"+GetResultsServiceImpl.protocolsDir+"\n");

      String read = in.readLine();
      while(read!=null){
      System.out.println("Response: "+read);
      response+=read;
      read=in.readLine();
      }
      
      /*
      for(int i =0;i<child.length;i++){
      write.println(saveName);
      write.println(child[i]);
      response+=in.readLine();
      }
      */
      
     }catch(IOException ioe){
    	  	ioe.printStackTrace();
     }
        		return response;
	}
}

/*
*/