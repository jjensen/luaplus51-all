-- Load user interface file

ui = qt.load_ui("hello.ui");

-- Connect the quit button click signal to the application closeAllWindows slot.

qt.connect(ui.pushButton_2, "clicked", app, "closeAllWindows");

-- Connect the hello button click signal to a lua function which set the text content of the lineEdit widget.

qt.connect(ui.pushButton, "clicked",
	function ()
		ui.lineEdit:setText("hello world");
	end
	);

-- Show the user interface window

ui:show();
