package fcs.client.handlers;

import com.google.gwt.event.dom.client.ClickEvent;
import com.google.gwt.event.dom.client.ClickHandler;
import com.google.gwt.user.client.ui.ListBox;

public class SelectionButtonHandler implements ClickHandler {

	ListBox models;
	ListBox selectedModels;
	
	public SelectionButtonHandler(ListBox models, ListBox selectedModels){
		this.models=models;
		this.selectedModels=selectedModels;
	}
		@Override
		public void onClick(ClickEvent event) {
			int x = models.getSelectedIndex();
			if (x > -1) {
				String s = models.getItemText(x);
				selectedModels.addItem(s);
				for (int i = x + 1; i < models.getItemCount(); i++) {
					if (models.isItemSelected(i)) {
						String s2 = models.getItemText(i);
						selectedModels.addItem(s2);
					}
				}
			}
		}

	}