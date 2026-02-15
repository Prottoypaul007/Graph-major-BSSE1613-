#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <time.h>

#define PI 3.14159265358979323846
#define EARTH_RADIUS_KM 6371.0
#define MAX_LINE 10000

typedef enum { MODE_WALK, MODE_CAR, MODE_METRO, MODE_BUS_BIKOLPO, MODE_BUS_UTTARA } TransportMode;

typedef struct Edge { int target_id; double dist_km; TransportMode mode; struct Edge* next; } Edge;
typedef struct Node { int id; double lon; double lat; Edge* head_edge; } Node;
typedef struct Graph { Node* nodes; int total_nodes; } Graph;
typedef struct State { double weight; double current_time; double total_cost; } State;
typedef struct Coord { double lon; double lat; } Coord;

Coord* raw_coords; int raw_count = 0;

double haversine(double lon1, double lat1, double lon2, double lat2) {
    double dLat = (lat2 - lat1) * PI / 180.0, dLon = (lon2 - lon1) * PI / 180.0;
    double a = sin(dLat/2)*sin(dLat/2) + cos(lat1*PI/180.0)*cos(lat2*PI/180.0)*sin(dLon/2)*sin(dLon/2);
    return EARTH_RADIUS_KM * (2 * atan2(sqrt(a), sqrt(1 - a)));
}

int compare_coords(const void* a, const void* b) {
    Coord* c1 = (Coord*)a; Coord* c2 = (Coord*)b;
    if (c1->lon != c2->lon) return (c1->lon > c2->lon) - (c1->lon < c2->lon);
    return (c1->lat > c2->lat) - (c1->lat < c2->lat);
}

int find_node(Graph* g, double lon, double lat) {
    int low = 0, high = g->total_nodes - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (g->nodes[mid].lon == lon && g->nodes[mid].lat == lat) return mid;
        if (g->nodes[mid].lon < lon || (g->nodes[mid].lon == lon && g->nodes[mid].lat < lat)) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

void add_edge(Graph* g, int src, int tgt, TransportMode mode) {
    if (src == tgt) return;
    double dist = haversine(g->nodes[src].lon, g->nodes[src].lat, g->nodes[tgt].lon, g->nodes[tgt].lat);
    Edge* e = malloc(sizeof(Edge)); e->target_id = tgt; e->dist_km = dist; e->mode = mode; e->next = g->nodes[src].head_edge; g->nodes[src].head_edge = e;
    if (mode == MODE_CAR) {
        Edge* rev = malloc(sizeof(Edge)); rev->target_id = src; rev->dist_km = dist; rev->mode = mode; rev->next = g->nodes[tgt].head_edge; g->nodes[tgt].head_edge = rev;
    }
}

void parse_csv(const char* filename, Graph* g, int is_pass2, TransportMode mode) {
    FILE* file = fopen(filename, "r");
    if (!file) return;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), file)) {
        char* token = strtok(line, ",\r\n"); int prev_id = -1;
        while (token != NULL) {
            double lon = atof(token);
            if (lon > 88.0 && lon < 93.0) { 
                token = strtok(NULL, ",\r\n"); if (!token) break;
                double lat = atof(token);
                if (lat > 20.0 && lat < 27.0) { 
                    if (!is_pass2) { raw_coords[raw_count++] = (Coord){lon, lat}; } 
                    else {
                        int curr_id = find_node(g, lon, lat);
                        if (curr_id != -1 && prev_id != -1 && curr_id != prev_id) add_edge(g, prev_id, curr_id, mode);
                        prev_id = curr_id;
                    }
                }
            }
            token = strtok(NULL, ",\r\n");
        }
    }
    fclose(file);
}

void build_graph(Graph* g) {
    raw_coords = malloc(500000 * sizeof(Coord)); raw_count = 0;
    parse_csv("Roadmap-Dhaka.csv", g, 0, MODE_CAR); 
    parse_csv("Routemap-DhakaMetroRail.csv", g, 0, MODE_METRO);
    parse_csv("Routemap-BikolpoBus.csv", g, 0, MODE_BUS_BIKOLPO); 
    parse_csv("Routemap-UttaraBus.csv", g, 0, MODE_BUS_UTTARA);
    
    qsort(raw_coords, raw_count, sizeof(Coord), compare_coords);
    g->nodes = malloc(raw_count * sizeof(Node)); g->total_nodes = 0;
    
    for (int i = 0; i < raw_count; i++) {
        if (i == 0 || raw_coords[i].lon != raw_coords[i-1].lon || raw_coords[i].lat != raw_coords[i-1].lat) {
            g->nodes[g->total_nodes].id = g->total_nodes; g->nodes[g->total_nodes].lon = raw_coords[i].lon; g->nodes[g->total_nodes].lat = raw_coords[i].lat;
            g->nodes[g->total_nodes].head_edge = NULL; g->total_nodes++;
        }
    }
    
    parse_csv("Roadmap-Dhaka.csv", g, 1, MODE_CAR); 
    parse_csv("Routemap-DhakaMetroRail.csv", g, 1, MODE_METRO);
    parse_csv("Routemap-BikolpoBus.csv", g, 1, MODE_BUS_BIKOLPO); 
    parse_csv("Routemap-UttaraBus.csv", g, 1, MODE_BUS_UTTARA);
    free(raw_coords);
}

