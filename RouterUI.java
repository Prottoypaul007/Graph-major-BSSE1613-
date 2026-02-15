import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.*;
import java.net.URI;

public class RouterUI extends JFrame {

    private JComboBox<String> problemSelect;
    private JTextField srcLatField, srcLonField, destLatField, destLonField;
    private JTextArea consoleArea;
    private JButton calculateBtn;

    public RouterUI() {
        setTitle("Mr. Efficient Routing System - SPL Project");
        setSize(750, 650); 
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLayout(new BorderLayout());

        JPanel inputPanel = new JPanel(new GridLayout(6, 2, 10, 10));
        inputPanel.setBorder(BorderFactory.createEmptyBorder(15, 15, 15, 15));

        inputPanel.add(new JLabel("Select Routing Problem:"));
        String[] problems = {
                "1. Shortest Distance (Car Only)", "2. Cheapest Cost (Car/Metro)",
                "3. Cheapest Cost (All Modes)", "4. Cost with Wait Times",
                "5. Fastest Time with Schedules", "6. Hard Deadline Route"
        };
        problemSelect = new JComboBox<>(problems);
        inputPanel.add(problemSelect);

        inputPanel.add(new JLabel("Source Lat (A):"));
        srcLatField = new JTextField("23.855136");
        inputPanel.add(srcLatField);

        inputPanel.add(new JLabel("Source Lon (A):"));
        srcLonField = new JTextField("90.404772");
        inputPanel.add(srcLonField);

        inputPanel.add(new JLabel("Destination Lat (B):"));
        destLatField = new JTextField("23.757944");
        inputPanel.add(destLatField);

        inputPanel.add(new JLabel("Destination Lon (B):"));
        destLonField = new JTextField("90.439679");
        inputPanel.add(destLonField);

        calculateBtn = new JButton("Calculate Route & Open MyMaps");
        calculateBtn.setBackground(new Color(46, 204, 113));
        calculateBtn.setForeground(Color.WHITE);
        calculateBtn.setFont(new Font("Arial", Font.BOLD, 14));
        inputPanel.add(new JLabel("")); 
        inputPanel.add(calculateBtn);

        add(inputPanel, BorderLayout.NORTH);

        consoleArea = new JTextArea();
        consoleArea.setEditable(false);
        consoleArea.setBackground(new Color(245, 245, 245));
        consoleArea.setForeground(Color.BLACK);
        consoleArea.setFont(new Font("Monospaced", Font.PLAIN, 13));
        consoleArea.setMargin(new Insets(10, 10, 10, 10));
        JScrollPane scrollPane = new JScrollPane(consoleArea);
        scrollPane.setBorder(BorderFactory.createTitledBorder("Assignment Formatted Output"));
        add(scrollPane, BorderLayout.CENTER);

        calculateBtn.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(ActionEvent e) {
                runRoutingEngine();
            }
        });
    }

    private void runRoutingEngine() {
        consoleArea.setText("Starting C Backend Engine...\nProcessing map nodes (Please Wait)...\n\n");
        int probId = problemSelect.getSelectedIndex() + 1;
        String sLat = srcLatField.getText();
        String sLon = srcLonField.getText();
        String dLat = destLatField.getText();
        String dLon = destLonField.getText();

        new Thread(() -> {
            try {
                String execName = System.getProperty("os.name").toLowerCase().contains("win") ? "router.exe" : "./router";
                ProcessBuilder pb = new ProcessBuilder(execName, String.valueOf(probId), sLon, sLat, dLon, dLat);
                Process process = pb.start();

                BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream(), "UTF-8"));
                StringBuilder output = new StringBuilder();
                String line;
                boolean success = false;
                String generatedKmlFile = "route.kml"; // Default fallback
                
                while ((line = reader.readLine()) != null) {
                    if (line.startsWith("SUCCESS_KML_READY")) {
                        success = true;
                        // Extract the dynamic filename from the C output
                        if (line.contains(":")) {
                            generatedKmlFile = line.split(":")[1].trim();
                        }
                    } else {
                        output.append(line).append("\n");
                    }
                }
                process.waitFor();
                
                consoleArea.setText(output.toString());

                if (success) {
                    consoleArea.append("\n-------------------------------------------------\n");
                    consoleArea.append(">> Unique file '" + generatedKmlFile + "' successfully generated!\n");
                    consoleArea.append(">> Opening Google MyMaps...\n");
                    
                    Desktop.getDesktop().browse(new URI("https://www.google.com/mymaps"));

                    JOptionPane.showMessageDialog(null, 
                        "Advanced Route Generated Successfully!\n\n" +
                        "1. Google MyMaps is opening.\n" +
                        "2. Click '+ CREATE A NEW MAP'.\n" +
                        "3. Click 'Import' on the left panel.\n" +
                        "4. Find and upload the new file: \n   " + generatedKmlFile + "\n\nEnjoy your color-coded path!", 
                        "Upload Instructions", JOptionPane.INFORMATION_MESSAGE);
                }

            } catch (Exception ex) {
                consoleArea.append("\nSYSTEM ERROR: " + ex.getMessage() + "\nDid you compile router.c into router.exe?");
            }
        }).start();
    }

    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            new RouterUI().setVisible(true);
        });
    }
}