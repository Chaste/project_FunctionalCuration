package fcs.client;
import com.google.gwt.user.client.ui.TextArea;

import gwtupload.client.BaseUploadStatus;


public class MyUploadStatus extends BaseUploadStatus{
	TextArea text;
	public MyUploadStatus(TextArea text){
		super();
		this.text=text;
	}
	@Override
	public void setError(String msg){
		setStatus(Status.ERROR);
		String buff = text.getText();
		text.setText(buff+"Error: only cellml files can be uploaded\n");
	}
}