int get_nearest(Graph* g, double lon, double lat) {
    double min_d = DBL_MAX; int nearest = 0;
    for (int i = 0; i < g->total_nodes; i++) {
        double d = haversine(lon, lat, g->nodes[i].lon, g->nodes[i].lat);
        if (d < min_d) { min_d = d; nearest = i; }
    }
    return nearest;
}

int extract_min(State* states, bool* visited, int V) {
    double min_w = DBL_MAX; int min_idx = -1;
    for (int i=0; i<V; i++) if (!visited[i] && states[i].weight <= min_w) { min_w = states[i].weight; min_idx = i; }
    return min_idx;
}

void print_time(double mins) {
    int m = (int)mins;
    int h = (m / 60) % 24;
    int min = m % 60;
    char ampm[] = "AM";
    if (h >= 12) { ampm[0] = 'P'; if (h > 12) h -= 12; }
    if (h == 0) h = 12;
    printf("%d:%02d %s", h, min, ampm);
}

void solve_dynamic(Graph* g, int prob_id, double s_lon, double s_lat, double d_lon, double d_lat, double start_t, double deadline) {
    int start_node = get_nearest(g, s_lon, s_lat);
    int target_node = get_nearest(g, d_lon, d_lat);
    int V = g->total_nodes;
    
    State* states = malloc(V * sizeof(State)); 
    bool* visited = calloc(V, sizeof(bool)); 
    int* parent = malloc(V * sizeof(int));
    int* parent_mode = malloc(V * sizeof(int));
    
    for(int i=0; i<V; i++) { states[i].weight = DBL_MAX; parent[i] = -1; parent_mode[i] = MODE_WALK; }
    states[start_node].weight = 0; states[start_node].current_time = start_t; states[start_node].total_cost = 0;

    for (int count = 0; count < V - 1; count++) {
        int u = extract_min(states, visited, V);
        if (u == -1 || u == target_node) break;
        visited[u] = true;

        for (Edge* e = g->nodes[u].head_edge; e != NULL; e = e->next) {
            if (prob_id == 1 && e->mode != MODE_CAR) continue;
            if (prob_id == 2 && (e->mode != MODE_CAR && e->mode != MODE_METRO)) continue;

            double t = states[u].current_time, wait = 0, speed = (prob_id == 5) ? 10.0 : 30.0, cost = 0;
            
            if (e->mode == MODE_CAR) { 
                cost = e->dist_km * 20.0; if(prob_id==6) speed=20; 
            } else if (e->mode == MODE_METRO) { 
                cost = e->dist_km * 5.0; 
                if(prob_id==6){ speed=15; if(t<60 || t>1380) continue; wait = 5.0 - fmod(t,5); } 
                else { if(t<360||t>1380) continue; wait=15-fmod(t,15); } 
            } else if (e->mode == MODE_BUS_BIKOLPO || e->mode == MODE_BUS_UTTARA) {
                cost = (prob_id==6 && e->mode==MODE_BUS_UTTARA) ? e->dist_km * 10.0 : e->dist_km * 7.0;
                if(prob_id==6) {
                    if (e->mode == MODE_BUS_BIKOLPO) { speed=10; if(t<420||t>1320) continue; wait=20-fmod(t,20); }
                    else { speed=12; if(t<360||t>1380) continue; wait=10-fmod(t,10); }
                } else { 
                    if(t<360||t>1380) continue; wait=15-fmod(t,15); 
                }
            }
            
            double arr_t = t + wait + ((e->dist_km / speed) * 60.0);
            if (prob_id == 6 && arr_t > deadline) continue; 

            double new_weight = (prob_id == 1) ? states[u].weight + e->dist_km : ((prob_id == 5) ? arr_t : states[u].total_cost + cost);
            
            if (!visited[e->target_id] && new_weight < states[e->target_id].weight) {
                states[e->target_id].weight = new_weight; 
                states[e->target_id].current_time = arr_t; 
                states[e->target_id].total_cost = states[u].total_cost + cost;
                parent[e->target_id] = u;
                parent_mode[e->target_id] = e->mode;
            }
        }
    }

    if (states[target_node].weight == DBL_MAX) {
        printf("ERROR: No valid route found.\n");
    } else {
        int* path_rev = malloc(V * sizeof(int));
        int path_len = 0;
        int curr = target_node;
        while (curr != -1) { path_rev[path_len++] = curr; curr = parent[curr]; }
        
        int* path = malloc(path_len * sizeof(int));
        for(int i = 0; i < path_len; i++) path[i] = path_rev[path_len - 1 - i];

        printf("Problem No: %d\n", prob_id);
        printf("Source: (%f, %f)\n", s_lon, s_lat);
        printf("Destination: (%f, %f)\n", d_lon, d_lat);
        printf("Starting time at source: "); print_time(start_t); printf("\n");
        if (prob_id >= 4) { printf("Destination reaching time: "); print_time(states[target_node].current_time); printf("\n"); }
        printf("\n");

        int start_idx = 0;
        for (int i = 1; i < path_len; i++) {
            if (i == path_len - 1 || parent_mode[path[i]] != parent_mode[path[i+1]]) {
                
                double seg_cost = states[path[i]].total_cost - states[path[start_idx]].total_cost;
                printf("\t"); print_time(states[path[start_idx]].current_time); printf(" - ");
                print_time(states[path[i]].current_time);
                printf(", Cost: \u09F3%.2f: ", seg_cost);

                int m = parent_mode[path[i]];
                const char* m_str = "Walk";
                if(m == MODE_CAR) m_str = "Ride Car";
                else if(m == MODE_METRO) m_str = "Ride Metro";
                else if(m == MODE_BUS_BIKOLPO) m_str = "Ride Bikolpo Bus";
                else if(m == MODE_BUS_UTTARA) m_str = "Ride Uttara Bus";

                printf("%s from (%f, %f) to (%f, %f).\n", m_str,
                       g->nodes[path[start_idx]].lon, g->nodes[path[start_idx]].lat,
                       g->nodes[path[i]].lon, g->nodes[path[i]].lat);

                start_idx = i;
            }
        }

        // --- GENERATE UNIQUE KML FILENAME ---
        char kml_filename[100];
        sprintf(kml_filename, "route_prob%d_%ld.kml", prob_id, (long)time(NULL));

        // Tell Java what the generated filename is
        printf("\nSUCCESS_KML_READY:%s\n", kml_filename);

        FILE* f = fopen(kml_filename, "w");
        fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        fprintf(f, "<kml xmlns=\"http://earth.google.com/kml/2.1\">\n<Document>\n");
        
        fprintf(f, "<Style id=\"walk\"><LineStyle><color>ff000000</color><width>5</width></LineStyle></Style>\n");       
        fprintf(f, "<Style id=\"car\"><LineStyle><color>ffff0000</color><width>5</width></LineStyle></Style>\n");        
        fprintf(f, "<Style id=\"metro\"><LineStyle><color>ff0000ff</color><width>5</width></LineStyle></Style>\n");      
        fprintf(f, "<Style id=\"bus_bikolpo\"><LineStyle><color>ff00aa00</color><width>5</width></LineStyle></Style>\n"); 
        fprintf(f, "<Style id=\"bus_uttara\"><LineStyle><color>ff00ffff</color><width>5</width></LineStyle></Style>\n");  

        if (path_len > 1) {
            int current_mode = parent_mode[path[1]];
            int segment_start = 0;

            for (int i = 1; i < path_len; i++) {
                if (i == path_len - 1 || parent_mode[path[i+1]] != current_mode) {
                    
                    const char* style_id = "walk";
                    const char* name = "Walk";
                    if(current_mode == MODE_CAR) { style_id = "car"; name = "Car Ride"; }
                    else if(current_mode == MODE_METRO) { style_id = "metro"; name = "Metro Rail"; }
                    else if(current_mode == MODE_BUS_BIKOLPO) { style_id = "bus_bikolpo"; name = "Bikolpo Bus"; }
                    else if(current_mode == MODE_BUS_UTTARA) { style_id = "bus_uttara"; name = "Uttara Bus"; }

                    fprintf(f, "<Placemark>\n<name>%s</name>\n<styleUrl>#%s</styleUrl>\n", name, style_id);
                    fprintf(f, "<LineString>\n<tessellate>1</tessellate>\n<coordinates>\n");

                    for (int j = segment_start; j <= i; j++) {
                        fprintf(f, "%f,%f,0\n", g->nodes[path[j]].lon, g->nodes[path[j]].lat);
                    }

                    fprintf(f, "</coordinates>\n</LineString>\n</Placemark>\n");
                    
                    segment_start = i; 
                    if (i < path_len - 1) current_mode = parent_mode[path[i+1]];
                }
            }
        }
        fprintf(f, "</Document>\n</kml>\n");
        fclose(f);

        free(path); free(path_rev);
    }
    free(states); free(visited); free(parent); free(parent_mode);
}

int main(int argc, char* argv[]) {
    if (argc < 6) return 1;
    int prob = atoi(argv[1]);
    double slon = atof(argv[2]), slat = atof(argv[3]), dlon = atof(argv[4]), dlat = atof(argv[5]);
    double start_t = (argc>6) ? atof(argv[6]) : (18*60 + 45); // Default 6:45 PM
    double dead_t = (argc>7) ? atof(argv[7]) : (20*60 + 40);  // Default 8:40 PM

    Graph g; build_graph(&g);
    solve_dynamic(&g, prob, slon, slat, dlon, dlat, start_t, dead_t);
    return 0;
}