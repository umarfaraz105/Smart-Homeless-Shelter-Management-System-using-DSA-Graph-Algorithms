#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <ctime>
#include <climits>
#include <iomanip>
#include <sstream>

using namespace std;

// ==================== COLOR CODES FOR CONSOLE ====================
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

// ==================== DATA STRUCTURES ====================

struct Homeless {
    int id;
    string name;
    int age;
    string gender;
    int locationNodeID;
    bool medicalNeed;
    int priorityScore;
    string complaint;
    time_t reportedAt;
    bool allocated;
    int allocatedShelterID;
};

struct Shelter {
    int id;
    string name;
    int nodeID;
    int capacityTotal;
    int capacityOccupied;
    string contactNumber;
    vector<int> allocatedPersonIDs;
};

struct Station {
    int id;
    string name;
    int nodeID;
};

struct EmergencyCase {
    int homelessID;
    int priority;
    time_t timeReported;
    
    bool operator<(const EmergencyCase& other) const {
        return priority < other.priority; // Max heap
    }
};

struct Edge {
    int dest;
    int weight;
};

struct Report {
    string date;
    int totalRegistered;
    int totalAllocated;
    int emergenciesHandled;
    int sheltersAtCapacity;
};

// ==================== GLOBAL DATA ====================

vector<vector<Edge>> graph;
unordered_map<int, Homeless> homelessRecords;
vector<Homeless> homelessList;
vector<Shelter> shelters;
vector<Station> stations;
priority_queue<EmergencyCase> emergencyHeap;
vector<Report> dailyReports;
int nodeCount = 0;
int nextHomelessID = 106;

// ==================== UTILITY FUNCTIONS ====================

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void pressEnterToContinue() {
    cout << "\n" << CYAN << "Press Enter to continue..." << RESET;
    cin.ignore();
    cin.get();
}

void printHeader(const string& title) {
    cout << "\n" << BOLD << CYAN;
    cout << "╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║" << string(62, ' ') << "║\n";
    cout << "║";
    int padding = (62 - title.length()) / 2;
    cout << string(padding, ' ') << title << string(62 - padding - title.length(), ' ') << "║\n";
    cout << "║" << string(62, ' ') << "║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n";
    cout << RESET;
}

void printSubHeader(const string& title) {
    cout << "\n" << BOLD << YELLOW << "═══ " << title << " ═══" << RESET << "\n\n";
}

void printSuccess(const string& msg) {
    cout << GREEN << "✓ " << msg << RESET << "\n";
}

void printError(const string& msg) {
    cout << RED << "✗ " << msg << RESET << "\n";
}

void printWarning(const string& msg) {
    cout << YELLOW << "⚠ " << msg << RESET << "\n";
}

void printInfo(const string& msg) {
    cout << BLUE << "ℹ " << msg << RESET << "\n";
}

// ==================== ALGORITHM IMPLEMENTATIONS ====================

// 1️⃣ DIJKSTRA'S ALGORITHM - Shortest Path
// Time Complexity: O((V+E) log V)
// Space Complexity: O(V)
vector<int> dijkstra(int source) {
    vector<int> dist(nodeCount, INT_MAX);
    vector<bool> visited(nodeCount, false);
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    
    dist[source] = 0;
    pq.push({0, source});
    
    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();
        
        if (visited[u]) continue;
        visited[u] = true;
        
        for (const Edge& e : graph[u]) {
            int v = e.dest;
            int weight = e.weight;
            if (dist[u] != INT_MAX && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                pq.push({dist[v], v});
            }
        }
    }
    
    return dist;
}

// 2️⃣ BREADTH-FIRST SEARCH (BFS)
// Time Complexity: O(V + E)
// Space Complexity: O(V)
void traverseNearbyAreas(int stationNodeID) {
    printSubHeader("BFS: Nearby Area Traversal");
    
    if (stationNodeID < 0 || stationNodeID >= nodeCount) {
        printError("Invalid node ID!");
        return;
    }
    
    vector<bool> visited(nodeCount, false);
    queue<int> q;
    vector<int> traversalOrder;
    
    q.push(stationNodeID);
    visited[stationNodeID] = true;
    
    while (!q.empty()) {
        int node = q.front();
        q.pop();
        traversalOrder.push_back(node);
        
        for (const Edge& e : graph[node]) {
            if (!visited[e.dest]) {
                visited[e.dest] = true;
                q.push(e.dest);
            }
        }
    }
    
    cout << "Starting Node: " << stationNodeID << "\n";
    cout << "Traversal Order: ";
    for (int i = 0; i < traversalOrder.size(); i++) {
        cout << traversalOrder[i];
        if (i < traversalOrder.size() - 1) cout << " → ";
    }
    cout << "\n";
    printSuccess("BFS traversal completed");
}

// 3️⃣ DEPTH-FIRST SEARCH (DFS)
// Time Complexity: O(V + E)
// Space Complexity: O(V)
void dfsUtil(int node, vector<bool>& visited, vector<int>& dfsOrder) {
    visited[node] = true;
    dfsOrder.push_back(node);
    
    for (const Edge& e : graph[node]) {
        if (!visited[e.dest]) {
            dfsUtil(e.dest, visited, dfsOrder);
        }
    }
}

bool checkShelterConnectivity() {
    printSubHeader("DFS: Shelter Network Connectivity Check");
    
    if (shelters.empty()) {
        printWarning("No shelters registered in the system");
        return true;
    }
    
    vector<bool> visited(nodeCount, false);
    vector<int> dfsOrder;
    dfsUtil(shelters[0].nodeID, visited, dfsOrder);
    
    cout << "DFS Traversal from Shelter '" << shelters[0].name << "':\n";
    for (int i = 0; i < dfsOrder.size(); i++) {
        cout << dfsOrder[i];
        if (i < dfsOrder.size() - 1) cout << " → ";
    }
    cout << "\n\n";
    
    bool allConnected = true;
    cout << left << setw(25) << "Shelter Name" << setw(15) << "Node ID" << setw(15) << "Status\n";
    cout << string(55, '-') << "\n";
    
    for (const Shelter& s : shelters) {
        string status = visited[s.nodeID] ? GREEN "Connected" RESET : RED "UNREACHABLE" RESET;
        cout << left << setw(25) << s.name << setw(15) << s.nodeID << setw(15) << status << "\n";
        if (!visited[s.nodeID]) allConnected = false;
    }
    
    if (allConnected) {
        printSuccess("All shelters are connected in the network");
    } else {
        printError("Some shelters are unreachable - network has issues!");
    }
    
    return allConnected;
}

