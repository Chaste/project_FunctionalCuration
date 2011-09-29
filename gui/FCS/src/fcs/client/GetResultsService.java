package fcs.client;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.RemoteService;
import com.google.gwt.user.client.rpc.RemoteServiceRelativePath;

import fcs.shared.MenuData;
import fcs.shared.PlotData;

/**
 * The client side stub for the RPC service.
 */
@RemoteServiceRelativePath("results")
public interface GetResultsService extends RemoteService {
	PlotData getResults(ArrayList<String> modelList, String protocol, String xVar, String yVar) throws IllegalArgumentException;

	MenuData getMenuData();
}