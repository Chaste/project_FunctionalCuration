package fcs.client;

import java.util.ArrayList;

import com.google.gwt.user.client.rpc.AsyncCallback;

import fcs.shared.MenuData;
import fcs.shared.PlotData;

/**
 * The async counterpart of <code>GreetingService</code>.
 */
public interface GetResultsServiceAsync {
	void getResults(ArrayList<String> modelList, String protocol, AsyncCallback<PlotData> callback) throws IllegalArgumentException;
	void getMenuData(AsyncCallback<MenuData> callback);
}