// 4️⃣ HASHING - Fast Insert/Search
// Time Complexity: O(1) average case
// Space Complexity: O(n)
bool addHomelessRecord(Homeless h) {
    if (homelessRecords.find(h.id) != homelessRecords.end()) {
        printError("Duplicate ID detected! Record already exists.");
        return false;
    }
    
    if (h.locationNodeID < 0 || h.locationNodeID >= nodeCount) {
        printError("Invalid location node ID!");
        return false;
    }
    
    h.allocated = false;
    h.allocatedShelterID = -1;
    h.reportedAt = time(0);
    h.priorityScore = 0; // Will be calculated
    
    homelessRecords[h.id] = h;
    homelessList.push_back(h);
    printSuccess("Record added successfully: " + h.name + " (ID: " + to_string(h.id) + ")");
    return true;
}

Homeless* searchHomeless(int id) {
    auto it = homelessRecords.find(id);
    if (it != homelessRecords.end()) {
        return &(it->second);
    }
    return nullptr;
}

bool isDuplicate(int id) {
    return homelessRecords.find(id) != homelessRecords.end();
}

// 5️⃣ BINARY SEARCH
// Time Complexity: O(log n)
// Space Complexity: O(1)
int binarySearchRecord(int id) {
    vector<int> sortedIDs;
    for (const auto& pair : homelessRecords) {
        sortedIDs.push_back(pair.first);
    }
    sort(sortedIDs.begin(), sortedIDs.end());
    
    int left = 0, right = sortedIDs.size() - 1;
    int comparisons = 0;
    
    while (left <= right) {
        comparisons++;
        int mid = left + (right - left) / 2;
        if (sortedIDs[mid] == id) {
            cout << "Binary Search: Found in " << comparisons << " comparisons\n";
            return mid;
        } else if (sortedIDs[mid] < id) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    cout << "Binary Search: Not found after " << comparisons << " comparisons\n";
    return -1;
}

// 6️⃣ MERGE SORT - Sort Shelters by Capacity
// Time Complexity: O(n log n)
// Space Complexity: O(n)
void merge(vector<Shelter>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    vector<Shelter> L(n1), R(n2);
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int i = 0; i < n2; i++) R[i] = arr[mid + 1 + i];
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        int availL = L[i].capacityTotal - L[i].capacityOccupied;
        int availR = R[j].capacityTotal - R[j].capacityOccupied;
        
        if (availL >= availR) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }
    
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
}

