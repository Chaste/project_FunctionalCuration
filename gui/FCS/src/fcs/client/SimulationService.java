package fcs.client;

import com.google.gwt.user.client.rpc.RemoteService;
import com.google.gwt.user.client.rpc.RemoteServiceRelativePath;



/**
 * The client side stub for the RPC service.
 */
@RemoteServiceRelativePath("simulation")
public interface SimulationService extends RemoteService {
	String runSimulation(String model);
}