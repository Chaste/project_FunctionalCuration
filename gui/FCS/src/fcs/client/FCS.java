package fcs.client;

import java.util.ArrayList;

import fcs.client.graph.GraphComposite;
import fcs.client.handlers.SelectionButtonHandler;
import fcs.shared.FieldVerifier;
import fcs.shared.MenuData;
import fcs.shared.Plot;
import fcs.shared.PlotData;
import gwtupload.client.IUploadStatus.Status;
import gwtupload.client.IUploader;
import gwtupload.client.IUploader.OnFinishUploaderHandler;
import gwtupload.client.IUploader.UploadedInfo;
import gwtupload.client.MultiUploader;
import gwtupload.client.PreloadedImage;
import gwtupload.client.PreloadedImage.OnLoadPreloadedImageHandler;

import com.google.gwt.core.client.EntryPoint;
import com.google.gwt.core.client.GWT;
import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.event.dom.client.KeyCodes;
import com.google.gwt.event.dom.client.KeyUpEvent;
import com.google.gwt.event.dom.client.KeyUpHandler;
import com.google.gwt.user.client.rpc.AsyncCallback;
import com.google.gwt.user.client.ui.Button;
import com.google.gwt.user.client.ui.DialogBox;
import com.google.gwt.user.client.ui.FlexTable;
import com.google.gwt.user.client.ui.FlowPanel;
import com.google.gwt.user.client.ui.HTML;
import com.google.gwt.user.client.ui.HasHorizontalAlignment;
import com.google.gwt.user.client.ui.HorizontalPanel;
import com.google.gwt.user.client.ui.Label;
import com.google.gwt.user.client.ui.ListBox;
import com.google.gwt.user.client.ui.Panel;
import com.google.gwt.user.client.ui.RichTextArea;
import com.google.gwt.user.client.ui.RootPanel;
import com.google.gwt.user.client.ui.TextArea;
import com.google.gwt.user.client.ui.TextBox;
import com.google.gwt.user.client.ui.VerticalPanel;

/**
 * Entry point classes define <code>onModuleLoad()</code>.
 */
public class FCS implements EntryPoint {
	/**
	 * The message displayed to the user when the server cannot be reached or
	 * returns an error.
	 */
	private static final String SERVER_ERROR = "An error occurred while "
			+ "attempting to contact the server. Please check your network "
			+ "connection and try again.";

	/**
	 * Create a remote service proxy to talk to the server-side Greeting
	 * service.
	 */
	private final GreetingServiceAsync greetingService = GWT
			.create(GreetingService.class);

	private final GetResultsServiceAsync getRes = GWT
			.create(GetResultsService.class);

	private final SimulationServiceAsync simulation = GWT
			.create(SimulationService.class);

	/**
	 * This is the entry point method.
	 */

	// Load the image in the document and in the case of success attach it to
	// the viewer

	// final Label test = new
	// Label("TEEEEEEEEEEEEEEEEESSSSSSSSSSSSSSSSSTTTTTTTTTTTTT");
	final Label modelsLabel = new Label("Models:");
	final Label protocolsLabel = new Label("Protocols:");
	final ListBox models = new ListBox(true);
	final ListBox protocols = new ListBox();
	final Button getGraph = new Button("Display Simulation Results");
	final TextArea text = new TextArea();

	public static int numberOfGraphs =0;
	
	final ListBox selectedModels = new ListBox();
	private OnFinishUploaderHandler onFinishUploaderHandler = new OnFinishUploaderHandler() {
		@Override
		public void onFinish(IUploader uploader) {
			if (uploader.getStatus() == Status.SUCCESS) {

				// The server sends useful information to the client by default
				UploadedInfo info = uploader.getServerInfo();
				simulation.runSimulation(info.name,
						new AsyncCallback<String>() {

							@Override
							public void onSuccess(String result) {
								String currentText = text.getText();
								text.setText(currentText + "\n" + result);
							}

							@Override
							public void onFailure(Throwable caught) {
								// TODO Auto-generated method stub

							}
						});
				// System.out.println("File name " + info.name);
				// System.out.println("File content-type " + info.ctype);
				// System.out.println("File size " + info.size);

				// You can send any customized message and parse it
				// System.out.println("Server message " + info.message);

			} else {
				String buff = text.getText();
				text.setText(buff+"Error occured while uploading...\n");
				
			}
		}
	};