void mergeSort(vector<Shelter>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

void sortSheltersByCapacity() {
    printSubHeader("Merge Sort: Sorting Shelters by Available Capacity");
    
    if (shelters.empty()) {
        printWarning("No shelters to sort");
        return;
    }
    
    mergeSort(shelters, 0, shelters.size() - 1);
    
    cout << left << setw(5) << "Rank" << setw(25) << "Shelter Name" << setw(12) << "Total" 
         << setw(12) << "Occupied" << setw(12) << "Available" << setw(12) << "Status\n";
    cout << string(78, '-') << "\n";
    
    for (int i = 0; i < shelters.size(); i++) {
        const Shelter& s = shelters[i];
        int available = s.capacityTotal - s.capacityOccupied;
        double utilization = (s.capacityOccupied * 100.0) / s.capacityTotal;
        
        string status;
        if (utilization >= 90) status = RED "CRITICAL" RESET;
        else if (utilization >= 75) status = YELLOW "HIGH" RESET;
        else status = GREEN "NORMAL" RESET;
        
        cout << left << setw(5) << (i+1) << setw(25) << s.name 
             << setw(12) << s.capacityTotal << setw(12) << s.capacityOccupied 
             << setw(12) << available << setw(12) << status << "\n";
    }
    printSuccess("Shelters sorted by available capacity");
}

// 7️⃣ MAX HEAP - Emergency Case Prioritization
// Time Complexity: O(log n) for insertion, O(1) for peek, O(log n) for extraction
void addEmergencyCase(EmergencyCase e) {
    emergencyHeap.push(e);
    printSuccess("Emergency case added with priority: " + to_string(e.priority));
}

EmergencyCase getNextEmergency() {
    if (emergencyHeap.empty()) {
        return {-1, -1, 0};
    }
    EmergencyCase top = emergencyHeap.top();
    emergencyHeap.pop();
    return top;
}

// 8️⃣ RABIN-KARP - Keyword Search in Complaints
// Time Complexity: O(n + m) average case
// Space Complexity: O(1)
bool rabinKarpSearch(const string& text, const string& pattern) {
    int n = text.length();
    int m = pattern.length();
    
    if (m > n || m == 0) return false;
    
    const int d = 256;
    const int q = 101;
    
    int p = 0;
    int t = 0;
    int h = 1;
    
    for (int i = 0; i < m - 1; i++) {
        h = (h * d) % q;
    }
    
    for (int i = 0; i < m; i++) {
        p = (d * p + pattern[i]) % q;
        t = (d * t + text[i]) % q;
    }
    
    for (int i = 0; i <= n - m; i++) {
        if (p == t) {
            bool match = true;
            for (int j = 0; j < m; j++) {
                if (text[i + j] != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return true;
        }
        
        if (i < n - m) {
            t = (d * (t - text[i] * h) + text[i + m]) % q;
            if (t < 0) t += q;
        }
    }
    
    return false;
}

// 🔟 PRIORITY CALCULATION with detailed scoring
// Time Complexity: O(1)
int calculatePriority(Homeless& h) {
    int priority = 0;
    
    // Age-based priority
    if (h.age < 12) priority += 50; // Child
    else if (h.age > 65) priority += 40; // Elderly
    else if (h.age > 55) priority += 20; // Senior
    
    // Gender-based priority
    if (h.gender == "Female" || h.gender == "female") priority += 30;
    
    // Medical emergency
    if (h.medicalNeed) priority += 60;
    
    // Complaint analysis using Rabin-Karp
    string complaint = h.complaint;
    transform(complaint.begin(), complaint.end(), complaint.begin(), ::tolower);
    
    if (rabinKarpSearch(complaint, "emergency")) priority += 70;
    if (rabinKarpSearch(complaint, "critical")) priority += 60;
    if (rabinKarpSearch(complaint, "medical")) priority += 50;
    if (rabinKarpSearch(complaint, "child")) priority += 40;
    if (rabinKarpSearch(complaint, "urgent")) priority += 45;
    if (rabinKarpSearch(complaint, "danger")) priority += 55;
    
    h.priorityScore = priority;
    return priority;
}

// SHELTER ALLOCATION SYSTEM using Dijkstra
void allocateShelter(int homelessID) {
    printSubHeader("Dijkstra: Shelter Allocation System");
    
    Homeless* h = searchHomeless(homelessID);
    if (!h) {
        printError("Homeless person with ID " + to_string(homelessID) + " not found");
        return;
    }
    
    if (h->allocated) {
        printWarning("Person already allocated to a shelter");
        for (const Shelter& s : shelters) {
            if (s.id == h->allocatedShelterID) {
                cout << "Current Shelter: " << s.name << "\n";
                break;
            }
        }
        return;
    }
    
    cout << "\nPerson Details:\n";
    cout << "  Name: " << h->name << "\n";
    cout << "  Location: Node " << h->locationNodeID << "\n";
    cout << "  Priority Score: " << h->priorityScore << "\n\n";
    
    cout << "Running Dijkstra's algorithm...\n";
    vector<int> distances = dijkstra(h->locationNodeID);
    
    int bestShelter = -1;
    int minDist = INT_MAX;
    
    cout << "\nEvaluating shelters:\n";
    cout << left << setw(25) << "Shelter" << setw(12) << "Distance" << setw(12) << "Available" << setw(15) << "Status\n";
    cout << string(64, '-') << "\n";
    
    for (const Shelter& s : shelters) {
        int available = s.capacityTotal - s.capacityOccupied;
        string status;
        
        if (available > 0 && distances[s.nodeID] != INT_MAX) {
            if (distances[s.nodeID] < minDist) {
                minDist = distances[s.nodeID];
                bestShelter = s.id;
                status = GREEN "SELECTED" RESET;
            } else {
                status = "Available";
            }
        } else if (available == 0) {
            status = RED "FULL" RESET;
        } else {
            status = RED "UNREACHABLE" RESET;
        }
        
        cout << left << setw(25) << s.name << setw(12) 
             << (distances[s.nodeID] == INT_MAX ? "∞" : to_string(distances[s.nodeID])) 
             << setw(12) << available << setw(15) << status << "\n";
    }
    
    if (bestShelter == -1) {
        printError("No available shelter found!");
        return;
    }
    
    // Update records
    for (Shelter& s : shelters) {
        if (s.id == bestShelter) {
            s.capacityOccupied++;
            s.allocatedPersonIDs.push_back(homelessID);
            h->allocated = true;
            h->allocatedShelterID = bestShelter;
            
            cout << "\n";
            printSuccess("Allocation Successful!");
            cout << GREEN << "  → Shelter: " << s.name << "\n";
            cout << "  → Distance: " << minDist << " units\n";
            cout << "  → Contact: " << s.contactNumber << RESET << "\n";
            break;
        }
    }
    
    // Update in hash table
    homelessRecords[homelessID] = *h;
}

// COMPLAINT CLASSIFICATION
string classifyComplaint(const string& complaint) {
    string lower = complaint;
    transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    
    vector<string> categories;
    
    if (rabinKarpSearch(lower, "food") || rabinKarpSearch(lower, "hungry") || 
        rabinKarpSearch(lower, "meal") || rabinKarpSearch(lower, "eat")) {
        categories.push_back("Food");
    }
    
    if (rabinKarpSearch(lower, "medical") || rabinKarpSearch(lower, "sick") || 
        rabinKarpSearch(lower, "medicine") || rabinKarpSearch(lower, "health") ||
        rabinKarpSearch(lower, "doctor")) {
        categories.push_back("Medical");
    }
    
    if (rabinKarpSearch(lower, "safe") || rabinKarpSearch(lower, "danger") || 
        rabinKarpSearch(lower, "threat") || rabinKarpSearch(lower, "attack")) {
        categories.push_back("Safety");
    }
    
    if (rabinKarpSearch(lower, "shelter") || rabinKarpSearch(lower, "bed") || 
        rabinKarpSearch(lower, "sleep") || rabinKarpSearch(lower, "stay")) {
        categories.push_back("Shelter");
    }
    
    if (categories.empty()) {
        return "General";
    } else if (categories.size() == 1) {
        return categories[0];
    } else {
        string result = categories[0];
        for (int i = 1; i < categories.size(); i++) {
            result += ", " + categories[i];
        }
        return result;
    }
}

// ==================== SUBSYSTEM 1: REGISTRATION & DATA MANAGEMENT ====================

void registrationMenu() {
    while (true) {
        clearScreen();
        printHeader("REGISTRATION & DATA MANAGEMENT SUBSYSTEM");
        
        cout << "\n1. Register New Homeless Person\n";
        cout << "2. Search Person by ID (Binary Search)\n";
        cout << "3. View All Registered Persons\n";
        cout << "4. Update Person Information\n";
        cout << "5. Calculate Priority Score\n";
        cout << "6. Delete Record\n";
        cout << "0. Back to Main Menu\n";
        cout << "\n" << CYAN << "Enter choice: " << RESET;
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        if (choice == 0) break;
        
        switch (choice) {
            case 1: {
                clearScreen();
                printSubHeader("Register New Homeless Person");
                
                Homeless h;
                h.id = nextHomelessID++;
                
                cout << "Auto-generated ID: " << h.id << "\n\n";
                cout << "Enter Name: ";
                getline(cin, h.name);
                
                cout << "Enter Age: ";
                cin >> h.age;
                cin.ignore();
                
                cout << "Enter Gender (Male/Female): ";
                getline(cin, h.gender);
                
                cout << "Enter Location Node ID (0-14): ";
                cin >> h.locationNodeID;
                cin.ignore();
                
                cout << "Medical Need? (1=Yes, 0=No): ";
                cin >> h.medicalNeed;
                cin.ignore();
                
                cout << "Enter Complaint/Issue: ";
                getline(cin, h.complaint);
                
                calculatePriority(h);
                
                if (addHomelessRecord(h)) {
                    cout << "\nPriority Score: " << h.priorityScore << "\n";
                    cout << "Complaint Category: " << classifyComplaint(h.complaint) << "\n";
                    
                    if (h.priorityScore > 80) {
                        printWarning("HIGH PRIORITY CASE - Adding to emergency queue");
                        addEmergencyCase({h.id, h.priorityScore, time(0)});
                    }
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 2: {
                clearScreen();
                printSubHeader("Search Person by ID");
                
                int id;
                cout << "Enter ID to search: ";
                cin >> id;
                cin.ignore();
                
                cout << "\nPerforming Binary Search...\n";
                int index = binarySearchRecord(id);
                
                Homeless* found = searchHomeless(id);
                if (found) {
                    cout << "\n" << GREEN << "═══ Person Found ═══" << RESET << "\n";
                    cout << "ID: " << found->id << "\n";
                    cout << "Name: " << found->name << "\n";
                    cout << "Age: " << found->age << "\n";
                    cout << "Gender: " << found->gender << "\n";
                    cout << "Location Node: " << found->locationNodeID << "\n";
                    cout << "Medical Need: " << (found->medicalNeed ? "Yes" : "No") << "\n";
                    cout << "Priority Score: " << found->priorityScore << "\n";
                    cout << "Complaint: " << found->complaint << "\n";
                    cout << "Category: " << classifyComplaint(found->complaint) << "\n";
                    cout << "Allocated: " << (found->allocated ? "Yes" : "No") << "\n";
                    if (found->allocated) {
                        cout << "Shelter ID: " << found->allocatedShelterID << "\n";
                    }
                } else {
                    printError("Person not found");
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 3: {
                clearScreen();
                printSubHeader("All Registered Persons");
                
                if (homelessRecords.empty()) {
                    printWarning("No records found");
                } else {
                    cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(6) << "Age" 
                         << setw(10) << "Gender" << setw(10) << "Priority" << setw(12) << "Allocated\n";
                    cout << string(64, '-') << "\n";
                    
                    for (const auto& pair : homelessRecords) {
                        const Homeless& h = pair.second;
                        cout << left << setw(6) << h.id << setw(20) << h.name.substr(0, 18) 
                             << setw(6) << h.age << setw(10) << h.gender 
                             << setw(10) << h.priorityScore 
                             << setw(12) << (h.allocated ? GREEN "Yes" RESET : "No") << "\n";
                    }
                    
                    cout << "\nTotal: " << homelessRecords.size() << " persons\n";
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 4: {
                clearScreen();
                printSubHeader("Update Person Information");
                
                int id;
                cout << "Enter ID: ";
                cin >> id;
                cin.ignore();
                
                Homeless* h = searchHomeless(id);
                if (!h) {
                    printError("Person not found");
                } else {
                    cout << "\nCurrent Details:\n";
                    cout << "Name: " << h->name << "\n";
                    cout << "Complaint: " << h->complaint << "\n\n";
                    
                    cout << "Update Complaint (leave blank to keep current): ";
                    string newComplaint;
                    getline(cin, newComplaint);
                    
                    if (!newComplaint.empty()) {
                        h->complaint = newComplaint;
                        calculatePriority(*h);
                        homelessRecords[id] = *h;
                        printSuccess("Record updated. New priority: " + to_string(h->priorityScore));
                    }
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 5: {
                clearScreen();
                printSubHeader("Recalculate Priority Scores");
                
                for (auto& pair : homelessRecords) {
                    calculatePriority(pair.second);
                }
                
                printSuccess("All priority scores recalculated");
                pressEnterToContinue();
                break;
            }
            
            case 6: {
                clearScreen();
                printSubHeader("Delete Record");
                
                int id;
                cout << "Enter ID to delete: ";
                cin >> id;
                cin.ignore();
                
                if (homelessRecords.erase(id)) {
                    printSuccess("Record deleted");
                } else {
                    printError("Record not found");
                }
                
                pressEnterToContinue();
                break;
            }
            
            default:
                printError("Invalid choice");
                pressEnterToContinue();
        }
    }
}

// ==================== SUBSYSTEM 2: SHELTER ALLOCATION & MANAGEMENT ====================

void shelterManagementMenu() {
    while (true) {
        clearScreen();
        printHeader("SHELTER ALLOCATION & MANAGEMENT SUBSYSTEM");
        
        cout << "\n1. Allocate Shelter (Dijkstra's Algorithm)\n";
        cout << "2. View Shelter Status\n";
        cout << "3. Sort Shelters by Capacity (Merge Sort)\n";
        cout << "4. Check Shelter Connectivity (DFS)\n";
        cout << "5. Update Shelter Capacity\n";
        cout << "6. View Allocated Persons per Shelter\n";
        cout << "7. Release Person from Shelter\n";
        cout << "0. Back to Main Menu\n";
        cout << "\n" << CYAN << "Enter choice: " << RESET;
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        if (choice == 0) break;
        
        switch (choice) {
            case 1: {
                clearScreen();
                int id;
                cout << "Enter Homeless Person ID: ";
                cin >> id;
                cin.ignore();
                allocateShelter(id);
                pressEnterToContinue();
                break;
            }
            
            case 2: {
                clearScreen();
                printSubHeader("Shelter Status Overview");
                
                cout << left << setw(25) << "Shelter Name" << setw(10) << "Node" 
                     << setw(10) << "Total" << setw(10) << "Occupied" << setw(10) << "Available" 
                     << setw(15) << "Utilization\n";
                cout << string(80, '-') << "\n";
                
                for (const Shelter& s : shelters) {
                    int available = s.capacityTotal - s.capacityOccupied;
                    double util = (s.capacityOccupied * 100.0) / s.capacityTotal;
                    
                    cout << left << setw(25) << s.name << setw(10) << s.nodeID 
                         << setw(10) << s.capacityTotal << setw(10) << s.capacityOccupied 
                         << setw(10) << available << setw(15) << fixed << setprecision(1) << util << "%\n";
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 3: {
                clearScreen();
                sortSheltersByCapacity();
                pressEnterToContinue();
                break;
            }
            
            case 4: {
                clearScreen();
                checkShelterConnectivity();
                pressEnterToContinue();
                break;
            }
            
            case 5: {
                clearScreen();
                printSubHeader("Update Shelter Capacity");
                
                int shelterID;
                cout << "Enter Shelter ID: ";
                cin >> shelterID;
                
                bool found = false;
                for (Shelter& s : shelters) {
                    if (s.id == shelterID) {
                        found = true;
                        cout << "Current Capacity: " << s.capacityTotal << "\n";
                        cout << "Enter New Total Capacity: ";
                        int newCap;
                        cin >> newCap;
                        
                        if (newCap >= s.capacityOccupied) {
                            s.capacityTotal = newCap;
                            printSuccess("Capacity updated");
                        } else {
                            printError("Cannot set capacity below occupied count");
                        }
                        break;
                    }
                }
                
                if (!found) printError("Shelter not found");
                
                cin.ignore();
                pressEnterToContinue();
                break;
            }
            
            case 6: {
                clearScreen();
                printSubHeader("Allocated Persons per Shelter");
                
                for (const Shelter& s : shelters) {
                    cout << "\n" << BOLD << s.name << RESET << " (Occupied: " 
                         << s.capacityOccupied << "/" << s.capacityTotal << ")\n";
                    cout << string(50, '-') << "\n";
                    
                    if (s.allocatedPersonIDs.empty()) {
                        cout << "  No allocations\n";
                    } else {
                        for (int pid : s.allocatedPersonIDs) {
                            Homeless* h = searchHomeless(pid);
                            if (h) {
                                cout << "  • " << h->name << " (ID: " << pid << ", Priority: " 
                                     << h->priorityScore << ")\n";
                            }
                        }
                    }
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 7: {
                clearScreen();
                printSubHeader("Release Person from Shelter");
                
                int id;
                cout << "Enter Homeless Person ID: ";
                cin >> id;
                cin.ignore();
                
                Homeless* h = searchHomeless(id);
                if (!h) {
                    printError("Person not found");
                } else if (!h->allocated) {
                    printWarning("Person is not allocated to any shelter");
                } else {
                    int shelterID = h->allocatedShelterID;
                    for (Shelter& s : shelters) {
                        if (s.id == shelterID) {
                            s.capacityOccupied--;
                            auto it = find(s.allocatedPersonIDs.begin(), s.allocatedPersonIDs.end(), id);
                            if (it != s.allocatedPersonIDs.end()) {
                                s.allocatedPersonIDs.erase(it);
                            }
                            h->allocated = false;
                            h->allocatedShelterID = -1;
                            homelessRecords[id] = *h;
                            printSuccess("Person released from " + s.name);
                            break;
                        }
                    }
                }
                
                pressEnterToContinue();
                break;
            }
            
            default:
                printError("Invalid choice");
                pressEnterToContinue();
        }
    }
}

// ==================== SUBSYSTEM 3: EMERGENCY MANAGEMENT ====================

void emergencyManagementMenu() {
    while (true) {
        clearScreen();
        printHeader("EMERGENCY MANAGEMENT SUBSYSTEM");
        
        cout << "\n1. View Emergency Queue\n";
        cout << "2. Handle Next Emergency (Max Heap)\n";
        cout << "3. Add Person to Emergency Queue\n";
        cout << "4. View High Priority Cases\n";
        cout << "5. Emergency Allocation (Auto)\n";
        cout << "0. Back to Main Menu\n";
        cout << "\n" << CYAN << "Enter choice: " << RESET;
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        if (choice == 0) break;
        
        switch (choice) {
            case 1: {
                clearScreen();
                printSubHeader("Emergency Queue Status");
                
                if (emergencyHeap.empty()) {
                    printInfo("No pending emergencies");
                } else {
                    cout << "Pending Emergencies: " << emergencyHeap.size() << "\n\n";
                    
                    // Create temp copy to display without removing
                    priority_queue<EmergencyCase> tempHeap = emergencyHeap;
                    int rank = 1;
                    
                    cout << left << setw(6) << "Rank" << setw(12) << "Person ID" 
                         << setw(12) << "Priority" << setw(25) << "Name\n";
                    cout << string(55, '-') << "\n";
                    
                    while (!tempHeap.empty()) {
                        EmergencyCase ec = tempHeap.top();
                        tempHeap.pop();
                        
                        Homeless* h = searchHomeless(ec.homelessID);
                        string name = h ? h->name : "Unknown";
                        
                        cout << left << setw(6) << rank++ << setw(12) << ec.homelessID 
                             << setw(12) << ec.priority << setw(25) << name << "\n";
                    }
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 2: {
                clearScreen();
                printSubHeader("Handle Next Emergency");
                
                if (emergencyHeap.empty()) {
                    printInfo("No pending emergencies");
                } else {
                    EmergencyCase ec = getNextEmergency();
                    
                    Homeless* h = searchHomeless(ec.homelessID);
                    if (h) {
                        cout << RED << "🚨 EMERGENCY CASE 🚨" << RESET << "\n\n";
                        cout << "Person: " << h->name << "\n";
                        cout << "Priority: " << ec.priority << "\n";
                        cout << "Complaint: " << h->complaint << "\n\n";
                        
                        if (!h->allocated) {
                            cout << "Attempting emergency allocation...\n\n";
                            allocateShelter(ec.homelessID);
                        } else {
                            printInfo("Person already allocated to shelter");
                        }
                    }
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 3: {
                clearScreen();
                printSubHeader("Add Person to Emergency Queue");
                
                int id;
                cout << "Enter Homeless Person ID: ";
                cin >> id;
                cin.ignore();
                
                Homeless* h = searchHomeless(id);
                if (!h) {
                    printError("Person not found");
                } else {
                    calculatePriority(*h);
                    addEmergencyCase({id, h->priorityScore, time(0)});
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 4: {
                clearScreen();
                printSubHeader("High Priority Cases (Priority > 80)");
                
                bool found = false;
                cout << left << setw(6) << "ID" << setw(20) << "Name" << setw(12) << "Priority" 
                     << setw(30) << "Complaint\n";
                cout << string(68, '-') << "\n";
                
                for (const auto& pair : homelessRecords) {
                    const Homeless& h = pair.second;
                    if (h.priorityScore > 80) {
                        found = true;
                        cout << left << setw(6) << h.id << setw(20) << h.name.substr(0, 18) 
                             << setw(12) << h.priorityScore 
                             << setw(30) << h.complaint.substr(0, 28) << "\n";
                    }
                }
                
                if (!found) {
                    printInfo("No high priority cases");
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 5: {
                clearScreen();
                printSubHeader("Emergency Auto-Allocation");
                
                int allocated = 0;
                while (!emergencyHeap.empty()) {
                    EmergencyCase ec = getNextEmergency();
                    Homeless* h = searchHomeless(ec.homelessID);
                    
                    if (h && !h->allocated) {
                        cout << "\nProcessing: " << h->name << " (Priority: " << ec.priority << ")\n";
                        allocateShelter(ec.homelessID);
                        allocated++;
                    }
                }
                
                cout << "\n";
                printSuccess("Processed " + to_string(allocated) + " emergency cases");
                pressEnterToContinue();
                break;
            }
            
            default:
                printError("Invalid choice");
                pressEnterToContinue();
        }
    }
}

// ==================== SUBSYSTEM 4: ANALYSIS & REPORTING ====================

void analysisReportingMenu() {
    while (true) {
        clearScreen();
        printHeader("ANALYSIS & REPORTING SUBSYSTEM");
        
        cout << "\n1. Generate Daily Report\n";
        cout << "2. Shelter Utilization Analysis\n";
        cout << "3. Overcrowding Alert Check\n";
        cout << "4. Complaint Category Analysis\n";
        cout << "5. Priority Distribution Report\n";
        cout << "6. Allocation Efficiency Report\n";
        cout << "7. Network Analysis Report\n";
        cout << "0. Back to Main Menu\n";
        cout << "\n" << CYAN << "Enter choice: " << RESET;
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        if (choice == 0) break;
        
        switch (choice) {
            case 1: {
                clearScreen();
                printSubHeader("Daily Summary Report");
                
                int totalReg = homelessRecords.size();
                int totalAllocated = 0;
                int totalMedical = 0;
                int totalChildren = 0;
                
                for (const auto& pair : homelessRecords) {
                    if (pair.second.allocated) totalAllocated++;
                    if (pair.second.medicalNeed) totalMedical++;
                    if (pair.second.age < 18) totalChildren++;
                }
                
                int totalCap = 0, totalOcc = 0;
                for (const Shelter& s : shelters) {
                    totalCap += s.capacityTotal;
                    totalOcc += s.capacityOccupied;
                }
                
                cout << "╔════════════════════════════════════════╗\n";
                cout << "║         DAILY OPERATIONS REPORT        ║\n";
                cout << "╚════════════════════════════════════════╝\n\n";
                
                cout << BOLD << "Population Statistics:" << RESET << "\n";
                cout << "  Total Registered: " << totalReg << "\n";
                cout << "  Allocated to Shelters: " << totalAllocated << " (" 
                     << (totalReg > 0 ? (totalAllocated*100)/totalReg : 0) << "%)\n";
                cout << "  Awaiting Allocation: " << (totalReg - totalAllocated) << "\n";
                cout << "  Medical Cases: " << totalMedical << "\n";
                cout << "  Children (<18): " << totalChildren << "\n\n";
                
                cout << BOLD << "Shelter Statistics:" << RESET << "\n";
                cout << "  Total Shelters: " << shelters.size() << "\n";
                cout << "  Total Capacity: " << totalCap << " beds\n";
                cout << "  Currently Occupied: " << totalOcc << " beds\n";
                cout << "  Available: " << (totalCap - totalOcc) << " beds\n";
                cout << "  Utilization Rate: " << fixed << setprecision(1) 
                     << (totalCap > 0 ? (totalOcc*100.0)/totalCap : 0) << "%\n\n";
                
                cout << BOLD << "Emergency Queue:" << RESET << "\n";
                cout << "  Pending Cases: " << emergencyHeap.size() << "\n\n";
                
                pressEnterToContinue();
                break;
            }
            
            case 2: {
                clearScreen();
                printSubHeader("Shelter Utilization Analysis");
                
                cout << left << setw(25) << "Shelter" << setw(15) << "Utilization" 
                     << setw(15) << "Status" << setw(15) << "Action\n";
                cout << string(70, '-') << "\n";
                
                for (const Shelter& s : shelters) {
                    double util = (s.capacityOccupied * 100.0) / s.capacityTotal;
                    string status, action;
                    
                    if (util >= 90) {
                        status = RED "CRITICAL" RESET;
                        action = "Add capacity";
                    } else if (util >= 75) {
                        status = YELLOW "HIGH" RESET;
                        action = "Monitor";
                    } else if (util >= 50) {
                        status = GREEN "NORMAL" RESET;
                        action = "None";
                    } else {
                        status = BLUE "LOW" RESET;
                        action = "Redirect";
                    }
                    
                    cout << left << setw(25) << s.name << setw(15) << (to_string((int)util) + "%") 
                         << setw(15) << status << setw(15) << action << "\n";
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 3: {
                clearScreen();
                printSubHeader("Overcrowding Alert System");
                
                bool criticalFound = false;
                for (const Shelter& s : shelters) {
                    double util = (s.capacityOccupied * 100.0) / s.capacityTotal;
                    if (util >= 90) {
                        cout << RED << "🚨 CRITICAL: " << RESET << s.name 
                             << " is at " << fixed << setprecision(1) << util << "% capacity!\n";
                        criticalFound = true;
                    }
                }
                
                if (!criticalFound) {
                    printSuccess("No overcrowding detected - all shelters within limits");
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 4: {
                clearScreen();
                printSubHeader("Complaint Category Analysis (Rabin-Karp)");
                
                unordered_map<string, int> categoryCount;
                
                for (const auto& pair : homelessRecords) {
                    string category = classifyComplaint(pair.second.complaint);
                    categoryCount[category]++;
                }
                
                cout << left << setw(20) << "Category" << setw(10) << "Count" << setw(15) << "Percentage\n";
                cout << string(45, '-') << "\n";
                
                int total = homelessRecords.size();
                for (const auto& cat : categoryCount) {
                    double pct = (cat.second * 100.0) / total;
                    cout << left << setw(20) << cat.first << setw(10) << cat.second 
                         << setw(15) << (to_string((int)pct) + "%") << "\n";
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 5: {
                clearScreen();
                printSubHeader("Priority Distribution Report");
                
                int low = 0, medium = 0, high = 0, critical = 0;
                
                for (const auto& pair : homelessRecords) {
                    int p = pair.second.priorityScore;
                    if (p >= 100) critical++;
                    else if (p >= 70) high++;
                    else if (p >= 40) medium++;
                    else low++;
                }
                
                cout << left << setw(20) << "Priority Level" << setw(10) << "Count" << setw(15) << "Percentage\n";
                cout << string(45, '-') << "\n";
                
                int total = homelessRecords.size();
                cout << left << setw(20) << RED "Critical (100+)" << RESET << setw(10) << critical 
                     << setw(15) << (total > 0 ? to_string((critical*100)/total) + "%" : "0%") << "\n";
                cout << left << setw(20) << YELLOW "High (70-99)" << RESET << setw(10) << high 
                     << setw(15) << (total > 0 ? to_string((high*100)/total) + "%" : "0%") << "\n";
                cout << left << setw(20) << "Medium (40-69)" << setw(10) << medium 
                     << setw(15) << (total > 0 ? to_string((medium*100)/total) + "%" : "0%") << "\n";
                cout << left << setw(20) << "Low (0-39)" << setw(10) << low 
                     << setw(15) << (total > 0 ? to_string((low*100)/total) + "%" : "0%") << "\n";
                
                pressEnterToContinue();
                break;
            }
            
            case 6: {
                clearScreen();
                printSubHeader("Allocation Efficiency Report");
                
                int totalReg = homelessRecords.size();
                int allocated = 0;
                int highPriorityAllocated = 0;
                int highPriorityTotal = 0;
                
                for (const auto& pair : homelessRecords) {
                    if (pair.second.allocated) allocated++;
                    if (pair.second.priorityScore > 80) {
                        highPriorityTotal++;
                        if (pair.second.allocated) highPriorityAllocated++;
                    }
                }
                
                cout << "Overall Allocation Rate: " << (totalReg > 0 ? (allocated*100)/totalReg : 0) << "%\n";
                cout << "High Priority Allocation: " << (highPriorityTotal > 0 ? (highPriorityAllocated*100)/highPriorityTotal : 0) << "%\n\n";
                
                cout << "Efficiency Rating: ";
                double efficiency = totalReg > 0 ? (allocated*100.0)/totalReg : 0;
                if (efficiency >= 90) cout << GREEN << "EXCELLENT" << RESET << "\n";
                else if (efficiency >= 75) cout << GREEN << "GOOD" << RESET << "\n";
                else if (efficiency >= 50) cout << YELLOW << "AVERAGE" << RESET << "\n";
                else cout << RED << "NEEDS IMPROVEMENT" << RESET << "\n";
                
                pressEnterToContinue();
                break;
            }
            
            case 7: {
                clearScreen();
                printSubHeader("Network Analysis Report");
                
                cout << "Graph Statistics:\n";
                cout << "  Total Nodes: " << nodeCount << "\n";
                cout << "  Stations: " << stations.size() << "\n";
                cout << "  Shelters: " << shelters.size() << "\n\n";
                
                cout << "Running DFS connectivity check...\n";
                checkShelterConnectivity();
                
                pressEnterToContinue();
                break;
            }
            
            default:
                printError("Invalid choice");
                pressEnterToContinue();
        }
    }
}

// ==================== SUBSYSTEM 5: NETWORK & TRAVERSAL ====================

void networkTraversalMenu() {
    while (true) {
        clearScreen();
        printHeader("NETWORK & TRAVERSAL SUBSYSTEM");
        
        cout << "\n1. BFS Traversal from Station\n";
        cout << "2. DFS Network Connectivity\n";
        cout << "3. Shortest Path Analysis (Dijkstra)\n";
        cout << "4. View Network Topology\n";
        cout << "5. Find Nearest Shelter to Node\n";
        cout << "0. Back to Main Menu\n";
        cout << "\n" << CYAN << "Enter choice: " << RESET;
        
        int choice;
        cin >> choice;
        cin.ignore();
        
        if (choice == 0) break;
        
        switch (choice) {
            case 1: {
                clearScreen();
                cout << "Available Stations:\n";
                for (const Station& st : stations) {
                    cout << "  " << st.name << " (Node " << st.nodeID << ")\n";
                }
                
                int nodeID;
                cout << "\nEnter Node ID to start BFS: ";
                cin >> nodeID;
                cin.ignore();
                
                traverseNearbyAreas(nodeID);
                pressEnterToContinue();
                break;
            }
            
            case 2: {
                clearScreen();
                checkShelterConnectivity();
                pressEnterToContinue();
                break;
            }
            
            case 3: {
                clearScreen();
                printSubHeader("Shortest Path Analysis");
                
                int source;
                cout << "Enter source node (0-14): ";
                cin >> source;
                cin.ignore();
                
                if (source < 0 || source >= nodeCount) {
                    printError("Invalid node");
                } else {
                    cout << "\nCalculating shortest paths using Dijkstra...\n\n";
                    vector<int> dist = dijkstra(source);
                    
                    cout << left << setw(15) << "Destination" << setw(15) << "Distance\n";
                    cout << string(30, '-') << "\n";
                    
                    for (const Shelter& s : shelters) {
                        cout << left << setw(15) << s.name 
                             << setw(15) << (dist[s.nodeID] == INT_MAX ? "Unreachable" : to_string(dist[s.nodeID])) << "\n";
                    }
                }
                
                pressEnterToContinue();
                break;
            }
            
            case 4: {
                clearScreen();
                printSubHeader("Network Topology");
                
                cout << "Stations:\n";
                for (const Station& st : stations) {
                    cout << "  • " << st.name << " at Node " << st.nodeID << "\n";
                }
                
                cout << "\nShelters:\n";
                for (const Shelter& s : shelters) {
                    cout << "  • " << s.name << " at Node " << s.nodeID 
                         << " (Capacity: " << s.capacityOccupied << "/" << s.capacityTotal << ")\n";
                }
                
                cout << "\nTotal Graph Nodes: " << nodeCount << "\n";
                
                pressEnterToContinue();
                break;
            }
            
            case 5: {
                clearScreen();
                printSubHeader("Find Nearest Shelter");
                
                int nodeID;
                cout << "Enter node ID: ";
                cin >> nodeID;
                cin.ignore();
                
                if (nodeID < 0 || nodeID >= nodeCount) {
                    printError("Invalid node");
                } else {
                    vector<int> dist = dijkstra(nodeID);
                    
                    int nearest = -1;
                    int minDist = INT_MAX;
                    
                    for (const Shelter& s : shelters) {
                        if (dist[s.nodeID] < minDist) {
                            minDist = dist[s.nodeID];
                            nearest = s.id;
                        }
                    }
                    
                    if (nearest != -1) {
                        for (const Shelter& s : shelters) {
                            if (s.id == nearest) {
                                cout << "\nNearest Shelter: " << GREEN << s.name << RESET << "\n";
                                cout << "Distance: " << minDist << " units\n";
                                cout << "Available Beds: " << (s.capacityTotal - s.capacityOccupied) << "\n";
                                break;
                            }
                        }
                    }
                }
                
                pressEnterToContinue();
                break;
            }
            
            default:
                printError("Invalid choice");
                pressEnterToContinue();
        }
    }
}

// ==================== INITIALIZATION ====================

void initializeSampleData() {
    nodeCount = 15;
    graph.resize(nodeCount);
    
    // Build graph
    graph[0] = {{1, 5}, {2, 10}};
    graph[1] = {{0, 5}, {3, 7}, {4, 12}};
    graph[2] = {{0, 10}, {5, 8}};
    graph[3] = {{1, 7}, {6, 6}};
    graph[4] = {{1, 12}, {7, 9}};
    graph[5] = {{2, 8}, {8, 11}};
    graph[6] = {{3, 6}, {9, 4}};
    graph[7] = {{4, 9}, {10, 7}};
    graph[8] = {{5, 11}, {11, 5}};
    graph[9] = {{6, 4}, {12, 8}};
    graph[10] = {{7, 7}, {13, 6}};
    graph[11] = {{8, 5}, {14, 10}};
    graph[12] = {{9, 8}};
    graph[13] = {{10, 6}};
    graph[14] = {{11, 10}};
    
    // Stations
    stations = {
        {1, "Central Railway Station", 0},
        {2, "East Junction", 3},
        {3, "West Terminal", 6}
    };
    
    // Shelters
    shelters = {
        {1, "Hope Shelter", 4, 50, 35, "9876543210", {}},
        {2, "Care Center", 7, 40, 28, "9876543211", {}},
        {3, "Safe Haven", 10, 60, 45, "9876543212", {}},
        {4, "Community Home", 13, 30, 15, "9876543213", {}}
    };
    
    // Sample homeless records
    vector<Homeless> samples = {
        {101, "Ramesh Kumar", 45, "Male", 0, false, 0, "Need food urgently", time(0), false, -1},
        {102, "Lakshmi Devi", 65, "Female", 1, true, 0, "Medical help needed", time(0), false, -1},
        {103, "Anita", 8, "Female", 2, false, 0, "Child alone, scared", time(0), false, -1},
        {104, "Suresh", 32, "Male", 3, false, 0, "Looking for shelter", time(0), false, -1},
        {105, "Meera", 70, "Female", 5, true, 0, "Emergency medical case", time(0), false, -1}
    };
    
    for (Homeless& h : samples) {
        calculatePriority(h);
        homelessRecords[h.id] = h;
        homelessList.push_back(h);
        
        if (h.priorityScore > 80) {
            emergencyHeap.push({h.id, h.priorityScore, time(0)});
        }
    }
}

// ==================== MAIN MENU ====================

void displayMainMenu() {
    clearScreen();
    printHeader("SMART HOMELESS SHELTER MANAGEMENT SYSTEM");
    
    cout << BOLD << "\n┌─────────────────────────────────────────────────────────┐\n";
    cout << "│                    MAIN MENU - SUBSYSTEMS               │\n";
    cout << "└─────────────────────────────────────────────────────────┘\n" << RESET;
    
    cout << "\n" << CYAN << "1. 📋 Registration & Data Management" << RESET;
    cout << "\n     » Register, search, update homeless records";
    cout << "\n     » Uses: Hashing, Binary Search\n";
    
    cout << "\n" << GREEN << "2. 🏠 Shelter Allocation & Management" << RESET;
    cout << "\n     » Allocate shelters, manage capacity";
    cout << "\n     » Uses: Dijkstra's Algorithm, Merge Sort, DFS\n";
    
    cout << "\n" << RED << "3. 🚨 Emergency Management" << RESET;
    cout << "\n     » Handle emergency cases with priority";
    cout << "\n     » Uses: Max Heap, Priority Queue\n";
    
    cout << "\n" << YELLOW << "4. 📊 Analysis & Reporting" << RESET;
    cout << "\n     » Generate reports, analyze utilization";
    cout << "\n     » Uses: Rabin-Karp, Statistical Analysis\n";
    
    cout << "\n" << BLUE << "5. 🗺️  Network & Traversal" << RESET;
    cout << "\n     » BFS/DFS traversal, shortest paths";
    cout << "\n     » Uses: BFS, DFS, Dijkstra\n";
    
    cout << "\n" << MAGENTA << "6. ℹ️  System Information" << RESET;
    cout << "\n     » View algorithms, complexities, credits\n";
    
    cout << "\n0. ❌ Exit System\n";
    
    cout << "\n" << BOLD << CYAN << "═══════════════════════════════════════════════════════════\n";
    cout << "Enter your choice: " << RESET;
}

void displaySystemInfo() {
    clearScreen();
    printHeader("SYSTEM INFORMATION");
    
    cout << "\n" << BOLD << "🧠 ALGORITHMS IMPLEMENTED:\n" << RESET;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n";
    
    cout << "1.  " << GREEN << "Dijkstra's Algorithm" << RESET << "\n";
    cout << "    Purpose: Shortest path for shelter allocation\n";
    cout << "    Complexity: O((V+E) log V)\n\n";
    
    cout << "2.  " << GREEN << "Breadth-First Search (BFS)" << RESET << "\n";
    cout << "    Purpose: Nearby area traversal\n";
    cout << "    Complexity: O(V + E)\n\n";
    
    cout << "3.  " << GREEN << "Depth-First Search (DFS)" << RESET << "\n";
    cout << "    Purpose: Network connectivity check\n";
    cout << "    Complexity: O(V + E)\n\n";
    
    cout << "4.  " << GREEN << "Hashing" << RESET << "\n";
    cout << "    Purpose: Fast record storage/retrieval\n";
    cout << "    Complexity: O(1) average case\n\n";
    
    cout << "5.  " << GREEN << "Binary Search" << RESET << "\n";
    cout << "    Purpose: Efficient ID search\n";
    cout << "    Complexity: O(log n)\n\n";
    
    cout << "6.  " << GREEN << "Merge Sort" << RESET << "\n";
    cout << "    Purpose: Sort shelters by capacity\n";
    cout << "    Complexity: O(n log n)\n\n";
    
    cout << "7.  " << GREEN << "Max Heap (Priority Queue)" << RESET << "\n";
    cout << "    Purpose: Emergency prioritization\n";
    cout << "    Complexity: O(log n) insert/extract\n\n";
    
    cout << "8.  " << GREEN << "Rabin-Karp Algorithm" << RESET << "\n";
    cout << "    Purpose: Pattern matching in complaints\n";
    cout << "    Complexity: O(n + m) average case\n\n";
    
    cout << "\n" << BOLD << "📚 PROJECT DETAILS:\n" << RESET;
    cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    cout << "Title: Smart Homeless Shelter Management System\n";
    cout << "Domain: Social Welfare + Algorithm Optimization\n";
    cout << "Language: C++ (STL Only)\n";
    cout << "Features: 5 Major Subsystems, 17+ Algorithms\n";
    cout << "Purpose: Academic DAA Project\n";
    
    pressEnterToContinue();
}

int main() {
    // Initialize system
    cout << BOLD << CYAN << "\n╔════════════════════════════════════════════════════════╗\n";
    cout << "║                                                        ║\n";
    cout << "║     SMART HOMELESS SHELTER MANAGEMENT SYSTEM          ║\n";
    cout << "║     Design and Analysis of Algorithms Project         ║\n";
    cout << "║                                                        ║\n";
    cout << "╚════════════════════════════════════════════════════════╝\n" << RESET;
    
    cout << "\n" << YELLOW << "Initializing system..." << RESET << "\n";
    cout << "  • Loading graph network... ";
    initializeSampleData();
    cout << GREEN << "✓" << RESET << "\n";
    
    cout << "  • Loading sample data... ";
    cout << GREEN << "✓" << RESET << "\n";
    
    cout << "  • Initializing subsystems... ";
    cout << GREEN << "✓" << RESET << "\n";
    
    cout << "\n" << GREEN << BOLD << "System Ready!" << RESET << "\n";
    
    pressEnterToContinue();
    
    int choice;
    do {
        displayMainMenu();
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1:
                registrationMenu();
                break;
            
            case 2:
                shelterManagementMenu();
                break;
            
            case 3:
                emergencyManagementMenu();
                break;
            
            case 4:
                analysisReportingMenu();
                break;
            
            case 5:
                networkTraversalMenu();
                break;
            
            case 6:
                displaySystemInfo();
                break;
            
            case 0:
                clearScreen();
                printHeader("THANK YOU");
                cout << "\n" << GREEN << "System shutting down gracefully...\n";
                cout << "All data has been processed.\n";
                cout << "Goodbye! 👋\n" << RESET << "\n";
                break;
            
            default:
                printError("Invalid choice! Please try again.");
                pressEnterToContinue();
        }
    } while (choice != 0);
    
    return 0;
}
        