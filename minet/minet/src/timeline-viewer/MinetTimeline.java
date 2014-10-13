/*
Jeff Kwiat
Dr. Peter Dinda
Northwestern University
12:54 AM 09-26-2002
*/

import java.io.*;
import java.awt.Container;
import java.awt.GridBagLayout;
import java.util.StringTokenizer;
import java.io.BufferedWriter;
import java.io.FileOutputStream;
import javax.swing.table.AbstractTableModel;
import javax.swing.JTable;
import javax.swing.JScrollPane;
import javax.swing.table.DefaultTableModel;
import javax.swing.JFrame;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.event.*;
import java.util.Vector;
import java.util.Calendar;
import java.util.Hashtable;
import java.sql.Timestamp;
import java.text.DateFormat;
import java.sql.Date;
import javax.swing.table.TableColumn;
import java.text.*;

class MinetTimeline implements ActionListener {
    
    //Variable declarations.
    JPanel jPanel = new JPanel();
    JFrame jFrame = new JFrame("Minet Timeline");
    JMenuBar jMenuBar = new JMenuBar();
    JMenu jMenu0;
    JMenu jMenu1;
    JMenu jMenu2;
    JMenuItem jMenuItem0;
    JMenuItem jMenuItem1;
    JMenuItem jMenuItem2;
    JMenuItem jMenuItem3;
    JMenuItem jMenuItem4;
    JMenuItem jMenuItem5;
    JMenuItem jMenuItem6;
    JMenuItem jMenuItem7;
    JMenuItem jMenuItem8;
    JMenuItem jMenuItem9;
    JMenuItem jMenuItem10;
    JMenuItem jMenuItem11;	
    JScrollPane tablePane;
    JScrollPane textAreaPane;
    JTable jTable;
    JFileChooser fc = new JFileChooser();
    JSlider jSlider = new JSlider();
    JSlider jSlider0 = new JSlider(1, 6, 1);
    JSlider jSlider1;
    JTextArea jTextArea = new JTextArea("Load a file and then click on a cell in the \"Buffer\" column to view its contents.");
    Vector rowData;
    Vector columnNames;
    Vector tableVec;
    Vector selectVec;
    GridBagLayout gridbag = new GridBagLayout();
    GridBagConstraints c = new GridBagConstraints();
    private boolean ALLOW_COLUMN_SELECTION = false;
    private boolean ALLOW_ROW_SELECTION = true;
    ListSelectionModel jTableListener;
    String begTime = "";
    String endTime = "";
    String eventState = "ALL EVENTS";
    static int fileCounter = 0;
    static String myFile = "";
    
    public MinetTimeline() {
	initGui(jTable);
    }
    
