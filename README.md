# Mr. Efficient Routing System 🗺️🚗🚈

A multimodal, time-dependent routing engine built for a Software Project Lab (SPL). This project calculates the optimal path across Dhaka city using personal cars, public buses, and the Metro Rail. 

The system utilizes a high-performance **C backend** to crunch Dijkstra's algorithm over thousands of intersections, and a sleek **Java Swing frontend** to capture user input, display turn-by-turn textual directions, and dynamically generate a color-coded KML file for Google Maps visualization.

## ✨ Features
* **6 Dynamic Routing Problems:**
  1. **Shortest Distance:** Car-only route minimizing physical distance.
  2. **Cheapest Cost (Car/Metro):** Minimizes cost using only cars and metro.
  3. **Cheapest Cost (All Modes):** Minimizes cost using cars, metro, and local buses.
  4. **Cost with Wait Times:** Considers real-world transit schedules to calculate accurate wait times.
  5. **Fastest Time:** Minimizes total travel time, dynamically waiting for transit if it's faster than driving.
  6. **Hard Deadline Pruning:** Finds the cheapest route that guarantees arrival before a strict deadline.
* **Time-Dependent Dijkstra:** Dynamically calculates waiting times based on real-world transit schedules (e.g., Metro runs every 5 minutes, Bikolpo Bus every 20 minutes).
* **Location Snapping:** Uses the Haversine formula to snap random off-road GPS coordinates to the nearest valid road network node (simulating walk time to the road).
* **Segmented KML Generation:** Automatically generates a color-coded `route.kml` file where different transport modes are visually distinct on the map (e.g., Red for Metro, Blue for Car).

## 🛠️ Tech Stack
* **Backend Engine:** `C` (Graph Theory, Min-Heap Priority Queue, File I/O, Subprocess standard output)
* **Frontend UI:** `Java` (Swing Desktop Application, Multithreading, ProcessBuilder)
* **Visualization:** Google MyMaps (KML Data Standard)

## 📂 File Structure
Ensure all these files are in the same root directory before running the project:
* `router.c` (Native C Backend Engine)
* `RouterUI.java` (Java Desktop Interface)
* `Roadmap-Dhaka.csv` (Dhaka street network dataset)
* `Routemap-DhakaMetroRail.csv` (Metro transit dataset)
* `Routemap-BikolpoBus.csv` (Bikolpo bus transit dataset)
* `Routemap-UttaraBus.csv` (Uttara bus transit dataset)

---

## 🚀 How to Run the Project

### Prerequisites
1. **C Compiler (`gcc`):** * **Windows:** Install MinGW and ensure `gcc` is added to your System Environment PATH.
   * **Mac/Linux:** Usually pre-installed (or install via `xcode-select --install` / `sudo apt install build-essential`).
2. **Java Development Kit (JDK):** Ensure the `javac` and `java` commands are available in your terminal.

### Step 1: Compile the Code
Open your terminal/command prompt, navigate to your project folder, and compile both the C engine and the Java UI.

**1. Compile the C Engine:**
*(Note: The `-lm` flag is strictly required to link the math library for the Haversine formula)*
```bash
gcc router.c -o router.exe -lm

**2. Compile the java frontend:**
javac RouterUI.java

**3. Run the application
java RouterUI.java
