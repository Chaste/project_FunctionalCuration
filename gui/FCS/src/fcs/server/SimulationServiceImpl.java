package fcs.server;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;

import com.google.gwt.user.server.rpc.RemoteServiceServlet;

import fcs.client.SimulationService;

@SuppressWarnings("serial")
public class SimulationServiceImpl extends RemoteServiceServlet implements
		SimulationService {

	@Override
	public String runSimulation(String model) {
		String response = "";
		File dir = new File(GetResultsServiceImpl.protocolsSourceDir);
		String[] protocols = dir.list();

		// remove .cellml
		String modelName = model.substring(0, model.length() - 7);

		Socket sock = null;
		PrintWriter write = null;
		BufferedReader in = null;

		for (int i = 0; i < protocols.length; i++) {
		try {

			sock = new Socket("localhost", 7777);
			write = new PrintWriter(sock.getOutputStream(), true);

			in = new BufferedReader(
					new InputStreamReader(sock.getInputStream()));

			

				// remove .xml
				String protocolName = protocols[i].substring(0,
						protocols[i].length() - 4);

				write.println("Models/" + model + "\n" + "Protocols/"
						+ protocols[i] + "\n" + modelName + "/" + protocolName
						+ "\n");

				String read = in.readLine();
				while (read != null) {
					response += (read + "\n");
					read = in.readLine();
				}

			

		} catch (IOException ioe) {
			response += ioe.getMessage();
		}
		}
		return response;

	}
}

/*
*/