package fcs.client;

import com.google.gwt.user.client.rpc.AsyncCallback;

/**
 * The async counterpart of <code>SimulationService</code>.
 */
public interface SimulationServiceAsync {
	void runSimulation(String model, AsyncCallback<String> callback);
}