    public void initGui(JTable table) {
	
	jFrame.getContentPane().setLayout(gridbag);
	c.gridx = 0;
	c.weighty = 0.0;
	c.gridy = 0;
	c.fill = GridBagConstraints.HORIZONTAL;
	c.anchor = GridBagConstraints.NORTH;
	c.ipadx = 0;
	c.ipady = 0;
	
	//Create each menu item
	jMenuItem0 = new JMenuItem("Open...",  KeyEvent.VK_O);
	jMenuItem0.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O, ActionEvent.ALT_MASK));
	jMenuItem1 = new JMenuItem("Save As...", KeyEvent.VK_S);
	jMenuItem1.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S, ActionEvent.ALT_MASK));
	jMenuItem2 = new JMenuItem("Exit");
	jMenuItem3 = new JMenuItem("About", KeyEvent.VK_A);
	jMenuItem3.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_A, ActionEvent.ALT_MASK));
	
	jMenuItem4 = new JMenuItem("INIT", KeyEvent.VK_E);
	jMenuItem5 = new JMenuItem("CONNECT", KeyEvent.VK_E);	
	jMenuItem6 = new JMenuItem("SEND", KeyEvent.VK_E);
	jMenuItem7 = new JMenuItem("ACCEPT", KeyEvent.VK_E);
	jMenuItem8 = new JMenuItem("RECEIVE", KeyEvent.VK_E);
	jMenuItem9 = new JMenuItem("GETNEXTEVENT", KeyEvent.VK_E);
	jMenuItem10 = new JMenuItem("SENDTOMONITOR", KeyEvent.VK_E);
	jMenuItem11 = new JMenuItem("ALL EVENTS", KeyEvent.VK_E);
	
	//Add ActionListeners to each menuItem
	jMenuItem0.addActionListener(this);
	jMenuItem1.addActionListener(this);
	jMenuItem2.addActionListener(this);
	jMenuItem3.addActionListener(this);
	jMenuItem4.addActionListener(this);
	jMenuItem5.addActionListener(this);
	jMenuItem6.addActionListener(this);
	jMenuItem7.addActionListener(this);
	jMenuItem8.addActionListener(this);
	jMenuItem9.addActionListener(this);
	jMenuItem10.addActionListener(this);
	jMenuItem11.addActionListener(this);
	
	//Create each menu for the menuBar
	jMenu0 = new JMenu("File");
	jMenu1 = new JMenu("Help");
	jMenu2 = new JMenu("Select");
	
	//Add menu items to their respective menus
	jMenu0.add(jMenuItem0);
	jMenu0.add(jMenuItem1);
	jMenu0.add(jMenuItem2);
	jMenu2.add(jMenuItem4);
	jMenu2.add(jMenuItem5);
	jMenu2.add(jMenuItem6);
	jMenu2.add(jMenuItem7);
	jMenu2.add(jMenuItem8);
	jMenu2.add(jMenuItem9);
	jMenu2.add(jMenuItem10);
	jMenu2.add(jMenuItem11);
	jMenu1.add(jMenuItem3);
	
	//Add menus to menuBar
	jMenuBar.add(jMenu0);
	jMenuBar.add(jMenu2);
	jMenuBar.add(jMenu1);
	
	//Add menuBar to jFrame
	jFrame.setJMenuBar(jMenuBar);
	
	//Create label to describe jSlider
	JLabel sliderLabel = new JLabel("# of Viewable Rows", JLabel.CENTER);
	c.fill = GridBagConstraints.HORIZONTAL;
	gridbag.setConstraints(sliderLabel, c);
	
	//Add sliderLabel to jFrame
	jFrame.getContentPane().add(sliderLabel, c);
	
	c.gridy = 1;

	//Set properties of jSlider
	jSlider.setOrientation(0);
	jSlider.setPaintTicks(true);
	jSlider.setMajorTickSpacing(10);
	jSlider.setMinorTickSpacing(1);
	jSlider.setPaintLabels(true);
	jSlider.setSnapToTicks(true);
	jSlider.setMinimumSize(new Dimension(850, 50));
	
	//Add ChangeListener to jSlider
	jSlider.addChangeListener(new SliderListener());
	
	//Set properties of gridbag
	gridbag.setConstraints(jSlider, c);

	//Add jSlider to jFrame
	jFrame.getContentPane().add(jSlider, c);
	
	c.gridy = 2;
	
	JLabel sliderLabel0 = new JLabel("Viewable Window", JLabel.CENTER);
	c.fill = GridBagConstraints.HORIZONTAL;
	gridbag.setConstraints(sliderLabel0, c);

	//Add sliderLabel to jFrame
	jFrame.getContentPane().add(sliderLabel0, c);
	
	c.gridy = 3;
	
	//Set properties of jSlider
	jSlider0.setOrientation(0);
	jSlider0.setPaintTicks(true);
	jSlider0.setMajorTickSpacing(1);
	jSlider0.setMinorTickSpacing(1);
	jSlider0.setPaintLabels(true);
	jSlider0.setSnapToTicks(true);
	jSlider0.setMinimumSize(new Dimension(850, 50));
	
	//Labels for the Size of Window slider.
	Hashtable labelTable0 = new Hashtable();
	labelTable0.put(new Integer(1), new JLabel("1 ms"));
	labelTable0.put(new Integer(2), new JLabel("10 ms"));
	labelTable0.put(new Integer(3), new JLabel("1 sec"));
	labelTable0.put(new Integer(4), new JLabel("10 sec"));
	labelTable0.put(new Integer(5), new JLabel("1 min"));
	labelTable0.put(new Integer(6), new JLabel("All Times"));
	
	jSlider0.setLabelTable(labelTable0);
	jSlider0.setValue(6);
	
	//Add ChangeListener to jSlider
	jSlider0.addChangeListener(new SliderListener());
	
	//Set properties of gridbag
	gridbag.setConstraints(jSlider0, c);
	
	//Add jSlider to jFrame
	jFrame.getContentPane().add(jSlider0, c);
	
	c.gridy = 4;
	c.fill = GridBagConstraints.HORIZONTAL;
	
	//Initialize jTextarea
	jTextArea.setLineWrap(true);
	jTextArea.setEditable(false);
	jTextArea.setCaretPosition(0);
	
	//Initialize textAreaPane
	textAreaPane = new JScrollPane(jTextArea);
	textAreaPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
	textAreaPane.setPreferredSize(new Dimension(850, 100));
	textAreaPane.setMinimumSize(new Dimension(850, 100));
	
	//Set properties of jFrame
	jFrame.setSize(850, 400);
	jFrame.setResizable(false);
	jFrame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	
	c.fill = GridBagConstraints.HORIZONTAL;
	c.gridy = 7;
	gridbag.setConstraints(textAreaPane, c);

	
	//Add jTextArea to jFrame
	jFrame.getContentPane().add(textAreaPane);
	
	//pack() reduces the size to fit the contents exactly
	jFrame.pack();
	jFrame.setVisible(true);
	if (fileCounter == 1) {
	    loadTable2(myFile);
	}
    }

    //Set of Listeners for the menu items.  Each calls a corresponding function.
    public void actionPerformed(ActionEvent e) {
	
	if (e.getSource() == jMenuItem0) {
	    loadTable();
	}
	else if (e.getSource() == jMenuItem1) {
	    saveTable();
	}
	else if (e.getSource() == jMenuItem2) {
	    System.exit(1);
	}
	else if (e.getSource() == jMenuItem3) {
	    showAbout();
	}
	else if (e.getSource() == jMenuItem4) {
	    eventState = "INIT";
	    adjustWindow();
	}
	else if (e.getSource() == jMenuItem5) {
	    selectEvent("CONNECT");
	    eventState = "CONNECT";
	}
	else if (e.getSource() == jMenuItem6) {
	    selectEvent("SEND");
	    eventState = "SEND";
	}
	else if (e.getSource() == jMenuItem7) {
	    selectEvent("ACCEPT");
	    eventState = "ACCEPT";
	}
	else if (e.getSource() == jMenuItem8) {
	    selectEvent("RECEIVE");
	    eventState = "RECEIVE";
	}
	else if (e.getSource() == jMenuItem9) {
	    selectEvent("GETNEXTEVENT");
	    eventState = "GETNEXTEVENT";
	}
	else if (e.getSource() == jMenuItem10) {
	    selectEvent("SENDTOMONITOR");
	    eventState = "SENDTOMONITOR ";
	}
	else if (e.getSource() == jMenuItem11) {
	    eventState = "ALL EVENTS";
	    adjustWindow();
	}    
    }
    
    //Converts a string in the form "seconds since 1970.fractional part of the last second"
    //to a human-readable format for display within the table.
    public String getTimestamp(String timeIn) {
	StringTokenizer tokTime = new StringTokenizer(timeIn, ".");
	String timeLeft = tokTime.nextToken();
	String timeRight = tokTime.nextToken();
	String uTS = "";
	String unixTime = "";

	timeRight = timeRight.substring(0, 8);
	
	Calendar myCal = Calendar.getInstance();

	myCal.set(1970, 0, 1, 0, 0, 0);

	DateFormat df = new SimpleDateFormat("yyyy-mm-dd HH:mm:ss");

	myCal.add(Calendar.SECOND, Integer.parseInt(timeLeft));
	unixTime = df.format(myCal.getTime());

	Timestamp unixTS = new Timestamp(0);

	unixTS = unixTS.valueOf(unixTime.toString());
	unixTS.setNanos(Integer.parseInt(timeRight));
	uTS = unixTS.toString();
	uTS = uTS.substring(11);

	return uTS;
    }

    //Loads a table when File -> Open is selected from the menu.
    public void loadTable() {
	
	if (rowData == null) {
	    int returnVal = fc.showOpenDialog(jMenuBar);
	    if (returnVal == JFileChooser.APPROVE_OPTION) {
		File file = fc.getSelectedFile();
		
		try {
		    FileReader inFile = new FileReader(file);
		    BufferedReader in = new BufferedReader(inFile);
		    
		    int counter = 0;
		    int counter1 = 0;
		    int counter2 = 0;
		    
		    rowData = new Vector(10, 1);
		    Vector temp = new Vector(7, 0);
		    columnNames = new Vector(7);
		    
		    String line = "";
		    String time = "";
		    String tempLine = "";
		    String myBuffer = "";
		    String token2 = "";
		    
		    //Create the column Names
		    columnNames.add("TimeStamp");
		    columnNames.add("Event");
		    columnNames.add("Buffer");
		    columnNames.add("OpType");
		    columnNames.add("Source");
		    columnNames.add("Dest");
		    columnNames.add("Origin");
		    
		    while((line = in.readLine()) != null) {
			
			counter2++;
			Vector victor = new Vector(7, 0);
			String lineCopy = line;
			String token0 = "";
			String token1 = "";
			String ts = "";
			String org = "";
			String src = "";
			String dest = "";
			String op = "";
			String dt = "";
			String buf = "";
		       	StringTokenizer tokIn = new StringTokenizer(line, "=", true);

			ts = tokIn.nextToken("=");
			ts = tokIn.nextToken(",");
			ts = ts.substring(1);

			String tsIdentifier = ts.substring(2, 3);

			if (tsIdentifier.equals(":") == false) {
			    //Convert the timestamp into a human-readable format.
			    ts = getTimestamp(ts);
			}			

			//Divide the MinetMonitoringEventDesc string into separate pieces.
			tokIn.nextToken("=");
			org = tokIn.nextToken(",");
			org = org.substring(7);

			tokIn.nextToken("=");
			src = tokIn.nextToken(",");
			src = src.substring(7);

			tokIn.nextToken("=");
			dest = tokIn.nextToken(",");
			dest = dest.substring(7);

			tokIn.nextToken("=");
			op = tokIn.nextToken(",");
			op = op.substring(7);

			tokIn.nextToken("=");
			dt = tokIn.nextToken(")");
			dt = dt.substring(7);

			tokIn.nextToken(" ");
			tokIn.nextToken(" ");
			buf = tokIn.nextToken("\n") + ")";
			buf = buf.substring(2);			
			
			endTime = ts;
			
			//If this is the first timestamp, we set it as the beginning Timestamp
			if (counter1 == 0) {
			    begTime = ts;
			    counter1 = 1;
			}
			
			//Add each part to a Vector.  This represents one row in the table.
			victor.add(ts);
			victor.add(op);
			victor.add(buf);
			victor.add(dt);
			victor.add(org);
			victor.add(src);
			victor.add(dest);
			
			//Add the row to another Vector to create the table.
			rowData.add(victor);
		    }
		    
		    //This divisor variable is used to decide where to place
		    //the slider labels for the "Position in File" slider.
		    //This is because you can not fit too many on the slider
		    //or it looks cluttered.

		    int divisor = counter2 / 4;
		    String begTimeStamp = begTime;
		    String endTimeStamp = endTime;
		    StringTokenizer strTok2 = new StringTokenizer(begTime, ":", true);
		    String hour = strTok2.nextToken();
		    
		    strTok2.nextToken();
		    String min = strTok2.nextToken();
		    
		    strTok2.nextToken();
		    String sec = strTok2.nextToken(".");
		    
		    strTok2.nextToken();
		    String milli = strTok2.nextToken();
		    
		    begTime = hour + min + sec + milli;
		    begTime = begTime.substring(0, 10);
		    
		    int begin = (int)Integer.parseInt(begTime);
		    
		    StringTokenizer strTok3 = new StringTokenizer(endTime, ":", true);
		    String hour1 = strTok3.nextToken();
		    
		    strTok3.nextToken();
		    String min1 = strTok3.nextToken();
		    
		    strTok3.nextToken();
		    String sec1 = strTok3.nextToken(".");
		    
		    strTok3.nextToken();
		    String milli1 = strTok3.nextToken();
		    
		    endTime = hour1 + min1 + sec1 + milli1;
		    endTime = endTime.substring(0, 10);
		    
		    int end = (int)Integer.parseInt(endTime);
		    
		    //Set rowData to be tableVec
		    tableVec = rowData;
		    
		    //Initialize JTable
		    jTable = new JTable(tableVec, columnNames) {
			    public boolean isCellEditable(int rowIndex, int colIndex) {
				return false;
			    }
			};
		    
		    //Create a tablepane and add our table to it for display.
		    tablePane = new JScrollPane(jTable);
		    jTable.setRowHeight(15);
		    jTable.setMinimumSize(new Dimension(850, 10));
		    jTable.updateUI();
		    tablePane.setMinimumSize(new Dimension(850, 50));
		    
		    //Create a table listener for when a user clicks on a row.
		    jTableListener = jTable.getSelectionModel();
		    jTableListener.addListSelectionListener(new TableListener());
		    
		    //Only allow, at most, 20 rows to be viewed at one time
		    jSlider.setMinimum(0);
		    if (jTable.getRowCount() > 30) {
			jSlider.setMaximum(30);
		    }
		    else {
			jSlider.setMaximum(jTable.getRowCount());
		    }
		    jSlider.setValue(10);
		    
		    c.gridy = 4;
		    
		    JLabel sliderLabel1 = new JLabel("Position in File", JLabel.CENTER);
		    c.fill = GridBagConstraints.HORIZONTAL;
		    gridbag.setConstraints(sliderLabel1, c);
		    
		    //Add sliderLabel to jFrame
		    jFrame.getContentPane().add(sliderLabel1, c);
		    
		    c.gridy = 5;
		    c.fill = GridBagConstraints.HORIZONTAL;
		    
		    jSlider1 = new JSlider(begin, end, begin);
		    //Set properties of jSlider
		    jSlider1.setOrientation(0);
		    jSlider1.setPaintTicks(true);
		    jSlider1.setMajorTickSpacing(divisor);
		    jSlider1.setMinorTickSpacing(divisor / 10);
		    jSlider1.setPaintLabels(true);
		    jSlider1.setSnapToTicks(false);
		    jSlider1.setMinimumSize(new Dimension(800, 80));
		    
		    Hashtable labelTable = new Hashtable();
		    labelTable.put(new Integer(begin), new JLabel(begTimeStamp));					
		    
		    for (int a=0; a<jTable.getColumnCount(); a++) {
			if (jTable.getColumnName(a) == "TimeStamp") {
			    
			    String cell1 = (String)jTable.getValueAt(divisor * 2, a);
			    String pcCell1 = parseCell(cell1);
			    int intCell1 = Integer.parseInt(pcCell1);
			    
			    labelTable.put(new Integer(intCell1), new JLabel(cell1));
			    
			}
		    }
		    
		    labelTable.put(new Integer(end), new JLabel(endTimeStamp));
		    jSlider1.setLabelTable(labelTable);
		    
		    //Add ChangeListener to jSlider
		    jSlider1.addChangeListener(new SliderListener());
		    
		    //Set properties of gridbag
		    gridbag.setConstraints(jSlider1, c);
		    
		    //Add jSlider to jFrame
		    jFrame.getContentPane().add(jSlider1, c);
		    
		}
		catch(IOException fe) {
		    System.out.println("File Not Found: " + fe.getMessage());
		}
		c.gridx = 0;
		c.gridy = 8;
		c.weighty = 1.0;
		c.fill = GridBagConstraints.HORIZONTAL;
		c.anchor = GridBagConstraints.SOUTH;
		
		gridbag.setConstraints(jTable, c);
		jFrame.getContentPane().add(tablePane, c);
		jFrame.validate();
		jFrame.pack();
	    }
	    else {
		System.out.println("Load action was cancelled by user.");
	    }
	}
	else {
	    twoTables();
	}
    }
    
    //Loads a table when a filename is given at the command line.
    public void loadTable2(String filename) {
	
	if (rowData == null) {
	    
	    
	    try {
		FileReader inFile = new FileReader(filename);
		BufferedReader in = new BufferedReader(inFile);
		
		int counter = 0;
		int counter1 = 0;
		int counter2 = 0;
		
		rowData = new Vector(10, 1);
		Vector temp = new Vector(7, 0);
		columnNames = new Vector(7);
		
		String line = "";
		String time = "";
		String tempLine = "";
		String myBuffer = "";
		String token2 = "";
		
		//Create the column Names
		columnNames.add("TimeStamp");
		columnNames.add("Event");
		columnNames.add("Buffer");
		columnNames.add("OpType");
		columnNames.add("Source");
		columnNames.add("Dest");
		columnNames.add("Origin");					
		
		while((line = in.readLine()) != null) {
		    counter2++;
		    Vector victor = new Vector(7, 0);
		    String lineCopy = line;
		    String token0 = "";
		    String token1 = "";
		    String myTimeStamp = "";
		    String myEvent = "";
		    String myOrg = "";
		    String mySrc = "";
		    String myDest = "";
		    String myOpType = "";
		    String op = "";
		    String dt = "";
		    String desc = "";
		    
		    myTimeStamp = line;
		    if (counter1 == 0) {
			begTime = myTimeStamp;
			counter1 = 1;
		    }
		    endTime = line;
		    myEvent = in.readLine();
		    myBuffer = in.readLine();
		    myOpType = in.readLine();
		    myOrg = in.readLine();
		    mySrc = in.readLine();
		    myDest = in.readLine();
		    
		    //Add each variable to the Vector "victor"
		    victor.add(myTimeStamp);
		    victor.add(myEvent);
		    victor.add(myBuffer);
		    victor.add(myOpType);
		    victor.add(myOrg);
		    victor.add(mySrc);
		    victor.add(myDest);
		    
		    rowData.add(victor);
		}
		
		int divisor = counter2 / 4;
		
		String begTimeStamp = begTime;
		String endTimeStamp = endTime;
		
		StringTokenizer strTok2 = new StringTokenizer(begTime, ":", true);
		String hour = strTok2.nextToken();
		
		strTok2.nextToken();
		String min = strTok2.nextToken();
		
		strTok2.nextToken();
		String sec = strTok2.nextToken(".");
		
		strTok2.nextToken();
		String milli = strTok2.nextToken();
		
		begTime = hour + min + sec + milli;
		
		int begin = (int)Integer.parseInt(begTime);
		
		StringTokenizer strTok3 = new StringTokenizer(endTime, ":", true);
		String hour1 = strTok3.nextToken();
		
		strTok3.nextToken();
		String min1 = strTok3.nextToken();
		
		strTok3.nextToken();
		String sec1 = strTok3.nextToken(".");
		
		strTok3.nextToken();
		String milli1 = strTok3.nextToken();
		
		endTime = hour1 + min1 + sec1 + milli1;
		
		int end = (int)Integer.parseInt(endTime);
		
		//Set rowData to be tableVec; tableVec is the data structure displayed in the table.
		tableVec = rowData;
		
		//Initialize JTable
		jTable = new JTable(tableVec, columnNames) {
			public boolean isCellEditable(int rowIndex, int colIndex) {
			    return false;
			}
		    };
		tablePane = new JScrollPane(jTable);
		jTable.setRowHeight(15);
		jTable.setMinimumSize(new Dimension(850, 10));
		jTable.updateUI();
		
		tablePane.setMinimumSize(new Dimension(850, 50));
		
		jTableListener = jTable.getSelectionModel();
		jTableListener.addListSelectionListener(new TableListener());
		
		//Only allow, at most, 20 rows to be viewed at one time
		jSlider.setMinimum(0);
		if (jTable.getRowCount() > 30) {
		    jSlider.setMaximum(30);
		}
		else {
		    jSlider.setMaximum(jTable.getRowCount());
		}
		jSlider.setValue(10);
		
		c.gridy = 4;
		
		JLabel sliderLabel1 = new JLabel("Position in File", JLabel.CENTER);
		c.fill = GridBagConstraints.HORIZONTAL;
		gridbag.setConstraints(sliderLabel1, c);
		
		//Add sliderLabel to jFrame
		jFrame.getContentPane().add(sliderLabel1, c);
		
		c.gridy = 5;
		c.fill = GridBagConstraints.HORIZONTAL;
		
		jSlider1 = new JSlider(begin, end, begin);
		//Set properties of jSlider
		jSlider1.setOrientation(0);
		jSlider1.setPaintTicks(true);
		jSlider1.setMajorTickSpacing(divisor);
		jSlider1.setMinorTickSpacing(divisor / 10);
		jSlider1.setPaintLabels(true);
		jSlider1.setSnapToTicks(false);
		jSlider1.setMinimumSize(new Dimension(800, 80));
		
		Hashtable labelTable = new Hashtable();
		labelTable.put(new Integer(begin), new JLabel(begTimeStamp));					
		
		for (int a=0; a<jTable.getColumnCount(); a++) {
		    if (jTable.getColumnName(a) == "TimeStamp") {
			
			String cell1 = (String)jTable.getValueAt(divisor * 2, a);
			String pcCell1 = parseCell(cell1);
			int intCell1 = Integer.parseInt(pcCell1);
			
			labelTable.put(new Integer(intCell1), new JLabel(cell1));
		    }
		}
		
		labelTable.put(new Integer(end), new JLabel(endTimeStamp));
		jSlider1.setLabelTable(labelTable);
		
		//Add ChangeListener to jSlider
		jSlider1.addChangeListener(new SliderListener());
		
		//Set properties of gridbag
		gridbag.setConstraints(jSlider1, c);
		
		//Add jSlider to jFrame
		jFrame.getContentPane().add(jSlider1, c);
		
	    }
	    catch(IOException fe) {
		System.out.println("File Not Found: " + fe.getMessage());
	    }
	    c.gridx = 0;
	    c.gridy = 8;
	    c.weighty = 1.0;
	    c.fill = GridBagConstraints.HORIZONTAL;
	    c.anchor = GridBagConstraints.SOUTH;
	    
	    gridbag.setConstraints(jTable, c);
	    jFrame.getContentPane().add(tablePane, c);
	    jFrame.validate();
	    jFrame.pack();
	}
	else {
	    twoTables();
	}
    }
    
    //Function to allow specific events to be displayed
    //This is called by selecting an item from the "Select"
    //menu and at the end of a call to adjustWindow();
    public void selectEvent(String str) {
	if (tableVec != null) {
	    selectVec = new Vector(10, 1);
	    if (str.equals("ALL EVENTS") == false) {

		for (int i=0; i<tableVec.size(); i++) {
		    Vector eventVec = (Vector)tableVec.elementAt(i);
		    if (eventVec.elementAt(1).equals(str)) {
			selectVec.add(eventVec);
		    }
		}
		tableVec = selectVec;
	    }
	    
	    jTable = new JTable(tableVec, columnNames) {
		    //Makes sure the cells of the new jTable are not editable.
		    public boolean isCellEditable(int rowIndex, int colIndex) {
			return false;
		    }
		};
	    //Refreshes the table for display.
	    jFrame.getContentPane().remove(tablePane);
	    tablePane = null;
	    tablePane = new JScrollPane(jTable);
	    jTable.updateUI();
	    jFrame.validate();
	    jFrame.pack();
	    c.gridx = 0;
	    c.gridy = 8;
	    c.weighty = 1.0;
	    c.fill = GridBagConstraints.HORIZONTAL;
	    c.anchor = GridBagConstraints.SOUTH;
	    gridbag.setConstraints(jTable, c);
	    jFrame.getContentPane().add(tablePane, c);
	    jTable.updateUI();
	    jFrame.validate();
	    jFrame.pack();
	    jTableListener = jTable.getSelectionModel();
	    jTableListener.addListSelectionListener(new TableListener());
	    adjustTable();
	}
	else {
	    System.out.println("You must first load a table by selecting \"File\" --> \"Open\".");
	}
    }

    //Saves the contents of the table to a new file
    public void saveTable() {
	int returnVal = fc.showSaveDialog(jMenuBar);
	if (returnVal == JFileChooser.APPROVE_OPTION) {
	    File file = fc.getSelectedFile();
	    String cellVal = "";
	    
	    try {
	      
		FileWriter outFile = new FileWriter(file);
		FileOutputStream out = new FileOutputStream(file);
		PrintStream p = new PrintStream(out);

		for (int i=0; i<tableVec.size(); i++) {
		    Vector row = (Vector)tableVec.elementAt(i);
		    String desc = "MinetMonitoringEventDesc(" + 
			"timestamp=" + row.elementAt(0) + 
			", source=MINET_" + row.elementAt(6) + 
			", from=MINET_" + row.elementAt(4) + 
			", to=MINET_" + row.elementAt(5) + 
			", optype=MINET_" + row.elementAt(3) + 
			", datatype=MINET_" + row.elementAt(1) +
			") : " + row.elementAt(2);
		    
		    p.println(desc);
		}
		p.close();
	    }
	    catch(IOException fe) {
		System.out.println("Output Error: " + fe.getMessage());
	    }
	}
	else {
	    System.out.println("Save action was cancelled by user.");
	}
	
    }

    //Help --> About menu
    public void showAbout() {
	JOptionPane.showMessageDialog(jMenuBar, "MinetTimelineGUI Version: 1.0\n\n" +
				      "Professor Peter Dinda\n" + "Jeff Kwiat\n" +
				      "Northwestern University\n\n" +
				      "Copywright 2002" );
    }
	
    //If someone selects to open a new table, this function brings up a pop-up menu
    //confirming that the user wants to close the currently open table (Since, only
    //one table can be open at a time).
    public void twoTables() {
	if (rowData != null) {
	    String message = "This action will close the currently open table\nDo you want to continue.";
	    JOptionPane pane = new JOptionPane();
	    int n = pane.showConfirmDialog(jMenuBar, 
					   message, "choose one", JOptionPane.YES_NO_OPTION);
	    
	    if (n == JOptionPane.YES_OPTION) {
		jFrame.getContentPane().remove(tablePane);
		jFrame.getContentPane().remove(jSlider1);
		jTable.updateUI();
		jFrame.validate();
		jFrame.pack();
		rowData = null;
		tableVec = null;
		jSlider1 = null;
	    }
	    else {
		return;
	    }
	}
	loadTable();
	adjustTable();
    }
    
    //Changes the number of rows in the JTable that are visible are at a single time.
    public void adjustTable() {
	int value = jSlider.getValue();
	int numRows = value * jTable.getRowHeight();
	
	jTable.setPreferredScrollableViewportSize(new Dimension(800, numRows));
	jTable.updateUI();
	jFrame.validate();
	jFrame.pack();
    }
    
    //Jumps to a row within the table.  A user will call this function when he adjusts
    //the starting value of the window size or when the window size slider is called.
    public void jumpToRow() {
	int value2 = jSlider1.getValue();
	//When the slider has not been moved, the value is -1 which can cause hanging
	//problems.
	if (value2 == -1) {
	    value2 = 0;
	}
	if (jSlider1.getValueIsAdjusting() == false) {
	    if (rowData != null) {
		for (int i=0; i<jTable.getRowCount(); i++) {
		    for (int j=0; j<jTable.getColumnCount(); j++) {
			if (jTable.getColumnName(j) == "TimeStamp") {
			    String cellVal = (String)jTable.getValueAt(i, j);
			    String strCell = parseCell(cellVal);
			    String strVal = String.valueOf(value2);			
			    
			    int howSimilar = 0;
			    int soFar = 0;
			    
			    if (Integer.parseInt(strCell) >= Integer.parseInt(strVal)) {
				jTable.scrollRectToVisible(jTable.getCellRect(i, j, true));
				
				jTable.updateUI();
				jTable.setRowSelectionInterval(i, i);
				jFrame.validate();
				jFrame.pack();
				return;
			    }
			}		
		    }
		}
	    }
	}
    }
	
    //Internal function to facilitate taking a timestamp value in as 20:16:30.4567
    //and returning 2016304567
    public String parseCell(String cell) {
	StringTokenizer strTok = new StringTokenizer(cell, ":", true);
	
	String hour = strTok.nextToken();
	
	strTok.nextToken();
	String min = strTok.nextToken();
	
	strTok.nextToken();
	String sec = strTok.nextToken(".");
	
	strTok.nextToken();
	String milli = strTok.nextToken();
	
	String strCell = hour + min + sec + milli;
	strCell = strCell.substring(0, 10);
	
	return strCell;
    }
    
    //Adjusts the timestamp window that is currently viewed by the user.
    public void adjustWindow() {
	jumpToRow();
	int startTime = jSlider1.getValue();
	int winSize = jSlider0.getValue();
	int row = jTable.getSelectedRow();
	int intSli = startTime;
	String strSli = String.valueOf(intSli);
	String hour = strSli.substring(0, 2);
	String min = strSli.substring(2, 4);
	String sec = strSli.substring(4, 6);
	String milli = strSli.substring(6);
		
	Calendar beg = Calendar.getInstance();
	beg.set(Calendar.HOUR_OF_DAY, Integer.parseInt(hour));
	beg.set(Calendar.MINUTE, Integer.parseInt(min));
	beg.set(Calendar.SECOND, Integer.parseInt(sec));
	beg.set(Calendar.MILLISECOND, Integer.parseInt(milli));			
		
	if ((jSlider1.getValueIsAdjusting() == false) && (jSlider0.getValueIsAdjusting() == false)) {
	    tableVec = null;
	    tableVec = new Vector(1, 1);
	    
	    //The "Size of Window" slider has certain integer values attached
	    //to the labels (e.g. "1ms" = 1, "10ms" = 2, etc.).
	    //The corresponding amount of time is added to the beginning of
	    //the window to determine the end time and thus the window size.
	    if (winSize == 1) {
		beg.add(Calendar.MILLISECOND, 1);
	    }
	    else if (winSize == 2) {
		beg.add(Calendar.MILLISECOND, 10);
		
	    }
	    else if (winSize == 3) {
		beg.add(Calendar.SECOND, 1);
	    }
	    else if (winSize == 4) {
		beg.add(Calendar.SECOND, 10);
	    }
	    else if (winSize == 5) {
		beg.add(Calendar.MINUTE, 1);
	    }
	    else if (winSize == 6) {
		tableVec = rowData;
		selectEvent(eventState);
		return;
		
	    }
	    
	    String strHour = String.valueOf(beg.get(Calendar.HOUR_OF_DAY));
	    String strMin = String.valueOf(beg.get(Calendar.MINUTE));
	    String strSec = String.valueOf(beg.get(Calendar.SECOND));
	    String strMilli = String.valueOf(beg.get(Calendar.MILLISECOND));
	    
	    if (Integer.parseInt(strHour) < 10) {
		strHour = "0" + strHour;
	    }
	    if (Integer.parseInt(strMin) < 10) {
		strMin = "0" + strMin;
	    }
	    if (Integer.parseInt(strSec) < 10) {
		strSec = "0" + strSec;
	    }
	    if (Integer.parseInt(strMilli) < 1000) {
		if (Integer.parseInt(strMilli) < 100) {
		    if (Integer.parseInt(strMilli) < 10) {
			strMilli = "000" + strMilli;
		    }
		    else {
			strMilli = "00" + strMilli;
		    }
		}
		else {
		    strMilli = "0" + strMilli;
		}
	    }
	    
	    String endTime = strHour + strMin + strSec + strMilli;
	    
	    
	    for (int i=0; i<rowData.size(); i++) {
		Vector windowVec = (Vector)rowData.elementAt(i);
		String timeStamp = (String)windowVec.elementAt(0);
		timeStamp = parseCell(timeStamp);

		if (Integer.parseInt(timeStamp) >= startTime) {
		    if (Integer.parseInt(timeStamp) <= Integer.parseInt(endTime)) {
			tableVec.add(windowVec);
		    }
		}
	    }
	    
	    jTable = new JTable(tableVec, columnNames) {
		    //Make sure the cells of the new jTable are not-editable
		    public boolean isCellEditable(int rowIndex, int colIndex) {
			return false;
		    }
		};
	    jFrame.getContentPane().remove(tablePane);

	    //If you don't call selectEvent() here, you will have to refresh the table.
	    //There is code above to do this.

	    selectEvent(eventState);
	    adjustTable();
	}
    }

    //Listens for changes in the values of the three sliders and calls the appropriate
    //function.
    class SliderListener implements ChangeListener {
	public void stateChanged(ChangeEvent e) {
	    if (e.getSource() == jSlider) {
		if (rowData != null) {
		    adjustTable();
		    jFrame.validate();
		    jFrame.pack();
		}
		else {
		    System.out.println("No Table has been added, so you can not resize it");
		    System.out.println("Add a table by going to File -> Open... and choosing a file\n");
		}
	    }
	    else if (e.getSource() == jSlider1) {
		if (tableVec != null) {
		    adjustWindow();
		    jTable.updateUI();
		    jFrame.validate();
		    jFrame.pack();
		}
		else {
		    System.out.println("No Table has been added, so you can not resize it");
		    System.out.println("Add a table by going to File -> Open... and choosing a file\n");
		}
	    }
	    else if (e.getSource() == jSlider0) {
		if (tableVec != null) {
		    adjustWindow();
		    jTable.updateUI();
		    jFrame.validate();
		    jFrame.pack();
		}
		else {
		    System.out.println("No Table has been added, so you can not resize it");
		    System.out.println("Add a table by going to File -> Open... and choosing a file\n");
		}
	    }
	    else {
		System.out.println("Nothing has happened.");
	    }
	}
    }
    
    //This class shows the value of the buffer column within the textarea.
    class TableListener implements ListSelectionListener {
	public void valueChanged(ListSelectionEvent e) {
	    ListSelectionModel lsm = (ListSelectionModel)e.getSource();
	    
	    int row = jTable.getSelectedRow();
	    Vector rowVector = new Vector(1, 0);
	    rowVector = (Vector)tableVec.elementAt(row);
	    String value = (String)rowVector.elementAt(2);
	    
	    jTextArea.setText(value);
	    jFrame.validate();
	    jFrame.pack();
	}
    }
    
    public boolean isCellEditable(int rowIndex, int colIndex) {
	return false;
    }

    //Main program.
    public static void main(String[] args) {
	if (args.length != 1) {
	    MinetTimeline timeLine = new MinetTimeline();
	    //This is a global variable when set to 0 will
	    //call initGUI() and wait for the user to load
	    //a table at a later time from the File -> Open
	    //menu.
	    fileCounter = 0;
	}
	else {
	    myFile = args[0];
	    //Calls loadTable2 to run the function that will
	    //load the table from the command line.
	    fileCounter = 1;
	    MinetTimeline timeLine = new MinetTimeline();
	}
    }   
}