	/**
	 * This is the entry point method.
	 */
	@Override
	public void onModuleLoad() {

		final VerticalPanel mainPanel = new VerticalPanel();
		final VerticalPanel graphPanel = new VerticalPanel();
		final HorizontalPanel menuPanel = new HorizontalPanel();
		// mainPanel.add(test);
		final FlexTable table = new FlexTable();
		final Label enter = new Label("Please enter your name:");
		final Button sendButton = new Button("Send");
		final TextBox nameField = new TextBox();

		nameField.setText("GWT User");
		nameField.selectAll();

		final Label errorLabel = new Label();

		// We can add style names to widgets
		sendButton.addStyleName("sendButton");

		table.setWidget(0, 0, enter);
		table.setWidget(1, 0, nameField);
		table.setWidget(1, 1, sendButton);

		// mainPanel.add(table);

		final Panel selectionPanel = new HorizontalPanel();
		getRes.getMenuData(new AsyncCallback<MenuData>() {

			@Override
			public void onFailure(Throwable caught) {
				System.out.println("failed");
				caught.printStackTrace();
			}

			@Override
			public void onSuccess(MenuData result) {
				initSelectionPanel(selectionPanel, result);
				selectionPanel.add(getGraph);
			}
		});

		
		selectionPanel.setStyleName("panel");
		menuPanel.add(selectionPanel);
	
		
		final VerticalPanel upLoadPanel = new VerticalPanel();
		// Create a new uploader panel and attach it to the document
		MyUploadStatus status = new MyUploadStatus(text);
		MultiUploader defaultUploader = new MultiUploader(status);

		String ext = "cellml";
		defaultUploader.setValidExtensions(ext);
		// Add a finish handler which will load the image once the upload
		// finishes
		defaultUploader.addOnFinishUploadHandler(onFinishUploaderHandler);
		text.setWidth("400px");
		text.setHeight("200px");
		text.setReadOnly(true);
		text.setText("Console:\n");
		
		Label up = new Label("Upload a model:");
		upLoadPanel.add(up);
		upLoadPanel.add(defaultUploader);
		upLoadPanel.setStyleName("panel");
		// int width = upLoadPanel.getOffsetWidth();

		// System.out.println("width "+width);
		// text.setWidth(""+width+"px");

		upLoadPanel.add(text);
		
		menuPanel.add(upLoadPanel);
		mainPanel.add(menuPanel);
		

		// final Panel graphPanel = new VerticalPanel();
		// initGraphPanel(graphPanel);

		// final Graph graph = new Graph("second", getValues());
		// graphPanel.add(graph);
		// m_graph.addListener(this);
		// mainPanel.add(graphPanel);

		// Focus the cursor on the name field when the app loads
		nameField.setFocus(true);
		nameField.selectAll();

		// Create the popup dialog box
		final DialogBox dialogBox = new DialogBox();
		dialogBox.setText("Remote Procedure Call");
		dialogBox.setAnimationEnabled(true);
		final Button closeButton = new Button("Close");
		// We can set the id of a widget by accessing its Element
		closeButton.getElement().setId("closeButton");
		final Label textToServerLabel = new Label();
		final HTML serverResponseLabel = new HTML();
		VerticalPanel dialogVPanel = new VerticalPanel();
		dialogVPanel.addStyleName("dialogVPanel");
		dialogVPanel.add(new HTML("<b>Sending name to the server:</b>"));
		dialogVPanel.add(textToServerLabel);
		dialogVPanel.add(new HTML("<br><b>Server replies:</b>"));
		dialogVPanel.add(serverResponseLabel);
		dialogVPanel.setHorizontalAlignment(HasHorizontalAlignment.ALIGN_RIGHT);
		dialogVPanel.add(closeButton);
		dialogBox.setWidget(dialogVPanel);

		// Add a handler to close the DialogBox
		closeButton.addClickHandler(new ClickHandler() {
			@Override
			public void onClick(ClickEvent event) {
				dialogBox.hide();
				sendButton.setEnabled(true);
				sendButton.setFocus(true);
			}
		});

		// Create a handler for the sendButton and nameField
		class GetDataHandler implements ClickHandler {
			/**
			 * Fired when the user clicks on the sendButton.
			 */
			@Override
			public void onClick(ClickEvent event) {

				// Then, we send the input to the server.
				getGraph.setEnabled(false);

				ArrayList<String> modelList = new ArrayList<String>();
				for (int i = 0; i < models.getItemCount(); ++i) {
					if (models.isItemSelected(i)) {
						modelList.add(models.getItemText(i));
					}
				}
				// String model = models.getItemText(models.getSelectedIndex());
				String protocol = protocols.getItemText(protocols
						.getSelectedIndex());
				getRes.getResults(modelList, protocol,
						new AsyncCallback<PlotData>() {
							@Override
							public void onFailure(Throwable caught) {

								System.out.println("FAILED !!!");
							}

							@Override
							public void onSuccess(PlotData result) {
								ArrayList<Plot> plots = result.getPlots();
								
									GraphComposite graph = new GraphComposite(
											plots, Integer.toString(numberOfGraphs));
									numberOfGraphs++;
									
								
									graphPanel.clear();
									graphPanel.add(graph);
							
								
							}

						});
				getGraph.setEnabled(true);
			}
		}

		// Create a handler for the sendButton and nameField
		class MyHandler implements ClickHandler, KeyUpHandler {
			/**
			 * Fired when the user clicks on the sendButton.
			 */
			@Override
			public void onClick(ClickEvent event) {
				sendNameToServer();
			}

			/**
			 * Fired when the user types in the nameField.
			 */
			@Override
			public void onKeyUp(KeyUpEvent event) {
				if (event.getNativeKeyCode() == KeyCodes.KEY_ENTER) {
					sendNameToServer();
				}
			}

			/**
			 * Send the name from the nameField to the server and wait for a
			 * response.
			 */
			private void sendNameToServer() {
				// First, we validate the input.
				errorLabel.setText("");
				String textToServer = nameField.getText();
				if (!FieldVerifier.isValidName(textToServer)) {
					errorLabel.setText("Please enter at least four characters");
					return;
				}

				// Then, we send the input to the server.
				sendButton.setEnabled(false);
				textToServerLabel.setText(textToServer);
				serverResponseLabel.setText("");
				greetingService.greetServer(textToServer,
						new AsyncCallback<String>() {
							@Override
							public void onFailure(Throwable caught) {
								// Show the RPC error message to the user
								dialogBox
										.setText("Remote Procedure Call - Failure");
								serverResponseLabel
										.addStyleName("serverResponseLabelError");
								serverResponseLabel.setHTML(SERVER_ERROR);
								dialogBox.center();
								closeButton.setFocus(true);
							}

							@Override
							public void onSuccess(String result) {
								dialogBox.setText("Remote Procedure Call");
								serverResponseLabel
										.removeStyleName("serverResponseLabelError");
								serverResponseLabel.setHTML(result);
								dialogBox.center();
								closeButton.setFocus(true);
							}
						});
			}
		}

		// Add a handler to send the name to the server
		MyHandler handler = new MyHandler();
		sendButton.addClickHandler(handler);
		nameField.addKeyUpHandler(handler);
		getGraph.addClickHandler(new GetDataHandler());
		mainPanel.add(graphPanel);
		RootPanel.get("body").add(mainPanel);

	}

	private void initSelectionPanel(Panel selectionPanel, MenuData data) {

		for (String s : data.getModels()) {
			models.addItem(s);
		}

		for (String s : data.getProtocols()) {
			protocols.addItem(s);
		}

		FlexTable table = new FlexTable();

		models.setVisibleItemCount(1);
		// protocols.setVisibleItemCount(5);
		// selectedModels.setVisibleItemCount(15);
		models.setItemSelected(0, true);

		final Button modelSelectionButton = new Button("select");
		modelSelectionButton.addClickHandler(new SelectionButtonHandler(models,
				selectedModels));

		table.setWidget(0, 0, modelsLabel);
		table.setWidget(0, 1, protocolsLabel);
		table.setWidget(1, 0, models);
		table.setWidget(1, 1, protocols);

		selectionPanel.add(table);
		// selectionPanel.add(modelSelectionButton);
		// selectionPanel.add(selectedModels);
	}

}
